if(BUILD_SHARED_LIBS)
    set(qmarkdown_library_type SHARED)
else()
    set(qmarkdown_library_type STATIC)
endif()
qt_add_qml_module(QMarkdown
    ${qmarkdown_library_type}
    URI QMarkdown
    VERSION 1.0
    PLUGIN_TARGET QMarkdownPlugin
    CLASS_NAME QMarkdownPlugin
    RESOURCE_PREFIX /qt/qml
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/QMarkdown"
    OUTPUT_TARGETS qmarkdown_auxiliary_targets
    QML_FILES MarkdownView.qml
    SOURCES private/decodescheduler.h private/decodescheduler.cpp markdownresourcepolicy.h private/resourcecontroller.h private/resourcecontroller.cpp private/imageitem.h moduleanchor.h markdownstyle.h markdownstyle.cpp
        private/document.h private/document.cpp private/viewstate.h private/viewstate.cpp
        private/inline.h private/inlineadapter.h private/inline.cpp private/formattedtext.h private/formattedtext.cpp private/formatintervals.h private/formatintervals.cpp
)
target_include_directories(QMarkdown PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private")
target_sources(QMarkdown PRIVATE $<TARGET_OBJECTS:qmarkdown_cmark>)
target_include_directories(QMarkdown PRIVATE "${PROJECT_SOURCE_DIR}/third_party/cmark/src")
add_library(QMarkdown::QMarkdown ALIAS QMarkdown)
target_link_libraries(QMarkdown PUBLIC Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Network)
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
