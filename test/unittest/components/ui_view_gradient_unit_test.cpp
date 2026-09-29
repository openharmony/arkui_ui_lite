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
 * @file ui_view_gradient_unit_test.cpp
 * @brief Unit tests for LinearGradientBuilder (pure logic, no rasterization).
 *
 * Covered interfaces:
 *   - LinearGradientBuilder::CalcEndpoints    gradient line geometry
 *       * standard angles 0/90/180/270 degrees
 *       * diagonal angles 45/135/225/315 degrees
 *       * non square drawing areas
 *       * center symmetry (MATH_ROUND regression guard)
 *       * invalid sizes and non finite angles
 *   - LinearGradientBuilder::Build            parameter construction
 *       * error handling: nullptr, invalid info, zero size area
 *       * standard angle ramps, direction keywords, angle normalization
 *       * multi color stops
 *   - LinearGradientBuilder::ConvertToLegacy  two stop GradientInfo to GradientColor
 *       * successful downgrade, multi stop rejection, custom angle rejection,
 *         invalid input rejection
 */

#include "components/linear_gradient_builder.h"

#include "gfx_utils/gradient_info.h"
#include <cmath>
#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace {
/* Geometry constants; an even size keeps the center on integer coordinates. */
const int16_t GEO_SIZE = 100;
const int16_t GEO_HALF = 50;
const int16_t GEO_WIDE = 200;
const int16_t GEO_WIDE_HALF = 100;
const int16_t GEO_SENTINEL = -12345;

const float ANGLE_0 = 0.0f;
const float ANGLE_30 = 30.0f;
const float ANGLE_45 = 45.0f;
const float ANGLE_90 = 90.0f;
const float ANGLE_135 = 135.0f;
const float ANGLE_150 = 150.0f;
const float ANGLE_180 = 180.0f;
const float ANGLE_225 = 225.0f;
const float ANGLE_270 = 270.0f;
const float ANGLE_315 = 315.0f;

const uint8_t TWO_COLORS = 2;
const uint8_t THREE_COLORS = 3;

/* A rectangle path has 5 vertices: MoveTo + 3 LineTo + ClosePolygon. */
const uint32_t RECT_VERTEX_COUNT = 5;
} // namespace

class UIViewGradientTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}

    void SetUp() override {}
    void TearDown() override {}

    /* Build a two color ramp (red to blue) for the given direction or angle. */
    static void MakeTwoColorGradient(GradientInfo& info, CssGradientDirection dir, float angle,
                                     uint32_t beginFull = 0xFFFF0000, uint32_t endFull = 0xFF0000FF)
    {
        info.direction = dir;
        info.angle = angle;
        GradientColorStop stops[TWO_COLORS];
        stops[0].color.full = beginFull;
        stops[0].offset = 0.0f;
        stops[1].color.full = endFull;
        stops[1].offset = 1.0f;
        info.SetColorStops(stops, TWO_COLORS);
    }
};

/* ==========================================================================
 * LinearGradientBuilder::Build - error handling
 * ========================================================================== */

/**
 * @tc.name: BuildNullInfo001
 * @tc.desc: Error handling: a null GradientInfo makes Build return valid=false
 *           and leaves the output params in a safe default state.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildNullInfo001, TestSize.Level1)
{
    // Arrange: nullptr input
    // Act
    auto params = LinearGradientBuilder::Build(nullptr, GEO_SIZE, GEO_SIZE);

    // Assert: valid must be false
    EXPECT_FALSE(params.valid);

    // Assert: output params must be in a safe/default state
    // Paint should not have gradient style set on failure
    EXPECT_NE(params.paint.GetStyle(), Paint::GRADIENT);
    // Vertices should be empty
    EXPECT_EQ(params.vertices.GetTotalVertices(), 0U);
}

/**
 * @tc.name: BuildInvalidGradientInfo002
 * @tc.desc: Error handling: a default GradientInfo (no stops) makes Build return
 *           valid=false with safe output params.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildInvalidGradientInfo002, TestSize.Level1)
{
    // Arrange: default state: colorStops = nullptr, colorCount = 0
    GradientInfo info;

    // Act
    auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, GEO_SIZE);

    // Assert
    EXPECT_FALSE(params.valid);
    EXPECT_NE(params.paint.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(params.vertices.GetTotalVertices(), 0U);
}

/**
 * @tc.name: BuildZeroRect003
 * @tc.desc: Boundary: a zero width or zero height area returns valid=false with
 *           safe output params.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildZeroRect003, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    MakeTwoColorGradient(info, CssGradientDirection::CUSTOM_ANGLE, ANGLE_90);

    // Act & Assert: zero width
    {
        auto params = LinearGradientBuilder::Build(&info, 0, GEO_SIZE);
        EXPECT_FALSE(params.valid);
        EXPECT_NE(params.paint.GetStyle(), Paint::GRADIENT);
        EXPECT_EQ(params.vertices.GetTotalVertices(), 0U);
    }

    // Act & Assert: zero height
    {
        auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, 0);
        EXPECT_FALSE(params.valid);
        EXPECT_NE(params.paint.GetStyle(), Paint::GRADIENT);
        EXPECT_EQ(params.vertices.GetTotalVertices(), 0U);
    }
}

/**
 * @tc.name: BuildNaNAngle004
 * @tc.desc: Error handling: a NaN angle is rejected by CalcEndpoints, so Build
 *           returns valid=false instead of rendering an undefined direction.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildNaNAngle004, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    MakeTwoColorGradient(info, CssGradientDirection::CUSTOM_ANGLE, NAN);

    // Act
    auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, GEO_SIZE);

    // Assert: NaN is rejected by CalcEndpoints, so Build fails
    EXPECT_FALSE(params.valid);
    EXPECT_NE(params.paint.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(params.vertices.GetTotalVertices(), 0U);
}

/* ==========================================================================
 * LinearGradientBuilder::CalcEndpoints - standard angles
 * ========================================================================== */

/**
 * @tc.name: CalcEndpoints0Deg
 * @tc.desc: 0 degrees = to top: gradient line starts at the bottom edge and
 *           ends at the top edge.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpoints0Deg, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_0, GEO_SIZE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, GEO_HALF);
    EXPECT_EQ(start.y, GEO_SIZE);
    EXPECT_EQ(end.x, GEO_HALF);
    EXPECT_EQ(end.y, 0);
}

/**
 * @tc.name: CalcEndpoints90Deg
 * @tc.desc: 90 degrees = to right: gradient line starts on the left edge and
 *           ends on the right edge.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpoints90Deg, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_90, GEO_SIZE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, 0);
    EXPECT_EQ(start.y, GEO_HALF);
    EXPECT_EQ(end.x, GEO_SIZE);
    EXPECT_EQ(end.y, GEO_HALF);
}

/**
 * @tc.name: CalcEndpoints180Deg
 * @tc.desc: 180 degrees = to bottom, the CSS default: gradient line starts at
 *           the top edge and ends at the bottom edge.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpoints180Deg, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_180, GEO_SIZE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, GEO_HALF);
    EXPECT_EQ(start.y, 0);
    EXPECT_EQ(end.x, GEO_HALF);
    EXPECT_EQ(end.y, GEO_SIZE);
}

/**
 * @tc.name: CalcEndpoints270Deg
 * @tc.desc: 270 degrees = to left: gradient line starts on the right edge and
 *           ends on the left edge.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpoints270Deg, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_270, GEO_SIZE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, GEO_SIZE);
    EXPECT_EQ(start.y, GEO_HALF);
    EXPECT_EQ(end.x, 0);
    EXPECT_EQ(end.y, GEO_HALF);
}

/* ==========================================================================
 * LinearGradientBuilder::CalcEndpoints - diagonal angles
 * ========================================================================== */

/**
 * @tc.name: CalcEndpoints45Deg
 * @tc.desc: 45 degrees = to top right: gradient line runs from bottom left
 *           corner to top right corner.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpoints45Deg, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_45, GEO_SIZE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, 0);
    EXPECT_EQ(start.y, GEO_SIZE);
    EXPECT_EQ(end.x, GEO_SIZE);
    EXPECT_EQ(end.y, 0);
}

/**
 * @tc.name: CalcEndpoints135Deg
 * @tc.desc: 135 degrees = to bottom right: gradient line runs from top left
 *           corner to bottom right corner.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpoints135Deg, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_135, GEO_SIZE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, 0);
    EXPECT_EQ(start.y, 0);
    EXPECT_EQ(end.x, GEO_SIZE);
    EXPECT_EQ(end.y, GEO_SIZE);
}

/**
 * @tc.name: CalcEndpoints225Deg
 * @tc.desc: 225 degrees = to bottom left: gradient line runs from top right
 *           corner to bottom left corner.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpoints225Deg, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_225, GEO_SIZE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, GEO_SIZE);
    EXPECT_EQ(start.y, 0);
    EXPECT_EQ(end.x, 0);
    EXPECT_EQ(end.y, GEO_SIZE);
}

/**
 * @tc.name: CalcEndpoints315Deg
 * @tc.desc: 315 degrees = to top left: gradient line runs from bottom right
 *           corner to top left corner.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpoints315Deg, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_315, GEO_SIZE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, GEO_SIZE);
    EXPECT_EQ(start.y, GEO_SIZE);
    EXPECT_EQ(end.x, 0);
    EXPECT_EQ(end.y, 0);
}

/* ==========================================================================
 * LinearGradientBuilder::CalcEndpoints - non square areas
 * ========================================================================== */

/**
 * @tc.name: CalcEndpointsNonSquareHorizontal
 * @tc.desc: 200 x 100, horizontal ramp: the gradient line spans the full width.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpointsNonSquareHorizontal, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_90, GEO_WIDE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, 0);
    EXPECT_EQ(start.y, GEO_HALF);
    EXPECT_EQ(end.x, GEO_WIDE);
    EXPECT_EQ(end.y, GEO_HALF);
}

/**
 * @tc.name: CalcEndpointsNonSquareVertical
 * @tc.desc: 200 x 100, vertical ramp: the gradient line spans the full height.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpointsNonSquareVertical, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_0, GEO_WIDE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, GEO_WIDE_HALF);
    EXPECT_EQ(start.y, GEO_SIZE);
    EXPECT_EQ(end.x, GEO_WIDE_HALF);
    EXPECT_EQ(end.y, 0);
}

/**
 * @tc.name: CalcEndpointsNonSquareDiagonal
 * @tc.desc: 200 x 100 at 45 degrees: endpoints fall outside the rectangle on
 *           purpose, so the ramp covers every corner exactly once.
 *           halfLen = |100*cos45| + |50*sin45| = 106.066
 *           start = (100, 50) - (cos45, -sin45)*106.066 = (25, 125)
 *           end   = (100, 50) + (cos45, -sin45)*106.066 = (175, -25)
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpointsNonSquareDiagonal, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_45, GEO_WIDE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, 25);
    EXPECT_EQ(start.y, 125);
    EXPECT_EQ(end.x, 175);
    EXPECT_EQ(end.y, -25);
}

/* ==========================================================================
 * LinearGradientBuilder::CalcEndpoints - center symmetry
 * ========================================================================== */

/**
 * @tc.name: CalcEndpoints30DegSymmetry
 * @tc.desc: Regression guard for MATH_ROUND. 30 degrees over 100x100:
 *           halfLen = |50*cos60| + |50*sin60| = 68.301
 *           start.x = 50 - cos60*68.301 = 15.849 -> 16 (truncation gives 15)
 *           end.x   = 50 + cos60*68.301 = 84.151 -> 84
 *           With truncation the midpoint of [15, 84] is 49.5 instead of 50.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpoints30DegSymmetry, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_30, GEO_SIZE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, 16);
    EXPECT_EQ(start.y, 109);
    EXPECT_EQ(end.x, 84);
    EXPECT_EQ(end.y, -9);
}

/**
 * @tc.name: CalcEndpoints150DegSymmetry
 * @tc.desc: The mirrored angle (150 deg) must mirror the Y coordinates and keep
 *           X unchanged, verifying the endpoint symmetry around the center.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpoints150DegSymmetry, TestSize.Level1)
{
    // Arrange
    Point start = {0, 0};
    Point end = {0, 0};

    // Act
    bool result = LinearGradientBuilder::CalcEndpoints(ANGLE_150, GEO_SIZE, GEO_SIZE, start, end);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(start.x, 16);
    EXPECT_EQ(start.y, -9);
    EXPECT_EQ(end.x, 84);
    EXPECT_EQ(end.y, 109);
}

/**
 * @tc.name: CalcEndpointsFullSweepSymmetry
 * @tc.desc: Full sweep: the midpoint of the gradient line must land exactly on
 *           the center of the area, so the two coordinates of an axis always add
 *           up to the extent of that axis. Truncation breaks it for 15 of the 18
 *           angles below.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpointsFullSweepSymmetry, TestSize.Level1)
{
    const float sweep[] = {0.0f, 15.0f, 30.0f, 45.0f, 60.0f, 75.0f, 90.0f, 105.0f, 120.0f,
                           135.0f, 150.0f, 165.0f, 180.0f, 210.0f, 240.0f, 270.0f, 300.0f, 330.0f};
    for (float angle : sweep) {
        // Arrange
        Point start = {0, 0};
        Point end = {0, 0};

        // Act
        bool result = LinearGradientBuilder::CalcEndpoints(angle, GEO_SIZE, GEO_SIZE, start, end);

        // Assert
        ASSERT_TRUE(result) << "angle " << angle << " unexpectedly rejected";
        EXPECT_EQ(start.x + end.x, GEO_SIZE)
            << "angle " << angle << " is not centered horizontally";
        EXPECT_EQ(start.y + end.y, GEO_SIZE)
            << "angle " << angle << " is not centered vertically";
    }
}

/* ==========================================================================
 * LinearGradientBuilder::CalcEndpoints - invalid input
 * ========================================================================== */

/**
 * @tc.name: CalcEndpointsInvalidInput
 * @tc.desc: Error handling: a non positive size or a non finite angle is
 *           rejected and the output points are left untouched.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, CalcEndpointsInvalidInput, TestSize.Level1)
{
    // Arrange
    Point start = {GEO_SENTINEL, GEO_SENTINEL};
    Point end = {GEO_SENTINEL, GEO_SENTINEL};

    // Act & Assert: non-positive sizes
    EXPECT_FALSE(LinearGradientBuilder::CalcEndpoints(ANGLE_90, 0, GEO_SIZE, start, end));
    EXPECT_FALSE(LinearGradientBuilder::CalcEndpoints(ANGLE_90, -1, GEO_SIZE, start, end));
    EXPECT_FALSE(LinearGradientBuilder::CalcEndpoints(ANGLE_90, GEO_SIZE, 0, start, end));
    EXPECT_FALSE(LinearGradientBuilder::CalcEndpoints(ANGLE_90, GEO_SIZE, -1, start, end));

    // Act & Assert: non-finite angle
    EXPECT_FALSE(LinearGradientBuilder::CalcEndpoints(NAN, GEO_SIZE, GEO_SIZE, start, end));

    // Assert: output points are left untouched
    EXPECT_EQ(start.x, GEO_SENTINEL);
    EXPECT_EQ(start.y, GEO_SENTINEL);
    EXPECT_EQ(end.x, GEO_SENTINEL);
    EXPECT_EQ(end.y, GEO_SENTINEL);
}

/* ==========================================================================
 * LinearGradientBuilder::Build - standard angle ramps, param validation
 * ========================================================================== */

/**
 * @tc.name: BuildAngle90Horizontal
 * @tc.desc: 90 degree ramp (horizontal): validates Build returns valid=true,
 *           Paint has GRADIENT style, correct linear gradient endpoints, 2 color
 *           stops, and vertices form a closed rectangle.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildAngle90Horizontal, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    MakeTwoColorGradient(info, CssGradientDirection::CUSTOM_ANGLE, ANGLE_90);
    ASSERT_TRUE(info.isValid);

    // Act
    auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, GEO_SIZE);

    // Assert
    ASSERT_TRUE(params.valid);

    // Paint must have gradient style
    EXPECT_EQ(params.paint.GetStyle(), Paint::GRADIENT);

    // Gradient endpoints: 90 deg = left to right
    auto gradientPt = params.paint.GetLinearGradientPoint();
    EXPECT_EQ(static_cast<int16_t>(gradientPt.x0), 0);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.y0), GEO_HALF);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.x1), GEO_SIZE);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.y1), GEO_HALF);

    // Color stops: 2 stops (red at 0.0, blue at 1.0)
    auto stopList = params.paint.getStopAndColor();
    EXPECT_EQ(stopList.Size(), 2U);

    // Vertices: closed rectangle = 5 vertices
    EXPECT_EQ(params.vertices.GetTotalVertices(), RECT_VERTEX_COUNT);
}

/**
 * @tc.name: BuildAngle0Vertical
 * @tc.desc: 0 degree ramp (vertical, bottom to top): validates Build returns
 *           valid=true with correct gradient endpoints.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildAngle0Vertical, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    MakeTwoColorGradient(info, CssGradientDirection::CUSTOM_ANGLE, ANGLE_0);
    ASSERT_TRUE(info.isValid);

    // Act
    auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, GEO_SIZE);

    // Assert
    ASSERT_TRUE(params.valid);
    EXPECT_EQ(params.paint.GetStyle(), Paint::GRADIENT);

    // Gradient endpoints: 0 deg = bottom to top
    auto gradientPt = params.paint.GetLinearGradientPoint();
    EXPECT_EQ(static_cast<int16_t>(gradientPt.x0), GEO_HALF);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.y0), GEO_SIZE);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.x1), GEO_HALF);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.y1), 0);

    EXPECT_EQ(params.vertices.GetTotalVertices(), RECT_VERTEX_COUNT);
}

/**
 * @tc.name: BuildPredefinedDirection180Deg
 * @tc.desc: Direction keyword TO_BOTTOM (180 degrees): validates that the
 *           direction-to-angle mapping works correctly.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildPredefinedDirection180Deg, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    MakeTwoColorGradient(info, CssGradientDirection::TO_BOTTOM, ANGLE_0);
    ASSERT_TRUE(info.isValid);

    // Act
    auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, GEO_SIZE);

    // Assert
    ASSERT_TRUE(params.valid);
    EXPECT_EQ(params.paint.GetStyle(), Paint::GRADIENT);

    // Gradient endpoints: 180 deg = top to bottom
    auto gradientPt = params.paint.GetLinearGradientPoint();
    EXPECT_EQ(static_cast<int16_t>(gradientPt.x0), GEO_HALF);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.y0), 0);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.x1), GEO_HALF);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.y1), GEO_SIZE);

    EXPECT_EQ(params.vertices.GetTotalVertices(), RECT_VERTEX_COUNT);
}

/**
 * @tc.name: BuildAngle45Diagonal
 * @tc.desc: 45 degree diagonal ramp: validates Build returns valid=true with
 *           corner-to-corner gradient endpoints.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildAngle45Diagonal, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    MakeTwoColorGradient(info, CssGradientDirection::CUSTOM_ANGLE, ANGLE_45);
    ASSERT_TRUE(info.isValid);

    // Act
    auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, GEO_SIZE);

    // Assert
    ASSERT_TRUE(params.valid);
    EXPECT_EQ(params.paint.GetStyle(), Paint::GRADIENT);

    // Gradient endpoints: 45 deg = bottom left to top right
    auto gradientPt = params.paint.GetLinearGradientPoint();
    EXPECT_EQ(static_cast<int16_t>(gradientPt.x0), 0);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.y0), GEO_SIZE);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.x1), GEO_SIZE);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.y1), 0);

    EXPECT_EQ(params.vertices.GetTotalVertices(), RECT_VERTEX_COUNT);
}

/**
 * @tc.name: BuildAngle135Diagonal
 * @tc.desc: 135 degree diagonal ramp: validates Build returns valid=true with
 *           corner-to-corner gradient endpoints.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildAngle135Diagonal, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    MakeTwoColorGradient(info, CssGradientDirection::CUSTOM_ANGLE, ANGLE_135);
    ASSERT_TRUE(info.isValid);

    // Act
    auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, GEO_SIZE);

    // Assert
    ASSERT_TRUE(params.valid);
    EXPECT_EQ(params.paint.GetStyle(), Paint::GRADIENT);

    // Gradient endpoints: 135 deg = top left to bottom right
    auto gradientPt = params.paint.GetLinearGradientPoint();
    EXPECT_EQ(static_cast<int16_t>(gradientPt.x0), 0);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.y0), 0);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.x1), GEO_SIZE);
    EXPECT_EQ(static_cast<int16_t>(gradientPt.y1), GEO_SIZE);

    EXPECT_EQ(params.vertices.GetTotalVertices(), RECT_VERTEX_COUNT);
}

/**
 * @tc.name: BuildAngleNormalization
 * @tc.desc: Angle normalization: 450 and -270 degrees both fold into 90 degrees,
 *           producing the same gradient endpoints.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildAngleNormalization, TestSize.Level1)
{
    // 450 degrees folds into 90 degrees
    {
        // Arrange
        GradientInfo info;
        MakeTwoColorGradient(info, CssGradientDirection::CUSTOM_ANGLE, 450.0f);

        // Act
        auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, GEO_SIZE);

        // Assert
        ASSERT_TRUE(params.valid);
        EXPECT_EQ(params.paint.GetStyle(), Paint::GRADIENT);
        auto gradientPt = params.paint.GetLinearGradientPoint();
        EXPECT_EQ(static_cast<int16_t>(gradientPt.x0), 0);
        EXPECT_EQ(static_cast<int16_t>(gradientPt.y0), GEO_HALF);
        EXPECT_EQ(static_cast<int16_t>(gradientPt.x1), GEO_SIZE);
        EXPECT_EQ(static_cast<int16_t>(gradientPt.y1), GEO_HALF);
    }

    // -270 degrees folds into 90 degrees as well
    {
        // Arrange
        GradientInfo info;
        MakeTwoColorGradient(info, CssGradientDirection::CUSTOM_ANGLE, -270.0f);

        // Act
        auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, GEO_SIZE);

        // Assert
        ASSERT_TRUE(params.valid);
        EXPECT_EQ(params.paint.GetStyle(), Paint::GRADIENT);
        auto gradientPt = params.paint.GetLinearGradientPoint();
        EXPECT_EQ(static_cast<int16_t>(gradientPt.x0), 0);
        EXPECT_EQ(static_cast<int16_t>(gradientPt.y0), GEO_HALF);
        EXPECT_EQ(static_cast<int16_t>(gradientPt.x1), GEO_SIZE);
        EXPECT_EQ(static_cast<int16_t>(gradientPt.y1), GEO_HALF);
    }
}

/**
 * @tc.name: BuildThreeColor
 * @tc.desc: Three color ramp (red, green, blue): validates that Build correctly
 *           configures all three color stops with their offsets.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, BuildThreeColor, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    info.direction = CssGradientDirection::CUSTOM_ANGLE;
    info.angle = ANGLE_90;
    GradientColorStop stops[THREE_COLORS];
    stops[0].color.full = 0xFFFF0000; /* red at 0%    */
    stops[0].offset = 0.0f;
    stops[1].color.full = 0xFF00FF00; /* green at 50% */
    stops[1].offset = 0.5f;
    stops[2].color.full = 0xFF0000FF; /* blue at 100% */
    stops[2].offset = 1.0f;
    info.SetColorStops(stops, THREE_COLORS);
    ASSERT_TRUE(info.isValid);

    // Act
    auto params = LinearGradientBuilder::Build(&info, GEO_SIZE, GEO_SIZE);

    // Assert
    ASSERT_TRUE(params.valid);
    EXPECT_EQ(params.paint.GetStyle(), Paint::GRADIENT);

    // 3 color stops
    auto stopList = params.paint.getStopAndColor();
    EXPECT_EQ(stopList.Size(), 3U);

    // Verify vertices
    EXPECT_EQ(params.vertices.GetTotalVertices(), RECT_VERTEX_COUNT);
}

/* ==========================================================================
 * LinearGradientBuilder::ConvertToLegacy
 * ========================================================================== */

/**
 * @tc.name: ConvertTwoColor
 * @tc.desc: A two stop GradientInfo carrying a direction keyword converts into
 *           the legacy GradientColor union with matching fields.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, ConvertTwoColor, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    MakeTwoColorGradient(info, CssGradientDirection::TO_RIGHT, ANGLE_0);

    // Act
    GradientColor outColor;
    bool result = LinearGradientBuilder::ConvertToLegacy(&info, outColor);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(outColor.direction, static_cast<uint8_t>(CssGradientDirection::TO_RIGHT));
    EXPECT_EQ(outColor.colorBegin.full, 0xFFFF0000u);
    EXPECT_EQ(outColor.colorEnd.full, 0xFF0000FFu);
    EXPECT_EQ(outColor.position, 0);
}

/**
 * @tc.name: ConvertMultiColor
 * @tc.desc: A ramp with three or more stops cannot be mapped onto the two color
 *           GradientColor union, so the conversion returns false.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, ConvertMultiColor, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    info.direction = CssGradientDirection::TO_RIGHT;
    GradientColorStop stops[THREE_COLORS];
    stops[0].color.full = 0xFFFF0000;
    stops[0].offset = 0.0f;
    stops[1].color.full = 0xFF00FF00;
    stops[1].offset = 0.5f;
    stops[2].color.full = 0xFF0000FF;
    stops[2].offset = 1.0f;
    info.SetColorStops(stops, THREE_COLORS);
    ASSERT_TRUE(info.isValid);

    // Act
    GradientColor outColor;
    bool result = LinearGradientBuilder::ConvertToLegacy(&info, outColor);

    // Assert
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ConvertCustomAngle
 * @tc.desc: A two color ramp using an explicit angle (CUSTOM_ANGLE) has no
 *           equivalent among the direction keywords, so conversion returns false.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, ConvertCustomAngle, TestSize.Level1)
{
    // Arrange
    GradientInfo info;
    MakeTwoColorGradient(info, CssGradientDirection::CUSTOM_ANGLE, ANGLE_45);

    // Act
    GradientColor outColor;
    bool result = LinearGradientBuilder::ConvertToLegacy(&info, outColor);

    // Assert
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ConvertInvalid
 * @tc.desc: Error handling: a null pointer or an invalid GradientInfo returns
 *           false.
 * @tc.type: FUNC
 */
HWTEST_F(UIViewGradientTest, ConvertInvalid, TestSize.Level1)
{
    // Act & Assert: null pointer
    {
        GradientColor outColor;
        EXPECT_FALSE(LinearGradientBuilder::ConvertToLegacy(nullptr, outColor));
    }

    // Act & Assert: default-constructed (invalid) GradientInfo
    {
        GradientInfo info;
        GradientColor outColor;
        EXPECT_FALSE(LinearGradientBuilder::ConvertToLegacy(&info, outColor));
    }
}

} // namespace OHOS