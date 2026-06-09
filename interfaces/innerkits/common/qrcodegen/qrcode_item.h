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

#ifndef __QRCODE_ITEM_H__
#define __QRCODE_ITEM_H__

#include "qrcode_stream.h"
#include "qrcode_inner.h"
#include "qrcode_list.h"

extern const int8_t QRCODE_ALPHA_BET_TABLE[128];

#define MODE_INDICATOR_SIZE 4

typedef enum {
    QR_BIT_ONE = 1,
    QR_BIT_TWO = 2,
    QR_BIT_THREE,
    QR_BIT_FOUR,
    QR_BIT_FIVE,
    QR_BIT_SIX,
    QR_BIT_SEVEN,
    QR_BIT_EIGHT,
    QR_BIT_NINE,
    QR_BIT_TEN,
} QRCODE_BIT_NUM;

typedef enum {
    SET_BIT_ONE = 1,
    SET_BIT_TWO = 2,
    SET_BIT_THREE,
    SET_BIT_FOUR,
    SET_BIT_FIVE,
    SET_BIT_SIX,
    SET_BIT_SEVEN,
    SET_BIT_EIGHT,
    SET_BIT_NINE,
    SET_BIT_TEN,
} SET_BIT_COUNT_NUM;

typedef struct {
    QrcodeMode mode;
    int32_t size;
    uint8_t *data;
    QRCODE_DL_LIST next;
} QrcodeItem;

extern QrcodeItemList *QrcodeItemListNew();

extern int32_t QrcodeItemListAdd(QrcodeItemList *list, QrcodeMode mode, int32_t size, const uint8_t *data);

extern void QrcodeItemListFree(QrcodeItemList *list);

extern int32_t QrcodeItemCheck(QrcodeMode mode, int32_t size, const uint8_t *data);

extern uint8_t *QrcodeItemListGetByteStream(QrcodeItemList *list);

extern int32_t QrcodeEstimateNum(int32_t size);

extern int32_t QrcodeEstimateAlphaBet(int32_t size);

extern int32_t QrcodeEstimate8(int32_t size);

extern int32_t QrcodeLookAtTable(unsigned char c);

bool QrcodeIsValidMode(QrcodeMode mode);

#endif /* __QRCODE_ITEM_H__ */