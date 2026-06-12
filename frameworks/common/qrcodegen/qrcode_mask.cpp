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

#include "common/qrcodegen/qrcode_mask.h"
#include "securec.h"
#include <cmath>
#include <cstdlib>
#include "common/qrcodegen/qrcode_generator.h"
#include "common/qrcodegen/qrcode_version.h"
#include "common/qrcodegen/qrcode_item.h"

#define QR_MASK_NUM (8)

#define N1 (3)
#define N2 (3)
#define N3 (40)
#define N4 (10)

typedef int32_t (*QrcodeMaskExpressionFunc)(int32_t x, int32_t y);

static int32_t QrcodeMaskCopyWithExpression(int32_t width, uint8_t *src, uint8_t *dest, QrcodeMaskExpressionFunc exp)
{
    int32_t blackCount = 0;
    if ((src == nullptr) || (dest == nullptr)) {
        return blackCount;
    }
    for (int32_t y = 0; y < width; y++) {
        for (int32_t x = 0; x < width; x++) {
            if (*src & 0x80) {
                *dest = *src;
            } else {
                *dest = *src ^ ((exp(x, y) == 0) ? 1U : 0U);
            }
            blackCount += (int)(*dest & 1);
            src++;
            dest++;
        }
    }

    return blackCount;
}

static int32_t QrcodeMaskFormatInformation(int32_t width, uint8_t *data, int32_t mask)
{
    uint32_t i = 0;
    int32_t blackCount = 0;
    uint8_t value = 0;
    if (data == nullptr) {
        return blackCount;
    }
    uint32_t format = QrcodeVersionGetFormatInfo(mask);
    for (i = 0; i < QR_BIT_EIGHT; i++) {
        if (format & 1) {
            blackCount += QR_BIT_TWO;
            value = 0x85;
        } else {
            value = 0x84;
        }
        data[width * QR_BIT_EIGHT + width - 1 - i] = value;
        if (i < QR_BIT_SIX) {
            data[width * i + SET_BIT_EIGHT] = value;
        } else {
            data[width * (i + 1) + SET_BIT_EIGHT] = value;
        }
        format = format >> 1;
    }
    for (i = 0; i < QR_BIT_SEVEN; i++) {
        if (format & 1) {
            blackCount += QR_BIT_TWO;
            value = 0x85;
        } else {
            value = 0x84;
        }
        data[width * (width - QR_BIT_SEVEN + i) + QR_BIT_EIGHT] = value;
        if (i == 0) {
            data[width * SET_BIT_EIGHT + SET_BIT_SEVEN] = value;
        } else {
            data[width * SET_BIT_EIGHT + SET_BIT_SIX - i] = value;
        }
        format = format >> 1;
    }

    return blackCount;
}

static int32_t QrcodeMaskExpression0(int32_t x, int32_t y)
{
    return ((uint32_t)(x + y)) & 1;
}

static int32_t QrcodeMaskExpression1(int32_t x, int32_t y)
{
    return ((uint32_t)y) & 1;
}

static int32_t QrcodeMaskExpression2(int32_t x, int32_t y)
{
    return x % QR_BIT_THREE;
}

static int32_t QrcodeMaskExpression3(int32_t x, int32_t y)
{
    return (((uint32_t)x) + ((uint32_t)y)) % QR_BIT_THREE;
}

static int32_t QrcodeMaskExpression4(int32_t x, int32_t y)
{
    return ((((uint32_t)y) / QR_BIT_TWO) + (((uint32_t)x) / QR_BIT_THREE)) & 1U;
}

static int32_t QrcodeMaskExpression5(int32_t x, int32_t y)
{
    return ((((uint32_t)x) * ((uint32_t)y)) & 1U) + (((uint32_t)x) * ((uint32_t)y)) % QR_BIT_THREE;
}

static int32_t QrcodeMaskExpression6(int32_t x, int32_t y)
{
    return (((((uint32_t)x) * ((uint32_t)y)) & 1U) + (((uint32_t)x) * ((uint32_t)y)) % QR_BIT_THREE) & 1U;
}

static int32_t QrcodeMaskExpression7(int32_t x, int32_t y)
{
    return (((((uint32_t)x) * ((uint32_t)y)) % QR_BIT_THREE) + ((((uint32_t)x) + ((uint32_t)y)) & 1U)) & 1U;
}

static QrcodeMaskExpressionFunc g_qrcodeMaskMakers[QR_MASK_NUM] = {
    QrcodeMaskExpression0, QrcodeMaskExpression1, QrcodeMaskExpression2, QrcodeMaskExpression3,
    QrcodeMaskExpression4, QrcodeMaskExpression5, QrcodeMaskExpression6, QrcodeMaskExpression7
};

static int32_t QrcodeMaskGetRoleCalcN1N3(uint8_t *buf, int32_t bufLen)
{
    int32_t i = 0;
    int32_t score = 0;
    int32_t continueCount = 1;
    int32_t flag = 0;
    const uint8_t pattenr1[] = { 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0 };
    const uint8_t pattenr2[] = { 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1 };

    if (buf == nullptr) {
        return -1;
    }

    for (i = 1; i < bufLen; i++) {
        if (buf[i - 1] == buf[i]) {
            continueCount++;
        } else {
            if (continueCount >= QR_BIT_FIVE) {
                score += continueCount + N1 - QR_BIT_FIVE;
            }

            continueCount = 1;
        }
    }

    if (continueCount >= QR_BIT_FIVE) {
        score += continueCount + N1 - QR_BIT_FIVE;
    }

    for (i = 0, flag = 0; i < bufLen; ++i) {
        if (pattenr1[flag] == buf[i]) {
            if (sizeof(pattenr1) == flag + 1) {
                score += N3;
            } else {
                flag++;
            }
        } else {
            flag = 0;
        }
    }

    for (i = 0, flag = 0; i < bufLen; ++i) {
        if (pattenr2[flag] == buf[i]) {
            if (sizeof(pattenr2) == (flag + 1)) {
                score += N3;
            } else {
                flag++;
            }
        } else {
            flag = 0;
        }
    }

    return score;
}

static uint8_t QrcodeMaskGetColor(uint8_t *data, int32_t width, int32_t x, int32_t y)
{
    if (data == nullptr) {
        return 0;
    }

    uint8_t c = data[x + y * width];
    if (c & 1) {
        return 1;
    }

    return 0;
}

static int32_t QrcodeMaskGetRoleCalcN2(int32_t width, uint8_t *data)
{
    int32_t score = 0;
    uint8_t *p = data;

    for (int32_t y = 0; y < width - 1; y++) {
        for (int32_t x = 0; x < width - 1; x++) {
            uint8_t color11 = QrcodeMaskGetColor(p, width, x, y);
            uint8_t color12 = QrcodeMaskGetColor(p, width, x + 1, y);
            uint8_t color21 = QrcodeMaskGetColor(p, width, x, y + 1);
            uint8_t color22 = QrcodeMaskGetColor(p, width, x + 1, y + 1);
            if (color11 == color12 && color12 == color21 && color21 == color22) {
                score += N2;
            }
        }
    }

    return score;
}

static int32_t QrcodeMaskGetBuffer(uint8_t *buff, uint8_t *src, int32_t width, int32_t isCol)
{
    int32_t moveLen = (isCol == 0) ? 1 : width;

    if (memset_s(buff, width, 0, width) != 0) {
        return 0;
    }
    uint8_t *p = src;
    if ((p == nullptr) || (buff == nullptr)) {
        return 0;
    }
    for (int32_t i = 0; i < width; i++) {
        if (p[i * moveLen] & 1) {
            buff[i] = 1;
        } else {
            buff[i] = 0;
        }
    }

    return width;
}

static int32_t QrcodeMaskGetScore(int32_t width, uint8_t *data)
{
    int32_t x = 0;
    int32_t y = 0;
    int32_t score = 0;
    uint8_t buffer[QRCODE_VERSION_WIDTH_MAX + 1] = { 0 };

    if (data == nullptr) {
        return 0;
    }

    score += QrcodeMaskGetRoleCalcN2(width, data);

    for (y = 0; y < width; y++) {
        if (memset_s(buffer, sizeof(buffer), 0, sizeof(buffer)) != 0) {
            return 0;
        }
        QrcodeMaskGetBuffer(buffer, data + y * width, width, 0);
        score += QrcodeMaskGetRoleCalcN1N3(buffer, width);
    }

    for (x = 0; x < width; x++) {
        if (memset_s(buffer, sizeof(buffer), 0, sizeof(buffer)) != 0) {
            return 0;
        }
        QrcodeMaskGetBuffer(buffer, data + x, width, 1);
        int32_t bufLen = (width > (QRCODE_VERSION_WIDTH_MAX + 1)) ? (QRCODE_VERSION_WIDTH_MAX + 1) : width;
        score += QrcodeMaskGetRoleCalcN1N3(buffer, bufLen);
    }

    return score;
}

uint8_t *QrcodeMaskFindMask(int32_t width, uint8_t *data)
{
    int32_t total = width * width;
    uint8_t *curMask = (uint8_t *)QrcodeMalloc(total);
    if (curMask == nullptr) {
        return nullptr;
    }
    uint8_t *bestMask = nullptr;
    int32_t minScore = INT_MAX;
    for (int32_t i = 0; i < QR_MASK_NUM; i++) {
        int32_t score = 0;
        int32_t blackCounts = QrcodeMaskCopyWithExpression(width, data, curMask, g_qrcodeMaskMakers[i]);
        blackCounts += QrcodeMaskFormatInformation(width, curMask, i);
        // 200: 百分比计算的中间系数
        int32_t percent = (200 * blackCounts + total) / total / 2;
        // 50: 目标黑白比例,5: 容差档次
        score = (fabs(percent - 50) / 5) * N4;
        score += QrcodeMaskGetScore(width, curMask);
        if (score < minScore) {
            minScore = score;
            if (bestMask != nullptr) {
                QrcodeFree(bestMask);
            }
            bestMask = curMask;
            curMask = (unsigned char *)QrcodeMalloc(total);
            if (curMask == nullptr) {
                break;
            }
        }
    }
    if (curMask != nullptr) {
        QrcodeFree(curMask);
    }

    return bestMask;
}
