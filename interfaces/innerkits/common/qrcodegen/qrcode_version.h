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

#ifndef __QRCODE_VERSION_H__
#define __QRCODE_VERSION_H__

#include "qrcode_generator.h"
#include "qrcode_inner.h"

#define QRCODE_VERSION_WIDTH_MAX 177

typedef enum {
    QR_VER_NUM2 = 2,
    QR_VER_NUM7 = 7,
    QR_VER_NUM9 = 9,
    QR_VER_NUM26 = 26,
    QR_VER_NUM40 = 40,
} QRCODE_VERSION_NUM;

typedef enum {
    QR_VER_LINE_POS6 = 6,
    QR_VER_LINE_POS7 = 7,
    QR_VER_LINE_POS8 = 8,
    QR_VER_LINE_POS9 = 9,
    QR_VER_LINE_POS16 = 16,
} QRCODE_VERSION_LINE_POS;

typedef enum {
    QR_VER_LINE_COLOR_SEP = 0xc0,
    QR_VER_LINE_COLOR_FINDER_DATA = 0xc1,
    QR_VER_LINE_COLOR_FINDER_SEP = 0xc2,
    QR_VER_LINE_COLOR_MASK = 0x84,
    QR_VER_LINE_COLOR_TP1 = 0x91,
    QR_VER_LINE_COLOR_TP2 = 0x90,
} QRCODE_VERSION_LINE_COLOR;

extern int32_t QrcodeVersionGetDataLen(int32_t version);

extern int32_t QrcodeVersionGetEccLen(int32_t version);

extern int32_t QrcodeVersionGetMinVersion(int32_t size);

extern int32_t QrcodeVersionGetWidth(int32_t version);

extern int32_t QrcodeVersionGetRemainder(int32_t version);

extern int32_t QrcodeVersionModeLength(QrcodeMode mode, int32_t version);

extern int32_t QrcodeVersionMaxWordsInMode(int32_t version, QrcodeMode mode);

void QrcodeVersionSetEcc(QRCODE_ECC qrEcc);

void QrcodeVersionGetEccInfo(int32_t version, int32_t spec[5], uint32_t length);

static inline int32_t QrcodeVersionRsBlockNum(const int32_t *spec)
{
    return spec[0] + spec[3]; // 3：第2组块数
}

static inline int32_t QrcodeVersionRsBlockNum1(const int32_t *spec)
{
    return spec[0];
}

static inline int32_t QrcodeVersionRsData1(const int32_t *spec)
{
    return spec[1]; // 1：第1组每块数据码字数
}

static inline int32_t QrcodeVersionRsEcc1(const int32_t *spec)
{
    return spec[2]; // 2：第1组每块ECC码字数
}

static inline int32_t QrcodeVersionRsBlockNum2(const int32_t *spec)
{
    return spec[3]; // 3：第2组RS块数
}

static inline int32_t QrcodeVersionRsData2(const int32_t *spec)
{
    return spec[4]; // 4：第2组每块数据码字数
}

static inline int32_t QrcodeVersionRsEcc2(const int32_t *spec)
{
    return spec[2]; // 2：第2组每块ECC码字数
}

static inline int32_t QrcodeVersionRsDataLen(const int32_t *spec)
{
    return (QrcodeVersionRsBlockNum1(spec) * QrcodeVersionRsData1(spec)) +
        (QrcodeVersionRsBlockNum2(spec) * QrcodeVersionRsData2(spec));
}

static inline int32_t QrcodeVersionRsEccLen(const int32_t *spec)
{
    return QrcodeVersionRsBlockNum(spec) * QrcodeVersionRsEcc1(spec);
}

extern uint32_t QrcodeVersionGetPattern(int32_t version);

extern int32_t QrcodeVersionGetFormatInfo(int32_t mask);

extern uint8_t *QrcodeVersionNewData(int32_t version);

extern void QrcodeVersionClearCache(int32_t version);

#define QRCODE_VERSION_MODEID_NUM 1
#define QRCODE_VERSION_MODEID_AN 2
#define QRCODE_VERSION_MODEID_8 4

#endif /* __QRCODE_VERSION_H__ */
