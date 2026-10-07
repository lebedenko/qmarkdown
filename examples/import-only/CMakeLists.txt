qt_add_executable(qmarkdown-import-example main.cpp)
qt_add_qml_module(qmarkdown-import-example
    URI QMarkdownExample
    RESOURCE_PREFIX /qt/qml
    QML_FILES Main.qml
    IMPORT_PATH "${CMAKE_BINARY_DIR}/qml")
target_link_libraries(qmarkdown-import-example PRIVATE QMarkdown::QMarkdown)
if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(qmarkdown-import-example PRIVATE QMarkdown::QMarkdownPlugin)
    target_compile_definitions(qmarkdown-import-example PRIVATE QMARKDOWN_STATIC)
endif()
