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

#include "animator/path_animator_callback.h"

#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>

#include "animator/animator_manager.h"
#include "common/task_manager.h"
#include "components/ui_view.h"

using namespace testing::ext;
namespace OHOS {
namespace {
// effect duration used by the frame-driven cases: 320ms = 20 frames (16ms beat)
const uint32_t PATH_DURATION = 320;
// start coordinate of the target view; final position = base + path offset
const int16_t BASE_X = 10;
const int16_t BASE_Y = 20;
// path "M 0 0 L 240 0": a straight line of length 240 along +x
const char* PATH_LINE_X = "path(\"M 0 0 L 240 0\")";
} // namespace

class PathAnimatorCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/* ============ path input & configuration ============ */

/**
 * @tc.name: UIPathAnimatorCallbackSetPathPolyline_001
 * @tc.desc: Verify SetPath with a point array fills the polyline (100% point is the last one).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackSetPathPolyline_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    Vector2<int16_t> points[3] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(100, 0),
                                  Vector2<int16_t>(200, 0)};
    EXPECT_TRUE(callback.SetPath(points, 3));
    int16_t outX = 0;
    int16_t outY = 0;
    EXPECT_TRUE(callback.CalculatePoint(100, outX, outY));
    EXPECT_EQ(outX, 200);
    EXPECT_EQ(outY, 0);
}

/**
 * @tc.name: UIPathAnimatorCallbackSetPathPolyline_002
 * @tc.desc: Verify SetPath with nullptr or count < 2 fails and returns false.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackSetPathPolyline_002, TestSize.Level1)
{
    PathAnimatorCallback callback;
    Vector2<int16_t> points[2] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(100, 0)};
    EXPECT_FALSE(callback.SetPath(nullptr, 2));
    EXPECT_FALSE(callback.SetPath(points, 1));
    EXPECT_FALSE(callback.SetPath(points, 0));
}

/**
 * @tc.name: UIPathAnimatorCallbackSetPathString_001
 * @tc.desc: Verify SetPathString parses an SVG path string and enters polyline mode.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackSetPathString_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    EXPECT_TRUE(callback.SetPathString(PATH_LINE_X));
    int16_t outX = 0;
    int16_t outY = 0;
    EXPECT_TRUE(callback.CalculatePoint(100, outX, outY));
    EXPECT_EQ(outX, 240);
    EXPECT_EQ(outY, 0);
}

/**
 * @tc.name: UIPathAnimatorCallbackSetPathStringFail_001
 * @tc.desc: Verify SetPathString returns false for invalid path strings
 *           (missing path prefix, empty body, unsupported command).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackSetPathStringFail_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    EXPECT_FALSE(callback.SetPathString("M 0 0 L 100 0"));
    EXPECT_FALSE(callback.SetPathString("path(\"\")"));
    EXPECT_FALSE(callback.SetPathString("path(\"M 0 0 H 100 0\")"));
}

/**
 * @tc.name: UIPathAnimatorCallbackSetPathStringNull_001
 * @tc.desc: Verify SetPathString returns false for nullptr input.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackSetPathStringNull_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    EXPECT_FALSE(callback.SetPathString(nullptr));
}

/**
 * @tc.name: UIPathAnimatorCallbackSetBezier_001
 * @tc.desc: Verify SetBezier enters Bezier mode: progress 0 -> start, 100 -> end,
 *           50 -> curve midpoint (100, -20).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackSetBezier_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    callback.SetBezier(Vector2<int16_t>(0, 0), Vector2<int16_t>(100, -40),
                       Vector2<int16_t>(200, 0));
    int16_t outX = 0;
    int16_t outY = 0;
    EXPECT_TRUE(callback.CalculatePoint(0, outX, outY));
    EXPECT_EQ(outX, 0);
    EXPECT_EQ(outY, 0);
    EXPECT_TRUE(callback.CalculatePoint(100, outX, outY));
    EXPECT_EQ(outX, 200);
    EXPECT_EQ(outY, 0);
    EXPECT_TRUE(callback.CalculatePoint(50, outX, outY));
    EXPECT_EQ(outX, 100);
    EXPECT_EQ(outY, -20);
}

/**
 * @tc.name: UIPathAnimatorCallbackSetPathPolylineObject_001
 * @tc.desc: Verify SetPath(const PathPolyline&) accepts a valid polyline and rejects
 *           an invalid one (count < 2).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackSetPathPolylineObject_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    PathPolyline poly;
    poly.x[0] = 0;
    poly.y[0] = 0;
    poly.x[1] = 100;
    poly.y[1] = 0;
    poly.count = 2;
    OffsetPathParser::BuildCumulativeLength(poly);
    EXPECT_TRUE(callback.SetPath(poly));

    int16_t outX = 0;
    int16_t outY = 0;
    EXPECT_TRUE(callback.CalculatePoint(100, outX, outY));
    EXPECT_EQ(outX, 100);
    EXPECT_EQ(outY, 0);

    PathPolyline invalidPoly;
    invalidPoly.count = 1;
    EXPECT_FALSE(callback.SetPath(invalidPoly));
}

/**
 * @tc.name: UIPathAnimatorCallbackSetPathTruncation_001
 * @tc.desc: Verify SetPath with a point array larger than MAX_POINTS truncates to MAX_POINTS.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackSetPathTruncation_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    Vector2<int16_t> points[PathPolyline::MAX_POINTS + 4];
    for (uint8_t i = 0; i < PathPolyline::MAX_POINTS + 4; i++) {
        points[i] = Vector2<int16_t>(i, 0);
    }
    EXPECT_TRUE(callback.SetPath(points, PathPolyline::MAX_POINTS + 4));
    int16_t outX = 0;
    int16_t outY = 0;
    EXPECT_TRUE(callback.CalculatePoint(100, outX, outY));
    EXPECT_EQ(outX, PathPolyline::MAX_POINTS - 1);
    EXPECT_EQ(outY, 0);
}

/* ============ positioning calculation (pure functions) ============ */

/**
 * @tc.name: UIPathAnimatorCallbackCalculatePolylinePoint_001
 * @tc.desc: Verify the CalculatePoint locates the correct point by progress in polyline mode.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackCalculatePolylinePoint_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    Vector2<int16_t> points[3] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(100, 0),
                                  Vector2<int16_t>(200, 0)};
    EXPECT_TRUE(callback.SetPath(points, 3));
    int16_t outX = 0;
    int16_t outY = 0;
    EXPECT_TRUE(callback.CalculatePoint(0, outX, outY));
    EXPECT_EQ(outX, 0);
    EXPECT_EQ(outY, 0);
    EXPECT_TRUE(callback.CalculatePoint(50, outX, outY));
    EXPECT_EQ(outX, 100);
    EXPECT_EQ(outY, 0);
    EXPECT_TRUE(callback.CalculatePoint(100, outX, outY));
    EXPECT_EQ(outX, 200);
    EXPECT_EQ(outY, 0);
}

/**
 * @tc.name: UIPathAnimatorCallbackCalculatePolylinePoint_002
 * @tc.desc: Verify CalculatePoint returns false for an invalid polyline path (count < 2).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackCalculatePolylinePoint_002, TestSize.Level1)
{
    PathAnimatorCallback callback;
    Vector2<int16_t> points[1] = {Vector2<int16_t>(0, 0)};
    EXPECT_FALSE(callback.SetPath(points, 1));
    int16_t outX = 0;
    int16_t outY = 0;
    EXPECT_FALSE(callback.CalculatePoint(50, outX, outY));
}

/**
 * @tc.name: UIPathAnimatorCallbackBezierCalculatePoint_001
 * @tc.desc: Verify CalculatePoint works in Bezier mode for a non-trivial curve.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackBezierCalculatePoint_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    callback.SetBezier(Vector2<int16_t>(0, 0), Vector2<int16_t>(120, -40),
                       Vector2<int16_t>(240, 0));
    int16_t outX = 0;
    int16_t outY = 0;
    EXPECT_TRUE(callback.CalculatePoint(0, outX, outY));
    EXPECT_EQ(outX, 0);
    EXPECT_EQ(outY, 0);
    EXPECT_TRUE(callback.CalculatePoint(50, outX, outY));
    EXPECT_EQ(outX, 120);
    EXPECT_EQ(outY, -20);
    EXPECT_TRUE(callback.CalculatePoint(100, outX, outY));
    EXPECT_EQ(outX, 240);
    EXPECT_EQ(outY, 0);
}

/**
 * @tc.name: UIPathAnimatorCallbackCalculatePointClamp_001
 * @tc.desc: Verify CalculatePoint clamps progress to 0~100.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackCalculatePointClamp_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    Vector2<int16_t> points[2] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(100, 0)};
    EXPECT_TRUE(callback.SetPath(points, 2));
    int16_t outX = 0;
    int16_t outY = 0;
    EXPECT_TRUE(callback.CalculatePoint(150, outX, outY));
    EXPECT_EQ(outX, 100);
    EXPECT_EQ(outY, 0);
    EXPECT_TRUE(callback.CalculatePoint(0, outX, outY));
    EXPECT_EQ(outX, 0);
    EXPECT_EQ(outY, 0);
}

/**
 * @tc.name: UIPathAnimatorCallbackCalculatePointNoneMode_001
 * @tc.desc: Verify CalculatePoint returns false when no path is configured (MODE_NONE).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackCalculatePointNoneMode_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    int16_t outX = 0;
    int16_t outY = 0;
    EXPECT_FALSE(callback.CalculatePoint(50, outX, outY));
}

/* ============ tangent angle (pure functions) ============ */

/**
 * @tc.name: UIPathAnimatorCallbackTangentHorizontal_001
 * @tc.desc: Verify the tangent angle output on a horizontal segment is 0 degree.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackTangentHorizontal_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    Vector2<int16_t> points[2] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(100, 0)};
    EXPECT_TRUE(callback.SetPath(points, 2));
    int16_t outX = 0;
    int16_t outY = 0;
    float tangent = -999.0f;
    EXPECT_TRUE(callback.CalculatePoint(50, outX, outY, &tangent));
    EXPECT_EQ(outX, 50);
    EXPECT_EQ(outY, 0);
    EXPECT_FLOAT_EQ(tangent, 0.0f);
}

/**
 * @tc.name: UIPathAnimatorCallbackTangentSlope_001
 * @tc.desc: Verify tangent angles on +/-45 degree slopes.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackTangentSlope_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    int16_t outX = 0;
    int16_t outY = 0;
    float tangent = -999.0f;
    Vector2<int16_t> down[2] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(100, 100)};
    EXPECT_TRUE(callback.SetPath(down, 2));
    EXPECT_TRUE(callback.CalculatePoint(50, outX, outY, &tangent));
    EXPECT_FLOAT_EQ(tangent, 45.0f);
    Vector2<int16_t> up[2] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(100, -100)};
    EXPECT_TRUE(callback.SetPath(up, 2));
    tangent = -999.0f;
    EXPECT_TRUE(callback.CalculatePoint(50, outX, outY, &tangent));
    EXPECT_FLOAT_EQ(tangent, -45.0f);
}

/**
 * @tc.name: UIPathAnimatorCallbackTangentVertical_001
 * @tc.desc: Verify tangent angles on vertical segments (+90 down, -90 up).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackTangentVertical_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    int16_t outX = 0;
    int16_t outY = 0;
    float tangent = -999.0f;
    Vector2<int16_t> down[2] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(0, 100)};
    EXPECT_TRUE(callback.SetPath(down, 2));
    EXPECT_TRUE(callback.CalculatePoint(50, outX, outY, &tangent));
    EXPECT_FLOAT_EQ(tangent, 90.0f);
    Vector2<int16_t> up[2] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(0, -100)};
    EXPECT_TRUE(callback.SetPath(up, 2));
    tangent = -999.0f;
    EXPECT_TRUE(callback.CalculatePoint(50, outX, outY, &tangent));
    EXPECT_FLOAT_EQ(tangent, -90.0f);
}

/**
 * @tc.name: UIPathAnimatorCallbackTangentZeroLenSeg_001
 * @tc.desc: Verify zero-length segments fall back for tangent angle.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackTangentZeroLenSeg_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    Vector2<int16_t> points[3] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(0, 0),
                                  Vector2<int16_t>(0, 100)}; // zero-length first segment, vertical second segment
    EXPECT_TRUE(callback.SetPath(points, 3));
    int16_t outX = 0;
    int16_t outY = 0;
    float tangent = -999.0f;
    EXPECT_TRUE(callback.CalculatePoint(0, outX, outY, &tangent));
    EXPECT_FLOAT_EQ(tangent, 0.0f);
    tangent = -999.0f;
    EXPECT_TRUE(callback.CalculatePoint(50, outX, outY, &tangent));
    EXPECT_FLOAT_EQ(tangent, 90.0f);
}

/**
 * @tc.name: UIPathAnimatorCallbackTangentBezier_001
 * @tc.desc: Verify the tangent angle from the quadratic Bezier derivative B'(t).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackTangentBezier_001, TestSize.Level1)
{
    PathAnimatorCallback callback;
    callback.SetBezier(Vector2<int16_t>(0, 0), Vector2<int16_t>(100, -40),
                       Vector2<int16_t>(200, 0));
    int16_t outX = 0;
    int16_t outY = 0;
    float tangent = -999.0f;
    EXPECT_TRUE(callback.CalculatePoint(0, outX, outY, &tangent));
    EXPECT_NEAR(tangent, -21.8f, 0.2f);
    tangent = -999.0f;
    EXPECT_TRUE(callback.CalculatePoint(50, outX, outY, &tangent));
    EXPECT_NEAR(tangent, 0.0f, 0.2f);
    tangent = -999.0f;
    EXPECT_TRUE(callback.CalculatePoint(100, outX, outY, &tangent));
    EXPECT_NEAR(tangent, 21.8f, 0.2f);
}

/**
 * @tc.name: UIPathAnimatorCallbackRunToPosition_001
 * @tc.desc: Drive Callback frame by frame: the view reaches the path end after all frames
 *           (320ms = 20 frames; final = base + (240, 0)).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackRunToPosition_001, TestSize.Level1)
{
    UIView view;
    view.SetPosition(BASE_X, BASE_Y);
    PathAnimatorCallback callback;
    EXPECT_TRUE(callback.SetPathString(PATH_LINE_X));
    // progress 0%: base synced, view stays at base
    callback.ApplyFrame(&view, 0);
    EXPECT_EQ(view.GetX(), BASE_X);
    EXPECT_EQ(view.GetY(), BASE_Y);
    // progress 50%: position = base + (120, 0)
    callback.ApplyFrame(&view, 500);
    EXPECT_EQ(view.GetX(), BASE_X + 120);
    EXPECT_EQ(view.GetY(), BASE_Y);
    // progress 100%: final = base + (240, 0)
    callback.ApplyFrame(&view, 1000);
    EXPECT_EQ(view.GetX(), BASE_X + 240);
    EXPECT_EQ(view.GetY(), BASE_Y);
}

/**
 * @tc.name: UIPathAnimatorCallbackBaseSync_001
 * @tc.desc: Verify the first Callback records the view's current position as the path base.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackBaseSync_001, TestSize.Level1)
{
    UIView view;
    view.SetPosition(50, 60);
    PathAnimatorCallback callback;
    Vector2<int16_t> points[2] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(100, 0)};
    EXPECT_TRUE(callback.SetPath(points, 2));
    // first ApplyFrame: base = (50, 60), dist = 0 -> position stays at base
    callback.ApplyFrame(&view, 0);
    EXPECT_EQ(view.GetX(), 50);
    EXPECT_EQ(view.GetY(), 60);
    // progress 50%: position must be base + path offset (not 0 + offset)
    callback.ApplyFrame(&view, 500);
    EXPECT_EQ(view.GetX(), 50 + 50);
    EXPECT_EQ(view.GetY(), 60);
}

/**
 * @tc.name: UIPathAnimatorCallbackReconfigReplay_001
 * @tc.desc: Verify reconfiguring the path resets the state machine:
 *           the next Callback replays from the first frame and re-syncs the base.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackReconfigReplay_001, TestSize.Level1)
{
    UIView view;
    view.SetPosition(BASE_X, BASE_Y);
    PathAnimatorCallback callback;
    EXPECT_TRUE(callback.SetPathString(PATH_LINE_X));
    // run to the end: base + (240, 0)
    callback.ApplyFrame(&view, 1000);
    EXPECT_EQ(view.GetX(), BASE_X + 240);
    // reconfigure a new path: state resets, base re-syncs to the current view position
    Vector2<int16_t> points[2] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(100, 0)};
    EXPECT_TRUE(callback.SetPath(points, 2));
    // first ApplyFrame after reconfigure: dist = 0, base = current position
    callback.ApplyFrame(&view, 0);
    EXPECT_EQ(view.GetX(), BASE_X + 240); // base re-synced to (BASE_X + 240, BASE_Y)
    EXPECT_EQ(view.GetY(), BASE_Y);
    // progress 50% of the new path: base + 50
    callback.ApplyFrame(&view, 500);
    EXPECT_EQ(view.GetX(), BASE_X + 240 + 50);
    EXPECT_EQ(view.GetY(), BASE_Y);
}

/**
 * @tc.name: UIPathAnimatorCallbackWrapAfterFullPass_001
 * @tc.desc: Verify progress wraps after a full pass (frameCount > totalFrames):
 *           the frame after the end frame replays from progress 1/totalFrames.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackContinueAfterEnd_001, TestSize.Level1)
{
    UIView view;
    view.SetPosition(BASE_X, BASE_Y);
    PathAnimatorCallback callback;
    EXPECT_TRUE(callback.SetPathString(PATH_LINE_X));
    // run to the end: base + (240, 0)
    callback.ApplyFrame(&view, 1000);
    EXPECT_EQ(view.GetX(), BASE_X + 240);
    // applying a small progress after the end moves the view forward from the current base
    callback.ApplyFrame(&view, 50);
    EXPECT_GE(view.GetX(), BASE_X + 240);              // not before the end position
    EXPECT_LT(view.GetX(), BASE_X + 240 + 24);         // within the first 10% of the path
    EXPECT_EQ(view.GetY(), BASE_Y);
}

/**
 * @tc.name: UIPathAnimatorCallbackNoAdvanceWithoutDuration_001
 * @tc.desc: Verify Callback is a no-op when the duration was never set (durationMs_ = 0).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackNoAdvanceWithoutDuration_001, TestSize.Level1)
{
    UIView view;
    view.SetPosition(BASE_X, BASE_Y);
    PathAnimatorCallback callback;
    EXPECT_TRUE(callback.SetPathString(PATH_LINE_X)); // duration NOT set
    callback.Callback(&view);
    EXPECT_EQ(view.GetX(), BASE_X); // no advance
    EXPECT_EQ(view.GetY(), BASE_Y);
}

/**
 * @tc.name: UIPathAnimatorCallbackBezierRunToPosition_001
 * @tc.desc: Drive Callback frame by frame in Bezier mode: final = base + bezier end point.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackBezierRunToPosition_001, TestSize.Level1)
{
    UIView view;
    view.SetPosition(BASE_X, BASE_Y);
    PathAnimatorCallback callback;
    callback.SetBezier(Vector2<int16_t>(0, 0), Vector2<int16_t>(120, -40),
                       Vector2<int16_t>(240, 0));
    callback.ApplyFrame(&view, 1000);
    EXPECT_EQ(view.GetX(), BASE_X + 240);
    EXPECT_EQ(view.GetY(), BASE_Y);
}

/**
 * @tc.name: UIPathAnimatorCallbackFixedRotateAngle_001
 * @tc.desc: Verify SetFixedRotate applies a constant angle to the view's transform map.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackFixedRotateAngle_001, TestSize.Level1)
{
    UIView view;
    view.SetPosition(BASE_X, BASE_Y);
    PathAnimatorCallback callback;
    EXPECT_TRUE(callback.SetPathString(PATH_LINE_X));
    callback.SetFixedRotate(30);
    callback.ApplyFrame(&view, 0);
    EXPECT_EQ(view.GetTransformMap().GetRotateAngle(), 30);
    callback.SetFixedRotate(0); // back to no rotation
    callback.ApplyFrame(&view, 0);
    SUCCEED();
}

/**
 * @tc.name: UIPathAnimatorCallbackAutoRotateAngle_001
 * @tc.desc: Verify SetAutoRotate applies the path tangent angle to the view's transform map
 *           (45-degree slope -> rotate angle 45).
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackAutoRotateAngle_001, TestSize.Level1)
{
    UIView view;
    view.SetPosition(BASE_X, BASE_Y);
    PathAnimatorCallback callback;
    Vector2<int16_t> points[2] = {Vector2<int16_t>(0, 0), Vector2<int16_t>(100, 100)};
    EXPECT_TRUE(callback.SetPath(points, 2));
    callback.SetAutoRotate(0);
    callback.ApplyFrame(&view, 0);
    EXPECT_EQ(view.GetTransformMap().GetRotateAngle(), 45);
}

/**
 * @tc.name: UIPathAnimatorCallbackApplyFrameExternalDist_001
 * @tc.desc: Verify ApplyFrame works as the standalone effect-execution entry driven by
 *           an external progress sequence (no SetDuration, no Callback, no frameCount_):
 *           base syncs on the first call, and positions follow the given dist exactly.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackApplyFrameExternalDist_001, TestSize.Level1)
{
    UIView view;
    view.SetPosition(BASE_X, BASE_Y);
    PathAnimatorCallback callback;
    EXPECT_TRUE(callback.SetPathString(PATH_LINE_X)); // NOTE: no SetDuration needed
    callback.ApplyFrame(&view, 0);                    // first call: base synced, dist 0
    EXPECT_EQ(view.GetX(), BASE_X);
    EXPECT_EQ(view.GetY(), BASE_Y);
    callback.ApplyFrame(&view, 500);                  // 50%: base + 120
    EXPECT_EQ(view.GetX(), BASE_X + 120);
    callback.ApplyFrame(&view, 1000);                 // 100%: base + 240
    EXPECT_EQ(view.GetX(), BASE_X + 240);
    EXPECT_EQ(view.GetY(), BASE_Y);
}

/* ============ integration with Animator (L2) ============ */

/**
 * @tc.name: UIPathAnimatorCallbackAnimatorDrivenRunToPosition_001
 * @tc.desc: Verify the callback is correctly driven by an Animator end to end:
 *           Start registers it to AnimatorManager, TaskManager drives the frames,
 *           and a non-repeating Animator stops naturally at the path end.
 * @tc.type: FUNC
 * @tc.require: SR000H6TLV
 */
HWTEST_F(PathAnimatorCallbackTest, UIPathAnimatorCallbackAnimatorDrivenRunToPosition_001, TestSize.Level2)
{
    UIView view;
    view.SetPosition(BASE_X, BASE_Y);
    PathAnimatorCallback callback;
    callback.SetDuration(PATH_DURATION);
    EXPECT_TRUE(callback.SetPathString(PATH_LINE_X));

    Animator animator(&callback, &view, PATH_DURATION, false); // non-repeating
    AnimatorManager::GetInstance()->Init();
    animator.Start();
    EXPECT_EQ(animator.GetState(), Animator::START);
    TaskManager::GetInstance()->SetTaskRun(true);
    while (animator.GetState() != Animator::STOP) {
        TaskManager::GetInstance()->TaskHandler();
    }
    EXPECT_EQ(view.GetX(), BASE_X + 240);
    EXPECT_EQ(view.GetY(), BASE_Y);
    TaskManager::GetInstance()->SetTaskRun(false);
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}
} // namespace OHOS
