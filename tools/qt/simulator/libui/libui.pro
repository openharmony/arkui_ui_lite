CONFIG -= qt

TEMPLATE = lib

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
    CJSON_HIDE_SYMBOLS \
    "ENABLE_SHAPING=0" \
    "ENABLE_ICU=1" \
    "ENABLE_VECTOR_FONT=1" \
    "ENABLE_SPANNABLE_STRING=1" \
    "ENABLE_BITMAP_FONT=0" \
    "ENABLE_MULTI_FONT=1" \
    "ENABLE_STATIC_FONT=0" \
    "GRAPHIC_ENABLE_LINECAP_FLAG=1" \
    "GRAPHIC_ENABLE_LINEJOIN_FLAG=1" \
    "GRAPHIC_ENABLE_ELLIPSE_FLAG=1" \
    "GRAPHIC_ENABLE_BEZIER_ARC_FLAG=1" \
    "GRAPHIC_ENABLE_ARC_FLAG=1" \
    "GRAPHIC_ENABLE_ROUNDEDRECT_FLAG=1" \
    "GRAPHIC_ENABLE_DASH_GENERATE_FLAG=1" \
    "GRAPHIC_ENABLE_BLUR_EFFECT_FLAG=1" \
    "GRAPHIC_ENABLE_SHADOW_EFFECT_FLAG=1" \
    "GRAPHIC_ENABLE_GRADIENT_FILL_FLAG=1" \
    "GRAPHIC_ENABLE_PATTERN_FILL_FLAG=1" \
    "GRAPHIC_ENABLE_DRAW_IMAGE_FLAG=1" \
    "GRAPHIC_ENABLE_DRAW_TEXT_FLAG=1" \
    "ENABLE_CANVAS_EXTEND=1" \
    "DEFAULT_ANIMATION=1" 


DEFINES += QT_COMPILER
#DEFINES += ENABLE_GIFLIB

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ../libs

SOURCES += \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/common/paint.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/depiction/depict_curve.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/rasterizer/rasterizer_cells_antialias.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/rasterizer/rasterizer_scanline_antialias.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/rasterizer/rasterizer_scanline_clip.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/graphic_timer.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/trans_affine.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/vertexgenerate/vertex_generate_dash.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/vertexgenerate/vertex_generate_stroke.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/vertexprimitive/geometry_arc.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/vertexprimitive/geometry_bezier_arc.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/vertexprimitive/geometry_curves.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/diagram/vertexprimitive/geometry_shorten_path.cpp \
    ../../../../frameworks/animator/animator.cpp \
    ../../../../frameworks/animator/animator_manager.cpp \
    ../../../../frameworks/animator/easing_equation.cpp \
    ../../../../frameworks/animator/interpolation.cpp \
    ../../../../frameworks/common/graphic_startup.cpp \
    ../../../../frameworks/common/image_decode_ability.cpp \
    ../../../../frameworks/common/image.cpp \
    ../../../../frameworks/common/input_device_manager.cpp \
    ../../../../frameworks/common/screen.cpp \
    ../../../../frameworks/common/task.cpp \
    ../../../../frameworks/common/text.cpp \
    ../../../../frameworks/common/spannable_string.cpp \
    ../../../../frameworks/common/typed_text.cpp \
    ../../../../frameworks/components/root_view.cpp \
    ../../../../frameworks/components/text_adapter.cpp \
    ../../../../frameworks/components/ui_abstract_clock.cpp \
    ../../../../frameworks/components/ui_abstract_progress.cpp \
    ../../../../frameworks/components/ui_abstract_scroll.cpp \
    ../../../../frameworks/components/ui_abstract_scroll_bar.cpp \
    ../../../../frameworks/components/ui_analog_clock.cpp \
    ../../../../frameworks/components/ui_arc_label.cpp \
    ../../../../frameworks/components/ui_arc_scroll_bar.cpp \
    ../../../../frameworks/components/ui_axis.cpp \
    ../../../../frameworks/components/ui_box_progress.cpp \
    ../../../../frameworks/components/ui_tree_manager.cpp \
    ../../../../frameworks/components/ui_box_scroll_bar.cpp \
    ../../../../frameworks/components/ui_button.cpp \
    ../../../../frameworks/components/ui_canvas.cpp \
    ../../../../frameworks/components/ui_chart.cpp \
    ../../../../frameworks/components/ui_checkbox.cpp \
    ../../../../frameworks/components/ui_circle_progress.cpp \
    ../../../../frameworks/components/ui_dialog.cpp \
    ../../../../frameworks/components/ui_digital_clock.cpp \
    ../../../../frameworks/components/ui_edit_text.cpp \
    ../../../../frameworks/components/ui_image_animator.cpp \
    ../../../../frameworks/components/ui_image_view.cpp \
    ../../../../frameworks/components/ui_label.cpp \
    ../../../../frameworks/components/ui_label_button.cpp \
    ../../../../frameworks/components/ui_list.cpp \
    ../../../../frameworks/components/ui_picker.cpp \
    ../../../../frameworks/components/ui_qrcode.cpp \
    ../../../../frameworks/components/ui_radio_button.cpp \
    ../../../../frameworks/components/ui_repeat_button.cpp \
    ../../../../frameworks/components/ui_scroll_view.cpp \
    ../../../../frameworks/components/ui_slider.cpp \
    ../../../../frameworks/components/ui_swipe_view.cpp \
    ../../../../frameworks/components/ui_texture_mapper.cpp \
    ../../../../frameworks/components/ui_time_picker.cpp \
    ../../../../frameworks/components/ui_toggle_button.cpp \
    ../../../../frameworks/components/ui_view.cpp \
    ../../../../frameworks/components/ui_view_group.cpp \
    ../../../../frameworks/components/ui_extend_image_view.cpp \
    ../../../../frameworks/core/input_method_manager.cpp \
    ../../../../frameworks/dock/focus_manager.cpp \
    ../../../../frameworks/core/render_manager.cpp \
    ../../../../frameworks/core/task_manager.cpp \
    ../../../../frameworks/default_resource/check_box_res.cpp \
    ../../../../frameworks/dfx/event_injector.cpp \
    ../../../../frameworks/dfx/key_event_injector.cpp \
    ../../../../frameworks/dfx/performance_task.cpp \
    ../../../../frameworks/dfx/point_event_injector.cpp \
    ../../../../frameworks/dfx/ui_dump_dom_tree.cpp \
    ../../../../frameworks/dfx/ui_view_bounds.cpp \
    ../../../../frameworks/dock/input_device.cpp \
    ../../../../frameworks/dock/key_input_device.cpp \
    ../../../../frameworks/dock/pointer_input_device.cpp \
    ../../../../frameworks/dock/rotate_input_device.cpp \
    ../../../../frameworks/dock/screen_device_proxy.cpp \
    ../../../../frameworks/dock/vibrator_manager.cpp \
    ../../../../frameworks/dock/virtual_input_device.cpp \
    ../../../../frameworks/engines/gfx/gfx_engine_manager.cpp \
    ../../../../frameworks/engines/gfx/soft_engine.cpp \
    ../../../../frameworks/draw/clip_utils.cpp \
    ../../../../frameworks/draw/draw_arc.cpp \
    ../../../../frameworks/draw/draw_canvas.cpp \
    ../../../../frameworks/draw/draw_curve.cpp \
    ../../../../frameworks/draw/draw_image.cpp \
    ../../../../frameworks/draw/draw_label.cpp \
    ../../../../frameworks/draw/draw_line.cpp \
    ../../../../frameworks/draw/draw_rect.cpp \
    ../../../../frameworks/draw/draw_triangle.cpp \
    ../../../../frameworks/draw/draw_utils.cpp \
    ../../../../frameworks/events/event.cpp \
    ../../../../frameworks/font/base_font.cpp \
    ../../../../frameworks/font/font_ram_allocator.cpp \
    ../../../../frameworks/font/glyphs_cache.cpp \
    ../../../../frameworks/font/glyphs_file.cpp \
    ../../../../frameworks/font/glyphs_manager.cpp \
    ../../../../frameworks/font/ui_font.cpp \
    ../../../../frameworks/font/ui_font_adaptor.cpp \
    ../../../../frameworks/font/ui_font_allocator.cpp \
    ../../../../frameworks/font/ui_font_bitmap.cpp \
    ../../../../frameworks/font/ui_font_builder.cpp \
    ../../../../frameworks/font/ui_font_cache.cpp \
    ../../../../frameworks/font/ui_font_cache_manager.cpp \
    ../../../../frameworks/font/ui_font_vector.cpp \
    ../../../../frameworks/font/ui_line_break.cpp \
    ../../../../frameworks/font/ui_multi_font_manager.cpp \
    ../../../../frameworks/font/ui_text_shaping.cpp \
    ../../../../frameworks/imgdecode/cache_manager.cpp \
    ../../../../frameworks/imgdecode/file_img_decoder.cpp \
    ../../../../frameworks/imgdecode/image_load.cpp \
    ../../../../frameworks/layout/flex_layout.cpp \
    ../../../../frameworks/layout/grid_layout.cpp \
    ../../../../frameworks/layout/list_layout.cpp \
    ../../../../frameworks/render/render_base.cpp \
    ../../../../frameworks/render/render_pixfmt_rgba_blend.cpp \
    ../../../../frameworks/themes/theme.cpp \
    ../../../../frameworks/themes/theme_manager.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/color.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/geometry2d.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/graphic_math.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/graphic_performance.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/hal_tick.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/mem_api.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/style.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/sys_info.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/transform.cpp \
    ../../../../../../graphic/graphic_utils_lite/frameworks/version.cpp \
    ../../../../../../../third_party/bounds_checking_function/src/memset_s.c \
    ../../../../../../../third_party/cJSON/cJSON.c \
    ../../../../frameworks/render/render_scanline.cpp \

HEADERS += \
    ../../../../../../graphic/graphic_utils_lite/interfaces/innerkits/graphic_neon_pipeline.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/innerkits/graphic_neon_utils.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/innerkits/graphic_timer.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/color.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/common/common_basics.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/common/common_clip_operate.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/common/common_math.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/common/paint.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/depiction/depict_adaptor_vertex_generate.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/depiction/depict_curve.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/depiction/depict_dash.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/depiction/depict_stroke.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/depiction/depict_transform.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/imagefilter/filter_blur.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/rasterizer/rasterizer_cells_antialias.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/rasterizer/rasterizer_scanline_antialias.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/rasterizer/rasterizer_scanline_clip.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/scanline/geometry_scanline.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/spancolorfill/fill_base.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/spancolorfill/fill_gradient.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/spancolorfill/fill_gradient_lut.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/spancolorfill/fill_interpolator.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/spancolorfill/fill_pattern_rgba.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertex_generate/vertex_generate_dash.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertex_generate/vertex_generate_stroke.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_arc.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_array.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_bezier_arc.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_curves.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_dda_line.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_ellipse.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_math_stroke.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_path_storage.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_plaindata_array.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_range_adapter.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_shorten_path.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/gfx_utils/diagram/vertexprimitive/geometry_vertex_sequence.h \
    ../../../../frameworks/common/typed_text.h \
    ../../../../frameworks/core/render_manager.h \
    ../../../../frameworks/default_resource/check_box_res.h \
    ../../../../frameworks/dfx/key_event_injector.h \
    ../../../../frameworks/dfx/point_event_injector.h \
    ../../../../frameworks/components/ui_tree_manager.h \
    ../../../../frameworks/dfx/ui_view_bounds.h \
    ../../../../frameworks/dock/input_device.h \
    ../../../../frameworks/dock/pointer_input_device.h \
    ../../../../frameworks/dock/virtual_input_device.h \
    ../../../../frameworks/draw/clip_utils.h \
    ../../../../frameworks/draw/draw_arc.h \
    ../../../../frameworks/draw/draw_canvas.h \
    ../../../../frameworks/draw/draw_curve.h \
    ../../../../frameworks/draw/draw_image.h \
    ../../../../frameworks/draw/draw_label.h \
    ../../../../frameworks/draw/draw_line.h \
    ../../../../frameworks/draw/draw_rect.h \
    ../../../../frameworks/draw/draw_triangle.h \
    ../../../../frameworks/draw/draw_utils.h \
    ../../../../frameworks/font/ui_font_adaptor.h \
    ../../../../frameworks/font/ui_multi_font_manager.h \
    ../../../../frameworks/imgdecode/cache_manager.h \
    ../../../../frameworks/imgdecode/file_img_decoder.h \
    ../../../../frameworks/imgdecode/image_load.h \
    ../../../../frameworks/render/render_base.h \
    ../../../../frameworks/render/render_buffer.h \
    ../../../../frameworks/render/render_pixfmt_rgba_blend.h \
    ../../../../frameworks/render/render_scanline.h \
    ../../../../interfaces/innerkits/common/graphic_startup.h \
    ../../../../interfaces/innerkits/common/image_decode_ability.h \
    ../../../../interfaces/innerkits/common/input_device_manager.h \
    ../../../../interfaces/innerkits/common/input_method_manager.h \
    ../../../../interfaces/innerkits/common/task_manager.h \
    ../../../../interfaces/innerkits/dock/focus_manager.h \
    ../../../../interfaces/innerkits/dock/rotate_input_device.h \
    ../../../../interfaces/innerkits/dock/vibrator_manager.h \
    ../../../../interfaces/innerkits/font/ui_font_builder.h \
    ../../../../interfaces/innerkits/engines/gfx/gfx_engine_manager.h \
    ../../../../interfaces/innerkits/engines/gfx/soft_engine.h \
    ../../../../interfaces/kits/animator/animator.h \
    ../../../../interfaces/kits/animator/easing_equation.h \
    ../../../../interfaces/kits/animator/interpolation.h \
    ../../../../interfaces/kits/common/image.h \
    ../../../../interfaces/kits/common/screen.h \
    ../../../../interfaces/kits/common/task.h \
    ../../../../interfaces/kits/common/text.h \
    ../../../../interfaces/kits/common/spannable_string.h \
    ../../../../interfaces/kits/components/abstract_adapter.h \
    ../../../../interfaces/kits/components/root_view.h \
    ../../../../interfaces/kits/components/text_adapter.h \
    ../../../../interfaces/kits/components/ui_abstract_clock.h \
    ../../../../interfaces/kits/components/ui_abstract_progress.h \
    ../../../../interfaces/kits/components/ui_abstract_scroll.h \
    ../../../../interfaces/kits/components/ui_analog_clock.h \
    ../../../../interfaces/kits/components/ui_arc_label.h \
    ../../../../interfaces/kits/components/ui_axis.h \
    ../../../../interfaces/kits/components/ui_box_progress.h \
    ../../../../interfaces/kits/components/ui_button.h \
    ../../../../interfaces/kits/components/ui_canvas.h \
    ../../../../interfaces/kits/components/ui_chart.h \
    ../../../../interfaces/kits/components/ui_checkbox.h \
    ../../../../interfaces/kits/components/ui_circle_progress.h \
    ../../../../interfaces/kits/components/ui_dialog.h \
    ../../../../interfaces/kits/components/ui_digital_clock.h \
    ../../../../interfaces/kits/components/ui_edit_text.h \
    ../../../../interfaces/kits/components/ui_image_animator.h \
    ../../../../interfaces/kits/components/ui_image_view.h \
    ../../../../interfaces/kits/components/ui_label.h \
    ../../../../interfaces/kits/components/ui_label_button.h \
    ../../../../interfaces/kits/components/ui_list.h \
    ../../../../interfaces/kits/components/ui_picker.h \
    ../../../../interfaces/kits/components/ui_qrcode.h \
    ../../../../interfaces/kits/components/ui_radio_button.h \
    ../../../../interfaces/kits/components/ui_repeat_button.h \
    ../../../../interfaces/kits/components/ui_scroll_view.h \
    ../../../../interfaces/kits/components/ui_slider.h \
    ../../../../interfaces/kits/components/ui_swipe_view.h \
    ../../../../interfaces/kits/components/ui_texture_mapper.h \
    ../../../../interfaces/kits/components/ui_time_picker.h \
    ../../../../interfaces/kits/components/ui_toggle_button.h \
    ../../../../interfaces/kits/components/ui_view.h \
    ../../../../interfaces/kits/components/ui_view_group.h \
    ../../../../interfaces/kits/components/ui_extend_image_view.h \
    ../../../../interfaces/kits/dfx/event_injector.h \
    ../../../../interfaces/kits/dfx/ui_dump_dom_tree.h \
    ../../../../interfaces/kits/events/aod_callback.h \
    ../../../../interfaces/kits/events/cancel_event.h \
    ../../../../interfaces/kits/events/click_event.h \
    ../../../../interfaces/kits/events/drag_event.h \
    ../../../../interfaces/kits/events/event.h \
    ../../../../interfaces/kits/events/long_press_event.h \
    ../../../../interfaces/kits/events/press_event.h \
    ../../../../interfaces/kits/events/release_event.h \
    ../../../../interfaces/kits/events/virtual_device_event.h \
    ../../../../interfaces/kits/font/ui_font.h \
    ../../../../interfaces/kits/font/ui_font_header.h \
    ../../../../interfaces/kits/layout/flex_layout.h \
    ../../../../interfaces/kits/layout/grid_layout.h \
    ../../../../interfaces/kits/layout/layout.h \
    ../../../../interfaces/kits/layout/list_layout.h \
    ../../../../interfaces/kits/themes/theme.h \
    ../../../../interfaces/kits/themes/theme_manager.h \
    ../../../../interfaces/kits/window/window.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/innerkits/graphic_config.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/innerkits/graphic_locker.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/innerkits/graphic_semaphore.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/innerkits/hal_tick.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/color.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/file.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/geometry2d.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/graphic_assert.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/graphic_log.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/graphic_math.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/graphic_types.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/heap_base.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/image_info.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/input_event_info.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/list.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/mem_api.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/rect.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/style.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/sys_info.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/transform.h \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits/version.h \
    ../../../../../wms/interfaces/innerkits/lite_wm_type.h \
    ../../../../../../../third_party/bounds_checking_function/include/securec.h \
    ../../../../../../../third_party/bounds_checking_function/include/securectype.h \
    ../../../../../../../third_party/bounds_checking_function/src/input.inl \
    ../../../../../../../third_party/bounds_checking_function/src/output.inl \
    ../../../../../../../third_party/bounds_checking_function/src/secinput.h \
    ../../../../../../../third_party/bounds_checking_function/src/securecutil.h \
    ../../../../../../../third_party/bounds_checking_function/src/secureprintoutput.h \
    ../../../../../../../third_party/cJSON/cJSON.h \
    ../../../../../../../third_party/cJSON/cJSON_Utils.h \
    ../../../../../../../tools/developer_tools_lite/graphic_tool/iar_project/config/gpu_2d/graphic_config.h


INCLUDEPATH += \
    ../../../../frameworks \
    ../../../../frameworks/diagram/include \
    ../../../../../../graphic/graphic_utils_lite/frameworks/windows \
    ../../../../../../graphic/graphic_utils_lite/interfaces/innerkits \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits \
    ../../../../../ui_lite/interfaces/innerkits \
    ../../../../../ui_lite/interfaces/kits \
    ../../../../../../../third_party/bounds_checking_function/include \
    ../../../../../../../third_party/harfbuzz \
    ../../../../../../../third_party/harfbuzz/harfbuzz-2.8.2 \
    ../../../../../../../third_party/icu/icu4c/source/common \
    ../../../../../../../third_party/freetype/include \
    ../../../../../../../third_party/cJSON \
    ../../../../../../../third_party/libjpeg-turbo \
    ../../../../../../../third_party/libjpeg-turbo/libjpeg-turbo-2.1.1 \
    ../../../../../../../third_party/libpng \
    ../../../../../../../third_party/libpng/libpng-1.6.38 \
    ../../../../../../../third_party/qrcodegen/cpp


LIBS += $$OUT_PWD/../libs/libpng.dll
LIBS += $$OUT_PWD/../libs/libjpeg-turbo.dll
LIBS += $$OUT_PWD/../libs/qrcodegen.dll
LIBS += $$OUT_PWD/../libs/freetype.dll
LIBS += $$OUT_PWD/../libs/libharfbuzz.a
LIBS += $$OUT_PWD/../libs/icu.dll

if(contains(DEFINES, ENABLE_GIFLIB)){
    INCLUDEPATH += ../../../../../../../third_party/giflib

    LIBS += $$OUT_PWD/../libs/giflib.dll
}
