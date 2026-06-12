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
#include "common/qrcodegen/qrcode_item.h"
#include "common/qrcodegen/qrcode_version.h"
#include "common/qrcodegen/qrcode_inner.h"

typedef struct {
    const char *p;
    const char *q;
    QrcodeMode mode;
    int32_t ret;
    int32_t len;
    int32_t dif;
    int32_t la;
    int32_t ln;
    int32_t l8;
    int32_t swcost;
} QrcodeStrGet8Data;

static inline bool IsNumber(uint8_t c)
{
    return (unsigned char)(c - '0') < 10; // 10: 数字字符的个数
}

int32_t QrcodeLookAtTable(unsigned char c)
{
    if (c & 0x80) { // 0x80：分界线
        return -1;
    }
    return QRCODE_ALPHA_BET_TABLE[(int32_t)c];
}

static inline int32_t IsAlphaBetNumber(unsigned char c)
{
    return QrcodeLookAtTable(c) >= 0;
}

static QrcodeMode QrcodeStrGetMode(uint8_t c)
{
    if (c == '\0') {
        return QRCODE_MODE_NUL;
    }
    if (IsNumber(c)) {
        return QRCODE_MODE_NUM;
    } else if (IsAlphaBetNumber(c)) {
        return QRCODE_MODE_AN;
    }

    return QRCODE_MODE_8;
}

static int32_t QrcodeStrGetNum(const char *string, QrcodeItemList *list);
static int32_t QrcodeStrGetAn(const char *string, QrcodeItemList *list);
static int32_t QrcodeStrGet8(const char *string, QrcodeItemList *list);

static int32_t QrcodeStrGetNum(const char *string, QrcodeItemList *list)
{
    if (list == nullptr) {
        return -1;
    }

    const char *p = string;
    while (IsNumber(*p)) {
        p++;
    }

    int32_t dif = 0;
    int32_t ln = QrcodeVersionModeLength(QRCODE_MODE_NUM, list->version);
    int32_t len = p - string;
    QrcodeMode mode = QrcodeStrGetMode(*p);
    if (mode == QRCODE_MODE_8) {
        dif = QrcodeEstimateNum(len) + SET_BIT_FOUR + ln + QrcodeEstimate8(1) - QrcodeEstimate8(len + 1);
        if (dif > 0) {
            return QrcodeStrGet8(string, list);
        }
    }
    if (mode == QRCODE_MODE_AN) {
        dif = QrcodeEstimateNum(len) + SET_BIT_FOUR + ln + QrcodeEstimateAlphaBet(1) - QrcodeEstimateAlphaBet(len + 1);
        if (dif > 0) {
            return QrcodeStrGetAn(string, list);
        }
    }

    if (QrcodeItemListAdd(list, QRCODE_MODE_NUM, len, (uint8_t *)string) < 0) {
        return -1;
    }

    return len;
}

static int32_t QrcodeStrGetAn(const char *string, QrcodeItemList *list)
{
    if (list == nullptr) {
        return -1;
    }

    const char *p = string;
    if (p == nullptr) {
        return 0;
    }

    const char *q = nullptr;
    int32_t ln = QrcodeVersionModeLength(QRCODE_MODE_NUM, list->version);
    int32_t dif = 0;
    while (IsAlphaBetNumber(*p)) {
        if (IsNumber(*p)) {
            q = p;
            while (IsNumber(*q)) {
                q++;
            }
            dif = QrcodeEstimateAlphaBet(p - string) + QrcodeEstimateNum(q - p) + SET_BIT_FOUR + ln +
                (IsAlphaBetNumber(*q) ? (SET_BIT_FOUR + ln) : 0) - QrcodeEstimateAlphaBet(q - string);
            if (dif < 0) {
                break;
            } else {
                p = q;
            }
        } else {
            p++;
        }
    }

    int32_t len = p - string;
    int32_t la = QrcodeVersionModeLength(QRCODE_MODE_AN, list->version);
    if (*p && ((uint32_t)(int32_t)*p) < sizeof(QRCODE_ALPHA_BET_TABLE) && !IsAlphaBetNumber(*p)) {
        dif = QrcodeEstimateAlphaBet(len) + SET_BIT_FOUR + la + QrcodeEstimate8(1) - QrcodeEstimate8(len + 1);
        if (dif > 0) {
            return QrcodeStrGet8(string, list);
        }
    }

    if (QrcodeItemListAdd(list, QRCODE_MODE_AN, len, (uint8_t *)string) < 0) {
        return -1;
    }

    return len;
}

static void QrcodeStrGet8Init(const char *string, QrcodeItemList *list, QrcodeStrGet8Data *strGet8Data)
{
    if ((list == nullptr) || (strGet8Data == nullptr)) {
        return;
    }
    strGet8Data->q = nullptr;
    strGet8Data->mode = QRCODE_MODE_NUL;
    strGet8Data->ret = 0;
    strGet8Data->len = 0;
    strGet8Data->dif = 0;
    strGet8Data->swcost = 0;
    strGet8Data->la = QrcodeVersionModeLength(QRCODE_MODE_AN, list->version);
    strGet8Data->ln = QrcodeVersionModeLength(QRCODE_MODE_NUM, list->version);
    strGet8Data->l8 = QrcodeVersionModeLength(QRCODE_MODE_8, list->version);
    strGet8Data->p = string + 1;
}

static int32_t QrcodeStrGet8(const char *string, QrcodeItemList *list)
{
    QrcodeStrGet8Data strGet8Data;
    QrcodeStrGet8Init(string, list, &strGet8Data);

    while (*(strGet8Data.p) != '\0') {
        strGet8Data.mode = QrcodeStrGetMode(*(strGet8Data.p));
        if (strGet8Data.mode == QRCODE_MODE_NUM) {
            strGet8Data.q = strGet8Data.p;
            while (IsNumber(*(strGet8Data.q))) {
                strGet8Data.q++;
            }
            strGet8Data.swcost = (QrcodeStrGetMode(*(strGet8Data.q)) == QRCODE_MODE_8) ?
                (SET_BIT_FOUR + strGet8Data.l8) : 0;
            strGet8Data.dif = QrcodeEstimate8(strGet8Data.p - string) +
                QrcodeEstimateNum(strGet8Data.q - strGet8Data.p) + SET_BIT_FOUR + strGet8Data.ln +
                strGet8Data.swcost - QrcodeEstimate8(strGet8Data.q - string);
            if (strGet8Data.dif < 0) {
                break;
            } else {
                strGet8Data.p = strGet8Data.q;
            }
        } else if (strGet8Data.mode == QRCODE_MODE_AN) {
            strGet8Data.q = strGet8Data.p;
            while (IsAlphaBetNumber(*(strGet8Data.q))) {
                strGet8Data.q++;
            }
            strGet8Data.swcost = (QrcodeStrGetMode(*(strGet8Data.q)) == QRCODE_MODE_8) ?
                (SET_BIT_FOUR + strGet8Data.l8) : 0;
            strGet8Data.dif = QrcodeEstimate8(strGet8Data.p - string) +
                QrcodeEstimateAlphaBet(strGet8Data.q - strGet8Data.p) + SET_BIT_FOUR + strGet8Data.la +
                strGet8Data.swcost - QrcodeEstimate8(strGet8Data.q - string);
            if (strGet8Data.dif < 0) {
                break;
            } else {
                strGet8Data.p = strGet8Data.q;
            }
        } else {
            strGet8Data.p++;
        }
    }
    strGet8Data.len = strGet8Data.p - string;
    strGet8Data.ret = QrcodeItemListAdd(list, QRCODE_MODE_8, strGet8Data.len, (unsigned char *)string);
    if (strGet8Data.ret < 0) {
        return -1;
    }

    return strGet8Data.len;
}

int32_t QrcodeStrToItemList(const char *str, QrcodeItemList *list)
{
    int32_t length = 0;
    QrcodeMode mode = QRCODE_MODE_NUM;
    const char *p = str;
    if (p == nullptr) {
        return -1;
    }
    while (*p != '\0') {
        mode = QrcodeStrGetMode(*p);
        if (mode == QRCODE_MODE_NUM) {
            length = QrcodeStrGetNum(p, list);
        } else if (mode == QRCODE_MODE_AN) {
            length = QrcodeStrGetAn(p, list);
        } else {
            length = QrcodeStrGet8(p, list);
        }
        if (length == 0) {
            break;
        }
        p = p + length;
        if (length < 0) {
            return -1;
        }
    }

    return 0;
}