/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common/qrcodegen/qrcode_version.h"
#include "common/qrcodegen/qrcode_item.h"
#include "securec.h"

#define QR_NUM_IS_FIVE 5

typedef struct {
    uint8_t width;
    uint16_t words;
    uint8_t remainder;
    uint16_t ec;
} QrcodeVersionInfo;

typedef struct {
    uint8_t *data;
    int32_t dataWidth;
} QrcodeVersionDrawLine;

static uint8_t g_qrcodeEcc = QRCODE_ECC_HIGH;

static const QrcodeVersionInfo g_qrcodeVersions[QRCODE_ECC_MAX][QRCODE_VERSION_MAX + 1] = {
    {
        { 0, 0, 0, 0 },         { 21, 26, 0, 10 },      { 25, 44, 7, 16 },      { 29, 70, 7, 26 },
        { 33, 100, 7, 36 },     { 37, 134, 7, 48 },     { 41, 172, 7, 64 },     { 45, 196, 0, 72 },
        { 49, 242, 0, 88 },     { 53, 292, 0, 110 },    { 57, 346, 0, 130 },    { 61, 404, 0, 150 },
        { 65, 466, 0, 176 },    { 69, 532, 0, 198 },    { 73, 581, 3, 216 },    { 77, 655, 3, 240 },
        { 81, 733, 3, 280 },    { 85, 815, 3, 308 },    { 89, 901, 3, 338 },    { 93, 991, 3, 364 },
        { 97, 1085, 3, 416 },   { 101, 1156, 4, 476 },  { 105, 1258, 4, 690 },  { 109, 1364, 4, 504 },
        { 113, 1474, 4, 560 },  { 117, 1588, 4, 588 },  { 121, 1706, 4, 644 },  { 125, 1828, 4, 700 },
        { 129, 1921, 3, 728 },  { 133, 2051, 3, 784 },  { 137, 2185, 3, 812 },  { 141, 2323, 3, 868 },
        { 145, 2465, 3, 924 },  { 149, 2611, 3, 980 },  { 153, 2761, 3, 1036 }, { 157, 2876, 0, 1064 },
        { 161, 3034, 0, 1120 }, { 165, 3196, 0, 1204 }, { 169, 3362, 0, 1260 }, { 173, 3532, 0, 1316 },
        { 177, 3706, 0, 1372}
    }, {
        { 0, 0, 0, 0 },         { 21, 26, 0, 17 },      { 25, 44, 7, 28 },      { 29, 70, 7, 44 },
        { 33, 100, 7, 64 },     { 37, 134, 7, 88 },     { 41, 172, 7, 112 },    { 45, 196, 0, 130 },
        { 49, 242, 0, 156 },    { 53, 292, 0, 192 },    { 57, 346, 0, 224 },    { 61, 404, 0, 264 },
        { 65, 466, 0, 308 },    { 69, 532, 0, 352 },    { 73, 581, 3, 384 },    { 77, 655, 3, 432 },
        { 81, 733, 3, 480 },    { 85, 815, 3, 532 },    { 89, 901, 3, 588 },    { 93, 991, 3, 650 },
        { 97, 1085, 3, 700 },   { 101, 1156, 4, 750 },  { 105, 1258, 4, 816 },  { 109, 1364, 4, 900 },
        { 113, 1474, 4, 960 },  { 117, 1588, 4, 1050 }, { 121, 1706, 4, 1110 }, { 125, 1828, 4, 1200 },
        { 129, 1921, 3, 1260 }, { 133, 2051, 3, 1350 }, { 137, 2185, 3, 1440 }, { 141, 2323, 3, 1530 },
        { 145, 2465, 3, 1620 }, { 149, 2611, 3, 1710 }, { 153, 2761, 3, 1800 }, { 157, 2876, 0, 1890 },
        { 161, 3034, 0, 1980 }, { 165, 3196, 0, 2100 }, { 169, 3362, 0, 2220 }, { 173, 3532, 0, 2310 },
        { 177, 3706, 0, 2430 },
    }
};

int32_t QrcodeVersionGetDataLen(int32_t version)
{
    return g_qrcodeVersions[g_qrcodeEcc][version].words - g_qrcodeVersions[g_qrcodeEcc][version].ec;
}

int32_t QrcodeVersionGetEccLen(int32_t version)
{
    return g_qrcodeVersions[g_qrcodeEcc][version].ec;
}

int32_t QrcodeVersionGetWidth(int32_t version)
{
    return g_qrcodeVersions[g_qrcodeEcc][version].width;
}

int32_t QrcodeVersionGetRemainder(int32_t version)
{
    return g_qrcodeVersions[g_qrcodeEcc][version].remainder;
}

int32_t QrcodeVersionGetMinVersion(int32_t size)
{
    int32_t dataNumber = 0;
    for (int32_t i = 1; i <= QRCODE_VERSION_MAX; i++) {
        dataNumber = QrcodeVersionGetDataLen(i);
        if (dataNumber >= size) {
            return i;
        }
    }
    return -1;
}

static const int32_t g_lengthTableBits[4][3] = { { 10, 12, 14 }, { 9, 11, 13 }, { 8, 16, 16 }, { 8, 10, 12 } };

#define QR_VER_NUM9_INDEX 0
#define QR_VER_NUM26_INDEX 1
#define QR_VER_OTHER_INDEX 2

static bool QrcodeVersionModelengthSetI(QrcodeMode mode, int32_t version, int32_t *i)
{
    if (!QrcodeIsValidMode(mode)) {
        return false;
    }
    if (version <= QR_VER_NUM9) {
        *i = QR_VER_NUM9_INDEX;
    } else if (version <= QR_VER_NUM26) {
        *i = QR_VER_NUM26_INDEX;
    } else {
        *i = QR_VER_OTHER_INDEX;
    }

    return true;
}

int32_t QrcodeVersionModeLength(QrcodeMode mode, int32_t version)
{
    int32_t i = 0;

    if (!QrcodeVersionModelengthSetI(mode, version, &i)) {
        return 0;
    }

    return g_lengthTableBits[mode][i];
}

int32_t QrcodeVersionMaxWordsInMode(int32_t version, QrcodeMode mode)
{
    int32_t i = 0;
    if (!QrcodeVersionModelengthSetI(mode, version, &i)) {
        return 0;
    }
    int32_t bitCount = g_lengthTableBits[mode][i];
    int32_t byteCount = (1U << ((uint32_t)bitCount)) - 1;

    return byteCount;
}

#define QR_VER_ECC_TABLE_NUM 2
static const uint8_t g_eccTable[QRCODE_ECC_MAX][QRCODE_VERSION_MAX + 1][QR_VER_ECC_TABLE_NUM] = {
    {
        { 0, 0 },  { 1, 0 },   { 1, 0 },   { 1, 0 },   { 2, 0 },  { 2, 0 },   { 4, 0 },   { 4, 0 },
        { 2, 2 },  { 3, 2 },   { 4, 1 },   { 1, 4 },   { 6, 2 },  { 8, 1 },   { 4, 5 },   { 5, 5 },
        { 7, 3 },  { 10, 1 },  { 9, 4 },   { 3, 11 },  { 3, 13 }, { 17, 0 },  { 17, 0 },  { 4, 14 },
        { 6, 14 }, { 8, 13 },  { 19, 4 },  { 22, 3 },  { 3, 23 }, { 21, 7 },  { 19, 10 }, { 2, 29 },
        { 10, 23}, { 14, 21 }, { 14, 23 }, { 12, 26 }, { 6, 34 }, { 29, 14 }, { 13, 32 }, { 40, 7 },
        { 18, 31 },
    }, {
        { 0, 0 },   { 1, 0 },   { 1, 0 },  { 2, 0 },   { 4, 0 },   { 2, 2 },   { 4, 0 },   { 4, 1 },
        { 4, 2 },   { 4, 4 },   { 6, 2 },  { 3, 8 },   { 7, 4 },   { 12, 4},   { 11, 5 },  { 11, 7 },
        { 3, 13 },  { 2, 17 },  { 2, 19 }, { 9, 16 },  { 15, 10 }, { 19, 6},   { 34, 0 },  { 16, 14 },
        { 30, 2 },  { 22, 13 }, { 33, 4 }, { 12, 28 }, { 11, 31 }, { 19, 26 }, { 23, 25 }, { 23, 28 },
        { 19, 35 }, { 11, 46 }, { 59, 1 }, { 22, 41 }, { 2, 64 },  { 24, 46 }, { 42, 32 }, { 10, 67 },
        { 20, 61 },
    }
};

#define QR_VERSION_RS_INDEX_BLOCK_NUM1 0
#define QR_VERSION_RS_INDEX_DATA1 1
#define QR_VERSION_RS_INDEX_ECC 2
#define QR_VERSION_RS_INDEX_BLOCK_NUM2 3
#define QR_VERSION_RS_INDEX_DATA2 4
void QrcodeVersionGetEccInfo(int32_t version, int32_t spec[5], uint32_t length)
{
    (void)length;
    if ((version < 0) || (version > QRCODE_VERSION_MAX) || (g_qrcodeEcc >= QRCODE_ECC_MAX)) {
        return;
    }
    int32_t block1 = g_eccTable[g_qrcodeEcc][version][0];
    int32_t block2 = g_eccTable[g_qrcodeEcc][version][1];
    int32_t data = QrcodeVersionGetDataLen(version);
    int32_t ecc = QrcodeVersionGetEccLen(version);
    if (block2 == 0) {
        spec[QR_VERSION_RS_INDEX_BLOCK_NUM1] = block1;
        spec[QR_VERSION_RS_INDEX_DATA1] = data / block1;
        spec[QR_VERSION_RS_INDEX_ECC] = ecc / block1;
        spec[QR_VERSION_RS_INDEX_BLOCK_NUM2] = 0;
        spec[QR_VERSION_RS_INDEX_DATA2] = 0;
    } else {
        spec[QR_VERSION_RS_INDEX_BLOCK_NUM1] = block1;
        spec[QR_VERSION_RS_INDEX_DATA1] = data / (block1 + block2);
        spec[QR_VERSION_RS_INDEX_ECC] = ecc / (block1 + block2);
        spec[QR_VERSION_RS_INDEX_BLOCK_NUM2] = block2;
        spec[QR_VERSION_RS_INDEX_DATA2] = spec[QR_VERSION_RS_INDEX_DATA1] + 1;
    }
}

static inline uint8_t *QrPtr(void *d, int32_t w, int32_t x, int32_t y)
{
    return (uint8_t *)d + y * w + x;
}

static const int32_t g_alignmentPattern[QRCODE_VERSION_MAX + 1][2] = {
    { 0, 0 },   { 0, 0 },   { 18, 0 },  { 22, 0 },  { 26, 0 },  { 30, 0 },  { 34, 0 },  { 22, 38 },
    { 24, 42 }, { 26, 46 }, { 28, 50 }, { 30, 54 }, { 32, 58 }, { 34, 62 }, { 26, 46 }, { 26, 48 },
    { 26, 50 }, { 30, 54 }, { 30, 56 }, { 30, 58 }, { 34, 62 }, { 28, 50 }, { 26, 50 }, { 30, 54 },
    { 28, 54 }, { 32, 58 }, { 30, 58 }, { 34, 62 }, { 26, 50 }, { 30, 54 }, { 26, 52 }, { 30, 56 },
    { 34, 60 }, { 30, 58 }, { 34, 62 }, { 30, 54 }, { 24, 50 }, { 28, 54 }, { 32, 58 }, { 26, 54 },
    { 30, 58 },
};

#define QR_VERSION_CENTER_ALIGNMENT_MARKER_MAX 2
#define QR_VERSION_CENTER_ALIGNMENT_MARKER_SIX 6
static void DrawAlignmentMarker(uint8_t *data, int32_t dataWidth, int32_t centerX, int32_t centerY)
{
    static const uint8_t finder[] = {
        0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0xa0, 0xa0, 0xa0, 0xa1, 0xa1, 0xa0, 0xa1,
        0xa0, 0xa1, 0xa1, 0xa0, 0xa0, 0xa0, 0xa1, 0xa1, 0xa1, 0xa1, 0xa1, 0xa1,
    };

    if (centerX < QR_VERSION_CENTER_ALIGNMENT_MARKER_MAX || centerY < QR_VERSION_CENTER_ALIGNMENT_MARKER_MAX) {
        return;
    }

    for (int32_t i = 0; i < QR_NUM_IS_FIVE; i++) {
        if (memcpy_s(QrPtr(data, dataWidth, centerX - 2, centerY - 2 + i), QR_NUM_IS_FIVE, // 2：偏移量
            finder + i * QR_NUM_IS_FIVE, QR_NUM_IS_FIVE) != 0) {
            return;
        }
    }
}

#define QR_VER_NUM2_WIDTH 4
static void DrawAlignmentPattern(int32_t version, uint8_t *data, int32_t width)
{
    int32_t w = 0;
    int32_t x = 0;
    int32_t y = 0;
    int32_t cx = 0;
    int32_t cy = 0;
    if (version < QR_VER_NUM2) {
        return;
    }
    int32_t d = g_alignmentPattern[version][1] - g_alignmentPattern[version][0];
    if (d < 0) {
        w = QR_VER_NUM2;
    } else {
        w = (width - g_alignmentPattern[version][0]) / d + QR_VER_NUM2;
    }

    if (w * w == QR_VER_NUM2_WIDTH) {
        x = g_alignmentPattern[version][0];
        y = g_alignmentPattern[version][0];
        DrawAlignmentMarker(data, width, x, y);
        return;
    }

    cx = g_alignmentPattern[version][0];
    for (x = 1; x < w - 1; x++) {
        DrawAlignmentMarker(data, width, QR_VERSION_CENTER_ALIGNMENT_MARKER_SIX, cx);
        DrawAlignmentMarker(data, width, cx, QR_VERSION_CENTER_ALIGNMENT_MARKER_SIX);
        cx += d;
    }

    cy = g_alignmentPattern[version][0];
    for (y = 0; y < w - 1; y++) {
        cx = g_alignmentPattern[version][0];
        for (x = 0; x < w - 1; x++) {
            DrawAlignmentMarker(data, width, cx, cy);
            cx += d;
        }
        cy += d;
    }
}

static const uint32_t g_versionPattern[QRCODE_VERSION_MAX -6] = {
    0x07C94, 0x085BC, 0x09A99, 0x0A4D3, 0x0BBF6, 0x0C762, 0x0D847, 0x0E60D, 0x0F928, 0x10B78, 0x1145D, 0x12A17,
    0x13532, 0x149A6, 0x15683, 0x168C9, 0x177EC, 0x18EC4, 0x191E1, 0x1AFAB, 0x1B08E, 0x1CC1A, 0x1D33F, 0x1ED75,
    0x1F250, 0x209D5, 0x216F0, 0x228BA, 0x2379F, 0x24B0B, 0x2542E, 0x26A64, 0x27541, 0x28C69
};

uint32_t QrcodeVersionGetPattern(int32_t version)
{
    if ((version < QR_VER_NUM7) || (version > QRCODE_VERSION_MAX)) {
        return 0;
    }

    return g_versionPattern[version - QR_VER_NUM7];
}

#define QR_VER_FORMAT_INFO_MAX 8
static const int32_t g_formatInfo[QRCODE_ECC_MAX][QR_VER_FORMAT_INFO_MAX] = {
    { 0x5412, 0x5125, 0x5e7c, 0x5b4b, 0x45f9, 0x40ce, 0x4f97, 0x4aa0 },
    { 0x1689, 0x13be, 0x1ce7, 0x19d0, 0x0762, 0x0255, 0x0d0c, 0x083b }
};

int32_t QrcodeVersionGetFormatInfo(int32_t mask)
{
    if ((mask < 0) || (mask > QR_VER_NUM7)) {
        return 0;
    }

    return g_formatInfo[g_qrcodeEcc][mask];
}

static uint8_t *g_cache[QRCODE_VERSION_MAX + 1];

static void DrawFinderPattern(uint8_t *data, int32_t x, int32_t y, int32_t dataWidth)
{
    static const uint8_t finder[] = {
        QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_SEP,
        QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_SEP,
        QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_SEP,
        QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_SEP,
        QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_SEP,
        QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_SEP, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA, QR_VER_LINE_COLOR_FINDER_DATA,
        QR_VER_LINE_COLOR_FINDER_DATA,
    };

    for (int32_t i = 0; i < QR_VER_NUM7; i++) {
        if (QrPtr(data, dataWidth, x, y + i) == nullptr) {
            return;
        }
        if (memcpy_s(QrPtr(data, dataWidth, x, y + i), sizeof(uint8_t) * QR_VER_NUM7,
            finder + i * QR_VER_NUM7, sizeof(uint8_t) * QR_VER_NUM7) != 0) {
            return;
        }
    }
}

static QrcodeVersionDrawLine g_drawLine;
static void DrawHLine(uint32_t x, uint32_t y, uint32_t len, uint8_t COLOR1, uint8_t COLOR2)
{
    if (len == 0) {
        return;
    }
    if (COLOR1 == COLOR2) {
        if (memset_s(QrPtr(g_drawLine.data, g_drawLine.dataWidth, x, y), len, COLOR1, len) != 0) {
            return;
        }
    } else {
        for (uint32_t i = 0; i < len; i++) {
            if ((i % QR_VER_NUM2) == 0) {
                *QrPtr(g_drawLine.data, g_drawLine.dataWidth, x + i, y) = COLOR1;
            } else {
                *QrPtr(g_drawLine.data, g_drawLine.dataWidth, x + i, y) = COLOR2;
            }
        }
    }
}

static void DrawVLine(uint32_t x, uint32_t y, uint32_t len, uint8_t COLOR1, uint8_t COLOR2)
{
    if (len == 0) {
        return;
    }

    for (uint32_t i = 0; i < len; i++) {
        if ((i % QR_VER_NUM2) == 0) {
            *QrPtr(g_drawLine.data, g_drawLine.dataWidth, x, y + i) = COLOR1;
        } else {
            *QrPtr(g_drawLine.data, g_drawLine.dataWidth, x, y + i) = COLOR2;
        }
    }
}

static void CreateDataAddSeparators(uint8_t *data, int32_t width)
{
    if (data == nullptr) {
        return;
    }

    DrawFinderPattern(data, 0, 0, width);
    DrawFinderPattern(data, width - QR_VER_LINE_POS7, 0, width);
    DrawFinderPattern(data, 0, width - QR_VER_LINE_POS7, width);

    g_drawLine.data = data;
    g_drawLine.dataWidth = width;
    DrawVLine(QR_VER_LINE_POS7, 0, QR_VER_LINE_POS7, QR_VER_LINE_COLOR_SEP, QR_VER_LINE_COLOR_SEP);
    DrawHLine(0, QR_VER_LINE_POS7, QR_VER_LINE_POS8, QR_VER_LINE_COLOR_SEP, QR_VER_LINE_COLOR_SEP);
    DrawVLine(QR_VER_LINE_POS7, width - QR_VER_LINE_POS7, QR_VER_LINE_POS7, QR_VER_LINE_COLOR_SEP,
              QR_VER_LINE_COLOR_SEP);
    DrawHLine(0, width - QR_VER_LINE_POS8, QR_VER_LINE_POS8, QR_VER_LINE_COLOR_SEP, QR_VER_LINE_COLOR_SEP);
    DrawVLine(width - QR_VER_LINE_POS8, 0, QR_VER_LINE_POS7, QR_VER_LINE_COLOR_SEP, QR_VER_LINE_COLOR_SEP);
    DrawHLine(width - QR_VER_LINE_POS8, QR_VER_LINE_POS7, QR_VER_LINE_POS8, QR_VER_LINE_COLOR_SEP,
              QR_VER_LINE_COLOR_SEP);

    DrawHLine(0, QR_VER_LINE_POS8, QR_VER_LINE_POS9, QR_VER_LINE_COLOR_MASK, QR_VER_LINE_COLOR_MASK);
    DrawHLine(width - QR_VER_LINE_POS8, QR_VER_LINE_POS8, QR_VER_LINE_POS8, QR_VER_LINE_COLOR_MASK,
              QR_VER_LINE_COLOR_MASK);
    DrawVLine(QR_VER_LINE_POS8, 0, QR_VER_LINE_POS8, QR_VER_LINE_COLOR_MASK, QR_VER_LINE_COLOR_MASK);
    DrawVLine(QR_VER_LINE_POS8, width - QR_VER_LINE_POS7, QR_VER_LINE_POS7, QR_VER_LINE_COLOR_MASK,
              QR_VER_LINE_COLOR_MASK);

    DrawHLine(QR_VER_LINE_POS8, QR_VER_LINE_POS6, width - QR_VER_LINE_POS16, QR_VER_LINE_COLOR_TP1,
              QR_VER_LINE_COLOR_TP2);
    DrawVLine(QR_VER_LINE_POS6, QR_VER_LINE_POS8, width - QR_VER_LINE_POS16, QR_VER_LINE_COLOR_TP1,
              QR_VER_LINE_COLOR_TP2);
}

static uint8_t *CreateData(int32_t version)
{
    if (version > QRCODE_VERSION_MAX) {
        return nullptr;
    }

    int32_t width = g_qrcodeVersions[g_qrcodeEcc][version].width;
    uint8_t *data = (unsigned char *)QrcodeMalloc(width * width);
    if (data == nullptr) {
        return nullptr;
    }

    if (memset_s(data, width * width, 0, width * width) != 0) {
        QrcodeFree(data);
        return nullptr;
    }
    CreateDataAddSeparators(data, width);
    DrawAlignmentPattern(version, data, width);

    if ((version >= QR_VER_NUM7) && (version < QRCODE_VERSION_MAX)) {
        uint32_t verinfo = QrcodeVersionGetPattern(version);
        uint32_t v = verinfo;
        int32_t x = 0;
        int32_t y = 0;
        for (x = 0; x < SET_BIT_SIX; x++) {
            for (y = 0; y < SET_BIT_THREE; y++) {
                // 11：距右边界的列数，0x88: 固定暗模块的位置
                *QrPtr(data, width, x, width - 11 + y) = 0x88 | (v & 1);
                v = v >> 1;
            }
        }
        v = verinfo;
        for (y = 0; y < SET_BIT_SIX; y++) {
            for (x = 0; x < SET_BIT_THREE; x++) {
                // 11：距右边界的列数，0x88: 固定暗模块的位置
                *QrPtr(data, width, x + width - 11, y) = 0x88 | (v & 1);
                v = v >> 1;
            }
        }
    }
    *QrPtr(data, width, 8, width - 8) = 0x81; // 8：格式信息区的列偏移，0x81： 黑色块的位置

    return data;
}

bool QrcodeIsInvalidVersion(int32_t version)
{
    return (version < 1) || (version > QRCODE_VERSION_MAX);
}

void ClearCacheData(int32_t version)
{
    if (g_cache[version] == nullptr) {
        return;
    }
    QrcodeFree(g_cache[version]);
    g_cache[version] = nullptr;
}

uint8_t *QrcodeVersionNewData(int32_t version)
{
    if (QrcodeIsInvalidVersion(version)) {
        return nullptr;
    }
    ClearCacheData(version);

    g_cache[version] = CreateData(version);
    if (g_cache[version] == nullptr) {
        return nullptr;
    }
    int32_t width = g_qrcodeVersions[g_qrcodeEcc][version].width;
    uint8_t *data = (uint8_t *)QrcodeMalloc(width * width);
    if (data == nullptr) {
        return nullptr;
    }
    if (memcpy_s(data, width * width, g_cache[version], width * width) != 0) {
        QrcodeFree(data);
        return nullptr;
    }
    QrcodeVersionClearCache(version);

    return data;
}

void QrcodeVersionClearCache(int32_t version)
{
    if (QrcodeIsInvalidVersion(version)) {
        return;
    }

    ClearCacheData(version);
}

void QrcodeVersionSetEcc(QRCODE_ECC qrEcc)
{
    g_qrcodeEcc = (qrEcc >= QRCODE_ECC_MAX) ? QRCODE_ECC_HIGH : qrEcc;
}