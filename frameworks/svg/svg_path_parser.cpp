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

#include <cmath>
#include <cstdlib>

#include "svg/svg_path_parser.h"
#include "svg/svg_attribute_parser.h"

namespace OHOS {

namespace {

// Flattening constants used by SvgPathPointAt and by the legacy canvas path storage.
constexpr uint8_t BEZIER_STEPS = 8;
constexpr float BEZIER_QUADRATIC_COEFF = 2.0f;
constexpr float BEZIER_CUBIC_COEFF = 3.0f;

// Degree elevation factor turning a quadratic Bezier into an equivalent cubic one.
constexpr float BEZIER_QUAD_TO_CUBIC = 2.0f / 3.0f;

// Arc approximation constants.
constexpr float DEGREE_PER_RADIAN = 180.0f / static_cast<float>(SVG_PI);
constexpr uint8_t MIN_POLYLINE_POINTS = 2;
constexpr uint16_t MAX_FLAT_POINTS = 512;

// SVG path command parameter counts (number of floating-point values).
constexpr uint8_t PATH_MOVE_COMPONENTS = 2;
constexpr uint8_t PATH_LINE_COMPONENTS = 2;
constexpr uint8_t PATH_HORIZONTAL_COMPONENTS = 1;
constexpr uint8_t PATH_VERTICAL_COMPONENTS = 1;
constexpr uint8_t PATH_CUBIC_COMPONENTS = 6;
constexpr uint8_t PATH_SMOOTH_CUBIC_COMPONENTS = 4;
constexpr uint8_t PATH_QUADRATIC_COMPONENTS = 4;
constexpr uint8_t PATH_SMOOTH_QUADRATIC_COMPONENTS = 2;
constexpr uint8_t PATH_ARC_COMPONENTS = 7;

// Semantic array indices for parsed path-command parameter arrays.
// Move / Line (2 params: x, y)
constexpr uint8_t MOVE_IDX_X = 0;
constexpr uint8_t MOVE_IDX_Y = 1;

// Cubic Bezier (6 params: x1, y1, x2, y2, x, y)
constexpr uint8_t CUBIC_IDX_X1 = 0;
constexpr uint8_t CUBIC_IDX_Y1 = 1;
constexpr uint8_t CUBIC_IDX_X2 = 2;
constexpr uint8_t CUBIC_IDX_Y2 = 3;
constexpr uint8_t CUBIC_IDX_X = 4;
constexpr uint8_t CUBIC_IDX_Y = 5;

// Smooth Cubic (4 params: x2, y2, x, y)
constexpr uint8_t SCUBIC_IDX_X2 = 0;
constexpr uint8_t SCUBIC_IDX_Y2 = 1;
constexpr uint8_t SCUBIC_IDX_X = 2;
constexpr uint8_t SCUBIC_IDX_Y = 3;

// Quadratic (4 params: x1, y1, x, y)
constexpr uint8_t QUAD_IDX_X1 = 0;
constexpr uint8_t QUAD_IDX_Y1 = 1;
constexpr uint8_t QUAD_IDX_X = 2;
constexpr uint8_t QUAD_IDX_Y = 3;

// Smooth Quadratic (2 params: x, y)
constexpr uint8_t SQUAD_IDX_X = 0;
constexpr uint8_t SQUAD_IDX_Y = 1;

// Arc (7 params: rx, ry, rot, large, sweep, x, y)
constexpr uint8_t ARC_IDX_RX = 0;
constexpr uint8_t ARC_IDX_RY = 1;
constexpr uint8_t ARC_IDX_ROT = 2;
constexpr uint8_t ARC_IDX_LARGE = 3;
constexpr uint8_t ARC_IDX_SWEEP = 4;
constexpr uint8_t ARC_IDX_X = 5;
constexpr uint8_t ARC_IDX_Y = 6;

struct PathState {
    float x = 0.0f;
    float y = 0.0f;
    float startX = 0.0f;
    float startY = 0.0f;
    // Control point of the previous C/S/Q/T command, used to reflect for the
    // smooth S/T commands. Any other command (M/L/H/V/A/Z) resets it to the
    // current point, so that reflection yields the current point (SVG spec).
    float ctrlX = 0.0f;
    float ctrlY = 0.0f;
    const TransAffine* transform = nullptr;
};

struct FlattenState {
    // The flattened polyline is kept on the stack. With MAX_FLAT_POINTS=512 and
    // Point being 4 bytes this is ~2KB; callers must ensure adequate stack depth.
    Point points[MAX_FLAT_POINTS];
    uint16_t count = 0;
    float x = 0.0f;
    float y = 0.0f;
    float startX = 0.0f;
    float startY = 0.0f;
    // Control point of the previous C/S/Q/T command, reflected for smooth S/T.
    // Any other command (M/L/H/V/A/Z) resets it to the current point (see FlatLineTo).
    float ctrlX = 0.0f;
    float ctrlY = 0.0f;
};

struct CubicPoints {
    float x1;
    float y1;
    float x2;
    float y2;
    float x;
    float y;
};

struct QuadPoints {
    float x1;
    float y1;
    float x;
    float y;
};

const char* SkipSeparators(const char* p)
{
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',') {
        p++;
    }
    return p;
}

const char* ParseNumber(const char* p, float& out)
{
    p = SkipSeparators(p);
    if (*p == '\0') {
        out = 0.0f;
        return p;
    }
    char* end = nullptr;
    out = strtof(p, &end);
    if (end == p || !std::isfinite(out)) {
        return p;
    }
    return end;
}

// Forward declaration: defined below, but used by the Parse* command helpers above.
bool ParseNumbers(const char*& p, float* out, uint8_t count);

void LineTo(UICanvas& canvas, PathState& state, float x, float y)
{
    float tx = x;
    float ty = y;
    if (state.transform != nullptr) {
        state.transform->Transform(&tx, &ty);
    }
    Point point = {SvgAttributeParser::ClampToInt16(tx), SvgAttributeParser::ClampToInt16(ty)};
    canvas.LineTo(point);
    state.x = x;
    state.y = y;
    state.ctrlX = x;
    state.ctrlY = y;
}

void MoveTo(UICanvas& canvas, PathState& state, float x, float y)
{
    float tx = x;
    float ty = y;
    if (state.transform != nullptr) {
        state.transform->Transform(&tx, &ty);
    }
    Point point = {SvgAttributeParser::ClampToInt16(tx), SvgAttributeParser::ClampToInt16(ty)};
    canvas.MoveTo(point);
    state.x = x;
    state.y = y;
    state.startX = x;
    state.startY = y;
    state.ctrlX = x;
    state.ctrlY = y;
}

void CubicBezier(UICanvas& canvas, PathState& state, const CubicPoints& pts)
{
    float tx1 = pts.x1;
    float ty1 = pts.y1;
    float tx2 = pts.x2;
    float ty2 = pts.y2;
    float tx = pts.x;
    float ty = pts.y;
    if (state.transform != nullptr) {
        state.transform->Transform(&tx1, &ty1);
        state.transform->Transform(&tx2, &ty2);
        state.transform->Transform(&tx, &ty);
    }
    canvas.CubicBezierTo({SvgAttributeParser::ClampToInt16(tx1), SvgAttributeParser::ClampToInt16(ty1)},
                         {SvgAttributeParser::ClampToInt16(tx2), SvgAttributeParser::ClampToInt16(ty2)},
                         {SvgAttributeParser::ClampToInt16(tx), SvgAttributeParser::ClampToInt16(ty)});
    state.x = pts.x;
    state.y = pts.y;
}

void QuadraticBezier(UICanvas& canvas, PathState& state, const QuadPoints& pts)
{
    float x0 = state.x;
    float y0 = state.y;
    float c1x = x0 + BEZIER_QUAD_TO_CUBIC * (pts.x1 - x0);
    float c1y = y0 + BEZIER_QUAD_TO_CUBIC * (pts.y1 - y0);
    float c2x = pts.x + BEZIER_QUAD_TO_CUBIC * (pts.x1 - pts.x);
    float c2y = pts.y + BEZIER_QUAD_TO_CUBIC * (pts.y1 - pts.y);
    CubicBezier(canvas, state, {c1x, c1y, c2x, c2y, pts.x, pts.y});
}

bool FlatLineTo(FlattenState& state, float x, float y)
{
    if (state.count >= MAX_FLAT_POINTS) {
        return false;
    }
    state.points[state.count++] = {SvgAttributeParser::ClampToInt16(x), SvgAttributeParser::ClampToInt16(y)};
    state.x = x;
    state.y = y;
    // Reset the smooth control point to the current point (SVG spec): a following S/T
    // with no preceding C/Q reflects to the current point.
    state.ctrlX = x;
    state.ctrlY = y;
    return true;
}

bool FlatMoveTo(FlattenState& state, float x, float y)
{
    bool ok = FlatLineTo(state, x, y);
    state.startX = x;
    state.startY = y;
    return ok;
}

bool FlatCubic(FlattenState& state, const CubicPoints& pts)
{
    float x0 = state.x;
    float y0 = state.y;
    for (uint8_t i = 1; i <= BEZIER_STEPS; i++) {
        float t = static_cast<float>(i) / static_cast<float>(BEZIER_STEPS);
        float mt = 1.0f - t;
        float bx = mt * mt * mt * x0 + BEZIER_CUBIC_COEFF * mt * mt * t * pts.x1 +
                   BEZIER_CUBIC_COEFF * mt * t * t * pts.x2 + t * t * t * pts.x;
        float by = mt * mt * mt * y0 + BEZIER_CUBIC_COEFF * mt * mt * t * pts.y1 +
                   BEZIER_CUBIC_COEFF * mt * t * t * pts.y2 + t * t * t * pts.y;
        if (!FlatLineTo(state, bx, by)) {
            return false;
        }
    }
    return true;
}

bool FlatQuadratic(FlattenState& state, const QuadPoints& pts)
{
    float x0 = state.x;
    float y0 = state.y;
    for (uint8_t i = 1; i <= BEZIER_STEPS; i++) {
        float t = static_cast<float>(i) / static_cast<float>(BEZIER_STEPS);
        float mt = 1.0f - t;
        float bx = mt * mt * x0 + BEZIER_QUADRATIC_COEFF * mt * t * pts.x1 + t * t * pts.x;
        float by = mt * mt * y0 + BEZIER_QUADRATIC_COEFF * mt * t * pts.y1 + t * t * pts.y;
        if (!FlatLineTo(state, bx, by)) {
            return false;
        }
    }
    return true;
}

bool ParseMove(const char*& p, UICanvas& canvas, PathState& state, float dx, float dy)
{
    float vals[PATH_MOVE_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_MOVE_COMPONENTS)) {
        return false;
    }
    MoveTo(canvas, state, vals[MOVE_IDX_X] + dx, vals[MOVE_IDX_Y] + dy);
    return true;
}

bool ParseLine(const char*& p, UICanvas& canvas, PathState& state, float dx, float dy)
{
    float vals[PATH_LINE_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_LINE_COMPONENTS)) {
        return false;
    }
    LineTo(canvas, state, vals[MOVE_IDX_X] + dx, vals[MOVE_IDX_Y] + dy);
    return true;
}

bool ParseHorizontal(const char*& p, UICanvas& canvas, PathState& state, float dx)
{
    float x = 0.0f;
    if (!ParseNumbers(p, &x, PATH_HORIZONTAL_COMPONENTS)) {
        return false;
    }
    LineTo(canvas, state, x + dx, state.y);
    return true;
}

bool ParseVertical(const char*& p, UICanvas& canvas, PathState& state, float dy)
{
    float y = 0.0f;
    if (!ParseNumbers(p, &y, PATH_VERTICAL_COMPONENTS)) {
        return false;
    }
    LineTo(canvas, state, state.x, y + dy);
    return true;
}

bool ParseCubic(const char*& p, UICanvas& canvas, PathState& state, float dx, float dy)
{
    float vals[PATH_CUBIC_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_CUBIC_COMPONENTS)) {
        return false;
    }
    CubicBezier(canvas, state,
                {vals[CUBIC_IDX_X1] + dx, vals[CUBIC_IDX_Y1] + dy,
                 vals[CUBIC_IDX_X2] + dx, vals[CUBIC_IDX_Y2] + dy,
                 vals[CUBIC_IDX_X] + dx, vals[CUBIC_IDX_Y] + dy});
    state.ctrlX = vals[CUBIC_IDX_X2] + dx;
    state.ctrlY = vals[CUBIC_IDX_Y2] + dy;
    return true;
}

bool ParseSmoothCubic(const char*& p, UICanvas& canvas, PathState& state, float dx, float dy)
{
    float vals[PATH_SMOOTH_CUBIC_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_SMOOTH_CUBIC_COMPONENTS)) {
        return false;
    }
    float rx = 2.0f * state.x - state.ctrlX;
    float ry = 2.0f * state.y - state.ctrlY;
    CubicBezier(canvas, state,
                {rx, ry, vals[SCUBIC_IDX_X2] + dx, vals[SCUBIC_IDX_Y2] + dy,
                 vals[SCUBIC_IDX_X] + dx, vals[SCUBIC_IDX_Y] + dy});
    state.ctrlX = vals[SCUBIC_IDX_X2] + dx;
    state.ctrlY = vals[SCUBIC_IDX_Y2] + dy;
    return true;
}

bool ParseQuadratic(const char*& p, UICanvas& canvas, PathState& state, float dx, float dy)
{
    float vals[PATH_QUADRATIC_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_QUADRATIC_COMPONENTS)) {
        return false;
    }
    QuadraticBezier(canvas, state,
                    {vals[QUAD_IDX_X1] + dx, vals[QUAD_IDX_Y1] + dy,
                     vals[QUAD_IDX_X] + dx, vals[QUAD_IDX_Y] + dy});
    state.ctrlX = vals[QUAD_IDX_X1] + dx;
    state.ctrlY = vals[QUAD_IDX_Y1] + dy;
    return true;
}

bool ParseSmoothQuadratic(const char*& p, UICanvas& canvas, PathState& state, float dx, float dy)
{
    float vals[PATH_SMOOTH_QUADRATIC_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_SMOOTH_QUADRATIC_COMPONENTS)) {
        return false;
    }
    float rx = 2.0f * state.x - state.ctrlX;
    float ry = 2.0f * state.y - state.ctrlY;
    QuadraticBezier(canvas, state,
                    {rx, ry, vals[SQUAD_IDX_X] + dx, vals[SQUAD_IDX_Y] + dy});
    state.ctrlX = rx;
    state.ctrlY = ry;
    return true;
}

bool ParseArc(const char*& p, UICanvas& canvas, PathState& state, float dx, float dy)
{
    float vals[PATH_ARC_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_ARC_COMPONENTS)) {
        return false;
    }
    UICanvas::SvgArcArgs args = {
        state.x, state.y, vals[ARC_IDX_RX], vals[ARC_IDX_RY], vals[ARC_IDX_ROT],
        static_cast<int16_t>(vals[ARC_IDX_LARGE]) != 0,
        static_cast<int16_t>(vals[ARC_IDX_SWEEP]) != 0,
        vals[ARC_IDX_X] + dx, vals[ARC_IDX_Y] + dy,
        state.transform
    };
    canvas.SvgArcTo(args);
    state.x = vals[ARC_IDX_X] + dx;
    state.y = vals[ARC_IDX_Y] + dy;
    state.ctrlX = state.x;
    state.ctrlY = state.y;
    return true;
}

bool ParseClose(UICanvas& canvas, PathState& state)
{
    canvas.ClosePath();
    state.x = state.startX;
    state.y = state.startY;
    state.ctrlX = state.startX;
    state.ctrlY = state.startY;
    return true;
}

constexpr char ASCII_CASE_MASK = 0x20;

inline char ToLowerCmd(char c)
{
    return c | ASCII_CASE_MASK;
}

// Parse a fixed number of floats from the path data. Returns false if any number
// could not be read (p did not advance).
bool ParseNumbers(const char*& p, float* out, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        const char* start = p;
        p = ParseNumber(p, out[i]);
        if (p == start) {
            return false;
        }
    }
    return true;
}

bool ParseCommand(char cmd, const char*& p, UICanvas& canvas, PathState& state)
{
    bool relative = (cmd >= 'a' && cmd <= 'z');
    float dx = relative ? state.x : 0.0f;
    float dy = relative ? state.y : 0.0f;
    char lower = ToLowerCmd(cmd);
    switch (lower) {
        case 'm':
            return ParseMove(p, canvas, state, dx, dy);
        case 'l':
            return ParseLine(p, canvas, state, dx, dy);
        case 'h':
            return ParseHorizontal(p, canvas, state, dx);
        case 'v':
            return ParseVertical(p, canvas, state, dy);
        case 'c':
            return ParseCubic(p, canvas, state, dx, dy);
        case 's':
            return ParseSmoothCubic(p, canvas, state, dx, dy);
        case 'q':
            return ParseQuadratic(p, canvas, state, dx, dy);
        case 't':
            return ParseSmoothQuadratic(p, canvas, state, dx, dy);
        case 'a':
            return ParseArc(p, canvas, state, dx, dy);
        case 'z':
            return ParseClose(canvas, state);
        default:
            p++;
            return false;
    }
}

bool FlatParseMove(const char*& p, FlattenState& state, float dx, float dy)
{
    float vals[PATH_MOVE_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_MOVE_COMPONENTS)) {
        return false;
    }
    return FlatMoveTo(state, vals[MOVE_IDX_X] + dx, vals[MOVE_IDX_Y] + dy);
}

bool FlatParseLine(const char*& p, FlattenState& state, float dx, float dy)
{
    float vals[PATH_LINE_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_LINE_COMPONENTS)) {
        return false;
    }
    return FlatLineTo(state, vals[MOVE_IDX_X] + dx, vals[MOVE_IDX_Y] + dy);
}

bool FlatParseHorizontal(const char*& p, FlattenState& state, float dx)
{
    float x = 0.0f;
    if (!ParseNumbers(p, &x, PATH_HORIZONTAL_COMPONENTS)) {
        return false;
    }
    return FlatLineTo(state, x + dx, state.y);
}

bool FlatParseVertical(const char*& p, FlattenState& state, float dy)
{
    float y = 0.0f;
    if (!ParseNumbers(p, &y, PATH_VERTICAL_COMPONENTS)) {
        return false;
    }
    return FlatLineTo(state, state.x, y + dy);
}

bool FlatParseCubic(const char*& p, FlattenState& state, float dx, float dy)
{
    float vals[PATH_CUBIC_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_CUBIC_COMPONENTS)) {
        return false;
    }
    if (!FlatCubic(state, {vals[CUBIC_IDX_X1] + dx, vals[CUBIC_IDX_Y1] + dy,
                             vals[CUBIC_IDX_X2] + dx, vals[CUBIC_IDX_Y2] + dy,
                             vals[CUBIC_IDX_X] + dx, vals[CUBIC_IDX_Y] + dy})) {
        return false;
    }
    state.ctrlX = vals[CUBIC_IDX_X2] + dx;
    state.ctrlY = vals[CUBIC_IDX_Y2] + dy;
    return true;
}

bool FlatParseQuadratic(const char*& p, FlattenState& state, float dx, float dy)
{
    float vals[PATH_QUADRATIC_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_QUADRATIC_COMPONENTS)) {
        return false;
    }
    if (!FlatQuadratic(state, {vals[QUAD_IDX_X1] + dx, vals[QUAD_IDX_Y1] + dy,
                                 vals[QUAD_IDX_X] + dx, vals[QUAD_IDX_Y] + dy})) {
        return false;
    }
    state.ctrlX = vals[QUAD_IDX_X1] + dx;
    state.ctrlY = vals[QUAD_IDX_Y1] + dy;
    return true;
}

bool FlatParseSmoothCubic(const char*& p, FlattenState& state, float dx, float dy)
{
    float vals[PATH_SMOOTH_CUBIC_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_SMOOTH_CUBIC_COMPONENTS)) {
        return false;
    }
    float rx = 2.0f * state.x - state.ctrlX;
    float ry = 2.0f * state.y - state.ctrlY;
    if (!FlatCubic(state, {rx, ry, vals[SCUBIC_IDX_X2] + dx, vals[SCUBIC_IDX_Y2] + dy,
                             vals[SCUBIC_IDX_X] + dx, vals[SCUBIC_IDX_Y] + dy})) {
        return false;
    }
    state.ctrlX = vals[SCUBIC_IDX_X2] + dx;
    state.ctrlY = vals[SCUBIC_IDX_Y2] + dy;
    return true;
}

bool FlatParseSmoothQuadratic(const char*& p, FlattenState& state, float dx, float dy)
{
    float vals[PATH_SMOOTH_QUADRATIC_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_SMOOTH_QUADRATIC_COMPONENTS)) {
        return false;
    }
    float rx = 2.0f * state.x - state.ctrlX;
    float ry = 2.0f * state.y - state.ctrlY;
    if (!FlatQuadratic(state, {rx, ry, vals[SQUAD_IDX_X] + dx, vals[SQUAD_IDX_Y] + dy})) {
        return false;
    }
    state.ctrlX = rx;
    state.ctrlY = ry;
    return true;
}

bool FlatParseArc(const char*& p, FlattenState& state, float dx, float dy)
{
    // The flattened path does not implement true arc tessellation; the 5 arc
    // parameters (rx, ry, rotation, largeArc, sweep) are parsed but discarded,
    // and the command is approximated as a straight line to the endpoint.
    // This affects path-length and tangent accuracy for <animateMotion> on arcs.
    float vals[PATH_ARC_COMPONENTS] = {0.0f};
    if (!ParseNumbers(p, vals, PATH_ARC_COMPONENTS)) {
        return false;
    }
    if (!FlatLineTo(state, vals[ARC_IDX_X] + dx, vals[ARC_IDX_Y] + dy)) {
        return false;
    }
    // SVG spec: an arc does not establish a smooth control point.
    // Reset the reflection point so a following S/T reflects to the current point.
    state.ctrlX = state.x;
    state.ctrlY = state.y;
    return true;
}

bool FlatParseClose(FlattenState& state)
{
    return FlatLineTo(state, state.startX, state.startY);
}

bool FlatParseCommand(char cmd, const char*& p, FlattenState& state)
{
    bool relative = (cmd >= 'a' && cmd <= 'z');
    float dx = relative ? state.x : 0.0f;
    float dy = relative ? state.y : 0.0f;
    switch (ToLowerCmd(cmd)) {
        case 'm':
            return FlatParseMove(p, state, dx, dy);
        case 'l':
            return FlatParseLine(p, state, dx, dy);
        case 'h':
            return FlatParseHorizontal(p, state, dx);
        case 'v':
            return FlatParseVertical(p, state, dy);
        case 'c':
            return FlatParseCubic(p, state, dx, dy);
        case 's':
            return FlatParseSmoothCubic(p, state, dx, dy);
        case 'q':
            return FlatParseQuadratic(p, state, dx, dy);
        case 't':
            return FlatParseSmoothQuadratic(p, state, dx, dy);
        case 'a':
            return FlatParseArc(p, state, dx, dy);
        case 'z':
            return FlatParseClose(state);
        default:
            p++;
            return false;
    }
}

// Forward declaration for the generic path-data loop used by FlattenPath.
template <typename State, typename Fn>
bool PathDataLoop(const char* d, State& state, Fn&& fn);

bool FlattenPath(const char* d, FlattenState& state)
{
    bool ok = PathDataLoop(d, state, [](char cmd, const char*& p, FlattenState& state) -> bool {
        return FlatParseCommand(cmd, p, state);
    });
    if (!ok) {
        return false;
    }
    return state.count >= MIN_POLYLINE_POINTS;
}

// Computes the total length of the flattened polyline.
// NOTE: this duplicates the per-segment length math in SamplePolyline. With
// MAX_FLAT_POINTS capped at 512 the cost is negligible; merging would add
// complexity (e.g. a cached length table) without meaningful benefit.
float ComputePolylineLength(const FlattenState& state)
{
    float total = 0.0f;
    for (uint16_t i = 1; i < state.count; i++) {
        float dx = static_cast<float>(state.points[i].x - state.points[i - 1].x);
        float dy = static_cast<float>(state.points[i].y - state.points[i - 1].y);
        total += sqrtf(dx * dx + dy * dy);
    }
    return total;
}

bool SamplePolyline(const FlattenState& state, float target, float& x, float& y, float& angle)
{
    if (state.count == 0) {
        return false;
    }
    float accumulated = 0.0f;
    for (uint16_t i = 1; i < state.count; i++) {
        float dx = static_cast<float>(state.points[i].x - state.points[i - 1].x);
        float dy = static_cast<float>(state.points[i].y - state.points[i - 1].y);
        float segLen = sqrtf(dx * dx + dy * dy);
        if (segLen > 0.0f && accumulated + segLen >= target) {
            float t = (target - accumulated) / segLen;
            x = state.points[i - 1].x + dx * t;
            y = state.points[i - 1].y + dy * t;
            angle = atan2f(dy, dx) * DEGREE_PER_RADIAN;
            return true;
        }
        accumulated += segLen;
    }
    // Target lies beyond the polyline (e.g. due to floating-point rounding).
    // Report failure so the caller does not use an incorrect angle.
    return false;
}

// Generic path-data token loop shared by Parse (canvas recording) and FlattenPath
// (polyline sampling). The caller supplies a functor that executes one command.
template <typename State, typename Fn>
bool PathDataLoop(const char* d, State& state, Fn&& fn)
{
    if (d == nullptr) {
        return false;
    }
    char cmd = ' ';
    const char* p = d;
    while (*p != '\0') {
        p = SkipSeparators(p);
        if (*p == '\0') {
            break;
        }
        bool consumedLetter = false;
        if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')) {
            cmd = *p++;
            consumedLetter = true;
        }
        const char* before = p;
        if (!fn(cmd, p, state)) {
            return false;
        }
        if (p == before && !consumedLetter) {
            if (*p == '\0') {
                break;
            }
            p++;
        }
        if (cmd == 'M' || cmd == 'm') {
            cmd = (cmd == 'm') ? 'l' : 'L';
        }
    }
    return true;
}

} // namespace

SvgResult SvgPathParser::Parse(const char* data, UICanvas& canvas, const TransAffine* transform)
{
    if (data == nullptr) {
        return SvgResult::SVG_RESULT_INVALID_PARAM;
    }
    data_ = data;
    PathState state;
    state.transform = transform;
    canvas.BeginSvgPath();
    bool ok = PathDataLoop(data, state, [&](char cmd, const char*& p, PathState& state) -> bool {
        if (!ParseCommand(cmd, p, canvas, state)) {
            // Reset the canvas path state so a partially recorded malformed path
            // does not leak into subsequent drawing.
            canvas.BeginSvgPath();
            return false;
        }
        return true;
    });
    if (!ok) {
        return SvgResult::SVG_RESULT_ERROR;
    }
    return SvgResult::SVG_RESULT_OK;
}

bool SvgPathPointAt(const char* d, float progress, float& x, float& y, float& angle)
{
    FlattenState state;
    if (!FlattenPath(d, state)) {
        return false;
    }
    if (progress < 0.0f) {
        progress = 0.0f;
    }
    if (progress > 1.0f) {
        progress = 1.0f;
    }
    // FlattenPath guarantees count >= MIN_POLYLINE_POINTS; guard anyway so
    // ComputePolylineLength / SamplePolyline never operate on degenerate data.
    if (state.count < MIN_POLYLINE_POINTS) {
        return false;
    }
    float total = ComputePolylineLength(state);
    if (total <= 0.0f) {
        return false;
    }
    return SamplePolyline(state, progress * total, x, y, angle);
}

} // namespace OHOS
