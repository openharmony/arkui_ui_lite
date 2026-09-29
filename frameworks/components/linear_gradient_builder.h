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
 * @file linear_gradient_builder.h
 * @brief Builder that translates a declarative GradientInfo into render parameters.
 *
 * LinearGradientBuilder separates the pure parameter construction logic from the
 * actual rendering (DrawCanvas::RenderGradientFill). This allows:
 *   - Unit testing of angle resolution, endpoint math, and paint configuration
 *     without a frame buffer.
 *   - Reuse across multiple UI components (UIView, UIButton, etc.).
 *
 * Call chain:
 *   UIView::DrawLinearGradient()
 *     -> LinearGradientBuilder::Build()          (pure logic, testable)
 *     -> DrawCanvas::RenderGradientFill()        (rendering, integration test)
 */

#ifndef GRAPHIC_UI_LINEAR_GRADIENT_BUILDER_H
#define GRAPHIC_UI_LINEAR_GRADIENT_BUILDER_H

#if defined(GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG) && GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG

#include "gfx_utils/geometry2d.h"
#include "gfx_utils/graphic_buffer.h"
#include "gfx_utils/diagram/common/paint.h"
#include "gfx_utils/style.h"

namespace OHOS {

/**
 * @brief Fully assembled parameters needed to render a linear gradient.
 *
 * A successful Build() call sets valid = true and fills every field; a failed
 * call sets valid = false and leaves the other fields in an unspecified state.
 */
struct GradientRenderParams {
    /** Paint object configured with the gradient style, endpoints, and color stops. */
    Paint paint;

    /** Rectangle fill path in local coordinates, already closed. */
    UICanvasVertices vertices;

    /** True when Build() succeeded; false when the input was invalid. */
    bool valid;
};

/**
 * @brief Stateless builder that converts a GradientInfo into ready to use
 *        render parameters.
 *
 * All methods are static and do not hold any mutable state, making the class
 * trivially testable and safe to call from any thread as long as the input
 * GradientInfo is not concurrently modified.
 */
class LinearGradientBuilder {
public:
    /**
     * @brief Build the full set of render parameters from a gradient declaration.
     *
     * This is the main entry point. It performs:
     *   1. Input validation (non null info, at least two color stops).
     *   2. CSS angle resolution and gradient line endpoint computation.
     *   3. Paint configuration (style, endpoints, color stops).
     *   4. Rectangle fill path construction.
     *
     * @param info   gradient payload from the CSS parser; must remain valid for
     *               the duration of the call.
     * @param width  width of the drawing area in pixels, must be > 0.
     * @param height height of the drawing area in pixels, must be > 0.
     * @return a GradientRenderParams with valid = true on success.
     */
    static GradientRenderParams Build(const GradientInfo* info, int16_t width, int16_t height);

    /**
     * @brief Compute the endpoints of the gradient line for a given CSS angle.
     *
     * The gradient line runs through the center of the area; its endpoints are
     * the extreme projections of the rectangle onto the gradient direction.
     *
     * Algorithm:
     *   1. CSS angle to math angle: mathDeg = 90 - cssDeg
     *   2. Math angle to radians:   rad = mathDeg * PI / 180
     *   3. Direction vector:        (cos(rad), sin(rad))
     *   4. Half diagonal projected: halfLen = |w/2 * cos| + |h/2 * sin|
     *   5. start = center - direction * halfLen
     *   6. end   = center + direction * halfLen
     *   The screen Y axis grows downwards, so the Y component is negated.
     *
     * Exposed so that unit tests can assert the geometry of every standard
     * angle without going through a frame buffer.
     *
     * @param cssAngleDeg CSS angle in degrees, already normalized.
     * @param width       width of the area in pixels, must be > 0.
     * @param height      height of the area in pixels, must be > 0.
     * @param startPt     [out] gradient start point, relative to area origin.
     * @param endPt       [out] gradient end point, relative to area origin.
     * @return true when the endpoints were written, false on invalid input.
     */
    static bool CalcEndpoints(float cssAngleDeg,
                              int16_t width,
                              int16_t height,
                              Point& startPt,
                              Point& endPt);

    /**
     * @brief Downgrade a two color gradient into the legacy GradientColor union.
     *
     * Only useful for legacy interfaces that still accept GradientColor. Multi
     * stop gradients and custom angle gradients must go through Build().
     *
     * @param info     source payload; must carry exactly two stops and a
     *                 direction keyword (an explicit angle cannot be represented).
     * @param outColor destination union.
     * @return true on success, false when the payload cannot be represented.
     */
    static bool ConvertToLegacy(const GradientInfo* info, GradientColor& outColor);

private:
    /** Prevent instantiation — all methods are static. */
    LinearGradientBuilder() = delete;
};

} // namespace OHOS

#endif // GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG

#endif // GRAPHIC_UI_LINEAR_GRADIENT_BUILDER_H
