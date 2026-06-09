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

#include "common/qrcodegen/qrcode_stream.h"
#include "common/qrcodegen/qrcode_generator.h"
#include "common/qrcodegen/qrcode_item.h"

QrcodeStream *QrcodeStreamNew(void)
{
    QrcodeStream *stream = (QrcodeStream *)QrcodeMalloc(sizeof(QrcodeStream));
    if (stream) {
        if (memset_s(stream, sizeof(QrcodeStream), 0, sizeof(QrcodeStream)) != 0) {
            return stream;
        }
    }

    return stream;
}

static int32_t QrcodeCharCount(int32_t n)
{
    return (n + 7) / 8; // 7:向上取整的补偿值， 8: 1字节
}

static int32_t QrcodeStreamAddBuf(QrcodeStream *stream, uint8_t *buf, int32_t bufLen)
{
    if (buf == nullptr) {
        return -1;
    }
    if (memset_s(buf, bufLen, 0, bufLen) != 0) {
        QrcodeFree(buf);
        return -1;
    }
    if (stream == nullptr) {
        return -1;
    }
    if (stream->bitCount) {
        int32_t len = (bufLen > QrcodeCharCount(stream->bitCount)) ? QrcodeCharCount(stream->bitCount) : bufLen;
        if (memcpy_s(buf, len, stream->data, len) != 0) {
            QrcodeFree(buf);
            return -1;
        }
    }

    return 0;
}

static int32_t QrcodeStreamAddByteInternal(QrcodeStream *stream, uint8_t *src, int32_t byteLen)
{
    if ((stream == nullptr) || (src == nullptr)) {
        return -1;
    }
    int32_t bufLen = QrcodeCharCount(stream->bitCount + byteLen);
    uint8_t *buf = (uint8_t *)QrcodeMalloc(bufLen);

    if (QrcodeStreamAddBuf(stream, buf, bufLen) != 0) {
        QrcodeFree(buf);
        return -1;
    }

    uint8_t *p = buf + stream->bitCount / QR_BIT_EIGHT;
    uint8_t pBitPos = stream->bitCount % QR_BIT_EIGHT;

    if ((stream->bitCount % QR_BIT_EIGHT) == 0) {
        if (memcpy_s(p, QrcodeCharCount(byteLen), src, QrcodeCharCount(byteLen)) != 0) {
            QrcodeFree(buf);
            return -1;
        }
    } else {
        for (int32_t i = 0; i < byteLen; i++) {
            uint8_t srcBitPos = i % QR_BIT_EIGHT;
            if (src[i / QR_BIT_EIGHT] & (0x80 >> srcBitPos)) {
                p[(pBitPos + i) / SET_BIT_EIGHT] |= ((uint8_t)(0x80 >> ((uint8_t)(pBitPos + i) % QR_BIT_EIGHT)));
            } else {
                p[(pBitPos + i) / SET_BIT_EIGHT] &= ~((uint8_t)(0x80 >> ((uint8_t)(pBitPos + i) % QR_BIT_EIGHT)));
            }
        }
    }
    if (stream->data) {
        QrcodeFree(stream->data);
    }
    stream->data = buf;
    stream->bitCount += byteLen;

    return 0;
}

int32_t QrcodeStreamAdd(QrcodeStream *stream, QrcodeStream *arg)
{
    if ((stream == nullptr) || (arg == nullptr)) {
        return -1;
    }

    return QrcodeStreamAddByteInternal(stream, arg->data, arg->bitCount);
}

int32_t QrcodeStreamAddNum(QrcodeStream *stream, int32_t bits, uint32_t num)
{
    if (stream == nullptr) {
        return -1;
    }
    if (bits == 0) {
        return 0;
    }
    int32_t bufLen = QrcodeCharCount(bits + stream->bitCount);
    uint8_t *buf = (uint8_t *)QrcodeMalloc(bufLen);
    if (buf == nullptr) {
        return -1;
    }
    if (QrcodeStreamAddBuf(stream, buf, bufLen) != 0) {
        QrcodeFree(buf);
        return -1;
    }
    uint8_t *p = buf + stream->bitCount / QR_BIT_EIGHT;
    uint8_t pBitPos = stream->bitCount % QR_BIT_EIGHT;
    uint32_t mask = 1U << ((uint32_t)(bits - 1));
    for (int32_t i = 0; i < bits; i++) {
        if (num & mask) {
            p[(pBitPos + i) / SET_BIT_EIGHT] |= ((uint8_t)(0x80 >> ((uint32_t)((pBitPos + i) % SET_BIT_EIGHT))));
        } else {
            p[(pBitPos + i) / SET_BIT_EIGHT] &= ~((uint8_t)(0x80 >> ((uint32_t)((pBitPos + i) % SET_BIT_EIGHT))));
        }
        mask = mask >> 1;
    }
    if (stream->data) {
        QrcodeFree(stream->data);
    }
    stream->data = buf;
    stream->bitCount += bits;

    return 0;
}

int32_t QrcodeStreamAddBytes(QrcodeStream *stream, int32_t size, uint8_t *data)
{
    if ((stream == nullptr) || (data == nullptr)) {
        return -1;
    }

    return QrcodeStreamAddByteInternal(stream, data, size * QR_BIT_EIGHT);
}

uint8_t *QrcodeStreamDupData(QrcodeStream *stream)
{
    uint8_t *data = (uint8_t *)QrcodeMalloc(QrcodeCharCount(stream->bitCount));
    if (data) {
        if (memcpy_s(data, QrcodeCharCount(stream->bitCount), stream->data,
            QrcodeCharCount(stream->bitCount)) != 0) {
            QrcodeFree(data);
            return nullptr;
        }
    }

    return data;
}

void QrcodeStreamFree(QrcodeStream *stream)
{
    if (stream) {
        if (stream->data) {
            QrcodeFree(stream->data);
        }
        QrcodeFree(stream);
    }
}

void QrcodeStreamClean(QrcodeStream *stream)
{
    if (!stream) {
        if (stream->data) {
            QrcodeFree(stream->data);
        }
        stream->data = nullptr;
        stream->bitCount = 0;
    }
}
