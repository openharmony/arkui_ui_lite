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

#ifndef GRAPHIC_LITE_SVG_COLOR_UTIL_H
#define GRAPHIC_LITE_SVG_COLOR_UTIL_H

#include <cstdint>
#include "gfx_utils/diagram/common/paint.h"

namespace OHOS {

constexpr uint8_t SVG_ALPHA_SHIFT = 24;
constexpr uint32_t SVG_RGB_MASK = 0x00FFFFFF;
constexpr uint8_t SVG_BYTE_MASK = 0xFF;

inline ColorType SvgApplyOpacity(ColorType color, uint8_t opacity)
{
    uint8_t alpha = (color.full >> SVG_ALPHA_SHIFT) & SVG_BYTE_MASK;
    uint16_t combined = (static_cast<uint16_t>(alpha) * opacity) / OPA_OPAQUE;
    color.full = (color.full & SVG_RGB_MASK) | ((combined & SVG_BYTE_MASK) << SVG_ALPHA_SHIFT);
    return color;
}

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_COLOR_UTIL_H
