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

#include "common/qrcodegen/qrcode_item.h"
#include "common/qrcodegen/qrcode_generator.h"
#include "common/qrcodegen/qrcode_stream.h"
#include "common/qrcodegen/qrcode_version.h"
#include "common/qrcodegen/qrcode_list.h"

#define THIRD_ENCODE 3
#define THIRD_ENCODE_TWO 2
#define THIRD_ENCODE_TWO_A 45

bool QrcodeIsValidMode(QrcodeMode mode)
{
    return ((mode >= QRCODE_MODE_NUM) && (mode <= QRCODE_MODE_8));
}

static QrcodeItem *QrcodeItemNew(QrcodeMode mode, int32_t size, const uint8_t *data)
{
    if (QrcodeItemCheck(mode, size, data)) {
        return nullptr;
    }

    QrcodeItem *item = (QrcodeItem *)QrcodeMalloc(sizeof(QrcodeItem));
    if (item == nullptr) {
        return nullptr;
    }
    if (memset_s(item, sizeof(QrcodeItem), 0, sizeof(QrcodeItem)) != 0) {
        QrcodeFree(item);
        return nullptr;
    }
    item->mode = mode;
    item->size = size;
    if (size > 0) {
        item->data = (uint8_t *)QrcodeMalloc(size);
        if (item->data == nullptr) {
            QrcodeFree(item);
            return nullptr;
        }
        if (memcpy_s(item->data, size, data, size) != 0) {
            QrcodeFree(item->data);
            QrcodeFree(item);
            return nullptr;
        }
    }

    return item;
}

static void QrcodeItemFree(QrcodeItem *item)
{
    if (item) {
        QrcodeFree(item->data);
        QrcodeFree(item);
    }
}

QrcodeItemList *QrcodeItemListNew()
{
    QrcodeItemList *list = (QrcodeItemList *)QrcodeMalloc(sizeof(QrcodeItemList));
    if (list == nullptr) {
        return nullptr;
    }
    QrcodeListInit(&list->list);
    list->version = 0;

    return list;
}

int32_t QrcodeItemListAdd(QrcodeItemList *list, QrcodeMode mode, int32_t size, const uint8_t *data)
{
    QrcodeItem *item = nullptr;
    if (list == nullptr) {
        return -1;
    }
    item = QrcodeItemNew(mode, size, data);
    if (item == nullptr) {
        return -1;
    }
    QrCodeListTailInsert(&list->list, &item->next);

    return 0;
}

void QrcodeItemListFree(QrcodeItemList *list)
{
    QrcodeItem *pos = nullptr;
    QrcodeItem *next = nullptr;
    if ((list == nullptr) || QrCodeListEmpty(&list->list)) {
        return;
    }
    QrcodeDlListIter iter;
    QrcodeDlListIterInitTyped(&iter, &list->list, &((QrcodeItem *)0)->next);
    while (QrcodeDlListIterHasNext(&iter)) {
        pos = (QrcodeItem *)QrcodeDlListIterNextSafe(&iter);
        QrcodeItemFree(pos);
    }
}

#define QT_VAL_HUNDRED 100
#define QT_VAL_TEN 10
static int32_t QrcodeItemEncodeNum(QrcodeStream *stream, QrcodeItem *item, int32_t version)
{
    int32_t ret = QrcodeStreamAddNum(stream, 4, QRCODE_VERSION_MODEID_NUM);
    if (ret < 0) {
        return -1;
    }
    if (item == nullptr) {
        return -1;
    }
    ret = QrcodeStreamAddNum(stream, QrcodeVersionModeLength(QRCODE_MODE_NUM, version), item->size);
    if (ret < 0) {
        return -1;
    }
    uint32_t val = 0;
    int32_t words = item->size / THIRD_ENCODE;
    for (int32_t i = 0; i < words; i++) {
        val = (item->data[i * THIRD_ENCODE] - '0') * QT_VAL_HUNDRED;
        val += (item->data[i * THIRD_ENCODE + SET_BIT_ONE] - '0') * QT_VAL_TEN;
        val += (item->data[i * THIRD_ENCODE + SET_BIT_TWO] - '0');
        ret = QrcodeStreamAddNum(stream, SET_BIT_TEN, val);
        if (ret < 0) {
            return -1;
        }
    }
    if ((item->size - words * THIRD_ENCODE) == QR_BIT_ONE) {
        val = item->data[words * THIRD_ENCODE] - '0';
        ret = QrcodeStreamAddNum(stream, SET_BIT_FOUR, val);
        if (ret < 0) {
            return -1;
        }
    } else if ((item->size - words * THIRD_ENCODE) == QR_BIT_TWO) {
        val = (item->data[words * THIRD_ENCODE] - '0') * QT_VAL_TEN;
        val += (item->data[words * THIRD_ENCODE + SET_BIT_ONE] - '0');
        ret = QrcodeStreamAddNum(stream, SET_BIT_SEVEN, val);
        if (ret < 0) {
            return -1;
        }
    }

    return 0;
}

const int8_t QRCODE_ALPHA_BET_TABLE[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, 36, -1, -1, -1, 37, 38, -1, -1, -1, -1, 39, 40, -1, 41, 42, 43,  0,  1,  2,  3,
    4,   5,  6,  7,  8,  9, 44, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int32_t QrcodeEstimateNum(int32_t size)
{
    int32_t width = size / THIRD_ENCODE;
    int32_t bitCount = width * SET_BIT_TEN;
    switch (size - width * THIRD_ENCODE) {
        case QR_BIT_ONE:
            bitCount += SET_BIT_FOUR;
            break;
        case QR_BIT_TWO:
            bitCount += SET_BIT_SEVEN;
            break;
        default:
            break;
    }

    return bitCount;
}

int32_t QrcodeEstimateAlphaBet(int32_t size)
{
    int32_t width = size / QR_BIT_TWO;
    int32_t bitCount = width * (SET_BIT_SEVEN + SET_BIT_FOUR);
    if (((uint32_t)size) & 1) {
        bitCount += 6; // 6:字母数字模式单字符编码长度
    }

    return bitCount;
}

static int32_t QRItemEncodeAlphabet(QrcodeStream *stream, QrcodeItem *item, int32_t version)
{
    if ((stream == nullptr) || (item == nullptr)) {
        return -1;
    }
    int32_t ret = QrcodeStreamAddNum(stream, 4, QRCODE_VERSION_MODEID_AN);
    if (ret < 0) {
        return -1;
    }
    ret = QrcodeStreamAddNum(stream, QrcodeVersionModeLength(QRCODE_MODE_AN, version), item->size);
    if (ret < 0) {
        return -1;
    }
    int32_t words = item->size / 2;
    uint32_t val = 0;
    for (int32_t i = 0; i < words; i++) {
        val = (uint32_t)QrcodeLookAtTable(item->data[i * THIRD_ENCODE_TWO]) * THIRD_ENCODE_TWO_A;
        val += (uint32_t)QrcodeLookAtTable(item->data[i * THIRD_ENCODE_TWO + 1]);
        ret = QrcodeStreamAddNum(stream, (SET_BIT_SEVEN + SET_BIT_FOUR), val);
        if (ret < 0) {
            return -1;
        }
    }
    if (((uint32_t)item->size) & 1) {
        val = (uint32_t)QrcodeLookAtTable(item->data[words * THIRD_ENCODE_TWO]);
        ret = QrcodeStreamAddNum(stream, SET_BIT_SIX, val);
        if (ret < 0) {
            return -1;
        }
    }

    return 0;
}

int32_t QrcodeEstimate8(int32_t size)
{
    return size * QR_BIT_EIGHT;
}

static int32_t QRItemEncode8(QrcodeStream *stream, QrcodeItem *item, int32_t version)
{
    int32_t ret = 0;
    if ((stream == nullptr) || (item == nullptr)) {
        return -1;
    }
    ret = QrcodeStreamAddNum(stream, SET_BIT_FOUR, QRCODE_VERSION_MODEID_8);
    if (ret < 0) {
        return -1;
    }
    ret = QrcodeStreamAddNum(stream, QrcodeVersionModeLength(QRCODE_MODE_8, version), item->size);
    if (ret < 0) {
        return -1;
    }
    ret = QrcodeStreamAddBytes(stream, item->size, item->data);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int32_t QrcodeItemCheck(QrcodeMode mode, int32_t size, const uint8_t *data)
{
    if ((size <= 0) || (data == nullptr)) {
        return -1;
    }
    switch (mode) {
        case QRCODE_MODE_NUM: {
            for (int32_t i = 0; i < size; i++) {
                if (data[i] < '0' || data[i] > '9') {
                    return -1;
                }
            }
            return 0;
        }
        case QRCODE_MODE_AN: {
            for (int32_t i = 0; i < size; i++) {
                if (QrcodeLookAtTable(data[i]) < 0) {
                    return -1;
                }
            }
            return 0;
        }
        case QRCODE_MODE_8:
            return 0;
        case QRCODE_MODE_NUL:
            break;
        default:
            break;
    }

    return -1;
}

static int32_t QRItemGetBitSize(QrcodeItem *item, int32_t version)
{
    int32_t bitCount = 0;
    if (item == nullptr) {
        return 0;
    }
    if (version == 0) {
        version = 1;
    }
    switch (item->mode) {
        case QRCODE_MODE_NUM:
            bitCount = QrcodeEstimateNum(item->size);
            break;
        case QRCODE_MODE_AN:
            bitCount = QrcodeEstimateAlphaBet(item->size);
            break;
        case QRCODE_MODE_8:
            bitCount = QrcodeEstimate8(item->size);
            break;
        default:
            return 0;
    }

    int32_t len = QrcodeVersionModeLength(item->mode, version);
    int32_t m = 1U << ((uint32_t)len);
    int32_t num = (item->size + m - 1) / m;
    bitCount += num * (MODE_INDICATOR_SIZE + len);

    return bitCount;
}

static int32_t QRItemListGetBitStreamSize(QrcodeItemList *list, int32_t version)
{
    QrcodeItem *pos = nullptr;
    int bits = 0;
    if ((list == nullptr) || QrCodeListEmpty(&list->list)) {
        return 0;
    }
    QrcodeDlListIter iter;
    QrcodeDlListIterInitTyped(&iter, &list->list, &((QrcodeItem *)0)->next);
    while (QrcodeDlListIterHasNext(&iter)) {
        pos = (QrcodeItem *)QrcodeDlListIterNext(&iter);
        bits += QRItemGetBitSize(pos, version);
    }

    return bits;
}

static int32_t QRItemListGetMinVersion(QrcodeItemList *list)
{
    int32_t bitCount = 0;
    int32_t version = 0;
    int32_t prev = 0;
    version = 0;
    do {
        prev = version;
        bitCount = QRItemListGetBitStreamSize(list, prev);
        version = QrcodeVersionGetMinVersion((bitCount + SET_BIT_SEVEN) / SET_BIT_EIGHT);
        if (version < 0) {
            return -1;
        }
    } while (version > prev);
    return version;
}

static int32_t QRItemItemToStreamMode(QrcodeStream *stream, QrcodeItem *item, int32_t version)
{
    if (item == nullptr) {
        return 0;
    }
    switch (item->mode) {
        case QRCODE_MODE_NUM:
            return QrcodeItemEncodeNum(stream, item, version);
        case QRCODE_MODE_AN:
            return QRItemEncodeAlphabet(stream, item, version);
        case QRCODE_MODE_8:
            return QRItemEncode8(stream, item, version);
        default:
            break;
    }

    return 0;
}

static int32_t QrcodeItemItemToStream(QrcodeStream *stream, QrcodeItem *item, int32_t version)
{
    int32_t ret = 0;
    QrcodeItem *st1 = nullptr;
    QrcodeItem *st2 = nullptr;
    if ((stream == nullptr) || (item == nullptr)) {
        return -1;
    }
    int32_t words = QrcodeVersionMaxWordsInMode(version, item->mode);
    if (words != 0 && item->size > words) {
        st1 = QrcodeItemNew(item->mode, words, item->data);
        if (st1 == nullptr) {
            goto ABORT;
        }
        st2 = QrcodeItemNew(item->mode, item->size - words, &item->data[words]);
        if (st2 == nullptr) {
            goto ABORT;
        }
        ret = QrcodeItemItemToStream(stream, st1, version);
        if (ret < 0) {
            goto ABORT;
        }
        ret = QrcodeItemItemToStream(stream, st2, version);
        if (ret < 0) {
            goto ABORT;
        }
        QrcodeItemFree(st1);
        QrcodeItemFree(st2);
    } else {
        if (QRItemItemToStreamMode(stream, item, version) < 0) {
            return -1;
        }
    }

    return ret;
ABORT:
    QrcodeItemFree(st1);
    QrcodeItemFree(st2);

    return -1;
}

static int32_t QrcodeItemListFillStream(QrcodeStream *stream, QrcodeItemList *list)
{
    QrcodeItem *pos = nullptr;
    if ((list == nullptr) || QrCodeListEmpty(&list->list)) {
        return -1;
    }
    QrcodeDlListIter iter;
    QrcodeDlListIterInitTyped(&iter, &list->list, &((QrcodeItem *)0)->next);
    while (QrcodeDlListIterHasNext(&iter)) {
        pos = (QrcodeItem *)QrcodeDlListIterNext(&iter);
        if (QrcodeItemItemToStream(stream, pos, list->version) < 0) {
            return -1;
        }
    }

    return 0;
}

static int32_t QrcodeItemListListToStream(QrcodeStream *stream, QrcodeItemList *list)
{
    if (list == nullptr) {
        return -1;
    }
    int32_t ver = QRItemListGetMinVersion(list);
    if (ver > list->version) {
        list->version = ver;
    }

    int32_t retryTimesCount = 0;
    while (retryTimesCount < QRCODE_VERSION_MAX) {
        QrcodeStreamClean(stream);
        if (QrcodeItemListFillStream(stream, list) < 0) {
            return -1;
        }
        ver = QrcodeVersionGetMinVersion((stream->bitCount + SET_BIT_SEVEN) / QR_BIT_NINE);
        if (ver < 0) {
            return -1;
        } else if (ver > list->version) {
            list->version = ver;
        } else {
            break;
        }
        retryTimesCount++;
    }
    if (retryTimesCount >= QRCODE_VERSION_MAX) {
        return -1;
    }

    return 0;
}

static int32_t QRItemListAddPaddingBitInit(QrcodeStream *stream, int32_t version, int32_t *bits, int32_t *maxwords,
                                           int32_t *maxbits)
{
    if ((maxwords == nullptr) || (maxbits == nullptr)) {
        return -1;
    }
    *bits = stream->bitCount;
    *maxwords = QrcodeVersionGetDataLen(version);
    *maxbits = (*maxwords) * SET_BIT_EIGHT;
    if (*maxbits < *bits) {
        return -1;
    }
    if (*maxbits == *bits) {
        return 0;
    }
    return 1;
}

static int32_t QRItemListAddPaddingBit(QrcodeStream *stream, int32_t version)
{
    int32_t bits = 0;
    int32_t maxbits = 0;
    int32_t maxwords = 0;
    int32_t ret = 0;
    uint8_t *buf = nullptr;
    int32_t words = 0;
    QrcodeStream *padding = nullptr;
    int32_t padLen = 0;

    ret = QRItemListAddPaddingBitInit(stream, version, &bits, &maxwords, &maxbits);
    if (ret <= 0) {
        return ret;
    }

    if (maxbits - bits <= SET_BIT_FOUR) {
        ret = QrcodeStreamAddNum(stream, maxbits - bits, 0);
        goto DONE;
    }
    words = (bits + SET_BIT_FOUR + SET_BIT_SEVEN) / QR_BIT_EIGHT;
    padding = QrcodeStreamNew();
    if (padding == nullptr) {
        return -1;
    }
    ret = QrcodeStreamAddNum(padding, words * QR_BIT_EIGHT - bits, 0);
    if (ret < 0) {
        goto DONE;
    }
    padLen = maxwords - words;
    if (padLen > 0) {
        buf = (uint8_t *)QrcodeMalloc(padLen);
        if (buf == nullptr) {
            ret = -1;
            goto DONE;
        }
        for (int32_t i = 0; i < padLen; i++) {
            buf[i] = (((uint32_t)i) & 1) ? 0x11 : 0xec;
        }
        ret = QrcodeStreamAddBytes(padding, padLen, buf);
        QrcodeFree(buf);
        if (ret < 0) {
            goto DONE;
        }
    }
    ret = QrcodeStreamAdd(stream, padding);
DONE:
    QrcodeStreamFree(padding);

    return ret;
}

static QrcodeStream *QRItemListMergeBitStream(QrcodeItemList *list)
{
    if ((list == nullptr) || QrCodeListEmpty(&list->list)) {
        return nullptr;
    }
    QrcodeStream *stream = QrcodeStreamNew();
    if (stream == nullptr) {
        return nullptr;
    }

    if (QrcodeItemListListToStream(stream, list) < 0) {
        QrcodeStreamFree(stream);
        return nullptr;
    }

    return stream;
}

uint8_t *QrcodeItemListGetByteStream(QrcodeItemList *list)
{
    QrcodeStream *stream = QRItemListMergeBitStream(list);
    if (stream == nullptr) {
        return nullptr;
    }

    if (QRItemListAddPaddingBit(stream, list->version) < 0) {
        QrcodeStreamFree(stream);
        return nullptr;
    }
    uint8_t *array = QrcodeStreamDupData(stream);
    QrcodeStreamFree(stream);

    return array;
}
