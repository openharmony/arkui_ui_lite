CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

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

HARFBUZZ_PATH = ../../../../../../../../third_party/harfbuzz
HARFBUZZ_SOURCE_PATH = ../../../../../../../../third_party/harfbuzz/harfbuzz-2.8.2
system("tar -xf $${HARFBUZZ_PATH}\\harfbuzz-2.8.2.tar.xz -C $${HARFBUZZ_PATH}")

SOURCES += \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-aat-layout.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-aat-map.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-blob.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-buffer-serialize.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-buffer.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-common.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-face.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-fallback-shape.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-font.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-map.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-number.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-cff1-table.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-cff2-table.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-face.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-font.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-layout.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-map.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-math.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-meta.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-metrics.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-name.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-arabic.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-default.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-hangul.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-hebrew.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-indic-table.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-indic.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-khmer.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-myanmar.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-syllabic.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-thai.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-use.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-complex-vowel-constraints.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-fallback.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape-normalize.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-shape.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-tag.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ot-var.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-set.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-shape-plan.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-shape.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-shaper.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-static.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-subset-cff2.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-subset-cff-common.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-ucd.cc \
    $${HARFBUZZ_SOURCE_PATH}/src/hb-unicode.cc

INCLUDEPATH += \
    ../../../../../../../../third_party/harfbuzz
