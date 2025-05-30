CONFIG -= qt

TEMPLATE = lib
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ../../libs

LIBPNG_PATH = ../../../../../../../../third_party/libpng
LIBPNG_SOURCE_PATH = ../../../../../../../../third_party/libpng/libpng-1.6.44
system("tar -zxf $${LIBPNG_PATH}\\libpng-1.6.44.tar.gz -C $${LIBPNG_PATH}")

SOURCES += \
    $${LIBPNG_SOURCE_PATH}/png.c \
    $${LIBPNG_SOURCE_PATH}/pngerror.c \
    $${LIBPNG_SOURCE_PATH}/pngget.c \
    $${LIBPNG_SOURCE_PATH}/pngmem.c \
    $${LIBPNG_SOURCE_PATH}/pngpread.c \
    $${LIBPNG_SOURCE_PATH}/pngread.c \
    $${LIBPNG_SOURCE_PATH}/pngrio.c \
    $${LIBPNG_SOURCE_PATH}/pngrtran.c \
    $${LIBPNG_SOURCE_PATH}/pngrutil.c \
    $${LIBPNG_SOURCE_PATH}/pngset.c \
    $${LIBPNG_SOURCE_PATH}/pngtrans.c \
    $${LIBPNG_SOURCE_PATH}/pngwio.c \
    $${LIBPNG_SOURCE_PATH}/pngwrite.c \
    $${LIBPNG_SOURCE_PATH}/pngwtran.c \
    $${LIBPNG_SOURCE_PATH}/pngwutil.c

INCLUDEPATH += \
    ../../../../../../../../third_party/libpng \
    ../../../../../../../../third_party/zlib

LIBS += $${DESTDIR}/zlib.dll
