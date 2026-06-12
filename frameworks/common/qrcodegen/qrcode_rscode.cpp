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

#include "common/qrcodegen/qrcode_rscode.h"
#include "common/qrcodegen/qrcode_generator.h"
#include "common/qrcodegen/qrcode_inner.h"
#include <cstring>

#define MM (rsCode->mm)
#define NN (rsCode->nn)
#define ALPHA_TO (rsCode->alphaTo)
#define INDEX_OF (rsCode->indexOf)
#define GENPOLY (rsCode->genpoly)
#define NROOTS (rsCode->nroots)
#define FCR (rsCode->fcr)
#define PRIM (rsCode->prim)
#define IPRIM (rsCode->iprim)
#define PAD (rsCode->pad)
#define A0 (NN)
#define SYM_SIZE_MAX 8

static ReedSolomonCode *g_rsList = nullptr;

static int32_t modnn(ReedSolomonCode *rs, int32_t x)
{
    if (rs == nullptr) {
        return -1;
    }
    while (x >= rs->nn) {
        x -= rs->nn;
        x = (((uint32_t)x) >> ((uint32_t)rs->mm)) + (((uint32_t)x) & ((uint32_t)rs->nn));
    }
    return x;
}

static bool InitRsCharCheck(ReedSolomonCodeData codeData)
{
    if (codeData.symsize < 0 || codeData.symsize > (int32_t)(SYM_SIZE_MAX * sizeof(uint8_t))) {
        return false;
    }
    if (codeData.fcr < 0 || codeData.fcr >= (1 << ((uint32_t)codeData.symsize))) {
        return false;
    }
    if (codeData.prim <= 0 || codeData.prim >= (1 << ((uint32_t)codeData.symsize))) {
        return false;
    }
    if (codeData.nroots < 0 || codeData.nroots >= (1 << ((uint32_t)codeData.symsize))) {
        return false;
    }
    if (codeData.pad < 0 || codeData.pad >= ((1 << ((uint32_t)codeData.symsize)) - 1 - codeData.nroots)) {
        return false;
    }

    return true;
}

static bool InitRsCharInit(ReedSolomonCode *rsCode, ReedSolomonCodeData codeData)
{
    if (rsCode == nullptr) {
        return false;
    }
    if (memset_s(rsCode, sizeof(ReedSolomonCode), 0, sizeof(ReedSolomonCode)) != 0) {
        QrcodeFree(rsCode);
        return false;
    }
    if ((codeData.symsize <= 0) || (codeData.symsize > (int32_t)(SYM_SIZE_MAX * sizeof(uint8_t)))) {
        QrcodeFree(rsCode);
        return false;
    }
    rsCode->mm = codeData.symsize;
    rsCode->nn = (1 << ((uint32_t)codeData.symsize)) - 1;
    rsCode->pad = codeData.pad;

    rsCode->alphaTo = (uint8_t *)QrcodeMalloc(sizeof(uint8_t) * (rsCode->nn + 1));
    if (rsCode->alphaTo == nullptr) {
        QrcodeFree(rsCode);
        return false;
    }
    rsCode->indexOf = (uint8_t *)QrcodeMalloc(sizeof(uint8_t) * (rsCode->nn + 1));
    if (rsCode->indexOf == nullptr) {
        QrcodeFree(rsCode->alphaTo);
        QrcodeFree(rsCode);
        return false;
    }
    rsCode->indexOf[0] = A0;
    rsCode->alphaTo[A0] = 0;

    return true;
}

static void InitRsCharGenpoly(ReedSolomonCode *rsCode, ReedSolomonCodeData codeData)
{
    int32_t iprim = 0;
    int32_t i = 0;
    if (rsCode == nullptr) {
        return;
    }
    for (iprim = 1; (iprim % codeData.prim) != 0; iprim += rsCode->nn)
        ;

    rsCode->iprim = iprim / codeData.prim;
    rsCode->genpoly[0] = 1;
    int32_t root = 0;
    for (i = 0, root = (codeData.fcr * codeData.prim); i < codeData.nroots; i++, root += codeData.prim) {
        rsCode->genpoly[i + 1] = 1;
        for (int32_t j = i; j > 0; j--) {
            if (rsCode->genpoly[j] != 0) {
                rsCode->genpoly[j] = rsCode->genpoly[j - 1] ^
                                     rsCode->alphaTo[modnn(rsCode, rsCode->indexOf[rsCode->genpoly[j]] + root)];
            } else {
                rsCode->genpoly[j] = rsCode->genpoly[j - 1];
            }
        }
        rsCode->genpoly[0] = rsCode->alphaTo[modnn(rsCode, rsCode->indexOf[rsCode->genpoly[0]] + root)];
    }
    for (i = 0; i <= codeData.nroots; i++) {
        rsCode->genpoly[i] = rsCode->indexOf[rsCode->genpoly[i]];
    }
}

static ReedSolomonCode *InitRsChar(ReedSolomonCodeData codeData)
{
    if (InitRsCharCheck(codeData) == false) {
        return nullptr;
    }

    ReedSolomonCode *rsCode = (ReedSolomonCode *)QrcodeMalloc(sizeof(ReedSolomonCode));
    if (InitRsCharInit(rsCode, codeData) == false) {
        QrcodeFree(rsCode);
        return nullptr;
    }

    uint32_t sr = 1;
    for (int32_t i = 0; i < rsCode->nn; i++) {
        rsCode->indexOf[sr] = i;
        rsCode->alphaTo[i] = sr;
        sr <<= 1U;
        if (sr & (1U << ((uint32_t)codeData.symsize))) {
            sr ^= (uint32_t)codeData.gfpoly;
            sr &= (uint32_t)rsCode->nn;
        }
    }
    if (sr != 1) {
        QrcodeFree(rsCode->alphaTo);
        QrcodeFree(rsCode->indexOf);
        QrcodeFree(rsCode);
        return nullptr;
    }

    rsCode->genpoly = (uint8_t *)QrcodeMalloc(sizeof(uint8_t) * (codeData.nroots + 1));
    if (rsCode->genpoly == nullptr) {
        QrcodeFree(rsCode->alphaTo);
        QrcodeFree(rsCode->indexOf);
        QrcodeFree(rsCode);
        return nullptr;
    }
    rsCode->fcr = codeData.fcr;
    rsCode->prim = codeData.prim;
    rsCode->nroots = codeData.nroots;
    rsCode->gfpoly = codeData.gfpoly;
    InitRsCharGenpoly(rsCode, codeData);

    return rsCode;
}

ReedSolomonCode *ReedSolomonCodeInit(ReedSolomonCodeData codeData)
{
    ReedSolomonCode *rsCode = nullptr;
    for (rsCode = g_rsList; rsCode != nullptr; rsCode = rsCode->next) {
        if ((rsCode->pad != codeData.pad) || (rsCode->nroots != codeData.nroots) || (rsCode->mm != codeData.symsize) ||
            (rsCode->gfpoly != codeData.gfpoly) || (rsCode->fcr != codeData.fcr) || (rsCode->prim != codeData.prim)) {
            continue;
        }
        goto DONE;
    }

    rsCode = InitRsChar(codeData);
    if (rsCode == nullptr) {
        goto DONE;
    }
    rsCode->next = g_rsList;
    g_rsList = rsCode;
DONE:

    return rsCode;
}

void ReedSolomonCodeFree(ReedSolomonCode *rs)
{
    QrcodeFree(rs->alphaTo);
    QrcodeFree(rs->indexOf);
    QrcodeFree(rs->genpoly);
    QrcodeFree(rs);
}

void ReedSolomonCodeFreeCache(void)
{
    ReedSolomonCode *rs = g_rsList;
    ReedSolomonCode *next = nullptr;

    while (rs != nullptr) {
        next = rs->next;
        ReedSolomonCodeFree(rs);
        rs = next;
    }
    g_rsList = nullptr;
}

void ReedSolomonCodeEncode(ReedSolomonCode *rsCode, const uint8_t *data, uint8_t *parity)
{
    if ((rsCode == nullptr) || (data == nullptr) || (parity == nullptr)) {
        return;
    }
    if (memset_s(parity, NROOTS * sizeof(uint8_t), 0, NROOTS * sizeof(uint8_t)) != 0) {
        return;
    }
    for (int32_t i = 0; i < NN - NROOTS - PAD; i++) {
        uint8_t feedback = INDEX_OF[data[i] ^ parity[0]];
        if (feedback != A0) {
            for (int32_t j = 1; j < NROOTS; j++) {
                parity[j] ^= ALPHA_TO[modnn(rsCode, feedback + GENPOLY[NROOTS - j])];
            }
        }
        if (memmove_s(&parity[0], sizeof(uint8_t) * (NROOTS - 1), &parity[1],
            sizeof(uint8_t) * (NROOTS - 1)) != 0) {
            return;
        }

        parity[NROOTS - 1] = (feedback != A0) ? ALPHA_TO[modnn(rsCode, feedback + GENPOLY[0])] : 0;
    }
}
