# Private pinned engine; deliberately independent of upstream build policy.
add_library(qmarkdown_microtex OBJECT

        # atom folder
        src/atom/atom_basic.cpp
        src/atom/atom_char.cpp
        src/atom/atom_impl.cpp
        src/atom/atom_matrix.cpp
        src/atom/atom_row.cpp
        src/atom/atom_space.cpp
        src/atom/colors_def.cpp
        src/atom/unit_conversion.cpp
        # box folder
        src/box/box.cpp
        src/box/box_factory.cpp
        src/box/box_group.cpp
        src/box/box_single.cpp
        # core folder
        src/core/core.cpp
        src/core/formula.cpp
        src/core/formula_def.cpp
        src/core/glue.cpp
        src/core/localized_num.cpp
        src/core/macro.cpp
        src/core/macro_def.cpp
        src/core/macro_impl.cpp
        src/core/parser.cpp
        # fonts folder
        src/fonts/alphabet.cpp
        src/fonts/font_basic.cpp
        src/fonts/font_info.cpp
        src/fonts/fonts.cpp
        # utils folder
        src/utils/string_utils.cpp
        src/utils/utf.cpp
        src/utils/utils.cpp
        # res folder
        src/res/builtin/formula_mappings.res.cpp
        src/res/builtin/symbol_mapping.res.cpp
        src/res/builtin/tex_param.res.cpp
        src/res/builtin/tex_symbols.res.cpp
        src/res/font/bi10.def.cpp
        src/res/font/bx10.def.cpp
        src/res/font/cmbsy10.def.cpp
        src/res/font/cmbx10.def.cpp
        src/res/font/cmbxti10.def.cpp
        src/res/font/cmex10.def.cpp
        src/res/font/cmmi10.def.cpp
        src/res/font/cmmi10_unchanged.def.cpp
        src/res/font/cmmib10.def.cpp
        src/res/font/cmmib10_unchanged.def.cpp
        src/res/font/cmr10.def.cpp
        src/res/font/cmss10.def.cpp
        src/res/font/cmssbx10.def.cpp
        src/res/font/cmssi10.def.cpp
        src/res/font/cmsy10.def.cpp
        src/res/font/cmti10.def.cpp
        src/res/font/cmti10_unchanged.def.cpp
        src/res/font/cmtt10.def.cpp
        src/res/font/dsrom10.def.cpp
        src/res/font/eufb10.def.cpp
        src/res/font/eufm10.def.cpp
        src/res/font/i10.def.cpp
        src/res/font/moustache.def.cpp
        src/res/font/msam10.def.cpp
        src/res/font/msbm10.def.cpp
        src/res/font/r10.def.cpp
        src/res/font/r10_unchanged.def.cpp
        src/res/font/rsfs10.def.cpp
        src/res/font/sb10.def.cpp
        src/res/font/sbi10.def.cpp
        src/res/font/si10.def.cpp
        src/res/font/special.def.cpp
        src/res/font/ss10.def.cpp
        src/res/font/stmary10.def.cpp
        src/res/font/tt10.def.cpp
        src/res/parser/font_parser.cpp
        src/res/parser/formula_parser.cpp
        src/res/reg/builtin_font_reg.cpp
        src/res/reg/builtin_syms_reg.cpp
        src/res/sym/amsfonts.def.cpp
        src/res/sym/amssymb.def.cpp
        src/res/sym/base.def.cpp
        src/res/sym/stmaryrd.def.cpp
        src/res/sym/symspecial.def.cpp

        src/latex.cpp
        src/render.cpp
        src/platform/qt/graphic_qt.cpp ../tinyxml2/tinyxml2.cpp)
set_target_properties(qmarkdown_microtex PROPERTIES POSITION_INDEPENDENT_CODE ON CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(qmarkdown_microtex PRIVATE BUILD_QT tex=qmarkdown_tex tinyxml2=qmarkdown_tinyxml2)
target_include_directories(qmarkdown_microtex PRIVATE src ../tinyxml2)
target_link_libraries(qmarkdown_microtex PRIVATE Qt6::Gui)
