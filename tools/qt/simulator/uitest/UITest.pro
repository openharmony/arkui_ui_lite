
QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
    "ENABLE_ICU=1" \
    "ENABLE_VECTOR_FONT=1" \
    "ENABLE_SPANNABLE_STRING=1" \
    "ENABLE_BITMAP_FONT=0"

DEFINES += QT_COMPILER

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ../libs

FORMS += \
    main_widget.ui

TRANSLATIONS += helloqt_zh_CN.ts

SOURCES += \
    ../drivers/display/gui_thread.cpp \
    ../drivers/display/monitor.cpp \
    ../drivers/display/task_thread.cpp \
    ../drivers/indev/key_input.cpp \
    ../drivers/indev/mouse_input.cpp \
    ../drivers/indev/mousewheel_input.cpp \
    main.cpp \
    main_widget.cpp

HEADERS += \
    ../drivers/config.h \
    ../drivers/display/gui_thread.h \
    ../drivers/display/task_thread.h \
    ../drivers/display/monitor.h \
    ../drivers/indev/key_input.h \
    ../drivers/indev/mouse_input.h \
    ../drivers/indev/mousewheel_input.h \
    main_widget.h

INCLUDEPATH += \
    ../drivers/display \
    ../drivers/indev \
    ../drivers \
    ../../../../../ui_lite/frameworks \
    ../../../../../ui_lite/interfaces/innerkits \
    ../../../../../ui_lite/interfaces/kits \
    ../../../../../ui_lite/test/framework \
    ../../../../../../graphic/graphic_utils_lite/interfaces/innerkits \
    ../../../../../../graphic/graphic_utils_lite/interfaces/kits \
    ../../../../../../../third_party/freetype/freetype-2.12.1/include \
    ../../../../test/framework/include \
    ../../../../../../../third_party/bounds_checking_function/include \
    ../../../../test/autotest/include \

LIBS += $$OUT_PWD/../libs/libui.dll
LIBS += $$OUT_PWD/../libs/test.dll
LIBS += $$OUT_PWD/../libs/auto_test.dll
