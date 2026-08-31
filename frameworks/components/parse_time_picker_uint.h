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

#ifndef GRAPHIC_LITE_PARSE_TIME_PICKER_UINT_H
#define GRAPHIC_LITE_PARSE_TIME_PICKER_UINT_H

#include <charconv>
#include <cstdint>
#include <cstring>
#include <system_error>

namespace OHOS {
inline bool ParseTimePickerUint(const char *first, const char *last, uint16_t &value)
{
    if (first == nullptr || last == nullptr || first == last) {
        return false;
    }
    uint16_t parsed = 0;
    auto result = std::from_chars(first, last, parsed);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    value = parsed;
    return true;
}

inline bool ParseTimePickerValue(const char *value, bool withSecond, uint16_t &hour, uint16_t &minute, uint16_t &second)
{
    if (value == nullptr || *value == '\0') {
        return false;
    }
    const char *start = value;
    const char *end = value + std::strlen(value);
    const char *colon1 = nullptr;
    const char *colon2 = nullptr;
    for (const char *p = start; p < end; ++p) {
        if (*p != ':') {
            continue;
        }
        if (colon1 == nullptr) {
            colon1 = p;
        } else if (colon2 == nullptr) {
            colon2 = p;
        } else {
            return false;
        }
    }
    uint16_t parsedHour = 0;
    uint16_t parsedMinute = 0;
    uint16_t parsedSecond = 0;
    if (withSecond) {
        if (colon1 == nullptr || colon2 == nullptr) {
            return false;
        }
        if (!ParseTimePickerUint(start, colon1, parsedHour) ||
            !ParseTimePickerUint(colon1 + 1, colon2, parsedMinute) ||
            !ParseTimePickerUint(colon2 + 1, end, parsedSecond)) {
            return false;
        }
        hour = parsedHour;
        minute = parsedMinute;
        second = parsedSecond;
        return true;
    }
    if (colon1 == nullptr || colon2 != nullptr) {
        return false;
    }
    if (!ParseTimePickerUint(start, colon1, parsedHour) || !ParseTimePickerUint(colon1 + 1, end, parsedMinute)) {
        return false;
    }
    hour = parsedHour;
    minute = parsedMinute;
    second = 0;
    return true;
}
} // namespace OHOS

#endif // GRAPHIC_LITE_PARSE_TIME_PICKER_UINT_H
