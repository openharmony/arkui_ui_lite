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

#ifndef __QRCODE_RSCODE_H__
#define __QRCODE_RSCODE_H__

#include <stdint.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct ReedSolomonCode {
    int32_t mm;
    int32_t nn;
    uint8_t *alphaTo;
    uint8_t *indexOf;
    uint8_t *genpoly;
    int32_t nroots;
    int32_t fcr;
    int32_t prim;
    int32_t iprim;
    int32_t pad;
    int32_t gfpoly;
    struct ReedSolomonCode *next;
} ReedSolomonCode;

typedef struct {
    int32_t symsize;
    int32_t gfpoly;
    int32_t fcr;
    int32_t prim;
    int32_t nroots;
    int32_t pad;
} ReedSolomonCodeData;

extern ReedSolomonCode *ReedSolomonCodeInit(ReedSolomonCodeData codeData);
extern void ReedSolomonCodeEncode(ReedSolomonCode *rsCode, const uint8_t *data, uint8_t *parity);
extern void ReedSolomonCodeFree(ReedSolomonCode *rs);
extern void ReedSolomonCodeFreeCache(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __QRCODE_RSCODE_H__ */
