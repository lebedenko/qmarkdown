cmake_minimum_required(VERSION 3.21)
project(QMarkdown VERSION 0.4.0 LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(QMARKDOWN_BUILD_EXAMPLES "Build examples" ${PROJECT_IS_TOP_LEVEL})
option(BUILD_TESTING "Build tests" ${PROJECT_IS_TOP_LEVEL})
set(QMARKDOWN_QML_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt6/qml" CACHE STRING
    "QML import directory, relative to the installation prefix or absolute")

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
qt_policy(SET QTP0001 NEW)

add_subdirectory(third_party/cmark)
add_subdirectory(src/QMarkdown)

configure_package_config_file(cmake/QMarkdownConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/QMarkdownConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QMarkdown"
    PATH_VARS QMARKDOWN_QML_INSTALL_DIR)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/QMarkdownConfigVersion.cmake"
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMinorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/QMarkdownConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/QMarkdownConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QMarkdown")
install(EXPORT QMarkdownTargets NAMESPACE QMarkdown::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QMarkdown")
install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/QMarkdown")

if(QMARKDOWN_BUILD_EXAMPLES)
    find_package(Qt6 6.8 REQUIRED COMPONENTS QuickControls2)
    add_subdirectory(examples/import-only)
    add_subdirectory(examples/viewer)
endif()
if(BUILD_TESTING)
    enable_testing()
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()
