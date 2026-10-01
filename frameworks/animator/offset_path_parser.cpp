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

#include "animator/offset_path_parser.h"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "gfx_utils/graphic_log.h"

namespace OHOS {
namespace {
/* sample points per Bezier segment (t = 1/8 .. 1, including the end point) */
constexpr uint8_t BEZIER_SAMPLES = 8;
/* temp point limit during parsing: covers 2 Bezier segments plus polyline points,
   truncated with a warning when exceeded */
constexpr uint8_t MAX_TEMP_POINTS = 64;
constexpr float FULL_PROGRESS = 1.0f;

/* coordinate count of one point (x, y) */
constexpr uint8_t COORDS_PER_POINT = 2;
/* skip "px" */
constexpr uint8_t PX_WORDS = 2;

/* float coordinate point, used for parse results, pen position and sample params */
struct PointF {
    float x = 0;
    float y = 0;
};

/* ParseOnePoint reads x,y in one shot through &pt.x, PointF must stay packed */
static_assert(sizeof(PointF) == COORDS_PER_POINT * sizeof(float), "PointF must be packed");

/* separator of SVG path data: whitespace or comma;
 * quotes are also treated as separators since the ace-loader product keeps
 * path("...") quoted, so they are skipped while unwrapping */
bool IsSeparator(char c)
{
    return (c == ' ') || (c == ',') || (c == '\t') || (c == '\n') || (c == '\r') ||
           (c == '"') || (c == '\'');
}

/* skip separators and return the position of the first valid character */
const char *SkipSeparators(const char *p)
{
    while ((p != nullptr) && IsSeparator(*p)) {
        p++;
    }
    return p;
}

/* convert command letter to uppercase (lowercase commands are treated as absolute
   coordinates the same as uppercase in this implementation) */
char ToUpperCmd(char c)
{
    return ((c >= 'a') && (c <= 'z')) ? static_cast<char>(c - ('a' - 'A')) : c;
}

/* check whether the command letter is in the supported subset */
bool IsSupportedCommand(char cmd)
{
    return (cmd == 'M') || (cmd == 'L') || (cmd == 'Q') || (cmd == 'C') || (cmd == 'Z');
}

/* parse one number (negative sign and decimals supported);
   return the end position on success, nullptr on failure */
const char *ParseNumber(const char *p, float &value)
{
    if (p == nullptr) {
        return nullptr;
    }
    p = SkipSeparators(p);
    if ((*p == '\0') || (*p == ')')) {
        return nullptr;
    }
    char *end = nullptr;
    errno = 0;
    value = strtof(p, &end);
    if (errno == ERANGE || value == HUGE_VALF || value == -HUGE_VALF || end == p) {
        return nullptr;
    }

    /* tolerate the "px" suffix (W3C standard is unitless number, handled here for leniency) */
    if ((end[0] == 'p') && (end[1] == 'x')) {
        end += PX_WORDS;
    }
    return end;
}

/* parse count coordinate values in sequence; return the advanced pointer only when all
   values are parsed successfully, otherwise nullptr */
const char *ParseValues(const char *p, float *values, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        p = ParseNumber(p, values[i]);
        if (p == nullptr) {
            return nullptr;
        }
    }
    return p;
}

/* Euclidean distance between two points (rounded to the nearest integer,
   full precision in uint32; the uint16 saturation belongs to the cumulative
   length table layer, see OffsetPathParser::BuildCumulativeLength) */
uint32_t Distance(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    float dx = static_cast<float>(x2 - x1);
    float dy = static_cast<float>(y2 - y1);
    return static_cast<uint32_t>(sqrtf(dx * dx + dy * dy) + 0.5f);
}

/* lexical context shared during path tokenizing */
struct PathContext {
    PointF cur;       // current pen position (start point of a Bezier segment)
    PointF start;     // for Z close path: the latest M point
    char cmd = '\0';
    int16_t tmpX[MAX_TEMP_POINTS] = {0};
    int16_t tmpY[MAX_TEMP_POINTS] = {0};
    uint8_t tmpCount = 0;
};

/* append one point (rounded to int16) to the temp array,
   truncate with a warning when exceeding the limit */
bool AppendPoint(const PointF &pt, PathContext &ctx)
{
    if (ctx.tmpCount >= MAX_TEMP_POINTS) {
        GRAPHIC_LOGW("offset-path points exceed temp limit, truncated");
        return false;
    }
    ctx.tmpX[ctx.tmpCount] = static_cast<int16_t>(pt.x + ((pt.x >= 0) ? 0.5f : -0.5f));
    ctx.tmpY[ctx.tmpCount] = static_cast<int16_t>(pt.y + ((pt.y >= 0) ? 0.5f : -0.5f));
    ctx.tmpCount++;
    return true;
}

/* quadratic Bezier sampling: B(t) = (1-t)^2*P0 + 2(1-t)t*Pc + t^2*P1 */
void SampleQuadratic(const PointF &p0, const PointF &ctrl, const PointF &end, PathContext &ctx)
{
    for (uint8_t i = 1; i <= BEZIER_SAMPLES; i++) {
        float t = static_cast<float>(i) / BEZIER_SAMPLES;
        float u = FULL_PROGRESS - t;
        PointF pt;
        pt.x = u * u * p0.x + 2.0f * u * t * ctrl.x + t * t * end.x;
        pt.y = u * u * p0.y + 2.0f * u * t * ctrl.y + t * t * end.y;
        AppendPoint(pt, ctx);
    }
}

/* cubic Bezier sampling: B(t) = (1-t)^3*P0 + 3(1-t)^2 t*Pc1 + 3(1-t)t^2*Pc2 + t^3*P1 */
void SampleCubic(const PointF &p0, const PointF &startControl, const PointF &endControl,
                 const PointF &end, PathContext &ctx)
{
    for (uint8_t i = 1; i <= BEZIER_SAMPLES; i++) {
        float t = static_cast<float>(i) / BEZIER_SAMPLES;
        float u = FULL_PROGRESS - t;
        PointF pt;
        pt.x = u * u * u * p0.x + 3.0f * u * u * t * startControl.x +
            3.0f * u * t * t * endControl.x + t * t * t * end.x;
        pt.y = u * u * u * p0.y + 3.0f * u * u * t * startControl.y +
            3.0f * u * t * t * endControl.y + t * t * t * end.y;
        AppendPoint(pt, ctx);
    }
}

/* unwrap path("..."): skip leading separators, require the "path" prefix and '(',
   return the body pointer on success */
bool UnwrapPathInput(const char *pathStr, const char *&body)
{
    const char *p = SkipSeparators(pathStr);
    if (strncmp(p, "path", strlen("path")) != 0) {
        GRAPHIC_LOGE("offset-path must start with path(...)");
        return false;
    }
    p = SkipSeparators(p + strlen("path"));
    if (*p != '(') {
        GRAPHIC_LOGE("offset-path missing '('");
        return false;
    }
    body = p + 1;
    return true;
}

/* read a command letter: update ctx.cmd; skip parameters of unsupported commands;
   handle Z close path immediately (Z has no parameters) */
void ReadCommand(PathContext &ctx, const char *&p)
{
    ctx.cmd = ToUpperCmd(*p); // unify to uppercase to simplify following checks
    p++;
    if (!IsSupportedCommand(ctx.cmd)) {
        /* unsupported command (H/V/S/T/A etc.): skip its parameters, warn and continue */
        GRAPHIC_LOGW("unsupported offset-path command, skipped");
        float dummy = 0;
        const char *next = ParseNumber(p, dummy);
        while (next != nullptr) {
            p = next;
            next = ParseNumber(p, dummy);
        }
        return;
    }
    if (ctx.cmd == 'Z') {
        AppendPoint(ctx.start, ctx);
        ctx.cur = ctx.start;
    }
}

/* parse count coordinate values and advance p on success;
   return false without advancing p on failure */
bool ParseAndAdvance(const char *&p, float *values, uint8_t count)
{
    const char *next = ParseValues(p, values, count);
    if (next == nullptr) {
        return false;
    }
    p = next;
    return true;
}

/* parse one point (x y) and advance p on success */
bool ParseOnePoint(const char *&p, PointF &pt)
{
    return ParseAndAdvance(p, &pt.x, COORDS_PER_POINT); // x,y are contiguous in PointF
}

/* parse two points and advance p on success (short-circuit on first failure) */
bool ParseTwoPoints(const char *&p, PointF &pt1, PointF &pt2)
{
    return ParseOnePoint(p, pt1) && ParseOnePoint(p, pt2);
}

/* parse three points and advance p on success (short-circuit on first failure) */
bool ParseThreePoints(const char *&p, PointF &pt1, PointF &pt2, PointF &pt3)
{
    return ParseOnePoint(p, pt1) && ParseOnePoint(p, pt2) && ParseOnePoint(p, pt3);
}

/* number segment: consume points according to ctx.cmd (M/L/Q/C) */
bool ConsumeSegment(PathContext &ctx, const char *&p)
{
    switch (ctx.cmd) {
        case 'M': {
            PointF pt;
            if (!ParseOnePoint(p, pt)) {
                return false;
            }
            ctx.cur = pt;
            ctx.start = pt;
            AppendPoint(pt, ctx);
            ctx.cmd = 'L'; // SVG spec: coordinate pairs after M are treated as L
            break;
        }
        case 'L': {
            PointF end;
            if (!ParseOnePoint(p, end)) {
                return false;
            }
            ctx.cur = end;
            AppendPoint(end, ctx);
            break;
        }
        case 'Q': {
            PointF ctrl;
            PointF end;
            if (!ParseTwoPoints(p, ctrl, end)) {
                return false;
            }
            SampleQuadratic(ctx.cur, ctrl, end, ctx);
            ctx.cur = end;
            break;
        }
        case 'C': {
            PointF startControl;
            PointF endControl;
            PointF end;
            if (!ParseThreePoints(p, startControl, endControl, end)) {
                return false;
            }
            SampleCubic(ctx.cur, startControl, endControl, end, ctx);
            ctx.cur = end;
            break;
        }
        default:
            GRAPHIC_LOGE("offset-path has no valid command");
            return false;
    }
    return true;
}

/* tokenize the path body into sampled polyline points (stored in ctx) */
bool TokenizePath(const char *body, PathContext &ctx)
{
    const char *p = body;
    p = SkipSeparators(p);
    while (*p != '\0' && *p != ')') {
        if (((*p >= 'a') && (*p <= 'z')) || ((*p >= 'A') && (*p <= 'Z'))) {
            ReadCommand(ctx, p);
        } else if (!ConsumeSegment(ctx, p)) {
            return false;
        }
        p = SkipSeparators(p);
    }
    return true;
}

/* thin out: when ctx.tmpCount > PathPolyline::MAX_POINTS, sample by step
   (keep first and last points), fill into out and return the output count */
uint8_t ThinPoints(const PathContext &ctx, PathPolyline &out)
{
    uint8_t outCount = 0;
    if (ctx.tmpCount <= PathPolyline::MAX_POINTS) {
        for (uint8_t i = 0; i < ctx.tmpCount; i++) {
            out.x[outCount] = ctx.tmpX[i];
            out.y[outCount] = ctx.tmpY[i];
            outCount++;
        }
        return outCount;
    }
    float step = static_cast<float>(ctx.tmpCount - 1) / (PathPolyline::MAX_POINTS - 1);
    for (uint8_t i = 0; i < PathPolyline::MAX_POINTS; i++) {
        uint8_t idx = static_cast<uint8_t>(i * step + 0.5f);
        if (idx >= ctx.tmpCount) {
            idx = ctx.tmpCount - 1;
        }
        out.x[outCount] = ctx.tmpX[idx];
        out.y[outCount] = ctx.tmpY[idx];
        outCount++;
    }
    GRAPHIC_LOGW("offset-path points thinned to max limit");
    return outCount;
}

} // namespace


/* build the cumulative length table (saturated to UINT16_MAX to keep monotonicity) */
void OffsetPathParser::BuildCumulativeLength(PathPolyline& out)
{
    out.cumLen[0] = 0;
    uint32_t acc = 0;
    for (uint8_t i = 1; i < out.count; i++) {
        acc += Distance(out.x[i - 1], out.y[i - 1], out.x[i], out.y[i]);
        if (acc > UINT16_MAX) {
            GRAPHIC_LOGW("offset-path length exceeds uint16 limit, saturated");
            acc = UINT16_MAX;
        }
        out.cumLen[i] = static_cast<uint16_t>(acc);
    }
    out.totalLen = static_cast<uint16_t>(acc);
}

bool OffsetPathParser::ParseToPolyline(const char* pathStr, PathPolyline& out)
{
    if (pathStr == nullptr) {
        return false;
    }
    out.totalLen = 0;
    out.count = 0;

    const char *body = nullptr;
    if (!UnwrapPathInput(pathStr, body)) {
        return false;
    }

    PathContext ctx;
    if (!TokenizePath(body, ctx)) {
        return false;
    }
    if (ctx.tmpCount < 2) { // a valid path needs at least 2 points
        GRAPHIC_LOGE("offset-path needs at least 2 points");
        return false;
    }

    out.count = ThinPoints(ctx, out);
    BuildCumulativeLength(out);
    return true;
}

} // namespace OHOS