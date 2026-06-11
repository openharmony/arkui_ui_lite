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

#include "common/qrcodegen/qrcode_generator.h"
#include <cstdlib>
#include "common/qrcodegen/qrcode_version.h"
#include "common/qrcodegen/qrcode_stream.h"
#include "common/qrcodegen/qrcode_item.h"
#include "common/qrcodegen/qrcode_rscode.h"
#include "common/qrcodegen/qrcode_inner.h"
#include "common/qrcodegen/qrcode_mask.h"
#include "securec.h"

#define MASK_SUM_NUM 8
#define BIT_SUM_NUM 8

typedef struct {
    int32_t width;
    uint8_t *data;
    int32_t x;
    int32_t y;
    int32_t deltaY;
    int32_t rightToLeft;
} QrcodeDirector;

typedef struct {
    int32_t width;
    int32_t version;
    QrcodeMergeCode *merge;
    uint8_t *data;
    uint8_t *masked;
    uint8_t *p;
    uint8_t code;
    uint8_t bit;
    QrcodeDirector *director;
    QrcodeImage *qrimg;
} QrcodeImageEncodeMaskData;

static QrcodeMemHooks g_memoryHooks = { malloc, free };
static bool g_qrcodeMemHooksInitialized = false;

void QrcodeMemInitHooks(const QrcodeMemHooks *hooks)
{
    if (g_qrcodeMemHooksInitialized) {
        return;
    }

    if ((hooks == nullptr) || (hooks->mallocFunc == nullptr) || (hooks->freeFunc == nullptr)) {
        return;
    }

    g_memoryHooks.mallocFunc = hooks->mallocFunc;
    g_memoryHooks.freeFunc = hooks->freeFunc;

    g_qrcodeMemHooksInitialized = true;
}

void *QrcodeMalloc(size_t size)
{
    return g_memoryHooks.mallocFunc(size);
}

void QrcodeFree(void *ptr)
{
    g_memoryHooks.freeFunc(ptr);
}

static void QrcodeBlockBlockInit(QrcodeBlock *block, uint8_t *data, int32_t dataLen, uint8_t *ecc, ReedSolomonCode *rs)
{
    if (block == nullptr) {
        return;
    }

    block->data = data;
    block->dataLen = dataLen;
    block->ecc = ecc;
    ReedSolomonCodeEncode(rs, data, ecc);
}

static int32_t QrcodeBlockInit(QrcodeBlock *blocks, int32_t spec[5], uint32_t length, uint8_t *data, uint8_t *ecc)
{
    if (length == 0) {
        return -1;
    }
    int32_t dataLen = QrcodeVersionRsData1(spec);
    int32_t eccLen = QrcodeVersionRsEcc1(spec);

    ReedSolomonCodeData codeData1 = { MASK_SUM_NUM, 0x11d, 0, 1, eccLen, 255 - dataLen - eccLen };
    ReedSolomonCode *rs = ReedSolomonCodeInit(codeData1);
    if (rs == nullptr) {
        return -1;
    }
    QrcodeBlock *block = blocks;
    uint8_t *dataPtr = data;
    uint8_t *eccPtr = ecc;
    for (int32_t i = 0; i < QrcodeVersionRsBlockNum1(spec); i++) {
        block->eccLen = eccLen;
        QrcodeBlockBlockInit(block, dataPtr, dataLen, eccPtr, rs);
        dataPtr += dataLen;
        eccPtr += eccLen;
        block++;
    }

    ReedSolomonCodeFreeCache();
    if (QrcodeVersionRsBlockNum2(spec) == 0) {
        return 0;
    }

    dataLen = QrcodeVersionRsData2(spec);
    eccLen = QrcodeVersionRsEcc2(spec);

    ReedSolomonCodeData codeData2 = { MASK_SUM_NUM, 0x11d, 0, 1, eccLen, 255 - dataLen - eccLen };
    rs = ReedSolomonCodeInit(codeData2);
    if (rs == nullptr) {
        return -1;
    }
    for (int32_t i = 0; i < QrcodeVersionRsBlockNum2(spec); i++) {
        block->eccLen = eccLen;
        QrcodeBlockBlockInit(block, dataPtr, dataLen, eccPtr, rs);
        dataPtr += dataLen;
        eccPtr += eccLen;
        block++;
    }

    ReedSolomonCodeFreeCache();

    return 0;
}

static void QrcodeMergeCodeFree(QrcodeMergeCode *merge);

static QrcodeMergeCode *QRMergeCodeNew(QrcodeItemList *list)
{
    if (list == nullptr) {
        return nullptr;
    }

    QrcodeMergeCode *merge = (QrcodeMergeCode *)QrcodeMalloc(sizeof(QrcodeMergeCode));
    if (merge == nullptr) {
        return nullptr;
    }
    merge->data = QrcodeItemListGetByteStream(list);
    if (merge->data == nullptr) {
        QrcodeFree(merge);
        return nullptr;
    }

    int32_t spec[5] = { 0 };
    QrcodeVersionGetEccInfo(list->version, spec, QR_BIT_FIVE);
    merge->version = list->version;
    merge->b1 = QrcodeVersionRsBlockNum1(spec);
    merge->dataLen = QrcodeVersionRsDataLen(spec);
    merge->eccLen = QrcodeVersionRsEccLen(spec);
    merge->ecc = (uint8_t *)QrcodeMalloc(merge->eccLen);
    if (merge->ecc == nullptr) {
        QrcodeFree(merge->data);
        QrcodeFree(merge);
        return nullptr;
    }

    merge->blocks = QrcodeVersionRsBlockNum(spec);
    merge->qrBlock = (QrcodeBlock *)QrcodeMalloc(merge->blocks * sizeof(QrcodeBlock));
    if (merge->qrBlock == nullptr) {
        QrcodeMergeCodeFree(merge);
        return nullptr;
    }

    if (memset_s(merge->qrBlock, merge->blocks * sizeof(QrcodeBlock), 0,
        merge->blocks * sizeof(QrcodeBlock)) != 0) {
        QrcodeMergeCodeFree(merge);
        return nullptr;
    }
    int32_t ret = QrcodeBlockInit(merge->qrBlock, spec, QR_BIT_FIVE, merge->data, merge->ecc);
    if (ret < 0) {
        QrcodeMergeCodeFree(merge);
        return nullptr;
    }
    merge->count = 0;

    return merge;
}

static uint8_t QrcodeMergeCodeGet(QrcodeMergeCode *merge)
{
    if (merge == nullptr) {
        return 0;
    }
    int32_t col = 0;
    int32_t row = 0;
    uint8_t ret = 0;
    if (merge->count < merge->dataLen) {
        row = merge->count % merge->blocks;
        col = merge->count / merge->blocks;
        if (col >= merge->qrBlock[0].dataLen) {
            row += merge->b1;
        }
        ret = merge->qrBlock[row].data[col];
    } else if (merge->count < merge->dataLen + merge->eccLen) {
        row = (merge->count - merge->dataLen) % merge->blocks;
        col = (merge->count - merge->dataLen) / merge->blocks;
        ret = merge->qrBlock[row].ecc[col];
    } else {
        return 0;
    }
    merge->count++;

    return ret;
}

static void QrcodeMergeCodeFree(QrcodeMergeCode *merge)
{
    if (merge) {
        QrcodeFree(merge->data);
        QrcodeFree(merge->ecc);
        QrcodeFree(merge->qrBlock);
        QrcodeFree(merge);
    }
}

static QrcodeDirector *QrcodeDirectorNew(int32_t width, uint8_t *data)
{
    QrcodeDirector *director = nullptr;
    director = (QrcodeDirector *)QrcodeMalloc(sizeof(QrcodeDirector));
    if (director == nullptr) {
        return nullptr;
    }
    director->width = width;
    director->data = data;
    director->x = width - 1;
    director->y = width - 1;
    director->deltaY = -1;
    director->rightToLeft = -1;

    return director;
}

static void QrcodeDirectorNextPosInit(QrcodeDirector *director, uint8_t **p, int32_t *x, int32_t *y, int32_t *width)
{
    if ((director == nullptr) || (p == nullptr) || (x == nullptr) || (width == nullptr)) {
        return;
    }
    *x = director->x;
    *y = director->y;
    *p = director->data;
    *width = director->width;
}

static void UpdatePositionBasedOnDirection(QrcodeDirector *director, int32_t *x, int32_t *y)
{
    if (director->rightToLeft == 1) {
        (*x)--;
        director->rightToLeft = 0;
    } else {
        (*x)++;
        *y += director->deltaY;
        director->rightToLeft = 1;
    }
}

static uint8_t *QrcodeDirectorNextPos(QrcodeDirector *director)
{
    if (director == nullptr) {
        return nullptr;
    }
    uint8_t *p = nullptr;
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;

    if (director->rightToLeft == -1) {
        director->rightToLeft = 1;
        return director->data + director->y * director->width + director->x;
    }

    QrcodeDirectorNextPosInit(director, &p, &x, &y, &width);
    UpdatePositionBasedOnDirection(director, &x, &y);
    if (director->deltaY < 0) {
        if (y < 0) {
            y = 0;
            x -= SET_BIT_TWO;
            director->deltaY = 1;
            if (x == SET_BIT_SIX) {
                x--;
                y = SET_BIT_NINE;
            }
        }
    } else {
        if (y == width) {
            y = width - 1;
            x -= SET_BIT_TWO;
            director->deltaY = -1;
            if (x == SET_BIT_SIX) {
                x--;
                y -= SET_BIT_EIGHT;
            }
        }
    }
    if (x < 0 || y < 0) {
        return nullptr;
    }
    director->x = x;
    director->y = y;
    if (p[y * width + x] & 0x80) {
        return QrcodeDirectorNextPos(director);
    }

    return &p[y * width + x];
}

static QrcodeImage *QrcodeImageNew(int32_t version, int32_t width, uint8_t *data)
{
    QrcodeImage *qrimg = (QrcodeImage *)QrcodeMalloc(sizeof(QrcodeImage));
    if (qrimg == nullptr) {
        return nullptr;
    }
    qrimg->version = version;
    qrimg->width = width;
    qrimg->data = data;

    return qrimg;
}

void QrcodeImageFree(QrcodeImage *qrimage)
{
    if (qrimage) {
        QrcodeFree(qrimage->data);
        QrcodeFree(qrimage);
    }
}

static bool QrcodeImageEncodeMaskInit(QrcodeItemList *list, QrcodeImageEncodeMaskData *encodeMaskdata)
{
    if ((list == nullptr) || (encodeMaskdata == nullptr)) {
        return false;
    }
    *encodeMaskdata = { 0, 0, nullptr, nullptr, nullptr, nullptr, 0, 0, nullptr, nullptr };

    if ((list->version < 0) || (list->version > QRCODE_VERSION_MAX)) {
        return false;
    }
    encodeMaskdata->merge = QRMergeCodeNew(list);
    if (encodeMaskdata->merge == nullptr) {
        return false;
    }

    encodeMaskdata->version = encodeMaskdata->merge->version;
    encodeMaskdata->width = QrcodeVersionGetWidth(encodeMaskdata->version);
    encodeMaskdata->data = QrcodeVersionNewData(encodeMaskdata->version);
    if (encodeMaskdata->data == nullptr) {
        QrcodeMergeCodeFree(encodeMaskdata->merge);
        return false;
    }

    return true;
}

QrcodeImage *QrcodeImageEncodeMask(QrcodeItemList *list)
{
    QrcodeImageEncodeMaskData encodeMaskdata;
    int32_t i = 0;
    int32_t j = 0;
    if (!QrcodeImageEncodeMaskInit(list, &encodeMaskdata)) {
        return nullptr;
    }

    encodeMaskdata.director = QrcodeDirectorNew(encodeMaskdata.width, encodeMaskdata.data);
    if (encodeMaskdata.director == nullptr) {
        QrcodeMergeCodeFree(encodeMaskdata.merge);
        QrcodeFree(encodeMaskdata.data);
        return nullptr;
    }

    for (i = 0; i < encodeMaskdata.merge->dataLen + encodeMaskdata.merge->eccLen; i++) {
        encodeMaskdata.code = QrcodeMergeCodeGet(encodeMaskdata.merge);
        encodeMaskdata.bit = 0x80;
        for (j = 0; j < BIT_SUM_NUM; j++) {
            encodeMaskdata.p = QrcodeDirectorNextPos(encodeMaskdata.director);
            if (encodeMaskdata.p == nullptr) {
                goto EXIT;
            }
            *(encodeMaskdata.p) = 0x02 | (((encodeMaskdata.bit & encodeMaskdata.code) != 0) ? 1U : 0U);
            encodeMaskdata.bit = encodeMaskdata.bit >> 1;
        }
    }
    QrcodeMergeCodeFree(encodeMaskdata.merge);
    encodeMaskdata.merge = nullptr;

    j = QrcodeVersionGetRemainder(encodeMaskdata.version);
    for (i = 0; i < j; i++) {
        encodeMaskdata.p = QrcodeDirectorNextPos(encodeMaskdata.director);
        if (encodeMaskdata.p == nullptr) {
            goto EXIT;
        }
        *(encodeMaskdata.p) = 0x02;
    }

    encodeMaskdata.masked = QrcodeMaskFindMask(encodeMaskdata.width, encodeMaskdata.data);
    if (encodeMaskdata.masked == nullptr) {
        goto EXIT;
    }
    encodeMaskdata.qrimg = QrcodeImageNew(encodeMaskdata.version, encodeMaskdata.width, encodeMaskdata.masked);
    if (encodeMaskdata.qrimg == nullptr) {
        QrcodeFree(encodeMaskdata.masked);
    }

EXIT:
    QrcodeMergeCodeFree(encodeMaskdata.merge);
    QrcodeFree(encodeMaskdata.director);
    QrcodeFree(encodeMaskdata.data);

    return encodeMaskdata.qrimg;
}

QrcodeImage *QrcodeImageEncodeString(const char *text, QRCODE_ECC qrEcc)
{
    if (text == nullptr) {
        return nullptr;
    }

    QrcodeVersionSetEcc(qrEcc);
    QrcodeItemList *list = QrcodeItemListNew();
    if (list == nullptr) {
        return nullptr;
    }

    if (QrcodeStrToItemList(text, list) < 0) {
        QrcodeItemListFree(list);
        return nullptr;
    }
    QrcodeImage *qrImg = QrcodeImageEncodeMask(list);
    QrcodeVersionClearCache(list->version);
    QrcodeItemListFree(list);
    QrcodeFree(list);

    return qrImg;
}