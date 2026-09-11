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

#include "svg/svg_attribute_parser.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace OHOS {
namespace SvgAttributeParser {

int32_t ParseInt(const char* str)
{
    if (str == nullptr) {
        return 0;
    }
    return static_cast<int32_t>(strtol(str, nullptr, DECIMAL_BASE));
}

float ParseFloat(const char* str)
{
    if (str == nullptr) {
        return 0.0f;
    }
    return strtof(str, nullptr);
}

int16_t ClampToInt16(float value)
{
    if (std::isnan(value)) {
        return 0;
    }
    if (value > static_cast<float>(INT16_MAX)) {
        return INT16_MAX;
    }
    if (value < static_cast<float>(INT16_MIN)) {
        return INT16_MIN;
    }
    return static_cast<int16_t>(value);
}

int16_t RoundToInt16(float value)
{
    if (std::isnan(value)) {
        return 0;
    }
    if (value > static_cast<float>(INT16_MAX)) {
        return INT16_MAX;
    }
    if (value < static_cast<float>(INT16_MIN)) {
        return INT16_MIN;
    }
    return static_cast<int16_t>(value >= 0.0f ? value + 0.5f : value - 0.5f);
}

int16_t ParseLength(const char* str, int16_t fallback)
{
    if (str == nullptr) {
        return fallback;
    }
    char* end = nullptr;
    float val = strtof(str, &end);
    if (end == str) {
        // No numeric token could be consumed: invalid value -> fall back to the
        // attribute's initial value rather than 0 (which would, e.g., drop a stroke).
        return fallback;
    }
    return ClampToInt16(val);
}

namespace {

inline float Clamp01(float v)
{
    return (v > 1.0f) ? 1.0f : ((v < 0.0f) ? 0.0f : v);
}

constexpr uint8_t HEX_SHORT_LEN = 3;
constexpr uint8_t HEX_SHORT_ALPHA_LEN = 4;
constexpr uint8_t HEX_LONG_LEN = 6;
constexpr uint8_t HEX_LONG_ALPHA_LEN = 8;
constexpr uint8_t NIBBLE_BITS = 4;
constexpr uint8_t NIBBLE_MASK = 0xF;
constexpr uint8_t COLOR_COMPONENT_MAX = 255;
constexpr float COLOR_COMPONENT_MAX_F = 255.0f;
constexpr float PERCENTAGE_FACTOR = 100.0f;
constexpr uint8_t RGB_PREFIX_LEN = 3;
constexpr uint8_t ALPHA_SHIFT = 24;
constexpr uint8_t RED_SHIFT = 16;
constexpr uint8_t GREEN_SHIFT = 8;
constexpr uint8_t BLUE_SHIFT = 0;
constexpr uint8_t DEGREES_PER_HALF_CIRCLE = 180;
constexpr uint8_t CMD_TRANSLATE_LEN = 9;
constexpr uint8_t CMD_SCALE_LEN = 5;
constexpr uint8_t CMD_ROTATE_LEN = 6;
constexpr uint8_t CMD_SKEW_X_LEN = 5;
constexpr uint8_t CMD_SKEW_Y_LEN = 5;
constexpr uint8_t CMD_MATRIX_LEN = 6;

static bool IsAsciiAlpha(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

constexpr uint8_t HEX_SHORT_RED_HIGH_SHIFT_NIBBLE = 5;
constexpr uint8_t HEX_SHORT_RED_LOW_SHIFT_NIBBLE = 4;
constexpr uint8_t HEX_SHORT_GREEN_HIGH_SHIFT_NIBBLE = 3;
constexpr uint8_t HEX_SHORT_GREEN_LOW_SHIFT_NIBBLE = 2;
constexpr uint8_t HEX_SHORT_BLUE_HIGH_SHIFT_NIBBLE = 1;
constexpr uint8_t HEX_SHORT_BLUE_LOW_SHIFT_NIBBLE = 0;

constexpr uint8_t HEX_SHORT_ALPHA_EXTRACT_NIBBLE = 3;
constexpr uint8_t HEX_SHORT_RED_EXTRACT_NIBBLE = 2;
constexpr uint8_t HEX_SHORT_GREEN_EXTRACT_NIBBLE = 1;
constexpr uint8_t HEX_SHORT_BLUE_EXTRACT_NIBBLE = 0;

constexpr uint8_t SLICE_LEN = 5;
constexpr uint8_t MEET_LEN = 4;
constexpr int32_t RGB_COMPONENT_COUNT = 3;
constexpr uint8_t DASH_ARRAY_PAIR_FACTOR = 2;

constexpr uint32_t COLOR_TRANSPARENT = 0x00000000;
constexpr uint32_t COLOR_OPAQUE_BLACK = 0xFF000000;
constexpr uint32_t COLOR_WHITE = 0xFFFFFFFF;
constexpr uint32_t COLOR_RED = 0xFFFF0000;
constexpr uint32_t COLOR_GREEN = 0xFF008000;
constexpr uint32_t COLOR_BLUE = 0xFF0000FF;
constexpr uint32_t COLOR_YELLOW = 0xFFFFFF00;
constexpr uint32_t COLOR_CYAN = 0xFF00FFFF;
constexpr uint32_t COLOR_MAGENTA = 0xFFFF00FF;

uint32_t ParseHexColor(const char* str)
{
    if (str == nullptr || str[0] != '#' || strlen(str) <= 1) {
        return COLOR_TRANSPARENT;
    }
    uint32_t value = static_cast<uint32_t>(strtoul(str + 1, nullptr, 16));
    uint32_t len = strlen(str + 1);
    if (len == HEX_SHORT_LEN) {
        uint32_t r = (value >> (NIBBLE_BITS * HEX_SHORT_RED_EXTRACT_NIBBLE)) & NIBBLE_MASK;
        uint32_t g = (value >> (NIBBLE_BITS * HEX_SHORT_GREEN_EXTRACT_NIBBLE)) & NIBBLE_MASK;
        uint32_t b = (value >> (NIBBLE_BITS * HEX_SHORT_BLUE_EXTRACT_NIBBLE)) & NIBBLE_MASK;
        return COLOR_OPAQUE_BLACK |
               (r << (NIBBLE_BITS * HEX_SHORT_RED_HIGH_SHIFT_NIBBLE)) |
               (r << (NIBBLE_BITS * HEX_SHORT_RED_LOW_SHIFT_NIBBLE)) |
               (g << (NIBBLE_BITS * HEX_SHORT_GREEN_HIGH_SHIFT_NIBBLE)) |
               (g << (NIBBLE_BITS * HEX_SHORT_GREEN_LOW_SHIFT_NIBBLE)) |
               (b << (NIBBLE_BITS * HEX_SHORT_BLUE_HIGH_SHIFT_NIBBLE)) |
               (b << (NIBBLE_BITS * HEX_SHORT_BLUE_LOW_SHIFT_NIBBLE));
    }
    if (len == HEX_SHORT_ALPHA_LEN) {
        uint32_t a = (value >> (NIBBLE_BITS * HEX_SHORT_ALPHA_EXTRACT_NIBBLE)) & NIBBLE_MASK;
        uint32_t r = (value >> (NIBBLE_BITS * HEX_SHORT_RED_EXTRACT_NIBBLE)) & NIBBLE_MASK;
        uint32_t g = (value >> (NIBBLE_BITS * HEX_SHORT_GREEN_EXTRACT_NIBBLE)) & NIBBLE_MASK;
        uint32_t b = (value >> (NIBBLE_BITS * HEX_SHORT_BLUE_EXTRACT_NIBBLE)) & NIBBLE_MASK;
        a = (a << NIBBLE_BITS) | a;
        r = (r << NIBBLE_BITS) | r;
        g = (g << NIBBLE_BITS) | g;
        b = (b << NIBBLE_BITS) | b;
        return (a << ALPHA_SHIFT) | (r << RED_SHIFT) | (g << GREEN_SHIFT) | b;
    }
    if (len == HEX_LONG_LEN) {
        return COLOR_OPAQUE_BLACK | value;
    }
    if (len == HEX_LONG_ALPHA_LEN) {
        return value;
    }
    // Unsupported hex length: return transparent so callers can distinguish
    // invalid input from opaque black.
    return COLOR_TRANSPARENT;
}

uint32_t ParseNamedColor(const char* str)
{
    if (strcmp(str, "none") == 0) {
        return COLOR_TRANSPARENT;
    }
    if (strcmp(str, "black") == 0) {
        return COLOR_OPAQUE_BLACK;
    }
    if (strcmp(str, "white") == 0) {
        return COLOR_WHITE;
    }
    if (strcmp(str, "red") == 0) {
        return COLOR_RED;
    }
    if (strcmp(str, "green") == 0) {
        return COLOR_GREEN;
    }
    if (strcmp(str, "blue") == 0) {
        return COLOR_BLUE;
    }
    if (strcmp(str, "yellow") == 0) {
        return COLOR_YELLOW;
    }
    if (strcmp(str, "cyan") == 0) {
        return COLOR_CYAN;
    }
    if (strcmp(str, "magenta") == 0) {
        return COLOR_MAGENTA;
    }
    return COLOR_OPAQUE_BLACK;
}

const char* SkipSpacesAndCommas(const char* p)
{
    while (*p != '\0' && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',')) {
        p++;
    }
    return p;
}

uint32_t ParseRgbColor(const char* str)
{
    const char* p = str + RGB_PREFIX_LEN;
    bool hasAlpha = false;
    if (*p == 'a') {
        hasAlpha = true;
        p++;
    }
    if (*p != '(') {
        return COLOR_OPAQUE_BLACK;
    }
    p++;

    // Missing colour components default to 0. For rgb() the alpha defaults to opaque;
    // for rgba() a missing alpha term defaults to transparent.
    float comp[4] = { 0.0f, 0.0f, 0.0f, hasAlpha ? 0.0f : 1.0f };
    for (int32_t i = 0; i < RGB_COMPONENT_COUNT; i++) {
        p = SkipSpacesAndCommas(p);
        char* end = nullptr;
        float v = strtof(p, &end);
        if (end != p) {
            p = end;
            p = SkipSpacesAndCommas(p);
            if (*p == '%') {
                v = v * COLOR_COMPONENT_MAX_F / PERCENTAGE_FACTOR;
                p++;
            }
            comp[i] = v;
        } else if (*p != '\0' && *p != ')') {
            // Malformed token: skip it so the loop keeps making progress.
            p++;
        }
    }

    if (hasAlpha) {
        p = SkipSpacesAndCommas(p);
        char* end = nullptr;
        float a = strtof(p, &end);
        if (end != p) {
            comp[COLOR_INDEX_ALPHA] = a;
            p = end;
        }
    }

    int32_t r = static_cast<int32_t>(comp[COLOR_INDEX_RED] + 0.5f);
    int32_t g = static_cast<int32_t>(comp[COLOR_INDEX_GREEN] + 0.5f);
    int32_t b = static_cast<int32_t>(comp[COLOR_INDEX_BLUE] + 0.5f);
    int32_t a = static_cast<int32_t>(comp[COLOR_INDEX_ALPHA] * COLOR_COMPONENT_MAX_F + 0.5f);
    r = (r < 0) ? 0 : (r > COLOR_COMPONENT_MAX) ? COLOR_COMPONENT_MAX : r;
    g = (g < 0) ? 0 : (g > COLOR_COMPONENT_MAX) ? COLOR_COMPONENT_MAX : g;
    b = (b < 0) ? 0 : (b > COLOR_COMPONENT_MAX) ? COLOR_COMPONENT_MAX : b;
    a = (a < 0) ? 0 : (a > COLOR_COMPONENT_MAX) ? COLOR_COMPONENT_MAX : a;
    return (static_cast<uint32_t>(a) << ALPHA_SHIFT) | (static_cast<uint32_t>(r) << RED_SHIFT) |
           (static_cast<uint32_t>(g) << GREEN_SHIFT) | static_cast<uint32_t>(b);
}

} // namespace

ColorType ParseColor(const char* str)
{
    ColorType color{};
    if (str == nullptr || str[0] == '\0') {
        return color;
    }
    if (str[0] == '#') {
        color.full = ParseHexColor(str);
        return color;
    }
    if (strncmp(str, "rgb", RGB_PREFIX_LEN) == 0) {
        color.full = ParseRgbColor(str);
        return color;
    }
    color.full = ParseNamedColor(str);
    return color;
}

uint8_t ParseOpacity(const char* str)
{
    if (str == nullptr) {
        return OPA_OPAQUE;
    }
    char* end = nullptr;
    float val = strtof(str, &end);
    if (end == str) {
        // Unparseable value: per SVG the attribute is ignored and falls back to its
        // initial value, which for opacity is 1 (fully opaque). Previously this returned
        // 0, incorrectly hiding the element.
        return OPA_OPAQUE;
    }
    val = Clamp01(val);
    return static_cast<uint8_t>(val * OPA_OPAQUE);
}

bool ParsePoints(const char* str, List<Point>& points)
{
    if (str == nullptr) {
        return false;
    }
    const char* p = str;
    while (*p != '\0') {
        p = SkipSpacesAndCommas(p);
        if (*p == '\0') {
            break;
        }
        const char* before = p;
        float x = strtof(p, const_cast<char**>(&p));
        if (p == before) {
            // strtof could not consume the token (e.g. a lone '.' or '-'): skip it to
            // guarantee progress instead of pushing {0, 0} forever.
            p++;
            continue;
        }
        p = SkipSpacesAndCommas(p);
        if (*p == '\0') {
            break;
        }
        before = p;
        float y = strtof(p, const_cast<char**>(&p));
        if (p == before) {
            p++;
            continue;
        }
        Point point = { ClampToInt16(x), ClampToInt16(y) };
        points.PushBack(point);
    }
    return true;
}

bool ParseUrlReference(const char* str, char* out, uint32_t outSize)
{
    if (str == nullptr || out == nullptr || outSize == 0) {
        return false;
    }
    out[0] = '\0';
    const char* p = str;
    while (*p != '\0' && *p != '#') {
        p++;
    }
    if (*p != '#') {
        return false;
    }
    p++;
    uint32_t i = 0;
    while (*p != '\0' && *p != ')' && *p != ' ' && *p != '\t' && i < outSize - 1) {
        out[i++] = *p++;
    }
    out[i] = '\0';
    // Reject truncated ids so callers do not proceed with a partial/malformed reference.
    if (*p != '\0' && *p != ')' && *p != ' ' && *p != '\t') {
        return false;
    }
    return i > 0;
}

namespace {

constexpr float SVG_PI = 3.14159265358979323846f;

float DegreesToRadians(float deg)
{
    return deg * SVG_PI / DEGREES_PER_HALF_CIRCLE;
}

const char* ParseTransformNumber(const char* p, float& out)
{
    const char* start = p;
    p = SkipSpacesAndCommas(p);
    if (*p == '\0') {
        return start;
    }
    // Use strtof's end pointer to advance past exactly one number. The previous hand-rolled
    // skip loop consumed the following number's sign too (e.g. "translate(10-5)" lost the "-5"),
    // so negative/implicit-separator coordinates were silently dropped.
    char* end = nullptr;
    float value = strtof(p, &end);
    if (end == nullptr || end == p || !std::isfinite(value)) {
        return start;
    }
    out = value;
    return end;
}

const char* ParseArgsStart(const char* p)
{
    p = SkipSpacesAndCommas(p);
    if (*p == '(') {
        p++;
    }
    return p;
}

const char* ParseArgsEnd(const char* p)
{
    p = SkipSpacesAndCommas(p);
    if (*p == ')') {
        p++;
    }
    return p;
}

bool HasNextNumber(const char* p)
{
    p = SkipSpacesAndCommas(p);
    return (*p != '\0' && ((*p >= '0' && *p <= '9') || *p == '.' || *p == '-' || *p == '+'));
}

// Returns true when the parser is at a point where a missing argument is allowed:
// end of string, closing ')', or only separators remain. Any other character means
// the argument was present but malformed (e.g. a letter where a number was expected).
bool IsMissingValue(const char* p)
{
    p = SkipSpacesAndCommas(p);
    return *p == '\0' || *p == ')';
}

bool ParseTranslate(const char*& p, TransAffine& matrix)
{
    p = ParseArgsStart(p);
    float x = 0.0f;
    float y = 0.0f;
    const char* start = p;
    p = ParseTransformNumber(p, x);
    if (p == start) {
        if (!IsMissingValue(p)) {
            return false;
        }
    } else if (HasNextNumber(p)) {
        start = p;
        p = ParseTransformNumber(p, y);
        if (p == start && !IsMissingValue(p)) {
            return false;
        }
    }
    PostMultiply(matrix, TransAffine::TransAffineTranslation(x, y));
    p = ParseArgsEnd(p);
    return true;
}

bool ParseScale(const char*& p, TransAffine& matrix)
{
    p = ParseArgsStart(p);
    float x = 0.0f;
    float y = 0.0f;
    const char* start = p;
    p = ParseTransformNumber(p, x);
    if (p == start) {
        if (!IsMissingValue(p)) {
            return false;
        }
    } else if (HasNextNumber(p)) {
        start = p;
        p = ParseTransformNumber(p, y);
        if (p == start && !IsMissingValue(p)) {
            return false;
        }
    } else {
        y = x;
    }
    PostMultiply(matrix, TransAffine::TransAffineScaling(x, y));
    p = ParseArgsEnd(p);
    return true;
}

static bool ParseRotateAngle(const char*& p, float& angle)
{
    const char* start = p;
    p = ParseTransformNumber(p, angle);
    if (p == start) {
        return IsMissingValue(p);
    }
    return true;
}

static bool ParseRotateCenter(const char*& p, float& cx, float& cy, bool& hasCenter)
{
    hasCenter = HasNextNumber(p);
    if (!hasCenter) {
        return true;
    }
    const char* start = p;
    p = ParseTransformNumber(p, cx);
    if (p == start) {
        if (!IsMissingValue(p)) {
            return false;
        }
        hasCenter = false;
        return true;
    }
    start = p;
    p = ParseTransformNumber(p, cy);
    if (p == start && !IsMissingValue(p)) {
        return false;
    }
    return true;
}

bool ParseRotate(const char*& p, TransAffine& matrix)
{
    p = ParseArgsStart(p);
    float angle = 0.0f;
    float cx = 0.0f;
    float cy = 0.0f;

    if (!ParseRotateAngle(p, angle)) {
        return false;
    }
    bool hasCenter = false;
    if (!ParseRotateCenter(p, cx, cy, hasCenter)) {
        return false;
    }
    float rad = DegreesToRadians(angle);
    if (hasCenter) {
        PostMultiply(matrix, TransAffine::TransAffineTranslation(cx, cy));
        PostMultiply(matrix, TransAffine::TransAffineRotation(rad));
        PostMultiply(matrix, TransAffine::TransAffineTranslation(-cx, -cy));
    } else {
        PostMultiply(matrix, TransAffine::TransAffineRotation(rad));
    }
    p = ParseArgsEnd(p);
    return true;
}

bool ParseSkewX(const char*& p, TransAffine& matrix)
{
    p = ParseArgsStart(p);
    float angle = 0.0f;
    const char* start = p;
    p = ParseTransformNumber(p, angle);
    if (p == start && !IsMissingValue(p)) {
        return false;
    }
    PostMultiply(matrix, TransAffine(1.0f, 0.0f, tanf(DegreesToRadians(angle)), 1.0f, 0.0f, 0.0f));
    p = ParseArgsEnd(p);
    return true;
}

bool ParseSkewY(const char*& p, TransAffine& matrix)
{
    p = ParseArgsStart(p);
    float angle = 0.0f;
    const char* start = p;
    p = ParseTransformNumber(p, angle);
    if (p == start && !IsMissingValue(p)) {
        return false;
    }
    PostMultiply(matrix, TransAffine(1.0f, tanf(DegreesToRadians(angle)), 0.0f, 1.0f, 0.0f, 0.0f));
    p = ParseArgsEnd(p);
    return true;
}

bool ParseMatrix(const char*& p, TransAffine& matrix)
{
    p = ParseArgsStart(p);
    float a = 0.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 0.0f;
    float e = 0.0f;
    float f = 0.0f;
    const char* start = p;
    p = ParseTransformNumber(p, a);
    if (p == start) {
        if (!IsMissingValue(p)) {
            return false;
        }
    }
    start = p;
    p = ParseTransformNumber(p, b);
    if (p == start && !IsMissingValue(p)) {
        return false;
    }
    start = p;
    p = ParseTransformNumber(p, c);
    if (p == start && !IsMissingValue(p)) {
        return false;
    }
    start = p;
    p = ParseTransformNumber(p, d);
    if (p == start && !IsMissingValue(p)) {
        return false;
    }
    start = p;
    p = ParseTransformNumber(p, e);
    if (p == start && !IsMissingValue(p)) {
        return false;
    }
    start = p;
    p = ParseTransformNumber(p, f);
    if (p == start && !IsMissingValue(p)) {
        return false;
    }
    PostMultiply(matrix, TransAffine(a, b, c, d, e, f));
    p = ParseArgsEnd(p);
    return true;
}

struct TransformCmdEntry {
    const char* name;
    uint8_t len;
    bool (*parse)(const char*&, TransAffine&);
};

bool ParseTransformCommand(const char*& p, TransAffine& matrix)
{
    static const TransformCmdEntry entries[] = {
        { "translate", CMD_TRANSLATE_LEN, ParseTranslate },
        { "scale", CMD_SCALE_LEN, ParseScale },
        { "rotate", CMD_ROTATE_LEN, ParseRotate },
        { "skewX", CMD_SKEW_X_LEN, ParseSkewX },
        { "skewY", CMD_SKEW_Y_LEN, ParseSkewY },
        { "matrix", CMD_MATRIX_LEN, ParseMatrix },
    };
    for (const auto& entry : entries) {
        if (strncmp(p, entry.name, entry.len) == 0 && !IsAsciiAlpha(p[entry.len])) {
            p += entry.len;
            return entry.parse(p, matrix);
        }
    }
    // Unknown token: skip stray identifiers so that a malformed fragment does not
    // prevent later, well-formed commands from being applied (e.g.
    // "rotate(45) junk skewX(30)"). If the token is followed by '(' it looks like
    // an unknown transform function and must fail the whole list.
    while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' && *p != ',' && *p != '(') {
        p++;
    }
    const char* tokenEnd = p;
    const char* next = SkipSpacesAndCommas(tokenEnd);
    if (*next == '(') {
        return false;
    }
    p = tokenEnd;
    return true;
}

bool ParseAlign(const char*& p, uint8_t& align)
{
    struct AlignEntry {
        const char* name;
        uint8_t value;
    };
    const AlignEntry entries[] = {
        { "none", PRESERVE_ALIGN_NONE },
        { "xMinYMin", PRESERVE_ALIGN_XMINYMIN },
        { "xMidYMin", PRESERVE_ALIGN_XMIDYMIN },
        { "xMaxYMin", PRESERVE_ALIGN_XMAXYMIN },
        { "xMinYMid", PRESERVE_ALIGN_XMINYMID },
        { "xMidYMid", PRESERVE_ALIGN_XMIDYMID },
        { "xMaxYMid", PRESERVE_ALIGN_XMAXYMID },
        { "xMinYMax", PRESERVE_ALIGN_XMINYMAX },
        { "xMidYMax", PRESERVE_ALIGN_XMIDYMAX },
        { "xMaxYMax", PRESERVE_ALIGN_XMAXYMAX },
    };
    for (const auto& entry : entries) {
        uint32_t len = strlen(entry.name);
        if (strncmp(p, entry.name, len) == 0) {
            align = entry.value;
            p += len;
            return true;
        }
    }
    return false;
}

} // namespace

// Computes matrix = matrix * rhs. TransAffine::Multiply pre-multiplies (matrix = rhs * matrix),
// which would apply transform lists right to left; the SVG spec composes them left to right.
void PostMultiply(TransAffine& matrix, const TransAffine& rhs)
{
    const float* a = matrix.GetData();
    const float* b = rhs.GetData();
    float r0 = a[MATRIX_INDEX_A] * b[MATRIX_INDEX_A] + a[MATRIX_INDEX_B] * b[MATRIX_INDEX_D];
    float r1 = a[MATRIX_INDEX_A] * b[MATRIX_INDEX_B] + a[MATRIX_INDEX_B] * b[MATRIX_INDEX_E];
    float r2 = a[MATRIX_INDEX_A] * b[MATRIX_INDEX_C] + a[MATRIX_INDEX_B] * b[MATRIX_INDEX_F] +
               a[MATRIX_INDEX_C];
    float r3 = a[MATRIX_INDEX_D] * b[MATRIX_INDEX_A] + a[MATRIX_INDEX_E] * b[MATRIX_INDEX_D];
    float r4 = a[MATRIX_INDEX_D] * b[MATRIX_INDEX_B] + a[MATRIX_INDEX_E] * b[MATRIX_INDEX_E];
    float r5 = a[MATRIX_INDEX_D] * b[MATRIX_INDEX_C] + a[MATRIX_INDEX_E] * b[MATRIX_INDEX_F] +
               a[MATRIX_INDEX_F];
    matrix.SetData(MATRIX_INDEX_A, r0);
    matrix.SetData(MATRIX_INDEX_B, r1);
    matrix.SetData(MATRIX_INDEX_C, r2);
    matrix.SetData(MATRIX_INDEX_D, r3);
    matrix.SetData(MATRIX_INDEX_E, r4);
    matrix.SetData(MATRIX_INDEX_F, r5);
}

bool ParseTransform(const char* str, TransAffine& matrix)
{
    if (str == nullptr) {
        return false;
    }
    matrix.Reset();
    const char* p = str;
    bool hasAny = false;
    while (*p != '\0') {
        p = SkipSpacesAndCommas(p);
        if (*p == '\0') {
            break;
        }
        if (!ParseTransformCommand(p, matrix)) {
            return false;
        }
        hasAny = true;
    }
    return hasAny;
}

bool ParseViewBox(const char* str, float out[4])
{
    if (str == nullptr || out == nullptr) {
        return false;
    }
    const char* p = str;
    float vals[4] = { 0.0f };
    for (uint8_t i = VIEWBOX_INDEX_X; i <= VIEWBOX_INDEX_HEIGHT; i++) {
        p = SkipSpacesAndCommas(p);
        if (*p == '\0') {
            return false;
        }
        const char* before = p;
        vals[i] = strtof(p, const_cast<char**>(&p));
        if (p == before) {
            return false;
        }
    }
    out[VIEWBOX_INDEX_X] = vals[VIEWBOX_INDEX_X];
    out[VIEWBOX_INDEX_Y] = vals[VIEWBOX_INDEX_Y];
    out[VIEWBOX_INDEX_WIDTH] = vals[VIEWBOX_INDEX_WIDTH];
    out[VIEWBOX_INDEX_HEIGHT] = vals[VIEWBOX_INDEX_HEIGHT];
    return vals[VIEWBOX_INDEX_WIDTH] > 0.0f && vals[VIEWBOX_INDEX_HEIGHT] > 0.0f;
}

bool ParsePreserveAspectRatio(const char* str, uint8_t& align, bool& slice)
{
    if (str == nullptr) {
        return false;
    }
    align = PRESERVE_ALIGN_XMIDYMID; // xMidYMid
    slice = false;
    const char* p = str;
    p = SkipSpacesAndCommas(p);
    if (!ParseAlign(p, align)) {
        return false;
    }
    p = SkipSpacesAndCommas(p);
    if (strncmp(p, "slice", SLICE_LEN) == 0) {
        slice = true;
    } else if (strncmp(p, "meet", MEET_LEN) == 0) {
        slice = false;
    }
    return true;
}

namespace {

constexpr uint32_t MS_PER_SECOND = 1000;
constexpr uint32_t MS_PER_MINUTE = 60000;
constexpr uint32_t MS_PER_HOUR = 3600000;
constexpr uint32_t SECONDS_PER_MINUTE = 60;
constexpr uint32_t SECONDS_PER_HOUR = 3600;

uint32_t ToMilliseconds(uint32_t hours, uint32_t minutes, float seconds)
{
    float total = static_cast<float>(hours) * SECONDS_PER_HOUR +
                  static_cast<float>(minutes) * SECONDS_PER_MINUTE + seconds;
    return (total < 0.0f) ? 0U : static_cast<uint32_t>(total * MS_PER_SECOND);
}

uint32_t ParseFullClock(const char* str, const char* colon, const char* secondColon)
{
    char* end = nullptr;
    long hoursRaw = strtol(str, &end, DECIMAL_BASE);
    if (end != colon || hoursRaw > INT32_MAX || hoursRaw < 0) {
        return 0;
    }
    int32_t hours = static_cast<int32_t>(hoursRaw);
    long minutesRaw = strtol(colon + 1, &end, DECIMAL_BASE);
    if (end != secondColon || minutesRaw > INT32_MAX || minutesRaw < 0) {
        return 0;
    }
    int32_t minutes = static_cast<int32_t>(minutesRaw);
    float seconds = strtof(secondColon + 1, nullptr);
    if (seconds < 0.0f) {
        seconds = 0.0f;
    }
    return ToMilliseconds(static_cast<uint32_t>(hours), static_cast<uint32_t>(minutes), seconds);
}

uint32_t ParseMinutesSeconds(const char* str, const char* colon)
{
    char* end = nullptr;
    long minutesRaw = strtol(str, &end, DECIMAL_BASE);
    if (end != colon || minutesRaw > INT32_MAX || minutesRaw < 0) {
        return 0;
    }
    int32_t minutes = static_cast<int32_t>(minutesRaw);
    float seconds = strtof(colon + 1, nullptr);
    if (seconds < 0.0f) {
        seconds = 0.0f;
    }
    float total = static_cast<float>(minutes) * SECONDS_PER_MINUTE + seconds;
    return (total < 0.0f) ? 0U : static_cast<uint32_t>(total * MS_PER_SECOND);
}

uint32_t ParseSimpleClock(const char* str)
{
    float val = ParseFloat(str);
    if (val < 0.0f) {
        return 0;
    }
    if (strstr(str, "ms") != nullptr) {
        return static_cast<uint32_t>(val);
    }
    if (strstr(str, "min") != nullptr) {
        return static_cast<uint32_t>(val * MS_PER_MINUTE);
    }
    if (strchr(str, 'h') != nullptr) {
        return static_cast<uint32_t>(val * MS_PER_HOUR);
    }
    return static_cast<uint32_t>(val * MS_PER_SECOND);
}

} // namespace

uint32_t ParseClockValue(const char* str)
{
    if (str == nullptr) {
        return 0;
    }
    if (strcmp(str, "indefinite") == 0) {
        return UINT32_MAX;
    }
    const char* colon = strchr(str, ':');
    if (colon != nullptr) {
        const char* secondColon = strchr(colon + 1, ':');
        if (secondColon != nullptr) {
            return ParseFullClock(str, colon, secondColon);
        }
        return ParseMinutesSeconds(str, colon);
    }
    return ParseSimpleClock(str);
}

uint32_t ParseSemicolonFloats(const char* str, float*& outArray, bool* sawNonFinite)
{
    outArray = nullptr;
    if (str == nullptr) {
        return 0;
    }
    constexpr uint32_t MAX_VALUES = 64;
    float tmp[MAX_VALUES];
    uint32_t count = 0;
    const char* p = str;
    while (*p != '\0' && count < MAX_VALUES) {
        p = SkipSpacesAndCommas(p);
        if (*p == '\0' || *p == ';') {
            if (*p == ';') {
                p++;
            }
            continue;
        }
        const char* before = p;
        float value = strtof(p, const_cast<char**>(&p));
        if (p == before) {
            break;
        }
        // NaN/Inf are not legal SVG <number> tokens. Reject the whole list instead of
        // keeping the finite prefix: a truncated list would silently desync the value
        // count from keyTimes and produce a plausible but semantically wrong animation.
        if (!std::isfinite(value)) {
            if (sawNonFinite != nullptr) {
                *sawNonFinite = true;
            }
            return 0;
        }
        tmp[count] = value;
        count++;
        p = SkipSpacesAndCommas(p);
        if (*p == ';') {
            p++;
        }
    }
    if (count == 0) {
        return 0;
    }
    outArray = new float[count];
    if (outArray == nullptr) {
        return 0;
    }
    for (uint32_t i = 0; i < count; i++) {
        outArray[i] = tmp[i];
    }
    return count;
}

uint32_t ParseDashArray(const char* str, float*& outArray)
{
    outArray = nullptr;
    if (str == nullptr) {
        return 0;
    }
    if (strcmp(str, "none") == 0) {
        return 0;
    }
    constexpr uint32_t MAX_DASHES = 32;
    float tmp[MAX_DASHES];
    const char* p = str;
    uint32_t count = 0;
    while (*p != '\0' && count < MAX_DASHES) {
        p = SkipSpacesAndCommas(p);
        if (*p == '\0') {
            break;
        }
        const char* before = p;
        tmp[count] = strtof(p, const_cast<char**>(&p));
        if (p == before || tmp[count] < 0.0f) {
            break;
        }
        count++;
    }
    if (count == 0) {
        return 0;
    }
    uint32_t finalCount = count;
    if ((count % DASH_ARRAY_PAIR_FACTOR) != 0) {
        finalCount = count * DASH_ARRAY_PAIR_FACTOR;
    }
    outArray = new float[finalCount];
    if (outArray == nullptr) {
        return 0;
    }
    for (uint32_t i = 0; i < count; i++) {
        outArray[i] = tmp[i];
    }
    if (finalCount > count) {
        for (uint32_t i = count; i < finalCount; i++) {
            outArray[i] = tmp[i - count];
        }
    }
    return finalCount;
}

} // namespace SvgAttributeParser
} // namespace OHOS
