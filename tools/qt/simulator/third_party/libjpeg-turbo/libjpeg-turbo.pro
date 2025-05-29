CONFIG -= qt

TEMPLATE = lib
DEFINES += LIBJPEGTURBO_LIBRARY

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

LIBJPEG_TURBO_PATH = ../../../../../../../../third_party/libjpeg-turbo
LIBJPEG_TURBO_SOURCE_PATH = ../../../../../../../../third_party/libjpeg-turbo/libjpeg-turbo-3.1.0/src
system("tar -zxf $${LIBJPEG_TURBO_PATH}\\libjpeg-turbo-3.1.0.tar.gz -C $${LIBJPEG_TURBO_PATH}")

SOURCES += \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdlhuff.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jclossls.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jclhuff.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcdiffct.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jpeg_nbits.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcapimin.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcapistd.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jccoefct.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jccolor.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcdctmgr.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jchuff.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcicc.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcinit.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcmainct.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcmarker.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcmaster.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcomapi.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcparam.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcphuff.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcprepct.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jcsample.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jctrans.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdapimin.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdapistd.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdatadst.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdatasrc.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdcoefct.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdcolor.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jddctmgr.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdhuff.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdicc.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdinput.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdmainct.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdmarker.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdmaster.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdmerge.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdphuff.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdpostct.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdsample.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jdtrans.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jerror.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jfdctflt.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jfdctfst.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jfdctint.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jidctflt.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jidctfst.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jidctint.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jidctred.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jmemmgr.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jmemnobs.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jquant1.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jquant2.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jutils.c \
      $${LIBJPEG_TURBO_SOURCE_PATH}/jsimd_none.c

INCLUDEPATH += \
    ../../../../../../../../third_party/libjpeg-turbo/libjpeg-turbo-3.1.0/src
