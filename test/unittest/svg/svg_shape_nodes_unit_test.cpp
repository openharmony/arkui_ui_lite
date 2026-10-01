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

#include <gtest/gtest.h>
#include "svg/svg_document.h"
#include "svg/svg_paint_servers.h"
#include "svg/svg_shape_nodes.h"
#include "components/ui_canvas.h"

using namespace testing::ext;

namespace OHOS {

namespace {

class TestUICanvas : public UICanvas {
public:
    TestUICanvas() {}
    virtual ~TestUICanvas() {}
    UICanvasVertices* GetPath() { return vertices_; }
};

class TestSvgPathNode : public SvgPathNode {
public:
    using SvgPathNode::RecordGeometry;
};

class TestSvgCircleNode : public SvgCircleNode {
public:
    using SvgCircleNode::RecordGeometry;
    const Paint& GetInheritedPaint() const
    {
        return inheritedPaint_;
    }
};

class TestSvgEllipseNode : public SvgCircleNode {
public:
    TestSvgEllipseNode() : SvgCircleNode(SvgCircleNode::Mode::ELLIPSE) {}
    using SvgCircleNode::RecordGeometry;
};

class TestSvgRectNode : public SvgRectNode {
public:
    using SvgRectNode::RecordGeometry;
    const Paint& GetInheritedPaint() const
    {
        return inheritedPaint_;
    }
};

class TestSvgLineNode : public SvgLineNode {
public:
    explicit TestSvgLineNode(Mode mode) : SvgLineNode(mode) {}
    using SvgLineNode::RecordGeometry;
};

uint32_t CountVertices(UICanvasVertices* path)
{
    return (path == nullptr) ? 0 : path->GetTotalVertices();
}

struct VertexExpectation {
    uint32_t cmd;
    float x;
    float y;
    float tol = 0.0f;
};

void ExpectVertex(UICanvasVertices* path, uint32_t idx, const VertexExpectation& expected)
{
    ASSERT_NE(path, nullptr);
    float x = 0.0f;
    float y = 0.0f;
    path->Rewind(idx);
    uint32_t cmd = path->GenerateVertex(&x, &y);
    ASSERT_NE(cmd, PATH_CMD_STOP);
    EXPECT_EQ(cmd, expected.cmd);
    if (expected.tol == 0.0f) {
        EXPECT_FLOAT_EQ(x, expected.x);
        EXPECT_FLOAT_EQ(y, expected.y);
    } else {
        EXPECT_NEAR(x, expected.x, expected.tol);
        EXPECT_NEAR(y, expected.y, expected.tol);
    }
}

} // namespace

class SvgShapeNodesTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(SvgShapeNodesTest, RectBounds_001, TestSize.Level1)
{
    SvgRectNode rect;
    EXPECT_TRUE(rect.SetAttribute("x", "10"));
    EXPECT_TRUE(rect.SetAttribute("y", "20"));
    EXPECT_TRUE(rect.SetAttribute("width", "30"));
    EXPECT_TRUE(rect.SetAttribute("height", "40"));
    Rect b = rect.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 10);
    EXPECT_EQ(b.GetTop(), 20);
    EXPECT_EQ(b.GetWidth(), 30);
    EXPECT_EQ(b.GetHeight(), 40);
}

HWTEST_F(SvgShapeNodesTest, EllipseViaCircleNode_002, TestSize.Level1)
{
    SvgCircleNode ellipse(SvgCircleNode::Mode::ELLIPSE);
    EXPECT_TRUE(ellipse.SetAttribute("cx", "50"));
    EXPECT_TRUE(ellipse.SetAttribute("rx", "30"));
    EXPECT_TRUE(ellipse.SetAttribute("ry", "10"));
    Rect b = ellipse.GetLocalBounds();
    EXPECT_EQ(b.GetWidth(), 60);
    EXPECT_EQ(b.GetHeight(), 20);
}

HWTEST_F(SvgShapeNodesTest, PathBounds_003, TestSize.Level1)
{
    SvgPathNode path;
    EXPECT_TRUE(path.SetAttribute("d", "M0 0 L30 0 L30 40 Z"));
    Rect b = path.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 0);
    EXPECT_EQ(b.GetTop(), 0);
    EXPECT_EQ(b.GetRight(), 30);
    EXPECT_EQ(b.GetBottom(), 40);
}

HWTEST_F(SvgShapeNodesTest, CircleBounds_004, TestSize.Level1)
{
    SvgCircleNode circle(SvgCircleNode::Mode::CIRCLE);
    EXPECT_TRUE(circle.SetAttribute("cx", "50"));
    EXPECT_TRUE(circle.SetAttribute("r", "30"));
    Rect b = circle.GetLocalBounds();
    EXPECT_EQ(b.GetWidth(), 60);
    EXPECT_EQ(b.GetHeight(), 60);
}

HWTEST_F(SvgShapeNodesTest, LineBounds_005, TestSize.Level1)
{
    SvgLineNode line(SvgLineNode::LINE);
    EXPECT_TRUE(line.SetAttribute("x1", "10"));
    EXPECT_TRUE(line.SetAttribute("y1", "20"));
    EXPECT_TRUE(line.SetAttribute("x2", "50"));
    EXPECT_TRUE(line.SetAttribute("y2", "60"));
    Rect b = line.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 10);
    EXPECT_EQ(b.GetTop(), 20);
    EXPECT_EQ(b.GetRight(), 50);
    EXPECT_EQ(b.GetBottom(), 60);
}

HWTEST_F(SvgShapeNodesTest, PolylineBounds_006, TestSize.Level1)
{
    SvgLineNode polyline(SvgLineNode::POLYLINE);
    EXPECT_TRUE(polyline.SetAttribute("points", "0 0 30 0 30 40"));
    Rect b = polyline.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 0);
    EXPECT_EQ(b.GetTop(), 0);
    EXPECT_EQ(b.GetRight(), 30);
    EXPECT_EQ(b.GetBottom(), 40);
}

HWTEST_F(SvgShapeNodesTest, PolygonBounds_007, TestSize.Level1)
{
    SvgLineNode polygon(SvgLineNode::POLYGON);
    EXPECT_TRUE(polygon.SetAttribute("points", "0 0 30 0 30 40"));
    Rect b = polygon.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 0);
    EXPECT_EQ(b.GetTop(), 0);
    EXPECT_EQ(b.GetRight(), 30);
    EXPECT_EQ(b.GetBottom(), 40);
}

HWTEST_F(SvgShapeNodesTest, RectRoundCornerAttributes_008, TestSize.Level1)
{
    SvgRectNode rect;
    EXPECT_TRUE(rect.SetAttribute("rx", "5"));
    EXPECT_TRUE(rect.SetAttribute("ry", "10"));
    SUCCEED();
}

HWTEST_F(SvgShapeNodesTest, PathBoundsZReset_009, TestSize.Level1)
{
    SvgPathNode path;
    EXPECT_TRUE(path.SetAttribute("d", "M0 0 L10 0 L10 10 Z l5 5"));
    Rect b = path.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 0);
    EXPECT_EQ(b.GetTop(), 0);
    EXPECT_EQ(b.GetRight(), 10);
    EXPECT_EQ(b.GetBottom(), 10);
}

HWTEST_F(SvgShapeNodesTest, PathRecordedScaleTransform_010, TestSize.Level1)
{
    TestSvgPathNode path;
    path.SetAttribute("d", "M0 0 L10 0");
    Paint parentPaint;
    parentPaint.SetStyle(Paint::FILL_STYLE);
    TransAffine parentTransform = TransAffine::TransAffineScaling(2.0f, 2.0f);
    path.ApplyInheritedState(parentPaint, parentTransform);
    TestUICanvas canvas;
    path.RecordGeometry(canvas);
    UICanvasVertices* vertices = canvas.GetPath();
    ASSERT_NE(vertices, nullptr);
    ExpectVertex(vertices, 0, {PATH_CMD_MOVE_TO, 0.0f, 0.0f});
    ExpectVertex(vertices, 1, {PATH_CMD_LINE_TO, 20.0f, 0.0f});
}

HWTEST_F(SvgShapeNodesTest, CircleRecordedScaleTransform_011, TestSize.Level1)
{
    TestSvgCircleNode circle;
    circle.SetAttribute("cx", "10");
    circle.SetAttribute("cy", "10");
    circle.SetAttribute("r", "10");
    Paint parentPaint;
    TransAffine parentTransform = TransAffine::TransAffineScaling(2.0f, 2.0f);
    circle.ApplyInheritedState(parentPaint, parentTransform);
    TestUICanvas canvas;
    circle.RecordGeometry(canvas);
    UICanvasVertices* vertices = canvas.GetPath();
    ASSERT_NE(vertices, nullptr);
    ASSERT_GT(vertices->GetTotalVertices(), 3u);
    float maxX = 0.0f;
    float maxY = 0.0f;
    for (uint32_t i = 0; i < vertices->GetTotalVertices(); i++) {
        float x = 0.0f;
        float y = 0.0f;
        vertices->Rewind(i);
        uint32_t cmd = vertices->GenerateVertex(&x, &y);
        if (cmd == PATH_CMD_STOP) {
            break;
        }
        if (x > maxX) {
            maxX = x;
        }
        if (y > maxY) {
            maxY = y;
        }
    }
    // Unlike a path, a circle keeps its vertices in local coordinates and hands the world matrix
    // to the rasterizer through the paint, so the float CTM survives and the outline stays smooth.
    // The recorded extent therefore stays at cx + r, and the 2x scale shows up on the paint.
    EXPECT_NEAR(maxX, 20.0f, 1.0f);
    EXPECT_NEAR(maxY, 20.0f, 1.0f);
    TransAffine paintTransform = circle.GetInheritedPaint().GetTransAffine();
    EXPECT_NEAR(paintTransform.GetData()[0], 2.0f, 0.01f);
    EXPECT_NEAR(paintTransform.GetData()[4], 2.0f, 0.01f);
}

/**
 * @tc.name: RectEmptyBoundsForNonPositiveSize_012
 * @tc.desc: Verify a rect with a non positive width or height reports empty local bounds.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, RectEmptyBoundsForNonPositiveSize_012, TestSize.Level1)
{
    SvgRectNode noSize;
    Rect empty = noSize.GetLocalBounds();
    EXPECT_EQ(empty.GetLeft(), 0);
    EXPECT_EQ(empty.GetTop(), 0);
    EXPECT_EQ(empty.GetRight(), 0);
    EXPECT_EQ(empty.GetBottom(), 0);

    SvgRectNode zeroHeight;
    EXPECT_TRUE(zeroHeight.SetAttribute("width", "20"));
    EXPECT_TRUE(zeroHeight.SetAttribute("height", "0"));
    EXPECT_EQ(zeroHeight.GetLocalBounds().GetRight(), 0);
}

/**
 * @tc.name: RectRejectsUnknownGeometryAttribute_013
 * @tc.desc: Verify an attribute that is neither a paint nor a rect geometry attribute is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, RectRejectsUnknownGeometryAttribute_013, TestSize.Level1)
{
    SvgRectNode rect;
    EXPECT_FALSE(rect.SetAttribute("cx", "1"));
    EXPECT_FALSE(rect.SetAttribute("points", "0 0"));
}

/**
 * @tc.name: RectRecordsSquareCornerPath_014
 * @tc.desc: Verify a rect without corner radii records a plain rectangular path.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, RectRecordsSquareCornerPath_014, TestSize.Level1)
{
    TestSvgRectNode rect;
    rect.SetAttribute("width", "20");
    rect.SetAttribute("height", "20");
    rect.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas canvas;
    rect.RecordGeometry(canvas);
    EXPECT_GT(CountVertices(canvas.GetPath()), 3u);
}

/**
 * @tc.name: RectSkipsRecordForNonPositiveSize_015
 * @tc.desc: Verify a rect with no size records nothing on the canvas.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, RectSkipsRecordForNonPositiveSize_015, TestSize.Level1)
{
    TestSvgRectNode rect;
    rect.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas canvas;
    rect.RecordGeometry(canvas);
    EXPECT_EQ(canvas.GetPath(), nullptr);
}

/**
 * @tc.name: RectRecordsRoundedCornerPath_016
 * @tc.desc: Verify a rect with both corner radii records the rounded corner path.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, RectRecordsRoundedCornerPath_016, TestSize.Level1)
{
    TestSvgRectNode rect;
    rect.SetAttribute("width", "20");
    rect.SetAttribute("height", "20");
    rect.SetAttribute("rx", "4");
    rect.SetAttribute("ry", "4");
    rect.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas canvas;
    rect.RecordGeometry(canvas);
    EXPECT_GT(CountVertices(canvas.GetPath()), 0u);
}

/**
 * @tc.name: RectCornerRadiusFallsBackToTheOtherAxis_017
 * @tc.desc: Verify that setting only rx or only ry still produces a rounded corner, because the
 *           missing radius falls back to the provided one.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, RectCornerRadiusFallsBackToTheOtherAxis_017, TestSize.Level1)
{
    TestSvgRectNode onlyRx;
    onlyRx.SetAttribute("width", "20");
    onlyRx.SetAttribute("height", "20");
    onlyRx.SetAttribute("rx", "4");
    onlyRx.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas rxCanvas;
    onlyRx.RecordGeometry(rxCanvas);
    EXPECT_GT(CountVertices(rxCanvas.GetPath()), 0u);

    TestSvgRectNode onlyRy;
    onlyRy.SetAttribute("width", "20");
    onlyRy.SetAttribute("height", "20");
    onlyRy.SetAttribute("ry", "4");
    onlyRy.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas ryCanvas;
    onlyRy.RecordGeometry(ryCanvas);
    EXPECT_GT(CountVertices(ryCanvas.GetPath()), 0u);
}

/**
 * @tc.name: RectCornerRadiusClampedToHalfSize_018
 * @tc.desc: Verify an oversized corner radius is clamped to half the rect size instead of
 *           producing a degenerate path.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, RectCornerRadiusClampedToHalfSize_018, TestSize.Level1)
{
    TestSvgRectNode rect;
    rect.SetAttribute("width", "20");
    rect.SetAttribute("height", "10");
    rect.SetAttribute("rx", "1000");
    rect.SetAttribute("ry", "1000");
    rect.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas canvas;
    rect.RecordGeometry(canvas);
    EXPECT_GT(CountVertices(canvas.GetPath()), 0u);
}

/**
 * @tc.name: CircleEmptyBoundsForNonPositiveRadius_019
 * @tc.desc: Verify a circle without a radius reports empty bounds and records nothing.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, CircleEmptyBoundsForNonPositiveRadius_019, TestSize.Level1)
{
    TestSvgCircleNode circle;
    Rect b = circle.GetLocalBounds();
    EXPECT_EQ(b.GetRight(), 0);
    EXPECT_EQ(b.GetBottom(), 0);
    circle.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas canvas;
    circle.RecordGeometry(canvas);
    EXPECT_EQ(canvas.GetPath(), nullptr);
}

/**
 * @tc.name: EllipseEmptyBoundsForNonPositiveRadii_020
 * @tc.desc: Verify an ellipse missing either radius reports empty bounds and records nothing.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, EllipseEmptyBoundsForNonPositiveRadii_020, TestSize.Level1)
{
    TestSvgEllipseNode noRadii;
    EXPECT_EQ(noRadii.GetLocalBounds().GetRight(), 0);

    TestSvgEllipseNode onlyRx;
    EXPECT_TRUE(onlyRx.SetAttribute("rx", "10"));
    EXPECT_EQ(onlyRx.GetLocalBounds().GetRight(), 0);
    onlyRx.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas canvas;
    onlyRx.RecordGeometry(canvas);
    EXPECT_EQ(canvas.GetPath(), nullptr);
}

/**
 * @tc.name: CircleRejectsUnknownGeometryAttribute_021
 * @tc.desc: Verify a circle rejects attributes that belong to other shapes.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, CircleRejectsUnknownGeometryAttribute_021, TestSize.Level1)
{
    SvgCircleNode circle(SvgCircleNode::Mode::CIRCLE);
    EXPECT_FALSE(circle.SetAttribute("width", "10"));
    EXPECT_FALSE(circle.SetAttribute("d", "M0 0"));
}

/**
 * @tc.name: LineRejectsPolylineAttribute_022
 * @tc.desc: Verify LINE mode only accepts x1/y1/x2/y2 and polyline mode only accepts points.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, LineRejectsPolylineAttribute_022, TestSize.Level1)
{
    SvgLineNode line(SvgLineNode::LINE);
    EXPECT_FALSE(line.SetAttribute("points", "0 0 1 1"));

    SvgLineNode polyline(SvgLineNode::POLYLINE);
    EXPECT_FALSE(polyline.SetAttribute("x1", "10"));
    EXPECT_TRUE(polyline.SetAttribute("points", "0 0 1 1"));
}

/**
 * @tc.name: PolylineEmptyPointsRecordsNothing_023
 * @tc.desc: Verify a polyline without points reports empty bounds and records nothing.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PolylineEmptyPointsRecordsNothing_023, TestSize.Level1)
{
    TestSvgLineNode polyline(SvgLineNode::POLYLINE);
    Rect b = polyline.GetLocalBounds();
    EXPECT_EQ(b.GetRight(), 0);
    EXPECT_EQ(b.GetBottom(), 0);
    polyline.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas canvas;
    polyline.RecordGeometry(canvas);
    EXPECT_EQ(canvas.GetPath(), nullptr);
}

/**
 * @tc.name: LineRecordsTwoPointPath_024
 * @tc.desc: Verify LINE mode records a move and a line vertex.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, LineRecordsTwoPointPath_024, TestSize.Level1)
{
    TestSvgLineNode line(SvgLineNode::LINE);
    line.SetAttribute("x1", "0");
    line.SetAttribute("y1", "0");
    line.SetAttribute("x2", "10");
    line.SetAttribute("y2", "0");
    line.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas canvas;
    line.RecordGeometry(canvas);
    EXPECT_GE(CountVertices(canvas.GetPath()), 2u);
}

/**
 * @tc.name: PolygonRecordsClosedPath_025
 * @tc.desc: Verify polygon mode records the point list and closes the path.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PolygonRecordsClosedPath_025, TestSize.Level1)
{
    TestSvgLineNode polygon(SvgLineNode::POLYGON);
    polygon.SetAttribute("points", "0 0 10 0 10 10");
    polygon.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas polygonCanvas;
    polygon.RecordGeometry(polygonCanvas);
    EXPECT_GE(CountVertices(polygonCanvas.GetPath()), 3u);

    TestSvgLineNode polyline(SvgLineNode::POLYLINE);
    polyline.SetAttribute("points", "0 0 10 0 10 10");
    polyline.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas polylineCanvas;
    polyline.RecordGeometry(polylineCanvas);
    EXPECT_GE(CountVertices(polylineCanvas.GetPath()), 3u);
}

/**
 * @tc.name: PathRejectsUnknownGeometryAttribute_026
 * @tc.desc: Verify a path node only accepts the "d" attribute as geometry.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PathRejectsUnknownGeometryAttribute_026, TestSize.Level1)
{
    SvgPathNode path;
    EXPECT_FALSE(path.SetAttribute("width", "10"));
    EXPECT_EQ(path.GetPathData(), nullptr);
}

/**
 * @tc.name: PathWithoutDataRecordsNothing_027
 * @tc.desc: Verify a path without "d" reports empty bounds and records nothing.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PathWithoutDataRecordsNothing_027, TestSize.Level1)
{
    TestSvgPathNode path;
    Rect b = path.GetLocalBounds();
    EXPECT_EQ(b.GetRight(), 0);
    EXPECT_EQ(b.GetBottom(), 0);
    path.ApplyInheritedState(Paint(), TransAffine());
    TestUICanvas canvas;
    path.RecordGeometry(canvas);
    EXPECT_EQ(canvas.GetPath(), nullptr);
}

/**
 * @tc.name: PathEmptyDataHasEmptyBounds_028
 * @tc.desc: Verify an empty "d" string is stored but yields empty bounds.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PathEmptyDataHasEmptyBounds_028, TestSize.Level1)
{
    SvgPathNode path;
    EXPECT_TRUE(path.SetAttribute("d", ""));
    ASSERT_NE(path.GetPathData(), nullptr);
    Rect b = path.GetLocalBounds();
    EXPECT_EQ(b.GetRight(), 0);
    EXPECT_EQ(b.GetBottom(), 0);
}

/**
 * @tc.name: PathBoundsHorizontalAndVertical_029
 * @tc.desc: Verify the H and V bounds commands update only one axis each.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PathBoundsHorizontalAndVertical_029, TestSize.Level1)
{
    SvgPathNode path;
    EXPECT_TRUE(path.SetAttribute("d", "M0 0 H30 V40"));
    Rect b = path.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 0);
    EXPECT_EQ(b.GetTop(), 0);
    EXPECT_EQ(b.GetRight(), 30);
    EXPECT_EQ(b.GetBottom(), 40);
}

/**
 * @tc.name: PathBoundsCubicAndSmoothCubic_030
 * @tc.desc: Verify the C and S bounds commands include their control points and the reflected
 *           control point of the smooth variant.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PathBoundsCubicAndSmoothCubic_030, TestSize.Level1)
{
    SvgPathNode path;
    EXPECT_TRUE(path.SetAttribute("d", "M0 0 C10 -10 20 30 30 0 S50 20 60 0"));
    Rect b = path.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 0);
    EXPECT_EQ(b.GetTop(), -30);
    EXPECT_EQ(b.GetRight(), 60);
    EXPECT_EQ(b.GetBottom(), 30);
}

/**
 * @tc.name: PathBoundsQuadraticAndSmoothQuadratic_031
 * @tc.desc: Verify the Q and T bounds commands include their control points.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PathBoundsQuadraticAndSmoothQuadratic_031, TestSize.Level1)
{
    SvgPathNode path;
    EXPECT_TRUE(path.SetAttribute("d", "M0 0 Q10 20 20 0 T40 0"));
    Rect b = path.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 0);
    EXPECT_EQ(b.GetTop(), -20);
    EXPECT_EQ(b.GetRight(), 40);
    EXPECT_EQ(b.GetBottom(), 20);
}

/**
 * @tc.name: PathBoundsArcEndpoint_032
 * @tc.desc: Verify the A bounds command consumes all seven arc parameters and uses the endpoint.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PathBoundsArcEndpoint_032, TestSize.Level1)
{
    SvgPathNode path;
    EXPECT_TRUE(path.SetAttribute("d", "M0 0 A10 10 0 0 1 20 20"));
    Rect b = path.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 0);
    EXPECT_EQ(b.GetTop(), 0);
    EXPECT_EQ(b.GetRight(), 20);
    EXPECT_EQ(b.GetBottom(), 20);
}

/**
 * @tc.name: PathBoundsRelativeCommands_033
 * @tc.desc: Verify the lower case commands accumulate relative to the current point.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PathBoundsRelativeCommands_033, TestSize.Level1)
{
    SvgPathNode path;
    EXPECT_TRUE(path.SetAttribute("d", "m10 10 l10 0 h10 v10 c0 0 0 0 10 10 z"));
    Rect b = path.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 10);
    EXPECT_EQ(b.GetTop(), 10);
    EXPECT_EQ(b.GetRight(), 40);
    EXPECT_EQ(b.GetBottom(), 30);
}

/**
 * @tc.name: PathBoundsSkipsUnknownCommand_034
 * @tc.desc: Verify an unsupported path command is skipped without stalling the parser.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PathBoundsSkipsUnknownCommand_034, TestSize.Level1)
{
    SvgPathNode path;
    EXPECT_TRUE(path.SetAttribute("d", "M0 0 X5 5 L10 10"));
    Rect b = path.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 0);
    EXPECT_EQ(b.GetTop(), 0);
    EXPECT_EQ(b.GetRight(), 10);
    EXPECT_EQ(b.GetBottom(), 10);
}

/**
 * @tc.name: RectCloneCopiesGeometry_035
 * @tc.desc: Verify SvgRectNode::Clone duplicates position, size and corner radii.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, RectCloneCopiesGeometry_035, TestSize.Level1)
{
    SvgRectNode rect;
    rect.SetAttribute("x", "5");
    rect.SetAttribute("y", "6");
    rect.SetAttribute("width", "20");
    rect.SetAttribute("height", "10");
    rect.SetAttribute("rx", "2");
    rect.SetAttribute("ry", "3");
    SvgElementBase* source = &rect;
    SvgElementBase* clone = source->Clone();
    ASSERT_NE(clone, nullptr);
    SvgRectNode* cloned = dynamic_cast<SvgRectNode*>(clone);
    ASSERT_NE(cloned, nullptr);
    Rect b = cloned->GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 5);
    EXPECT_EQ(b.GetTop(), 6);
    EXPECT_EQ(b.GetWidth(), 20);
    EXPECT_EQ(b.GetHeight(), 10);
    delete clone;
}

/**
 * @tc.name: CircleAndEllipseCloneKeepMode_036
 * @tc.desc: Verify SvgCircleNode::Clone keeps the ellipse flag and the radii.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, CircleAndEllipseCloneKeepMode_036, TestSize.Level1)
{
    SvgCircleNode circle(SvgCircleNode::Mode::CIRCLE);
    circle.SetAttribute("cx", "10");
    circle.SetAttribute("cy", "10");
    circle.SetAttribute("r", "5");
    SvgElementBase* circleSource = &circle;
    SvgElementBase* circleClone = circleSource->Clone();
    ASSERT_NE(circleClone, nullptr);
    SvgCircleNode* clonedCircle = dynamic_cast<SvgCircleNode*>(circleClone);
    ASSERT_NE(clonedCircle, nullptr);
    EXPECT_EQ(clonedCircle->GetLocalBounds().GetWidth(), 10);
    delete circleClone;

    SvgCircleNode ellipse(SvgCircleNode::Mode::ELLIPSE);
    ellipse.SetAttribute("cx", "10");
    ellipse.SetAttribute("cy", "10");
    ellipse.SetAttribute("rx", "8");
    ellipse.SetAttribute("ry", "4");
    SvgElementBase* ellipseSource = &ellipse;
    SvgElementBase* ellipseClone = ellipseSource->Clone();
    ASSERT_NE(ellipseClone, nullptr);
    SvgCircleNode* clonedEllipse = dynamic_cast<SvgCircleNode*>(ellipseClone);
    ASSERT_NE(clonedEllipse, nullptr);
    EXPECT_EQ(clonedEllipse->GetLocalBounds().GetWidth(), 16);
    EXPECT_EQ(clonedEllipse->GetLocalBounds().GetHeight(), 8);
    delete ellipseClone;
}

/**
 * @tc.name: LineCloneCopiesPointList_037
 * @tc.desc: Verify SvgLineNode::Clone copies both the line endpoints and the point list.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, LineCloneCopiesPointList_037, TestSize.Level1)
{
    SvgLineNode line(SvgLineNode::LINE);
    line.SetAttribute("x1", "1");
    line.SetAttribute("y1", "2");
    line.SetAttribute("x2", "11");
    line.SetAttribute("y2", "12");
    SvgElementBase* lineSource = &line;
    SvgElementBase* lineClone = lineSource->Clone();
    ASSERT_NE(lineClone, nullptr);
    SvgLineNode* clonedLine = dynamic_cast<SvgLineNode*>(lineClone);
    ASSERT_NE(clonedLine, nullptr);
    EXPECT_EQ(clonedLine->GetLocalBounds().GetLeft(), 1);
    EXPECT_EQ(clonedLine->GetLocalBounds().GetRight(), 11);
    delete lineClone;

    SvgLineNode polygon(SvgLineNode::POLYGON);
    polygon.SetAttribute("points", "0 0 30 0 30 40");
    SvgElementBase* polygonSource = &polygon;
    SvgElementBase* polygonClone = polygonSource->Clone();
    ASSERT_NE(polygonClone, nullptr);
    SvgLineNode* clonedPolygon = dynamic_cast<SvgLineNode*>(polygonClone);
    ASSERT_NE(clonedPolygon, nullptr);
    EXPECT_EQ(clonedPolygon->GetLocalBounds().GetRight(), 30);
    EXPECT_EQ(clonedPolygon->GetLocalBounds().GetBottom(), 40);
    delete polygonClone;
}

/**
 * @tc.name: PathCloneCopiesData_038
 * @tc.desc: Verify SvgPathNode::Clone duplicates the path data, and clones an empty path safely.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, PathCloneCopiesData_038, TestSize.Level1)
{
    SvgPathNode path;
    path.SetAttribute("d", "M0 0 L30 0 L30 40 Z");
    SvgElementBase* source = &path;
    SvgElementBase* clone = source->Clone();
    ASSERT_NE(clone, nullptr);
    SvgPathNode* clonedPath = dynamic_cast<SvgPathNode*>(clone);
    ASSERT_NE(clonedPath, nullptr);
    ASSERT_NE(clonedPath->GetPathData(), nullptr);
    EXPECT_STREQ(clonedPath->GetPathData(), "M0 0 L30 0 L30 40 Z");
    delete clone;

    SvgPathNode emptyPath;
    SvgElementBase* emptySource = &emptyPath;
    SvgElementBase* emptyClone = emptySource->Clone();
    ASSERT_NE(emptyClone, nullptr);
    SvgPathNode* clonedEmpty = dynamic_cast<SvgPathNode*>(emptyClone);
    ASSERT_NE(clonedEmpty, nullptr);
    EXPECT_EQ(clonedEmpty->GetPathData(), nullptr);
    delete emptyClone;
}

/**
 * @tc.name: RectGradientUsesRecordTransform_039
 * @tc.desc: Verify a rect with objectBoundingBox gradient applies the record transform so that
 *           gradient coordinates live in the same baked-vertex space as the recorded path.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, RectGradientUsesRecordTransform_039, TestSize.Level1)
{
    SvgDocument doc;
    SvgLinearGradientResource* grad = new SvgLinearGradientResource();
    grad->SetAttribute("id", "grad1");
    grad->SetAttribute("x1", "0");
    grad->SetAttribute("y1", "0");
    grad->SetAttribute("x2", "1");
    grad->SetAttribute("y2", "0");
    SvgStopResource* stop = new SvgStopResource();
    stop->SetAttribute("offset", "0");
    stop->SetAttribute("stop-color", "#ff0000");
    grad->AppendChild(stop);
    doc.RegisterResource("grad1", grad);

    TestSvgRectNode rect;
    rect.OnDocumentAttached(&doc);
    rect.SetAttribute("width", "11");
    rect.SetAttribute("height", "11");
    rect.SetAttribute("fill", "url(#grad1)");
    Paint parentPaint;
    parentPaint.SetStyle(Paint::FILL_STYLE);
    rect.ApplyInheritedState(parentPaint, TransAffine::TransAffineScaling(2.0f, 2.0f));

    Paint::LinearGradientPoint pt = rect.GetInheritedPaint().GetLinearGradientPoint();
    // Rect width=11 (inclusive). objectBoundingBox x2 = 1 -> local x2 = 11.
    // record transform = Scale(2,2) -> baked x2 = 22.
    EXPECT_FLOAT_EQ(pt.x1, 22.0f);
    delete grad;
}

/**
 * @tc.name: CircleGradientStaysLocal_040
 * @tc.desc: Verify a circle with userSpaceOnUse gradient keeps gradient coordinates in leaf-local
 *           space while the paint transform carries the record transform.
 * @tc.type: FUNC
 */
HWTEST_F(SvgShapeNodesTest, CircleGradientStaysLocal_040, TestSize.Level1)
{
    SvgDocument doc;
    SvgRadialGradientResource* grad = new SvgRadialGradientResource();
    grad->SetAttribute("id", "grad1");
    grad->SetAttribute("gradientUnits", "userSpaceOnUse");
    grad->SetAttribute("cx", "5");
    grad->SetAttribute("cy", "5");
    grad->SetAttribute("r", "5");
    SvgStopResource* stop = new SvgStopResource();
    stop->SetAttribute("offset", "0");
    stop->SetAttribute("stop-color", "#ff0000");
    grad->AppendChild(stop);
    doc.RegisterResource("grad1", grad);

    TestSvgCircleNode circle;
    circle.OnDocumentAttached(&doc);
    circle.SetAttribute("cx", "5");
    circle.SetAttribute("cy", "5");
    circle.SetAttribute("r", "5");
    circle.SetAttribute("fill", "url(#grad1)");
    circle.ApplyInheritedState(Paint(), TransAffine::TransAffineScaling(2.0f, 2.0f));

    Paint::RadialGradientPoint pt = circle.GetInheritedPaint().GetRadialGradientPoint();
    EXPECT_FLOAT_EQ(pt.x1, 5.0f);
    EXPECT_FLOAT_EQ(pt.y1, 5.0f);
    EXPECT_FLOAT_EQ(pt.r1, 5.0f);

    TransAffine paintTransform = circle.GetInheritedPaint().GetTransAffine();
    EXPECT_NEAR(paintTransform.GetData()[0], 2.0f, 0.01f);
    EXPECT_NEAR(paintTransform.GetData()[4], 2.0f, 0.01f);
    delete grad;
}

} // namespace OHOS
