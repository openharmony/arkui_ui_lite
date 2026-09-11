/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef GRAPHIC_LITE_SVG_STRING_UTIL_H
#define GRAPHIC_LITE_SVG_STRING_UTIL_H

#include <cstring>
#include "securec.h"

namespace OHOS {

constexpr uint32_t MAX_STRING_COPY_LEN = 4096;

static inline char* AllocateStringCopy(const char* src, uint32_t len)
{
    // len==0 is kept as nullptr because allocating a zero-length array
    // provides no usable buffer. len is also capped to prevent excessive
    // allocation from unvalidated upstream sources.
    if (len == 0 || len > MAX_STRING_COPY_LEN) {
        return nullptr;
    }
    char* dst = new char[len];
    if (dst == nullptr || memcpy_s(dst, len, src, len) != EOK) {
        delete[] dst;
        return nullptr;
    }
    return dst;
}

inline char* CopyStringWithLimit(const char* src, uint32_t maxLen)
{
    if (src == nullptr) {
        return nullptr;
    }
    uint32_t len = strlen(src) + 1;
    if (len > maxLen) {
        return nullptr;
    }
    return AllocateStringCopy(src, len);
}

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_STRING_UTIL_H
