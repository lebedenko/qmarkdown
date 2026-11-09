cmake_minimum_required(VERSION 3.21)
project(scale_study LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
if(STUDY_QT)
find_package(Qt6 REQUIRED COMPONENTS Quick)
if(NOT Qt6_VERSION VERSION_EQUAL EXPECTED_QT)
message(FATAL_ERROR "Exact Qt mismatch")
endif()
add_executable(qt-compare qt.cpp)
target_include_directories(qt-compare PRIVATE "${PREPARED_STAGE}/src/QMarkdown")
target_link_libraries(qt-compare PRIVATE "${PREPARED_LIBRARY}" Qt6::Quick)
else()
find_package(PkgConfig REQUIRED)
pkg_check_modules(NATIVE_DEPS REQUIRED IMPORTED_TARGET sdl2 freetype2 harfbuzz icu-uc icu-i18n libpng fontconfig zlib vulkan gl)
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

add_executable(study-layout-check check.cpp)
if(STUDY_QT)
set(VIEWER qt-compare)
target_compile_definitions(study-layout-check PRIVATE STUDY_QT)
else()
set(VIEWER skia-compare)
endif()
foreach(PROP INCLUDE_DIRECTORIES COMPILE_OPTIONS COMPILE_DEFINITIONS LINK_LIBRARIES)
get_target_property(VALUE ${VIEWER} ${PROP})
if(VALUE)
set_target_properties(study-layout-check PROPERTIES ${PROP} "${VALUE}")
endif()
endforeach()
if(STUDY_QT)
target_compile_definitions(study-layout-check PRIVATE STUDY_QT)
else()
target_compile_options(study-layout-check PRIVATE -ffunction-sections -fdata-sections)
target_link_options(study-layout-check PRIVATE -Wl,--gc-sections)
endif()
