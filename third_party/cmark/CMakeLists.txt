# Private offline bundle; deliberately do not add upstream build/install targets.
set(cmark_sources
    blocks.c buffer.c cmark.c cmark_ctype.c commonmark.c
    houdini_href_e.c houdini_html_e.c houdini_html_u.c html.c inlines.c
    iterator.c latex.c man.c node.c references.c render.c scanners.c utf8.c xml.c)
list(TRANSFORM cmark_sources PREPEND "src/")
add_library(qmarkdown_cmark OBJECT ${cmark_sources})
set_target_properties(qmarkdown_cmark PROPERTIES POSITION_INDEPENDENT_CODE ON
    C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF
    C_VISIBILITY_PRESET hidden AUTOMOC OFF)
target_include_directories(qmarkdown_cmark PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(qmarkdown_cmark PRIVATE CMARK_STATIC_DEFINE)
if(MSVC)
    target_compile_options(qmarkdown_cmark PRIVATE "/FI${CMAKE_CURRENT_SOURCE_DIR}/symbols.h")
else()
    target_compile_options(qmarkdown_cmark PRIVATE -include "${CMAKE_CURRENT_SOURCE_DIR}/symbols.h")
endif()
install(FILES COPYING PROVENANCE.md DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/QMarkdown/cmark")
