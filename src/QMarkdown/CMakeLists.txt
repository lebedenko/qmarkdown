if(BUILD_SHARED_LIBS)
    set(qmarkdown_library_type SHARED)
else()
    set(qmarkdown_library_type STATIC)
endif()
set_source_files_properties(private/BlockSequence.qml PROPERTIES
    QT_QML_INTERNAL_TYPE TRUE QT_RESOURCE_ALIAS BlockSequence.qml)
set_source_files_properties(private/TextBlock.qml PROPERTIES
    QT_QML_INTERNAL_TYPE TRUE QT_RESOURCE_ALIAS TextBlock.qml)
set_source_files_properties(private/SelectableText.qml PROPERTIES
    QT_QML_INTERNAL_TYPE TRUE QT_RESOURCE_ALIAS SelectableText.qml)
set_source_files_properties(private/ImageBlock.qml PROPERTIES
    QT_QML_INTERNAL_TYPE TRUE QT_RESOURCE_ALIAS ImageBlock.qml)
set_source_files_properties(private/ListBlock.qml PROPERTIES
    QT_QML_INTERNAL_TYPE TRUE QT_RESOURCE_ALIAS ListBlock.qml)
set_source_files_properties(private/HeadingSeparator.qml PROPERTIES
    QT_QML_INTERNAL_TYPE TRUE QT_RESOURCE_ALIAS HeadingSeparator.qml)
set_source_files_properties(private/CodeSurface.qml PROPERTIES
    QT_QML_INTERNAL_TYPE TRUE QT_RESOURCE_ALIAS CodeSurface.qml)
set_source_files_properties(private/CodeActionHost.qml PROPERTIES
    QT_QML_INTERNAL_TYPE TRUE QT_RESOURCE_ALIAS CodeActionHost.qml)
set_source_files_properties(private/TableBlock.qml PROPERTIES
    QT_QML_INTERNAL_TYPE TRUE QT_RESOURCE_ALIAS TableBlock.qml)
set_source_files_properties(private/Layout.js PROPERTIES QT_QML_SKIP_QMLDIR_ENTRY TRUE)
set_source_files_properties(private/TaskIndicatorHost.qml PROPERTIES
    QT_QML_INTERNAL_TYPE TRUE QT_RESOURCE_ALIAS TaskIndicatorHost.qml)
qt_add_qml_module(QMarkdown
    ${qmarkdown_library_type}
    URI QMarkdown
    VERSION 1.11
    PLUGIN_TARGET QMarkdownPlugin
    CLASS_NAME QMarkdownPlugin
    RESOURCE_PREFIX /qt/qml
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/QMarkdown"
    OUTPUT_TARGETS qmarkdown_auxiliary_targets
    QML_FILES MarkdownView.qml
        private/BlockSequence.qml private/TextBlock.qml private/SelectableText.qml
        private/ImageBlock.qml private/ListBlock.qml private/HeadingSeparator.qml
        private/CodeSurface.qml private/CodeActionHost.qml private/TableBlock.qml private/TaskIndicatorHost.qml private/Layout.js
    SOURCES syntaxtheme.h syntaxtheme.cpp private/syntaxhighlighter.h private/syntaxhighlighter.cpp private/syntaxbackend.cpp
        private/linkcontroller.h private/linkcontroller.cpp private/taskindicatoritem.cpp private/taskindicatoritem.h private/selectionregistration.h private/selectionregistration.cpp
        private/presentation.h private/presentation.cpp private/decodescheduler.h private/decodescheduler.cpp markdownresourcepolicy.h private/resourcecontroller.h private/resourcecontroller.cpp private/imageitem.h moduleanchor.h markdownstyle.h markdownstyle.cpp
        private/document.h private/document.cpp private/viewstate.h private/viewstate.cpp
        private/inline.h private/inlineadapter.h private/inline.cpp private/formattedtext.h private/formattedtext.cpp private/formatintervals.h private/formatintervals.cpp
)
target_include_directories(QMarkdown PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private")
target_sources(QMarkdown PRIVATE $<TARGET_OBJECTS:qmarkdown_cmark>)
target_include_directories(QMarkdown PRIVATE "${PROJECT_SOURCE_DIR}/third_party/cmark/src")
add_library(QMarkdown::QMarkdown ALIAS QMarkdown)
target_link_libraries(QMarkdown PUBLIC Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Network)
if(QMARKDOWN_WITH_KSYNTAXHIGHLIGHTING)
    target_link_libraries(QMarkdown PRIVATE KF6::SyntaxHighlighting)
    target_compile_definitions(QMarkdown PRIVATE QMARKDOWN_WITH_KSYNTAXHIGHLIGHTING)
endif()
set_target_properties(QMarkdown PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

install(TARGETS QMarkdown ${qmarkdown_auxiliary_targets}
    EXPORT QMarkdownTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    OBJECTS DESTINATION "${CMAKE_INSTALL_LIBDIR}/QMarkdown")
if(BUILD_SHARED_LIBS)
    get_filename_component(qmarkdown_qml_destination "${QMARKDOWN_QML_INSTALL_DIR}"
        ABSOLUTE BASE_DIR "${CMAKE_INSTALL_PREFIX}")
    get_filename_component(qmarkdown_library_destination "${CMAKE_INSTALL_LIBDIR}"
        ABSOLUTE BASE_DIR "${CMAKE_INSTALL_PREFIX}")
    file(RELATIVE_PATH qmarkdown_plugin_library_path
        "${qmarkdown_qml_destination}/QMarkdown" "${qmarkdown_library_destination}")
    if(APPLE)
        set_target_properties(QMarkdownPlugin PROPERTIES
            INSTALL_RPATH "@loader_path/${qmarkdown_plugin_library_path}")
    elseif(UNIX)
        set_target_properties(QMarkdownPlugin PROPERTIES
            INSTALL_RPATH "$ORIGIN/${qmarkdown_plugin_library_path}")
    endif()
    install(TARGETS QMarkdownPlugin
        LIBRARY DESTINATION "${QMARKDOWN_QML_INSTALL_DIR}/QMarkdown"
        RUNTIME DESTINATION "${QMARKDOWN_QML_INSTALL_DIR}/QMarkdown")
else()
    add_library(QMarkdown::QMarkdownPlugin ALIAS QMarkdownPlugin)
    install(TARGETS QMarkdownPlugin EXPORT QMarkdownTargets
        ARCHIVE DESTINATION "${QMARKDOWN_QML_INSTALL_DIR}/QMarkdown")
endif()
install(FILES "${CMAKE_BINARY_DIR}/qml/QMarkdown/qmldir"
    "${CMAKE_BINARY_DIR}/qml/QMarkdown/QMarkdown.qmltypes"
    DESTINATION "${QMARKDOWN_QML_INSTALL_DIR}/QMarkdown")

# Implementation adapters are registered only in the private URI. Metadata supports
# tooling; runtime registration lives in the backing library for both variants.
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/qml/QMarkdown/Private")
configure_file(private/qmldir "${CMAKE_BINARY_DIR}/qml/QMarkdown/Private/qmldir" COPYONLY)
configure_file(private/private.qmltypes "${CMAKE_BINARY_DIR}/qml/QMarkdown/Private/private.qmltypes" COPYONLY)
install(FILES MarkdownView.qml DESTINATION "${QMARKDOWN_QML_INSTALL_DIR}/QMarkdown")
install(FILES private/qmldir private/private.qmltypes
    DESTINATION "${QMARKDOWN_QML_INSTALL_DIR}/QMarkdown/Private")

install(FILES private/BlockSequence.qml private/TextBlock.qml private/SelectableText.qml
    private/ImageBlock.qml private/ListBlock.qml private/HeadingSeparator.qml
    private/CodeSurface.qml private/CodeActionHost.qml private/TableBlock.qml private/TaskIndicatorHost.qml
    DESTINATION "${QMARKDOWN_QML_INSTALL_DIR}/QMarkdown")
install(FILES private/Layout.js DESTINATION "${QMARKDOWN_QML_INSTALL_DIR}/QMarkdown/private")
