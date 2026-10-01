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

#ifndef GRAPHIC_LITE_SVG_TYPES_H
#define GRAPHIC_LITE_SVG_TYPES_H

#include <cstdint>
#include "svg/svg_element_type.h"

namespace OHOS {

enum class SvgResult {
    SVG_RESULT_OK = 0,
    SVG_RESULT_ERROR,
    SVG_RESULT_NO_MEMORY,
    SVG_RESULT_INVALID_PARAM,
};

constexpr uint16_t SVG_MAX_ATTR_LEN = 256;
constexpr uint16_t SVG_MAX_TAG_LEN = 64;
constexpr uint16_t SVG_MAX_PATH_LEN = 4096;
constexpr uint16_t SVG_MAX_TEXT_LEN = 2048;
constexpr uint16_t SVG_MAX_STACK_DEPTH = 64;

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_TYPES_H
