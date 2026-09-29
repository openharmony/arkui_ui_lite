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

/**
 * @file linear_gradient_builder.cpp
 * @brief Implementation of the LinearGradientBuilder class.
 *
 * Core algorithm - gradient line endpoints:
 *
 * CSS angle convention:
 *   0 degrees   = pointing up    (to top)
 *   90 degrees  = pointing right (to right)
 *   180 degrees = pointing down  (to bottom, the CSS default)
 *   270 degrees = pointing left  (to left)
 *
 * The gradient line runs through the center of the drawing area and its
 * endpoints are the extreme projections of the rectangle onto the gradient
 * direction, so that the color ramp covers the rectangle exactly once:
 *
 *   1. CSS angle to math angle: mathDeg = 90 - cssDeg
 *   2. Math angle to radians:   rad     = mathDeg * PI / 180
 *   3. Direction vector:        (cos(rad), sin(rad))
 *   4. Projected half diagonal: halfLen = |w/2 * cos| + |h/2 * sin|
 *   5. start = center - direction * halfLen
 *   6. end   = center + direction * halfLen
 *   The screen Y axis grows downwards, so the Y component is negated.
 */

#include "linear_gradient_builder.h"
#include "gfx_utils/gradient_info.h"
#include "gfx_utils/graphic_log.h"
#include "gfx_utils/graphic_math.h"
#include <cmath>

namespace OHOS {
namespace {

/** Divisor used to take the middle of an edge or the center of the area. */
constexpr float HALF_DIVISOR = 2.0f;

/** Number of color stops a GradientColor union is able to carry. */
constexpr uint8_t GRADIENT_COLOR_UNION_STOP_COUNT = 2;

/** Index of the begin color inside a two stop gradient. */
constexpr uint8_t GRADIENT_BEGIN_STOP_INDEX = 0;

/** Index of the end color inside a two stop gradient. */
constexpr uint8_t GRADIENT_END_STOP_INDEX = 1;

/** GradientColor::position is unused by the linear path, keep it neutral. */
constexpr uint8_t GRADIENT_COLOR_UNUSED_POSITION = 0;

/** Origin of the local coordinate system used to build the fill path. */
constexpr int16_t LOCAL_ORIGIN = 0;

/**
 * @brief Convert a CSS angle in degrees to the matching math angle in radians.
 *
 * @param cssAngleDeg CSS angle in degrees.
 * @return the equivalent math angle expressed in radians.
 */
inline float CssAngleToMathRadians(float cssAngleDeg)
{
    const float mathDeg = static_cast<float>(QUARTER_IN_DEGREE) - cssAngleDeg;
    return mathDeg * static_cast<float>(UI_PI) / static_cast<float>(SEMICIRCLE_IN_DEGREE);
}
} // namespace

bool LinearGradientBuilder::CalcEndpoints(float cssAngleDeg,
                                          int16_t width,
                                          int16_t height,
                                          Point& startPt,
                                          Point& endPt)
{
    /* An empty area has no gradient line; report the failure to the caller. */
    if (width <= 0 || height <= 0) {
        return false;
    }

    /* A non finite angle would poison every downstream computation. */
    if (std::isnan(cssAngleDeg)) {
        return false;
    }

    /* Steps 1 and 2: CSS angle -> math angle -> radians. */
    const float rad = CssAngleToMathRadians(cssAngleDeg);

    /* Step 3: unit direction vector of the gradient line. */
    const float cosA = cosf(rad);
    const float sinA = sinf(rad);

    /* Center of the drawing area, in local coordinates. */
    const float halfWidth = static_cast<float>(width) / HALF_DIVISOR;
    const float halfHeight = static_cast<float>(height) / HALF_DIVISOR;

    /* Step 4: half diagonal projected onto the gradient direction. */
    const float halfLen = fabsf(halfWidth * cosA) + fabsf(halfHeight * sinA);

    /*
     * Steps 5 and 6: the Y axis grows downwards, so sinA is negated on Y.
     * The two offsets are computed once and reused by both endpoints; storing
     * them also keeps MATH_ROUND() cheap, because that macro evaluates its
     * argument more than once.
     */
    const float offsetX = cosA * halfLen;
    const float offsetY = sinA * halfLen;
    const float startX = halfWidth - offsetX;
    const float startY = halfHeight + offsetY;
    const float endX = halfWidth + offsetX;
    const float endY = halfHeight - offsetY;

    /*
     * MATH_ROUND() is used instead of a plain static_cast<int16_t>: truncation
     * always pulls a coordinate towards zero, which breaks the mirror symmetry
     * of the two endpoints around the center (a 30 degree ramp over a 100 pixel
     * wide area truncates to [15, 84], whose midpoint is 49.5 instead of 50) and
     * shifts the whole color ramp by up to one pixel.
     */
    startPt.x = MATH_ROUND(startX);
    startPt.y = MATH_ROUND(startY);
    endPt.x = MATH_ROUND(endX);
    endPt.y = MATH_ROUND(endY);

    return true;
}

GradientRenderParams LinearGradientBuilder::Build(const GradientInfo* info, int16_t width, int16_t height)
{
    GradientRenderParams params;
    params.valid = false;

    /* A linear gradient needs at least a begin and an end color. */
    if ((info == nullptr) || (info->colorStops == nullptr) || (!info->isValid) ||
        (info->colorCount < GRADIENT_MIN_COLOR_STOP_COUNT)) {
        return params;
    }

    if (width <= 0 || height <= 0) {
        return params;
    }

    /*
     * Custom angles must be finite before normalization: GradientInfo::ResolveCssAngle()
     * intentionally degrades NaN to the default angle to keep the UI safe, but the
     * builder must reject such payloads instead of rendering an undefined direction.
     */
    if (info->direction == CssGradientDirection::CUSTOM_ANGLE && std::isnan(info->angle)) {
        return params;
    }

    /* Resolve the effective CSS angle. */
    const float cssAngle = info->ResolveCssAngle();

    /* Endpoints are local to the drawing area. */
    Point relStart;
    Point relEnd;
    if (!CalcEndpoints(cssAngle, width, height, relStart, relEnd)) {
        return params;
    }

    /*
     * Build the paint. Endpoints and path vertices share the local coordinate
     * system whose origin is the top left corner of the drawing area;
     * DrawCanvas::RenderGradientFill() applies the translation to absolute
     * coordinates through its internal transform.
     */
    params.paint.SetStyle(Paint::GRADIENT);
    params.paint.createLinearGradient(static_cast<float>(relStart.x), static_cast<float>(relStart.y),
                                      static_cast<float>(relEnd.x), static_cast<float>(relEnd.y));
    for (uint8_t i = 0; i < info->colorCount; i++) {
        float stopOffset = info->colorStops[i].offset;
        /*
         * FillGradientLut::ColorPoint hard clamps every offset into [0, 1], so a
         * stop still sitting outside the painted segment would silently collapse
         * onto an endpoint and paint a ramp that violates the W3C rules for out
         * of range color stops. The CSS layer is expected to have projected them
         * already (CssGradientParser::ClipColorStopsToGradientLine); reaching
         * this branch means the payload bypassed that step, which is a defect
         * worth reporting before applying the same clamp the LUT would.
         */
        if (stopOffset < GRADIENT_OFFSET_MIN || stopOffset > GRADIENT_OFFSET_MAX) {
            GRAPHIC_LOGE("LinearGradientBuilder::Build: color stop offset %f out of range [%.1f, %.1f], "
                         "clamped (status=css_projection_missing, index=%u)",
                         stopOffset, GRADIENT_OFFSET_MIN, GRADIENT_OFFSET_MAX, static_cast<unsigned>(i));
            const float clamped = (stopOffset < GRADIENT_OFFSET_MIN) ? GRADIENT_OFFSET_MIN : GRADIENT_OFFSET_MAX;
            stopOffset = clamped;
        }
        params.paint.addColorStop(stopOffset, info->colorStops[i].color);
    }

    /* Rectangle path covering the whole drawing area. */
    params.vertices.MoveTo(LOCAL_ORIGIN, LOCAL_ORIGIN);
    params.vertices.LineTo(width, LOCAL_ORIGIN);
    params.vertices.LineTo(width, height);
    params.vertices.LineTo(LOCAL_ORIGIN, height);
    params.vertices.ClosePolygon();

    params.valid = true;
    return params;
}

bool LinearGradientBuilder::ConvertToLegacy(const GradientInfo* info, GradientColor& outColor)
{
    if ((info == nullptr) || (!info->isValid) || (info->colorStops == nullptr)) {
        return false;
    }

    /*
     * The GradientColor union only stores a begin color, an end color and a
     * direction keyword; multi stop ramps and explicit angles cannot be
     * represented and must go through Build().
     */
    if (info->colorCount != GRADIENT_COLOR_UNION_STOP_COUNT ||
        info->direction == CssGradientDirection::CUSTOM_ANGLE) {
        return false;
    }

    outColor.direction = info->GetDirectionValue();
    outColor.colorBegin = info->colorStops[GRADIENT_BEGIN_STOP_INDEX].color;
    outColor.colorEnd = info->colorStops[GRADIENT_END_STOP_INDEX].color;
    outColor.position = GRADIENT_COLOR_UNUSED_POSITION;

    return true;
}

} // namespace OHOS
