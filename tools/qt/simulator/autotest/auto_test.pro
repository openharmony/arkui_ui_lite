
QT += network

CONFIG += qt

TEMPLATE = lib

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
    "ENABLE_ICU=1" \
    "ENABLE_VECTOR_FONT=1" \
    "ENABLE_SPANNABLE_STRING=1" \
    "ENABLE_BITMAP_FONT=0" \
    "ENABLE_MULTI_FONT=1" \
    "ENABEL_UI_AUTO_TEST=0" \
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
    "DEFAULT_ANIMATION=1" 

DEFINES += QT_COMPILER
#DEFINES += ENABLE_GIFLIB

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ../libs

SOURCES += \
    ../../../../test/autotest/src/auto_test_manager.cpp \
    ../../../../test/autotest/src/compare_tools.cpp \
    ../../../../test/autotest/src/socket_thread.cpp \
    ../../../../test/autotest/src/tcp_socket.cpp \
    ../../../../test/autotest/src/tcp_socket_manager.cpp \
    ../../../../test/autotest/src/ui_auto_test.cpp \

HEADERS += \
    ../../../../test/autotest/include/auto_test_manager.h \
    ../../../../test/autotest/include/compare_tools.h \
    ../../../../test/autotest/include/scoket_thread.h \
    ../../../../test/autotest/include/tcp_socket.h \
    ../../../../test/autotest/include/tcp_socket_manager.h \
    ../../../../test/autotest/include/ui_auto_test.h \
    ../../../../test/autotest/include/ui_test_message.h \
	
INCLUDEPATH += \
    ../../../../frameworks \
    ../../../../../../graphic/graphic_utils_lite/frameworks/windows \
    ../../../../../../graphic/graphic_utils_lite/interfaces/innerkits \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits \
    ../../../../../ui_lite/interfaces/frameworks \
    ../../../../../ui_lite/interfaces/innerkits \
    ../../../../../ui_lite/interfaces/kits \
    ../../../../test/framework \
    ../../../../test/framework/include \
    ../../../../test/autotest/src \
    ../../../../test/autotest/include \
    ../../../../test/uitest \
    ../../../../../../../third_party/bounds_checking_function/include \
    ../../../../../../../third_party/harfbuzz \
    ../../../../../../../third_party/icu/icu4c/source/common \
    ../../../../../../../third_party/freetype/include \
    ../../../../../../../third_party/cJSON \
    ../../../../../../../third_party/libjpeg-turbo \
    ../../../../../../../third_party/libpng 

if(contains(DEFINES, ENABLE_GIFLIB)){
    INCLUDEPATH += ../../../../../../../third_party/giflib
}

LIBS += $$OUT_PWD/../libs/libui.dll
LIBS += $$OUT_PWD/../libs/test.dll
