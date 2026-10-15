qt_add_executable(qmarkdown-formatintervals-test tst_formatintervals.cpp
    ../src/QMarkdown/private/formatintervals.cpp)
target_include_directories(qmarkdown-formatintervals-test PRIVATE ../src/QMarkdown)
target_link_libraries(qmarkdown-formatintervals-test PRIVATE Qt6::Test)
add_test(NAME qmarkdown-formatintervals COMMAND qmarkdown-formatintervals-test)

qt_add_executable(qmarkdown-import-test tst_import.cpp)
target_link_libraries(qmarkdown-import-test PRIVATE QMarkdown::QMarkdown Qt6::Test)
if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(qmarkdown-import-test PRIVATE QMarkdown::QMarkdownPlugin)
    target_compile_definitions(qmarkdown-import-test PRIVATE QMARKDOWN_STATIC)
endif()
add_test(NAME qmarkdown-import COMMAND qmarkdown-import-test)
set_tests_properties(qmarkdown-import PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QML_IMPORT_PATH=${CMAKE_BINARY_DIR}/qml")

qt_add_executable(qmarkdown-parser-test tst_parser.cpp ../src/QMarkdown/private/document.cpp ../src/QMarkdown/private/inline.cpp
    $<TARGET_OBJECTS:qmarkdown_cmark>)
target_include_directories(qmarkdown-parser-test PRIVATE ../third_party/cmark/src)
target_include_directories(qmarkdown-parser-test PRIVATE ../src/QMarkdown)
target_link_libraries(qmarkdown-parser-test PRIVATE Qt6::Test Qt6::Gui)
target_compile_definitions(qmarkdown-parser-test PRIVATE QMARKDOWN_GFM_FIXTURE="${CMAKE_CURRENT_SOURCE_DIR}/gfm/task-lists.json"
    QMARKDOWN_TABLE_FIXTURE="${CMAKE_CURRENT_SOURCE_DIR}/gfm/tables.json"
    QMARKDOWN_TAG_FILTER_FIXTURE="${CMAKE_CURRENT_SOURCE_DIR}/gfm/tag-filter.json"
    QMARKDOWN_AUTOLINK_FIXTURE="${CMAKE_CURRENT_SOURCE_DIR}/gfm/extended-autolinks.json"
    QMARKDOWN_STRIKE_FIXTURE="${CMAKE_CURRENT_SOURCE_DIR}/gfm/strikethrough.json")
add_test(NAME qmarkdown-parser COMMAND qmarkdown-parser-test)

# Test-only evidence for the bundled parser and production model, with offline fixtures.
find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
add_test(NAME qmarkdown-qt-versions
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_qt_versions.py")
add_test(NAME qmarkdown-arch-recipe
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_arch_package.py")
add_test(NAME qmarkdown-release-upload
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_release_upload.py")
add_test(NAME qmarkdown-release-publish
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_release_publish.py")
add_test(NAME qmarkdown-release-delivery
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_release_delivery.py")
qt_add_executable(qmarkdown-commonmark-probe commonmark/probe.cpp
    ../src/QMarkdown/private/document.cpp ../src/QMarkdown/private/inline.cpp
    $<TARGET_OBJECTS:qmarkdown_cmark>)
target_include_directories(qmarkdown-commonmark-probe PRIVATE ../third_party/cmark/src ../src/QMarkdown)
target_link_libraries(qmarkdown-commonmark-probe PRIVATE Qt6::Core Qt6::Gui)
add_test(NAME qmarkdown-commonmark-oracle
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/commonmark/test_baseline.py"
        --probe "$<TARGET_FILE:qmarkdown-commonmark-probe>")
add_test(NAME qmarkdown-commonmark
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/scripts/verify-commonmark.py"
        --probe "$<TARGET_FILE:qmarkdown-commonmark-probe>"
        --report "${CMAKE_CURRENT_BINARY_DIR}/commonmark-report.json")

qt_add_executable(qmarkdown-view-test tst_view.cpp)
target_include_directories(qmarkdown-view-test PRIVATE ../src/QMarkdown)
if(QMARKDOWN_BUILD_EXAMPLES)
    target_compile_definitions(qmarkdown-view-test PRIVATE QMARKDOWN_VIEWER_SOURCE="${PROJECT_SOURCE_DIR}/examples/viewer/Main.qml")
    target_link_libraries(qmarkdown-view-test PRIVATE Qt6::QuickControls2 qmarkdown-font-editor)
endif()
target_link_libraries(qmarkdown-view-test PRIVATE QMarkdown::QMarkdown Qt6::Test)
if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(qmarkdown-view-test PRIVATE QMarkdown::QMarkdownPlugin)
    target_compile_definitions(qmarkdown-view-test PRIVATE QMARKDOWN_STATIC)
endif()
add_test(NAME qmarkdown-view COMMAND qmarkdown-view-test)
set_tests_properties(qmarkdown-view PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QML_IMPORT_PATH=${CMAKE_BINARY_DIR}/qml")

qt_add_executable(qmarkdown-resource-test tst_resources.cpp)
target_include_directories(qmarkdown-resource-test PRIVATE ../src/QMarkdown)
target_link_libraries(qmarkdown-resource-test PRIVATE QMarkdown::QMarkdown Qt6::Test)
add_test(NAME qmarkdown-resources COMMAND qmarkdown-resource-test)

if(QMARKDOWN_BUILD_EXAMPLES)
    qt_add_resources(qmarkdown-view-test "sample_images"
        PREFIX "/qmarkdown-samples" BASE "${PROJECT_SOURCE_DIR}/examples/viewer/images"
        FILES "${PROJECT_SOURCE_DIR}/examples/viewer/images/shapes.png" "${PROJECT_SOURCE_DIR}/examples/viewer/images/shapes.jpg")
endif()

if(QMARKDOWN_BUILD_BENCHMARKS)
    qt_add_executable(qmarkdown-inline-benchmark inlinebenchmark.cpp
        ../src/QMarkdown/private/formatintervals.cpp)
    target_include_directories(qmarkdown-inline-benchmark PRIVATE ../src/QMarkdown)
    target_link_libraries(qmarkdown-inline-benchmark PRIVATE Qt6::Core)
    target_compile_definitions(qmarkdown-inline-benchmark PRIVATE
        QMARKDOWN_BUILD_TYPE="$<IF:$<BOOL:$<CONFIG>>,$<CONFIG>,unspecified>"
        QMARKDOWN_COMPILER="${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    qt_add_executable(qmarkdown-search-benchmark searchbenchmark.cpp)
    target_link_libraries(qmarkdown-search-benchmark PRIVATE QMarkdown::QMarkdown)
    qt_add_executable(qmarkdown-benchmark benchmark.cpp)
    target_include_directories(qmarkdown-benchmark PRIVATE ../src/QMarkdown)
    target_link_libraries(qmarkdown-benchmark PRIVATE QMarkdown::QMarkdown Qt6::Test)
    target_compile_definitions(qmarkdown-benchmark PRIVATE
        QMARKDOWN_BUILD_TYPE="$<IF:$<BOOL:$<CONFIG>>,$<CONFIG>,unspecified>"
        QMARKDOWN_COMPILER="${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    if(NOT BUILD_SHARED_LIBS)
        target_link_libraries(qmarkdown-search-benchmark PRIVATE QMarkdown::QMarkdownPlugin)
        target_compile_definitions(qmarkdown-search-benchmark PRIVATE QMARKDOWN_STATIC)
        target_link_libraries(qmarkdown-benchmark PRIVATE QMarkdown::QMarkdownPlugin)
        target_compile_definitions(qmarkdown-benchmark PRIVATE QMARKDOWN_STATIC)
    endif()
endif()

qt_add_executable(qmarkdown-styling-test tst_styling.cpp)
target_include_directories(qmarkdown-styling-test PRIVATE ../src/QMarkdown)
target_link_libraries(qmarkdown-styling-test PRIVATE QMarkdown::QMarkdown Qt6::Test)
target_compile_definitions(qmarkdown-styling-test PRIVATE
    QMARKDOWN_PRESENTATION_DIR="${CMAKE_CURRENT_BINARY_DIR}/presentation")
if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(qmarkdown-styling-test PRIVATE QMarkdown::QMarkdownPlugin)
    target_compile_definitions(qmarkdown-styling-test PRIVATE QMARKDOWN_STATIC)
endif()
add_test(NAME qmarkdown-styling COMMAND qmarkdown-styling-test)
add_test(NAME qmarkdown-styling-fractional COMMAND qmarkdown-styling-test)
set_tests_properties(qmarkdown-styling PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QML_IMPORT_PATH=${CMAKE_BINARY_DIR}/qml")
set_tests_properties(qmarkdown-styling-fractional PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1.25;QML_IMPORT_PATH=${CMAKE_BINARY_DIR}/qml")

qt_add_executable(qmarkdown-selection-test tst_selection.cpp)
target_include_directories(qmarkdown-selection-test PRIVATE ../src/QMarkdown)
target_link_libraries(qmarkdown-selection-test PRIVATE QMarkdown::QMarkdown Qt6::Test)
if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(qmarkdown-selection-test PRIVATE QMarkdown::QMarkdownPlugin)
    target_compile_definitions(qmarkdown-selection-test PRIVATE QMARKDOWN_STATIC)
endif()
add_test(NAME qmarkdown-selection COMMAND qmarkdown-selection-test)
add_test(NAME qmarkdown-selection-fractional COMMAND qmarkdown-selection-test)
set_tests_properties(qmarkdown-selection PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QML_IMPORT_PATH=${CMAKE_BINARY_DIR}/qml")
set_tests_properties(qmarkdown-selection-fractional PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=1.25;QML_IMPORT_PATH=${CMAKE_BINARY_DIR}/qml")

qt_add_executable(qmarkdown-renderers-test tst_renderers.cpp)
target_include_directories(qmarkdown-renderers-test PRIVATE ../src/QMarkdown)
target_link_libraries(qmarkdown-renderers-test PRIVATE QMarkdown::QMarkdown Qt6::Test)
if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(qmarkdown-renderers-test PRIVATE QMarkdown::QMarkdownPlugin)
    target_compile_definitions(qmarkdown-renderers-test PRIVATE QMARKDOWN_STATIC)
endif()
add_test(NAME qmarkdown-renderers COMMAND qmarkdown-renderers-test)
set_tests_properties(qmarkdown-renderers PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QML_IMPORT_PATH=${CMAKE_BINARY_DIR}/qml")

qt_add_executable(qmarkdown-reading-test tst_reading.cpp)
target_include_directories(qmarkdown-reading-test PRIVATE ../src/QMarkdown)
target_link_libraries(qmarkdown-reading-test PRIVATE QMarkdown::QMarkdown Qt6::Test)
if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(qmarkdown-reading-test PRIVATE QMarkdown::QMarkdownPlugin)
    target_compile_definitions(qmarkdown-reading-test PRIVATE QMARKDOWN_STATIC)
endif()
add_test(NAME qmarkdown-reading COMMAND qmarkdown-reading-test)
set_tests_properties(qmarkdown-reading PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QML_IMPORT_PATH=${CMAKE_BINARY_DIR}/qml")
