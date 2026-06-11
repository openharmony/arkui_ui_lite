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

#ifndef __QRCODE_INNER_H__
#define __QRCODE_INNER_H__

#include "qrcode_generator.h"
#include "qrcode_list.h"

typedef enum {
    QRCODE_MODE_NUL = -1,
    QRCODE_MODE_NUM = 0,
    QRCODE_MODE_AN,
    QRCODE_MODE_8,
} QrcodeMode;

typedef struct {
    int32_t version;
    QRCODE_DL_LIST list;
} QrcodeItemList;

typedef struct {
    uint8_t *data;
    int32_t dataLen;
    uint8_t *ecc;
    int32_t eccLen;
} QrcodeBlock;

typedef struct {
    int32_t version;
    uint8_t *data;
    int32_t dataLen;
    uint8_t *ecc;
    int32_t eccLen;
    int32_t b1;
    QrcodeBlock *qrBlock;
    int32_t blocks;
    int32_t count;
} QrcodeMergeCode;

extern QrcodeImage *QrcodeImageEncodeMask(QrcodeItemList *list);
extern int32_t QrcodeStrToItemList(const char *str, QrcodeItemList *list);
extern void *QrcodeMalloc(size_t size);
extern void QrcodeFree(void *ptr);

#endif /* __QRCODE_INNER_H__ */