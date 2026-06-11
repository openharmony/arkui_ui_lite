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

#ifndef __QRCODE_GENERATOR_H__
#define __QRCODE_GENERATOR_H__
#include <stdint.h>
#include <cstddef>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    QRCODE_ECC_MEDIUM = 0,
    QRCODE_ECC_HIGH,
    QRCODE_ECC_MAX,
} QRCODE_ECC;

typedef struct {
    void *(*mallocFunc)(size_t size);
    void (*freeFunc)(void *ptr);
} QrcodeMemHooks;

#define QRCODE_VERSION_MAX 40

typedef struct {
    int32_t version;
    uint32_t width;
    uint8_t *data;
} QrcodeImage;

QrcodeImage *QrcodeImageEncodeString(const char *text, QRCODE_ECC qrEcc);

void QrcodeImageFree(QrcodeImage *qrImage);

void QrcodeMemInitHooks(const QrcodeMemHooks *hooks);

#if defined(__cplusplus)
}
#endif

#endif /* __QRCODE_GENERATOR_H__ */
