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

#ifndef GRAPHIC_LITE_SVG_ATTRIBUTE_PARSER_H
#define GRAPHIC_LITE_SVG_ATTRIBUTE_PARSER_H

#include <cstdint>

#include "gfx_utils/diagram/common/paint.h"
#include "gfx_utils/graphic_types.h"
#include "gfx_utils/list.h"
#include "gfx_utils/trans_affine.h"
#include "svg_types.h"

namespace OHOS {
namespace SvgAttributeParser {

// Recommended buffer size for callers of ParseUrlReference. The function will
// reject (return false) any id that does not fit in the provided buffer, so
// callers should allocate at least SVG_URL_ID_LEN bytes to handle typical ids.
constexpr uint32_t SVG_URL_ID_LEN = 64;

constexpr int32_t DECIMAL_BASE = 10;

constexpr uint8_t MATRIX_INDEX_A = 0;
constexpr uint8_t MATRIX_INDEX_B = 1;
constexpr uint8_t MATRIX_INDEX_C = 2;
constexpr uint8_t MATRIX_INDEX_D = 3;
constexpr uint8_t MATRIX_INDEX_E = 4;
constexpr uint8_t MATRIX_INDEX_F = 5;

constexpr uint8_t VIEWBOX_INDEX_X = 0;
constexpr uint8_t VIEWBOX_INDEX_Y = 1;
constexpr uint8_t VIEWBOX_INDEX_WIDTH = 2;
constexpr uint8_t VIEWBOX_INDEX_HEIGHT = 3;
constexpr uint8_t VIEWBOX_COMPONENT_COUNT = 4;

constexpr uint8_t COLOR_INDEX_RED = 0;
constexpr uint8_t COLOR_INDEX_GREEN = 1;
constexpr uint8_t COLOR_INDEX_BLUE = 2;
constexpr uint8_t COLOR_INDEX_ALPHA = 3;

enum PreserveAspectAlign : uint8_t {
    PRESERVE_ALIGN_XMINYMIN = 0,
    PRESERVE_ALIGN_XMIDYMIN,
    PRESERVE_ALIGN_XMAXYMIN,
    PRESERVE_ALIGN_XMINYMID,
    PRESERVE_ALIGN_XMIDYMID,
    PRESERVE_ALIGN_XMAXYMID,
    PRESERVE_ALIGN_XMINYMAX,
    PRESERVE_ALIGN_XMIDYMAX,
    PRESERVE_ALIGN_XMAXYMAX,
    PRESERVE_ALIGN_NONE = 9,
};

int32_t ParseInt(const char* str);

float ParseFloat(const char* str);

// Saturating conversion from float to int16_t. NaN maps to 0, values outside
// [INT16_MIN, INT16_MAX] saturate instead of invoking undefined behaviour.
int16_t ClampToInt16(float value);

// Rounds to nearest int16_t before saturating. Use for geometric vertices to
// avoid truncation bias in transformed paths.
int16_t RoundToInt16(float value);

// Parses an SVG <length>. When the string has no consumable numeric token the value is
// invalid; per SVG the attribute is then ignored and falls back to its initial value, so
// we return `fallback` instead of 0. Geometry attributes use fallback = 0; stroke-width
// uses fallback = 1 (its initial value).
int16_t ParseLength(const char* str, int16_t fallback = 0);

ColorType ParseColor(const char* str);

uint8_t ParseOpacity(const char* str);

bool ParsePoints(const char* str, List<Point>& points);

// Parses a viewBox attribute "minX minY width height" into out[4].
bool ParseViewBox(const char* str, float out[4]);

// Parses a preserveAspectRatio attribute. align values: 0=xMinYMin, 1=xMidYMin, 2=xMaxYMin,
// 3=xMinYMid, 4=xMidYMid, 5=xMaxYMid, 6=xMinYMax, 7=xMidYMax, 8=xMaxYMax. slice=true for slice.
bool ParsePreserveAspectRatio(const char* str, uint8_t& align, bool& slice);

// Parses a clock value such as "1.5s", "300ms", "00:00:05" or bare seconds into milliseconds.
// Returns 0 for invalid input. For "indefinite" returns UINT32_MAX.
uint32_t ParseClockValue(const char* str);

// Parses a semicolon-separated list of floats into a caller-owned array. Returns the count.
// A non-finite token (NaN/Inf) is not a legal SVG <number>, so the whole list is rejected
// (count 0); when sawNonFinite is provided, *sawNonFinite is set so the caller can mark the
// owning animation element as invalid (SVG/SMIL: an animation in error has no effect).
uint32_t ParseSemicolonFloats(const char* str, float*& outArray, bool* sawNonFinite = nullptr);
uint32_t ParseDashArray(const char* str, float*& outArray);

// Extracts the id from a reference like "url(#id)" or "#id" into out,
// stripping the trailing ')' of url(...). Returns false when str has no '#'.
bool ParseUrlReference(const char* str, char* out, uint32_t outSize);

bool ParseTransform(const char* str, TransAffine& matrix);

// Computes matrix = matrix * rhs. TransAffine::Multiply (operator*=) pre-multiplies
// (matrix = rhs * matrix); SVG transform composition requires post-multiplication.
void PostMultiply(TransAffine& matrix, const TransAffine& rhs);

} // namespace SvgAttributeParser
} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_ATTRIBUTE_PARSER_H
