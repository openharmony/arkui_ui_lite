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

#include "svg/svg_shape_nodes.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_path_parser.h"
#include "svg/svg_string_util.h"
#include <cmath>
#include <cstring>

namespace OHOS {

using SvgAttributeParser::MATRIX_INDEX_A;
using SvgAttributeParser::MATRIX_INDEX_B;
using SvgAttributeParser::MATRIX_INDEX_C;
using SvgAttributeParser::MATRIX_INDEX_D;
using SvgAttributeParser::MATRIX_INDEX_E;
using SvgAttributeParser::MATRIX_INDEX_F;

namespace {

const char* SkipSpaces(const char* p)
{
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',') {
        p++;
    }
    return p;
}

const char* ParseNumber(const char* p, float& out)
{
    p = SkipSpaces(p);
    if (*p == '\0') {
        out = 0.0f;
        return p;
    }
    char* end = nullptr;
    out = strtof(p, &end);
    return (end == nullptr || end == p) ? p : end;
}

struct BoundsState {
    float x = 0.0f;
    float y = 0.0f;
    float startX = 0.0f;
    float startY = 0.0f;
    float ctrlX = 0.0f;
    float ctrlY = 0.0f;
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    bool first = true;
};

// Non-owning view into a bounds region. The pointers must outlive the Box.
struct BoundsBox {
    float* minX;
    float* minY;
    float* maxX;
    float* maxY;
};

void UpdateBounds(float x, float y, BoundsBox box)
{
    if (x < *box.minX) {
        *box.minX = x;
    }
    if (x > *box.maxX) {
        *box.maxX = x;
    }
    if (y < *box.minY) {
        *box.minY = y;
    }
    if (y > *box.maxY) {
        *box.maxY = y;
    }
}

void UpdateBounds(float x, float y, BoundsState& state)
{
    UpdateBounds(x, y, BoundsBox { &state.minX, &state.minY, &state.maxX, &state.maxY });
}

char* CopyString(const char* src)
{
    return CopyStringWithLimit(src, SVG_MAX_PATH_LEN);
}

struct ArcBoundsInput {
    float x0;
    float y0;
    float x;
    float y;
    float rx;
    float ry;
    float phi;
    bool largeArc;
    bool sweep;
};

struct ArcBoundsCenter {
    float cx;
    float cy;
    float theta1;
    float deltaTheta;
};

void BoundsMove(const char*& p, BoundsState& state, float dx, float dy)
{
    float nx = 0.0f;
    float ny = 0.0f;
    p = ParseNumber(p, nx);
    p = ParseNumber(p, ny);
    state.x = nx + dx;
    state.y = ny + dy;
    if (state.first) {
        state.minX = state.maxX = state.x;
        state.minY = state.maxY = state.y;
        state.first = false;
    } else {
        UpdateBounds(state.x, state.y, state);
    }
    state.startX = state.x;
    state.startY = state.y;
    state.ctrlX = state.x;
    state.ctrlY = state.y;
}

void BoundsLine(const char*& p, BoundsState& state, float dx, float dy)
{
    float nx = 0.0f;
    float ny = 0.0f;
    p = ParseNumber(p, nx);
    p = ParseNumber(p, ny);
    state.x = nx + dx;
    state.y = ny + dy;
    UpdateBounds(state.x, state.y, state);
    state.ctrlX = state.x;
    state.ctrlY = state.y;
}

void BoundsHorizontal(const char*& p, BoundsState& state, float dx)
{
    float nx = 0.0f;
    p = ParseNumber(p, nx);
    state.x = nx + dx;
    UpdateBounds(state.x, state.y, state);
    state.ctrlX = state.x;
    state.ctrlY = state.y;
}

void BoundsVertical(const char*& p, BoundsState& state, float dy)
{
    float ny = 0.0f;
    p = ParseNumber(p, ny);
    state.y = ny + dy;
    UpdateBounds(state.x, state.y, state);
    state.ctrlX = state.x;
    state.ctrlY = state.y;
}

void BoundsCubic(const char*& p, BoundsState& state, float dx, float dy)
{
    float x1 = 0.0f;
    float y1 = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;
    float nx = 0.0f;
    float ny = 0.0f;
    p = ParseNumber(p, x1);
    p = ParseNumber(p, y1);
    p = ParseNumber(p, x2);
    p = ParseNumber(p, y2);
    p = ParseNumber(p, nx);
    p = ParseNumber(p, ny);
    UpdateBounds(x1 + dx, y1 + dy, state);
    UpdateBounds(x2 + dx, y2 + dy, state);
    state.x = nx + dx;
    state.y = ny + dy;
    UpdateBounds(state.x, state.y, state);
    state.ctrlX = x2 + dx;
    state.ctrlY = y2 + dy;
}

void BoundsSmoothCubic(const char*& p, BoundsState& state, float dx, float dy)
{
    float x2 = 0.0f;
    float y2 = 0.0f;
    float nx = 0.0f;
    float ny = 0.0f;
    p = ParseNumber(p, x2);
    p = ParseNumber(p, y2);
    p = ParseNumber(p, nx);
    p = ParseNumber(p, ny);
    float rx = 2.0f * state.x - state.ctrlX;
    float ry = 2.0f * state.y - state.ctrlY;
    UpdateBounds(rx, ry, state);
    UpdateBounds(x2 + dx, y2 + dy, state);
    state.x = nx + dx;
    state.y = ny + dy;
    UpdateBounds(state.x, state.y, state);
    state.ctrlX = x2 + dx;
    state.ctrlY = y2 + dy;
}

void BoundsQuadratic(const char*& p, BoundsState& state, float dx, float dy)
{
    float x1 = 0.0f;
    float y1 = 0.0f;
    float nx = 0.0f;
    float ny = 0.0f;
    p = ParseNumber(p, x1);
    p = ParseNumber(p, y1);
    p = ParseNumber(p, nx);
    p = ParseNumber(p, ny);
    UpdateBounds(x1 + dx, y1 + dy, state);
    state.x = nx + dx;
    state.y = ny + dy;
    UpdateBounds(state.x, state.y, state);
    state.ctrlX = x1 + dx;
    state.ctrlY = y1 + dy;
}

void BoundsSmoothQuadratic(const char*& p, BoundsState& state, float dx, float dy)
{
    float nx = 0.0f;
    float ny = 0.0f;
    p = ParseNumber(p, nx);
    p = ParseNumber(p, ny);
    float rx = 2.0f * state.x - state.ctrlX;
    float ry = 2.0f * state.y - state.ctrlY;
    UpdateBounds(rx, ry, state);
    state.x = nx + dx;
    state.y = ny + dy;
    UpdateBounds(state.x, state.y, state);
    state.ctrlX = rx;
    state.ctrlY = ry;
}

constexpr float ARC_TWO_PI = 2.0f * static_cast<float>(SVG_PI);
constexpr uint8_t ARC_CANDIDATE_COUNT = 4;

float NormalizeArcAngle(float a)
{
    while (a < 0.0f) {
        a += ARC_TWO_PI;
    }
    while (a >= ARC_TWO_PI) {
        a -= ARC_TWO_PI;
    }
    return a;
}

bool IsAngleOnArc(float a, float start, float delta)
{
    float diff = NormalizeArcAngle(a - start);
    if (delta > 0.0f) {
        return diff <= delta;
    }
    return diff >= (ARC_TWO_PI + delta);
}

void ArcCorrectRadii(float x1p, float y1p, float& rx, float& ry)
{
    float rxSq = rx * rx;
    float rySq = ry * ry;
    float lambda = (x1p * x1p) / rxSq + (y1p * y1p) / rySq;
    if (lambda > 1.0f) {
        float scale = sqrtf(lambda);
        rx *= scale;
        ry *= scale;
    }
}

void ComputeArcCenterForBounds(const ArcBoundsInput& input, ArcBoundsCenter& out)
{
    if (input.rx <= 0.0f || input.ry <= 0.0f) {
        out.cx = (input.x0 + input.x) / 2.0f;
        out.cy = (input.y0 + input.y) / 2.0f;
        out.theta1 = 0.0f;
        out.deltaTheta = 0.0f;
        return;
    }

    float cosPhi = cosf(input.phi);
    float sinPhi = sinf(input.phi);
    float mx = (input.x0 - input.x) / 2.0f;
    float my = (input.y0 - input.y) / 2.0f;
    float x1p = cosPhi * mx + sinPhi * my;
    float y1p = -sinPhi * mx + cosPhi * my;
    float rx = input.rx;
    float ry = input.ry;
    ArcCorrectRadii(x1p, y1p, rx, ry);

    float rxSq = rx * rx;
    float rySq = ry * ry;
    float denom = rxSq * y1p * y1p + rySq * x1p * x1p;
    float factor = 0.0f;
    if (denom > 0.0f) {
        float numer = rxSq * rySq - rxSq * y1p * y1p - rySq * x1p * x1p;
        if (numer >= 0.0f) {
            factor = sqrtf(numer / denom);
        }
    }
    if (input.largeArc == input.sweep) {
        factor = -factor;
    }

    float cxp = factor * rx * y1p / ry;
    float cyp = factor * -ry * x1p / rx;
    out.cx = cosPhi * cxp - sinPhi * cyp + (input.x0 + input.x) / 2.0f;
    out.cy = sinPhi * cxp + cosPhi * cyp + (input.y0 + input.y) / 2.0f;

    float ux = (x1p - cxp) / rx;
    float uy = (y1p - cyp) / ry;
    float vx = (-x1p - cxp) / rx;
    float vy = (-y1p - cyp) / ry;
    out.theta1 = atan2f(uy, ux);
    out.deltaTheta = atan2f(vy, vx) - atan2f(uy, ux);
    while (input.sweep && out.deltaTheta < 0.0f) {
        out.deltaTheta += ARC_TWO_PI;
    }
    while (!input.sweep && out.deltaTheta > 0.0f) {
        out.deltaTheta -= ARC_TWO_PI;
    }
}

struct ArcEllipseParams {
    float rx;
    float ry;
    float cosPhi;
    float sinPhi;
};

void AddArcPoint(const ArcBoundsCenter& center, float t, const ArcEllipseParams& ellipse, BoundsBox box)
{
    float px = center.cx + ellipse.rx * cosf(t) * ellipse.cosPhi - ellipse.ry * sinf(t) * ellipse.sinPhi;
    float py = center.cy + ellipse.rx * cosf(t) * ellipse.sinPhi + ellipse.ry * sinf(t) * ellipse.cosPhi;
    UpdateBounds(px, py, box);
}

void UpdateArcBounds(const ArcBoundsCenter& center, const ArcEllipseParams& ellipse, BoundsBox box)
{
    if (center.deltaTheta > 0.0f ? center.deltaTheta >= ARC_TWO_PI : center.deltaTheta <= -ARC_TWO_PI) {
        float xAmp = sqrtf(ellipse.rx * ellipse.rx * ellipse.cosPhi * ellipse.cosPhi +
                           ellipse.ry * ellipse.ry * ellipse.sinPhi * ellipse.sinPhi);
        float yAmp = sqrtf(ellipse.rx * ellipse.rx * ellipse.sinPhi * ellipse.sinPhi +
                           ellipse.ry * ellipse.ry * ellipse.cosPhi * ellipse.cosPhi);
        UpdateBounds(center.cx - xAmp, center.cy - yAmp, box);
        UpdateBounds(center.cx + xAmp, center.cy + yAmp, box);
        return;
    }

    AddArcPoint(center, center.theta1, ellipse, box);
    AddArcPoint(center, center.theta1 + center.deltaTheta, ellipse, box);

    float xExtrema = atan2f(-ellipse.ry * ellipse.sinPhi, ellipse.rx * ellipse.cosPhi);
    float yExtrema = atan2f(ellipse.ry * ellipse.cosPhi, ellipse.rx * ellipse.sinPhi);
    float candidates[ARC_CANDIDATE_COUNT] = {
        xExtrema,
        xExtrema + static_cast<float>(SVG_PI),
        yExtrema,
        yExtrema + static_cast<float>(SVG_PI)
    };
    for (int i = 0; i < ARC_CANDIDATE_COUNT; i++) {
        if (IsAngleOnArc(candidates[i], center.theta1, center.deltaTheta)) {
            AddArcPoint(center, candidates[i], ellipse, box);
        }
    }
}

void BoundsArc(const char*& p, BoundsState& state, float dx, float dy)
{
    float rx = 0.0f;
    float ry = 0.0f;
    float rotation = 0.0f;
    float largeArc = 0.0f;
    float sweep = 0.0f;
    float nx = 0.0f;
    float ny = 0.0f;
    p = ParseNumber(p, rx);
    p = ParseNumber(p, ry);
    p = ParseNumber(p, rotation);
    p = ParseNumber(p, largeArc);
    p = ParseNumber(p, sweep);
    p = ParseNumber(p, nx);
    p = ParseNumber(p, ny);

    float x0 = state.x;
    float y0 = state.y;
    float endX = nx + dx;
    float endY = ny + dy;
    BoundsBox box = { &state.minX, &state.minY, &state.maxX, &state.maxY };
    UpdateBounds(x0, y0, box);
    UpdateBounds(endX, endY, box);

    if (rx > 0.0f && ry > 0.0f) {
        float phi = rotation * static_cast<float>(SVG_PI) / 180.0f;
        bool largeArcFlag = (largeArc != 0.0f);
        bool sweepFlag = (sweep != 0.0f);
        ArcBoundsInput input = { x0, y0, endX, endY, rx, ry, phi,
                                 largeArcFlag, sweepFlag };
        ArcBoundsCenter center;
        ComputeArcCenterForBounds(input, center);
        ArcEllipseParams ellipse = { rx, ry, cosf(phi), sinf(phi) };
        UpdateArcBounds(center, ellipse, box);
    }

    state.x = endX;
    state.y = endY;
    state.ctrlX = state.x;
    state.ctrlY = state.y;
}

void BoundsClose(BoundsState& state)
{
    state.x = state.startX;
    state.y = state.startY;
    state.ctrlX = state.x;
    state.ctrlY = state.y;
}

bool ApplyBoundsCommand(const char*& p, char& cmd, BoundsState& state)
{
    if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')) {
        cmd = *p++;
    }
    bool relative = (cmd >= 'a' && cmd <= 'z');
    float dx = relative ? state.x : 0.0f;
    float dy = relative ? state.y : 0.0f;
    switch (cmd | 0x20) {
        case 'm':
            BoundsMove(p, state, dx, dy);
            cmd = (cmd == 'm') ? 'l' : 'L';
            return true;
        case 'l':
            BoundsLine(p, state, dx, dy);
            return true;
        case 'h':
            BoundsHorizontal(p, state, dx);
            return true;
        case 'v':
            BoundsVertical(p, state, dy);
            return true;
        case 'c':
            BoundsCubic(p, state, dx, dy);
            return true;
        case 's':
            BoundsSmoothCubic(p, state, dx, dy);
            return true;
        case 'q':
            BoundsQuadratic(p, state, dx, dy);
            return true;
        case 't':
            BoundsSmoothQuadratic(p, state, dx, dy);
            return true;
        case 'a':
            BoundsArc(p, state, dx, dy);
            return true;
        case 'z':
            BoundsClose(state);
            return true;
        default:
            if (*p != '\0') {
                p++;
            }
            return false;
    }
}

bool ComputePathBounds(const char* d, int16_t& minX, int16_t& minY, int16_t& maxX, int16_t& maxY)
{
    if (d == nullptr) {
        return false;
    }
    BoundsState state;
    char cmd = ' ';
    const char* p = d;
    while (*p != '\0') {
        p = SkipSpaces(p);
        if (*p == '\0') {
            break;
        }
        const char* before = p;
        ApplyBoundsCommand(p, cmd, state);
        if (p == before && *p != '\0') {
            p++;
        }
    }
    if (state.first) {
        return false;
    }
    minX = SvgAttributeParser::ClampToInt16(state.minX);
    minY = SvgAttributeParser::ClampToInt16(state.minY);
    maxX = SvgAttributeParser::ClampToInt16(state.maxX);
    maxY = SvgAttributeParser::ClampToInt16(state.maxY);
    return true;
}

void ApplyPaintToPath(UICanvas& canvas, const Paint& paint)
{
    Paint::PaintStyle style = paint.GetStyle();
    if (style == Paint::FILL_STYLE || style == Paint::STROKE_FILL_STYLE ||
        style == Paint::GRADIENT || style == Paint::PATTERN) {
        canvas.FillPathSvg(paint);
    }
    if (style == Paint::STROKE_STYLE || style == Paint::STROKE_FILL_STYLE) {
        canvas.DrawPathSvg(paint);
    }
}

struct CornerRadii {
    int16_t rx;
    int16_t ry;
};

// SVG rect corner radii: each radius is clamped to half the corresponding
// dimension, and a missing radius defaults to the other. The result is two
// independent radii so corners can be elliptical.
CornerRadii ComputeCornerRadii(int16_t rx, int16_t ry, int16_t halfW, int16_t halfH)
{
    if (rx <= 0) {
        rx = ry;
    }
    if (ry <= 0) {
        ry = rx;
    }
    if (rx > halfW) {
        rx = halfW;
    }
    if (ry > halfH) {
        ry = halfH;
    }
    if (rx < 0) {
        rx = 0;
    }
    if (ry < 0) {
        ry = 0;
    }
    return {rx, ry};
}

struct PathRecorder {
    UICanvas& canvas;
    const TransAffine& record;
};

struct ArcRecordParams {
    int16_t x;
    int16_t y;
    int16_t r;
    int16_t startAngle;
    int16_t endAngle;
};

struct RectParams {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
};

struct LineParams {
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
};

void RecordMoveTo(const PathRecorder& pr, int16_t x, int16_t y)
{
    float px = static_cast<float>(x);
    float py = static_cast<float>(y);
    pr.record.Transform(&px, &py);
    pr.canvas.MoveTo({SvgAttributeParser::RoundToInt16(px), SvgAttributeParser::RoundToInt16(py)});
}

void RecordLineTo(const PathRecorder& pr, int16_t x, int16_t y)
{
    float px = static_cast<float>(x);
    float py = static_cast<float>(y);
    pr.record.Transform(&px, &py);
    pr.canvas.LineTo({SvgAttributeParser::RoundToInt16(px), SvgAttributeParser::RoundToInt16(py)});
}

void RecordArcTo(const PathRecorder& pr, const ArcRecordParams& arc)
{
    float px = static_cast<float>(arc.x);
    float py = static_cast<float>(arc.y);
    pr.record.Transform(&px, &py);
    float sx = 1.0f;
    float sy = 1.0f;
    pr.record.ScalingAbs(&sx, &sy);
    int16_t scaledR = SvgAttributeParser::RoundToInt16(static_cast<float>(arc.r) * std::sqrt(sx * sy));
    pr.canvas.ArcTo({SvgAttributeParser::RoundToInt16(px), SvgAttributeParser::RoundToInt16(py)},
                    scaledR, arc.startAngle, arc.endAngle);
}

void RecordRectPath(const PathRecorder& pr, const RectParams& rect, const Paint& paint)
{
    int16_t right = rect.x + rect.w - 1;
    int16_t bottom = rect.y + rect.h - 1;
    pr.canvas.BeginSvgPath();
    RecordMoveTo(pr, rect.x, rect.y);
    RecordLineTo(pr, right, rect.y);
    RecordLineTo(pr, right, bottom);
    RecordLineTo(pr, rect.x, bottom);
    pr.canvas.ClosePath();
    ApplyPaintToPath(pr.canvas, paint);
}

void RecordCubicBezierTo(const PathRecorder& pr, const Point& c1, const Point& c2, const Point& end)
{
    float x1 = static_cast<float>(c1.x);
    float y1 = static_cast<float>(c1.y);
    float x2 = static_cast<float>(c2.x);
    float y2 = static_cast<float>(c2.y);
    float xe = static_cast<float>(end.x);
    float ye = static_cast<float>(end.y);
    pr.record.Transform(&x1, &y1);
    pr.record.Transform(&x2, &y2);
    pr.record.Transform(&xe, &ye);
    pr.canvas.CubicBezierTo({SvgAttributeParser::RoundToInt16(x1), SvgAttributeParser::RoundToInt16(y1)},
                            {SvgAttributeParser::RoundToInt16(x2), SvgAttributeParser::RoundToInt16(y2)},
                            {SvgAttributeParser::RoundToInt16(xe), SvgAttributeParser::RoundToInt16(ye)});
}

// Cubic Bezier approximation constant for a quarter ellipse/circle.
// k = (4/3) * tan(pi/8) ~= 0.551784777.
constexpr float ELLIPSE_ARC_K = 0.551784777f;

void RecordRoundedRectPath(const PathRecorder& pr, const RectParams& rect, int16_t rx, int16_t ry,
                           const Paint& paint)
{
    if (rx <= 0 || ry <= 0) {
        RecordRectPath(pr, rect, paint);
        return;
    }

    int16_t kx = static_cast<int16_t>(static_cast<float>(rx) * ELLIPSE_ARC_K + 0.5f);
    int16_t ky = static_cast<int16_t>(static_cast<float>(ry) * ELLIPSE_ARC_K + 0.5f);

    int16_t x = rect.x;
    int16_t y = rect.y;
    int16_t right = static_cast<int16_t>(rect.x + rect.w - 1);
    int16_t bottom = static_cast<int16_t>(rect.y + rect.h - 1);

    pr.canvas.BeginSvgPath();
    RecordMoveTo(pr, static_cast<int16_t>(x + rx), y);
    RecordLineTo(pr, static_cast<int16_t>(right - rx), y);
    // Top-right corner: top point -> right point.
    RecordCubicBezierTo(pr,
                        {static_cast<int16_t>(right - rx + kx), y},
                        {right, static_cast<int16_t>(y + ry - ky)},
                        {right, static_cast<int16_t>(y + ry)});
    RecordLineTo(pr, right, static_cast<int16_t>(bottom - ry));
    // Bottom-right corner: right point -> bottom point.
    RecordCubicBezierTo(pr,
                        {right, static_cast<int16_t>(bottom - ry + ky)},
                        {static_cast<int16_t>(right - rx + kx), bottom},
                        {static_cast<int16_t>(right - rx), bottom});
    RecordLineTo(pr, static_cast<int16_t>(x + rx), bottom);
    // Bottom-left corner: bottom point -> left point.
    RecordCubicBezierTo(pr,
                        {static_cast<int16_t>(x + rx - kx), bottom},
                        {x, static_cast<int16_t>(bottom - ry + ky)},
                        {x, static_cast<int16_t>(bottom - ry)});
    RecordLineTo(pr, x, static_cast<int16_t>(y + ry));
    // Top-left corner: left point -> top point.
    RecordCubicBezierTo(pr,
                        {x, static_cast<int16_t>(y + ry - ky)},
                        {static_cast<int16_t>(x + rx - kx), y},
                        {static_cast<int16_t>(x + rx), y});
    pr.canvas.ClosePath();
    ApplyPaintToPath(pr.canvas, paint);
}

void RecordLinePath(const PathRecorder& pr, const LineParams& line, const Paint& paint)
{
    pr.canvas.BeginSvgPath();
    RecordMoveTo(pr, line.x1, line.y1);
    RecordLineTo(pr, line.x2, line.y2);
    ApplyPaintToPath(pr.canvas, paint);
}

void RecordPolylinePath(const PathRecorder& pr, const List<Point>& points,
                        bool close, const Paint& paint)
{
    if (points.IsEmpty()) {
        return;
    }
    pr.canvas.BeginSvgPath();
    ListNode<Point>* node = points.Begin();
    RecordMoveTo(pr, node->data_.x, node->data_.y);
    node = node->next_;
    for (; node != points.End(); node = node->next_) {
        RecordLineTo(pr, node->data_.x, node->data_.y);
    }
    if (close && points.Size() > 1) {
        pr.canvas.ClosePath();
    }
    ApplyPaintToPath(pr.canvas, paint);
}

} // namespace

Rect SvgRectNode::GetLocalBounds() const
{
    if (width_ <= 0 || height_ <= 0) {
        return {0, 0, 0, 0};
    }
    int16_t right = x_ + width_ - 1;
    int16_t bottom = y_ + height_ - 1;
    return {x_, y_, right, bottom};
}

bool SvgRectNode::SetGeometryAttribute(const char* name, const char* value)
{
    if (strcmp(name, "x") == 0) {
        x_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "y") == 0) {
        y_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "width") == 0) {
        width_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "height") == 0) {
        height_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "rx") == 0) {
        rx_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "ry") == 0) {
        ry_ = SvgAttributeParser::ParseLength(value);
    } else {
        return false;
    }
    return true;
}

void SvgRectNode::RecordGeometry(UICanvas& canvas)
{
    if (width_ <= 0 || height_ <= 0) {
        return;
    }
    TransAffine record = GetRecordTransform();
    PathRecorder pr = { canvas, record };
    RectParams rect = { x_, y_, width_, height_ };
    CornerRadii radii = ComputeCornerRadii(rx_, ry_, width_ / 2, height_ / 2);
    if (radii.rx > 0 && radii.ry > 0) {
        RecordRoundedRectPath(pr, rect, radii.rx, radii.ry, inheritedPaint_);
    } else {
        RecordRectPath(pr, rect, inheritedPaint_);
    }
}

SvgElementBase* SvgRectNode::Clone() const
{
    SvgRectNode* clone = new SvgRectNode();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->x_ = x_;
    clone->y_ = y_;
    clone->width_ = width_;
    clone->height_ = height_;
    clone->rx_ = rx_;
    clone->ry_ = ry_;
    clone->CopyStateFrom(*this);
    return clone;
}

SvgCircleNode::SvgCircleNode(Mode mode) : mode_(mode) {}

Rect SvgCircleNode::GetLocalBounds() const
{
    if (mode_ == Mode::ELLIPSE) {
        if (rx_ <= 0 || ry_ <= 0) {
            return {0, 0, 0, 0};
        }
        int16_t left = cx_ - rx_;
        int16_t top = cy_ - ry_;
        int16_t right = cx_ + rx_ - 1;
        int16_t bottom = cy_ + ry_ - 1;
        return {left, top, right, bottom};
    }
    if (r_ <= 0) {
        return {0, 0, 0, 0};
    }
    int16_t left = cx_ - r_;
    int16_t top = cy_ - r_;
    int16_t right = cx_ + r_ - 1;
    int16_t bottom = cy_ + r_ - 1;
    return {left, top, right, bottom};
}

bool SvgCircleNode::SetGeometryAttribute(const char* name, const char* value)
{
    if (strcmp(name, "cx") == 0) {
        cx_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "cy") == 0) {
        cy_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "r") == 0) {
        r_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "rx") == 0) {
        rx_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "ry") == 0) {
        ry_ = SvgAttributeParser::ParseLength(value);
    } else {
        return false;
    }
    return true;
}

void SvgCircleNode::RecordGeometry(UICanvas& canvas)
{
    // Record the geometry in LOCAL coordinates and carry the (relative) world transform on the
    // paint, so the rasterizer applies the float CTM at draw time (sub-pixel anti-aliasing). This
    // matches the immediate-mode RM001-RM003 pipeline and keeps circle/ellipse borders smooth;
    // baking the transform into integer device coordinates (and replaying with an identity paint
    // transform) loses the sub-pixel CTM and exposes the polygon facets. The transform is the
    // relative record matrix (world transform minus the leaf's device origin) because OnDraw adds
    // the leaf position back as the replay rect offset, so the absolute placement stays correct.
    TransAffine record = GetRecordTransform();
    const float* d = record.GetData();
    inheritedPaint_.SetTransform(d[MATRIX_INDEX_A], d[MATRIX_INDEX_B], d[MATRIX_INDEX_D], d[MATRIX_INDEX_E],
                                 SvgAttributeParser::ClampToInt16(d[MATRIX_INDEX_C]),
                                 SvgAttributeParser::ClampToInt16(d[MATRIX_INDEX_F]));
    if (mode_ != Mode::ELLIPSE) {
        if (r_ <= 0) {
            return;
        }
        canvas.DrawCircleSvg({cx_, cy_}, static_cast<uint16_t>(r_), inheritedPaint_);
        return;
    }
    if (rx_ <= 0 || ry_ <= 0) {
        return;
    }
    canvas.DrawEllipse({cx_, cy_}, static_cast<uint16_t>(rx_), static_cast<uint16_t>(ry_), inheritedPaint_);
}

TransAffine SvgCircleNode::GetGradientCoordinateTransform() const
{
    // Circle/ellipse vertices are kept in leaf-local space; the rasterizer applies the
    // record transform through the paint. Gradient coordinates must stay in the same local
    // space, so no additional transform is needed here.
    TransAffine identity;
    return identity;
}

SvgElementBase* SvgCircleNode::Clone() const
{
    SvgCircleNode* clone = new SvgCircleNode(mode_);
    if (clone == nullptr) {
        return nullptr;
    }
    clone->cx_ = cx_;
    clone->cy_ = cy_;
    clone->r_ = r_;
    clone->rx_ = rx_;
    clone->ry_ = ry_;
    clone->CopyStateFrom(*this);
    return clone;
}

SvgLineNode::SvgLineNode(Mode mode) : mode_(mode) {}

Rect SvgLineNode::GetLocalBounds() const
{
    if (mode_ == LINE) {
        int16_t left = (x1_ < x2_) ? x1_ : x2_;
        int16_t top = (y1_ < y2_) ? y1_ : y2_;
        int16_t right = (x1_ > x2_) ? x1_ : x2_;
        int16_t bottom = (y1_ > y2_) ? y1_ : y2_;
        return {left, top, right, bottom};
    }
    if (points_.IsEmpty()) {
        return {0, 0, 0, 0};
    }
    int16_t left = INT16_MAX;
    int16_t top = INT16_MAX;
    int16_t right = INT16_MIN;
    int16_t bottom = INT16_MIN;
    ListNode<Point>* node = points_.Begin();
    for (; node != points_.End(); node = node->next_) {
        const Point& p = node->data_;
        if (p.x < left) {
            left = p.x;
        }
        if (p.x > right) {
            right = p.x;
        }
        if (p.y < top) {
            top = p.y;
        }
        if (p.y > bottom) {
            bottom = p.y;
        }
    }
    return {left, top, right, bottom};
}

bool SvgLineNode::SetGeometryAttribute(const char* name, const char* value)
{
    if (mode_ == LINE) {
        if (strcmp(name, "x1") == 0) {
            x1_ = SvgAttributeParser::ParseLength(value);
        } else if (strcmp(name, "y1") == 0) {
            y1_ = SvgAttributeParser::ParseLength(value);
        } else if (strcmp(name, "x2") == 0) {
            x2_ = SvgAttributeParser::ParseLength(value);
        } else if (strcmp(name, "y2") == 0) {
            y2_ = SvgAttributeParser::ParseLength(value);
        } else {
            return false;
        }
        return true;
    }
    if (strcmp(name, "points") == 0) {
        SvgAttributeParser::ParsePoints(value, points_);
        return true;
    }
    return false;
}

void SvgLineNode::RecordGeometry(UICanvas& canvas)
{
    TransAffine record = GetRecordTransform();
    PathRecorder pr = { canvas, record };
    if (mode_ == LINE) {
        RecordLinePath(pr, { x1_, y1_, x2_, y2_ }, inheritedPaint_);
        return;
    }
    if (points_.IsEmpty()) {
        return;
    }
    RecordPolylinePath(pr, points_, mode_ == POLYGON, inheritedPaint_);
}

SvgElementBase* SvgLineNode::Clone() const
{
    SvgLineNode* clone = new SvgLineNode(mode_);
    if (clone == nullptr) {
        return nullptr;
    }
    clone->x1_ = x1_;
    clone->y1_ = y1_;
    clone->x2_ = x2_;
    clone->y2_ = y2_;
    ListNode<Point>* node = points_.Begin();
    for (; node != points_.End(); node = node->next_) {
        clone->points_.PushBack(node->data_);
    }
    clone->CopyStateFrom(*this);
    return clone;
}

SvgPathNode::~SvgPathNode()
{
    delete[] d_;
    d_ = nullptr;
}

Rect SvgPathNode::GetLocalBounds() const
{
    int16_t minX = 0;
    int16_t minY = 0;
    int16_t maxX = 0;
    int16_t maxY = 0;
    if (!ComputePathBounds(d_, minX, minY, maxX, maxY)) {
        return {0, 0, 0, 0};
    }
    return {minX, minY, maxX, maxY};
}

bool SvgPathNode::SetGeometryAttribute(const char* name, const char* value)
{
    if (strcmp(name, "d") == 0) {
        delete[] d_;
        d_ = CopyString(value);
        return true;
    }
    return false;
}

void SvgPathNode::RecordGeometry(UICanvas& canvas)
{
    if (d_ == nullptr) {
        return;
    }
    SvgPathParser parser;
    TransAffine record = GetRecordTransform();
    if (parser.Parse(d_, canvas, &record) != SvgResult::SVG_RESULT_OK) {
        return;
    }
    ApplyPaintToPath(canvas, inheritedPaint_);
}

SvgElementBase* SvgPathNode::Clone() const
{
    SvgPathNode* clone = new SvgPathNode();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->d_ = CopyString(d_);
    clone->CopyStateFrom(*this);
    return clone;
}

} // namespace OHOS
