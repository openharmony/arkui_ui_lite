CONFIG -= qt

TEMPLATE = lib
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
    FT2_BUILD_LIBRARY \
    FT_DEBUG_LEVEL_ERROR \
    FT_DEBUG_LEVEL_TRACE \
    FT_CONFIG_OPTION_USE_PNG \

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ../../libs

LIBPNG_PATH = ../../../../../../../../third_party/freetype
LIBPNG_SOURCE_PATH = ../../../../../../../../third_party/freetype/freetype-2.12.1
system("tar -xf $${LIBPNG_PATH}\\freetype-2.12.1.tar.xz -C $${LIBPNG_PATH}")

SOURCES += \
    $${LIBPNG_SOURCE_PATH}/src/autofit/autofit.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftbase.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftbbox.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftbdf.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftbitmap.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftcid.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftfstype.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftgasp.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftglyph.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftgxval.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftinit.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftmm.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftotval.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftpatent.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftpfr.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftstroke.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftsynth.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftsystem.c \
    $${LIBPNG_SOURCE_PATH}/src/base/fttype1.c \
    $${LIBPNG_SOURCE_PATH}/src/base/ftwinfnt.c \
    $${LIBPNG_SOURCE_PATH}/src/bdf/bdf.c \
    $${LIBPNG_SOURCE_PATH}/src/cache/ftcache.c \
    $${LIBPNG_SOURCE_PATH}/src/cff/cff.c \
    $${LIBPNG_SOURCE_PATH}/src/cid/type1cid.c \
    $${LIBPNG_SOURCE_PATH}/src/gzip/ftgzip.c \
    $${LIBPNG_SOURCE_PATH}/src/lzw/ftlzw.c \
    $${LIBPNG_SOURCE_PATH}/src/pcf/pcf.c \
    $${LIBPNG_SOURCE_PATH}/src/pfr/pfr.c \
    $${LIBPNG_SOURCE_PATH}/src/psaux/psaux.c \
    $${LIBPNG_SOURCE_PATH}/src/pshinter/pshinter.c \
    $${LIBPNG_SOURCE_PATH}/src/psnames/psmodule.c \
    $${LIBPNG_SOURCE_PATH}/src/raster/raster.c \
    $${LIBPNG_SOURCE_PATH}/src/sdf/ftbsdf.c \
    $${LIBPNG_SOURCE_PATH}/src/sdf/ftsdf.c \
    $${LIBPNG_SOURCE_PATH}/src/sdf/ftsdfcommon.c \
    $${LIBPNG_SOURCE_PATH}/src/sdf/ftsdfrend.c \
    $${LIBPNG_SOURCE_PATH}/src/sfnt/sfnt.c \
    $${LIBPNG_SOURCE_PATH}/src/smooth/smooth.c \
    $${LIBPNG_SOURCE_PATH}/src/svg/ftsvg.c \
    $${LIBPNG_SOURCE_PATH}/src/truetype/truetype.c \
    $${LIBPNG_SOURCE_PATH}/src/type1/type1.c \
    $${LIBPNG_SOURCE_PATH}/src/type42/type42.c \
    $${LIBPNG_SOURCE_PATH}/src/winfonts/winfnt.c \
    $${LIBPNG_SOURCE_PATH}/builds/windows/ftdebug.c

INCLUDEPATH += \
    ../../../../../../../../third_party/freetype/include \
    ../../../../../../../../third_party/freetype/freetype-2.12.1/include \
    ../../../../../../../../third_party/libpng \
    ../../../../../../../../third_party/libpng/libpng-1.6.44

LIBS += $$OUT_PWD/../../libs/libpng.dll
