cmake_minimum_required(VERSION 3.21)
project(fragment_preflight LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
option(PATCHED "Enable the isolated upstream boundary preflight" OFF)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED IMPORTED_TARGET freetype2 harfbuzz icu-uc icu-i18n libpng fontconfig zlib)
if(NOT EXISTS "${SKIA}/comparison-build.json")
    message(FATAL_ERROR "SKIA must be the recorded pinned build")
endif()
add_executable(fragment-preflight main.cpp)
target_include_directories(fragment-preflight PRIVATE "${SKIA}")
file(STRINGS "${SKIA}/out/comparison/obj/skia.ninja" DEFINES REGEX "^defines = ")
string(REPLACE "defines = " "" DEFINES "${DEFINES}")
separate_arguments(DEFINES UNIX_COMMAND "${DEFINES}")
target_compile_options(fragment-preflight PRIVATE ${DEFINES})
if(PATCHED)
    target_compile_definitions(fragment-preflight PRIVATE FRAGMENT_PATCHED)
endif()
file(GLOB LIBS "${SKIA}/out/comparison/*.a")
target_link_libraries(fragment-preflight PRIVATE "-Wl,--start-group" ${LIBS} "-Wl,--end-group" PkgConfig::DEPS dl pthread)

# Historical object diagnostics reuse the original adapter without changing it.
# Remove the unused GPU entry point; this target is strictly CPU/offscreen.
pkg_check_modules(SDL REQUIRED IMPORTED_TARGET sdl2)
add_executable(historical-objects historical_objects.cpp)
target_include_directories(historical-objects PRIVATE "${SKIA}")
target_compile_options(historical-objects PRIVATE ${DEFINES} -ffunction-sections -fdata-sections)
target_link_options(historical-objects PRIVATE -Wl,--gc-sections)
target_link_libraries(historical-objects PRIVATE "-Wl,--start-group" ${LIBS} "-Wl,--end-group" PkgConfig::DEPS PkgConfig::SDL dl pthread)

if(PATCHED)
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
add_executable(skia-compare native.cpp
    "${SKIA}/tools/window/WindowContext.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/VulkanWindowContext.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/VkTestUtils.cpp"
    "${SKIA}/tools/gpu/vk/VkTestMemoryAllocator.cpp"
    "${SKIA}/tools/library/LoadDynamicLibrary_posix.cpp")
target_include_directories(skia-compare PRIVATE "${SKIA}" "${SKIA}/third_party/externals/vulkanmemoryallocator/include")
target_compile_definitions(skia-compare PRIVATE SK_GANESH SK_VULKAN SK_GL FRAGMENT_PATCHED)
file(GLOB SKIA_LIBRARIES "${SKIA}/out/comparison/*.a")
target_link_libraries(skia-compare PRIVATE "-Wl,--start-group" ${SKIA_LIBRARIES} "-Wl,--end-group" PkgConfig::NATIVE_DEPS dl pthread)

# Internal upstream swapchain helpers must use the library's exact ABI defines.
file(STRINGS "${SKIA}/out/comparison/obj/skia.ninja" SKIA_DEFINES REGEX "^defines = ")
string(REPLACE "defines = " "" SKIA_DEFINES "${SKIA_DEFINES}")
separate_arguments(SKIA_DEFINES UNIX_COMMAND "${SKIA_DEFINES}")
target_compile_options(skia-compare PRIVATE ${SKIA_DEFINES} -Wno-attributes)

# Upstream sample passes -1 as the initial extent hint. Wayland leaves extent
# selection to the client; use the actual initial SDL drawable extent.
file(READ "${SKIA}/tools/window/VulkanWindowContext.cpp" VK_WINDOW)
string(REPLACE "createSwapchain(-1, -1)" "createSwapchain(800, 600)" VK_WINDOW "${VK_WINDOW}")
# Initialize the present queue before swapchain creation so failure cleanup never
# waits on an uninitialized queue (demonstrated by the retained startup backtrace).
string(REPLACE "if (!this->createSwapchain(800, 600))" "fGetDeviceQueue(fDevice, fPresentQueueIndex, 0, &fPresentQueue);
    if (!this->createSwapchain(800, 600))" VK_WINDOW "${VK_WINDOW}")
string(REPLACE "return false;" "fprintf(stderr, \"Vulkan window failure at %d\\n\", __LINE__); return false;" VK_WINDOW "${VK_WINDOW}")
string(REPLACE "if (!fImages[i].fSurface)" "fprintf(stderr, \"Vulkan wrap: context=%d extent=%dx%d format=%d usage=%u samples=%d\\n\", bool(fContext), fWidth, fHeight, int(format), usageFlags, fSampleCount);
        if (!fImages[i].fSurface)" VK_WINDOW "${VK_WINDOW}")
# This pinned helper maps an sRGB VkFormat to the UNORM SkColorType, which
# Ganesh rejects. Match the pinned GrVkCaps format/color-type compatibility table.
string(REPLACE "case VK_FORMAT_R8G8B8A8_UNORM: // fall through
        case VK_FORMAT_R8G8B8A8_SRGB:" "case VK_FORMAT_R8G8B8A8_SRGB:
            colorType = kSRGBA_8888_SkColorType;
            break;
        case VK_FORMAT_R8G8B8A8_UNORM:" VK_WINDOW "${VK_WINDOW}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/VulkanWindowContext.cpp" "${VK_WINDOW}")


endif()
