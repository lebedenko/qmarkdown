qt_add_executable(qmarkdown-viewer main.cpp)
qt_add_qml_module(qmarkdown-viewer
    URI QMarkdownViewer
    RESOURCE_PREFIX /qt/qml
    QML_FILES Main.qml Samples.qml StylePanel.qml
    IMPORT_PATH "${CMAKE_BINARY_DIR}/qml")
target_link_libraries(qmarkdown-viewer PRIVATE QMarkdown::QMarkdown Qt6::QuickControls2)
if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(qmarkdown-viewer PRIVATE QMarkdown::QMarkdownPlugin)
    target_compile_definitions(qmarkdown-viewer PRIVATE QMARKDOWN_STATIC)
endif()
