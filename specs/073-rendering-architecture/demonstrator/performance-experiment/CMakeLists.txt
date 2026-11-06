cmake_minimum_required(VERSION 3.21)
project(fragment_preflight LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
option(PATCHED "Enable the isolated upstream boundary preflight" OFF)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED IMPORTED_TARGET freetype2 harfbuzz icu-uc icu-i18n libpng fontconfig zlib)
if(NOT EXISTS "${SKIA}/comparison-build.json")
    message(FATAL_ERROR "SKIA must be the recorded pinned build")
endif()
add_executable(perf-cpu cpu.cpp)
target_include_directories(perf-cpu PRIVATE "${SKIA}")
file(STRINGS "${SKIA}/out/comparison/obj/skia.ninja" DEFINES REGEX "^defines = ")
string(REPLACE "defines = " "" DEFINES "${DEFINES}")
separate_arguments(DEFINES UNIX_COMMAND "${DEFINES}")
target_compile_options(perf-cpu PRIVATE ${DEFINES})
if(PATCHED)
    target_compile_definitions(perf-cpu PRIVATE FRAGMENT_PATCHED)
endif()
file(GLOB LIBS "${SKIA}/out/comparison/*.a")
target_link_libraries(perf-cpu PRIVATE "-Wl,--start-group" ${LIBS} "-Wl,--end-group" PkgConfig::DEPS dl pthread)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NATIVE_DEPS REQUIRED IMPORTED_TARGET sdl2 freetype2 harfbuzz icu-uc icu-i18n libpng fontconfig zlib vulkan gl)
if(NOT EXISTS "${SKIA}/comparison-build.json")
    message(FATAL_ERROR "Build the pinned checkout with build_skia.py first")
endif()
# Reuse the pinned upstream swapchain implementation. Its test instance extension
# allowlist predates Wayland; extend only the generated local helper copy.
file(READ "${SKIA}/tools/gpu/vk/VkTestUtils.cpp" VK_HELPER)
string(REPLACE "\"VK_KHR_xcb_surface\"," "\"VK_KHR_wayland_surface\", \"VK_KHR_xcb_surface\"," VK_HELPER "${VK_HELPER}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/VkTestUtils.cpp" "${VK_HELPER}")
add_executable(perf-native native.cpp
    "${SKIA}/tools/window/WindowContext.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/VulkanWindowContext.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/VkTestUtils.cpp"
    "${SKIA}/tools/gpu/vk/VkTestMemoryAllocator.cpp"
    "${SKIA}/tools/library/LoadDynamicLibrary_posix.cpp")
target_include_directories(perf-native PRIVATE "${SKIA}" "${SKIA}/third_party/externals/vulkanmemoryallocator/include")
target_compile_definitions(perf-native PRIVATE SK_GANESH SK_VULKAN SK_GL)
file(GLOB SKIA_LIBRARIES "${SKIA}/out/comparison/*.a")
target_link_libraries(perf-native PRIVATE "-Wl,--start-group" ${SKIA_LIBRARIES} "-Wl,--end-group" PkgConfig::NATIVE_DEPS dl pthread)

# Internal upstream swapchain helpers must use the library's exact ABI defines.
file(STRINGS "${SKIA}/out/comparison/obj/skia.ninja" SKIA_DEFINES REGEX "^defines = ")
string(REPLACE "defines = " "" SKIA_DEFINES "${SKIA_DEFINES}")
separate_arguments(SKIA_DEFINES UNIX_COMMAND "${SKIA_DEFINES}")
target_compile_options(perf-native PRIVATE ${SKIA_DEFINES} -Wno-attributes)

# Upstream sample passes -1 as the initial extent hint. Wayland leaves extent
# selection to the client; use the actual initial SDL drawable extent.
file(READ "${SKIA}/tools/window/VulkanWindowContext.cpp" VK_WINDOW)
string(REPLACE "createSwapchain(-1, -1)" "createSwapchain(800, 600)" VK_WINDOW "${VK_WINDOW}")
# Initialize the present queue before swapchain creation so failure cleanup never
# waits on an uninitialized queue (demonstrated by the retained startup backtrace).
string(REPLACE "if (!this->createSwapchain(800, 600))" "fGetDeviceQueue(fDevice, fPresentQueueIndex, 0, &fPresentQueue);
    if (!this->createSwapchain(800, 600))" VK_WINDOW "${VK_WINDOW}")
string(REPLACE "return false;" "fprintf(stderr, \"Vulkan window failure at %d\\n\", __LINE__); return false;" VK_WINDOW "${VK_WINDOW}")
# This pinned helper maps an sRGB VkFormat to the UNORM SkColorType, which
# Ganesh rejects. Match the pinned GrVkCaps format/color-type compatibility table.
string(REPLACE "case VK_FORMAT_R8G8B8A8_UNORM: // fall through
        case VK_FORMAT_R8G8B8A8_SRGB:" "case VK_FORMAT_R8G8B8A8_SRGB:
            colorType = kSRGBA_8888_SkColorType;
            break;
        case VK_FORMAT_R8G8B8A8_UNORM:" VK_WINDOW "${VK_WINDOW}")
string(PREPEND VK_WINDOW "#include <chrono>\n#include \"include/gpu/ganesh/GrTypes.h\"\nextern GrFlushInfo perfFlushInfo;\nextern double perfSubmissionMs;\nextern int perfVkPresentMode;\n")
string(REPLACE "VkSwapchainCreateInfoKHR swapchainCreateInfo;" "perfVkPresentMode = int(mode); VkSwapchainCreateInfoKHR swapchainCreateInfo;" VK_WINDOW "${VK_WINDOW}")
string(REPLACE "GrFlushInfo info;" "GrFlushInfo info = perfFlushInfo;" VK_WINDOW "${VK_WINDOW}")
string(REPLACE "this->submitToGpu();" "auto perfStart = std::chrono::steady_clock::now(); this->submitToGpu(); perfSubmissionMs = std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-perfStart).count();" VK_WINDOW "${VK_WINDOW}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/VulkanWindowContext.cpp" "${VK_WINDOW}")



if(PATCHED)
    target_compile_definitions(perf-native PRIVATE FRAGMENT_PATCHED)
endif()
option(COUNTERS "Diagnostic counters only" OFF)
if(COUNTERS)
    target_compile_definitions(perf-cpu PRIVATE PERF_COUNTERS)
endif()
pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
execute_process(COMMAND ${WAYLAND_SCANNER} client-header /usr/share/wayland-protocols/stable/presentation-time/presentation-time.xml ${CMAKE_CURRENT_BINARY_DIR}/presentation.h COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND ${WAYLAND_SCANNER} private-code /usr/share/wayland-protocols/stable/presentation-time/presentation-time.xml ${CMAKE_CURRENT_BINARY_DIR}/presentation.c COMMAND_ERROR_IS_FATAL ANY)
target_sources(perf-native PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/presentation.c)
target_include_directories(perf-native PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(perf-native PRIVATE PkgConfig::WAYLAND)

# Full historical geometry/source-mapping inspector against each exact ABI.
add_executable(perf-geometry ../boundary-experiment/main.cpp)
target_include_directories(perf-geometry PRIVATE "${SKIA}")
target_compile_options(perf-geometry PRIVATE ${DEFINES})
if(PATCHED)
    target_compile_definitions(perf-geometry PRIVATE FRAGMENT_PATCHED)
endif()
target_link_libraries(perf-geometry PRIVATE "-Wl,--start-group" ${LIBS} "-Wl,--end-group" PkgConfig::DEPS dl pthread)

option(CONTROL_COUNTERS "Diagnostic upstream shaping counter only" OFF)
if(CONTROL_COUNTERS)
    target_compile_definitions(perf-cpu PRIVATE PERF_CONTROL_COUNTERS)
endif()
