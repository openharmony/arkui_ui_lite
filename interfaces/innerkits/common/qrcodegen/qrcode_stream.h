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

#ifndef __QRCODE_STREAM_H__
#define __QRCODE_STREAM_H__

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct {
    uint8_t *data;
    int32_t bitCount;
} QrcodeStream;

extern QrcodeStream *QrcodeStreamNew(void);
extern int32_t QrcodeStreamAdd(QrcodeStream *stream, QrcodeStream *arg);
extern int32_t QrcodeStreamAddNum(QrcodeStream *stream, int32_t bits, uint32_t num);
extern int32_t QrcodeStreamAddBytes(QrcodeStream *stream, int32_t size, uint8_t *data);
extern uint8_t *QrcodeStreamDupData(QrcodeStream *stream);
extern void QrcodeStreamFree(QrcodeStream *stream);
extern void QrcodeStreamClean(QrcodeStream *stream);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __QRCODE_STREAM_H__ */
