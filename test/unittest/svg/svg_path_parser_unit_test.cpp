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
#include "components/ui_canvas.h"
#include "gfx_utils/diagram/common/common_basics.h"
#include "gfx_utils/trans_affine.h"
#include "svg/svg_path_parser.h"

using namespace testing::ext;

namespace OHOS {

namespace {

class TestUICanvas : public UICanvas {
public:
    TestUICanvas() {}
    virtual ~TestUICanvas() {}

    UICanvasVertices* GetPath() { return vertices_; }
};

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

class SvgPathParserTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() { canvas_ = new TestUICanvas(); }
    void TearDown()
    {
        delete canvas_;
        canvas_ = nullptr;
    }

    TestUICanvas* canvas_;
};

/**
 * @tc.name: SvgPathParserAbsoluteCommands_001
 * @tc.desc: Verify parser handles all absolute path commands.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserAbsoluteCommands_001, TestSize.Level1)
{
    SvgPathParser parser;
    SvgResult result = parser.Parse("M0,0 L100,0 H150 V100 C200,200 300,0 400,100 "
                                    "S500,200 600,100 Q700,0 800,100 T900,100 A50,50 0 0,1 950,150 Z",
                                    *canvas_);
    EXPECT_EQ(result, SvgResult::SVG_RESULT_OK);
}

/**
 * @tc.name: SvgPathParserRelativeCommands_001
 * @tc.desc: Verify parser handles all relative path commands.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserRelativeCommands_001, TestSize.Level1)
{
    SvgPathParser parser;
    SvgResult result = parser.Parse("m0,0 l100,0 h50 v100 c50,100 150,-100 250,0 "
                                    "s100,100 200,0 q100,-100 200,0 t100,0 a50,50 0 0,1 50,50 z",
                                    *canvas_);
    EXPECT_EQ(result, SvgResult::SVG_RESULT_OK);
}

/**
 * @tc.name: SvgPathParserInvalidInput_001
 * @tc.desc: Verify parser returns invalid param for null data.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserInvalidInput_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse(nullptr, *canvas_), SvgResult::SVG_RESULT_INVALID_PARAM);
}

/**
 * @tc.name: SvgPathParserUnknownCommand_001
 * @tc.desc: Verify parser reports an error when an unknown command is encountered.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserUnknownCommand_001, TestSize.Level1)
{
    SvgPathParser parser;
    SvgResult result = parser.Parse("M0,0 X100,100 L50,50", *canvas_);
    EXPECT_EQ(result, SvgResult::SVG_RESULT_ERROR);
}

/**
 * @tc.name: SvgPathParserImplicitLineTo_001
 * @tc.desc: Verify parser treats coordinate pairs after M/m as implicit lineto commands.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserImplicitLineTo_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M10,10 20,20 30,30", *canvas_), SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(parser.Parse("m10,10 5,5 10,10", *canvas_), SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(parser.Parse("M0,0 L10,10 20,20", *canvas_), SvgResult::SVG_RESULT_OK);
}

/**
 * @tc.name: SvgPathParserEmptyAndWhitespace_001
 * @tc.desc: Verify parser accepts empty and separator-only path data.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserEmptyAndWhitespace_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("", *canvas_), SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(parser.Parse("  , \t\n ", *canvas_), SvgResult::SVG_RESULT_OK);
}

/**
 * @tc.name: SvgPathParserNumberFormats_001
 * @tc.desc: Verify parser handles signs, decimals and exponents in coordinates.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserNumberFormats_001, TestSize.Level1)
{
    SvgPathParser parser;
    SvgResult result = parser.Parse("M1e1,-2.5 L+3.5,4E0 L.5,5.", *canvas_);
    EXPECT_EQ(result, SvgResult::SVG_RESULT_OK);
}

/**
 * @tc.name: SvgPathParserTruncatedCommands_001
 * @tc.desc: Verify parser reports an error for commands with missing coordinates.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserTruncatedCommands_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M10", *canvas_), SvgResult::SVG_RESULT_ERROR);
    EXPECT_EQ(parser.Parse("M0,0 L5", *canvas_), SvgResult::SVG_RESULT_ERROR);
    EXPECT_EQ(parser.Parse("M0,0 H", *canvas_), SvgResult::SVG_RESULT_ERROR);
    EXPECT_EQ(parser.Parse("M0,0 V", *canvas_), SvgResult::SVG_RESULT_ERROR);
    EXPECT_EQ(parser.Parse("M0,0 C1,2 3", *canvas_), SvgResult::SVG_RESULT_ERROR);
    EXPECT_EQ(parser.Parse("M0,0 S1", *canvas_), SvgResult::SVG_RESULT_ERROR);
    EXPECT_EQ(parser.Parse("M0,0 Q1,2 3", *canvas_), SvgResult::SVG_RESULT_ERROR);
    EXPECT_EQ(parser.Parse("M0,0 T", *canvas_), SvgResult::SVG_RESULT_ERROR);
    EXPECT_EQ(parser.Parse("M0,0 A1", *canvas_), SvgResult::SVG_RESULT_ERROR);
}

/**
 * @tc.name: SvgPathParserClosePathMidStream_001
 * @tc.desc: Verify parser resets state at Z and continues with following subpaths.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserClosePathMidStream_001, TestSize.Level1)
{
    SvgPathParser parser;
    SvgResult result = parser.Parse("M0,0 L10,10 Z M20,20 L30,30 Z", *canvas_);
    EXPECT_EQ(result, SvgResult::SVG_RESULT_OK);
}

/**
 * @tc.name: SvgPathParserSeparatorVariants_001
 * @tc.desc: Verify parser accepts tabs, newlines and carriage returns as separators.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserSeparatorVariants_001, TestSize.Level1)
{
    SvgPathParser parser;
    SvgResult result = parser.Parse("M0,0\tL10,10\nL20,20\rL30,30 Z", *canvas_);
    EXPECT_EQ(result, SvgResult::SVG_RESULT_OK);
}

/**
 * @tc.name: SvgPathParserUnknownLowercaseCommand_001
 * @tc.desc: Verify parser reports an error when an unknown lowercase command is encountered.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserUnknownLowercaseCommand_001, TestSize.Level1)
{
    SvgPathParser parser;
    SvgResult result = parser.Parse("M10,10 x5,5 L20,20", *canvas_);
    EXPECT_EQ(result, SvgResult::SVG_RESULT_ERROR);
}

/**
 * @tc.name: SvgPathParserRepeatedMove_001
 * @tc.desc: Verify parser handles repeated moveto commands starting new subpaths.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserRepeatedMove_001, TestSize.Level1)
{
    SvgPathParser parser;
    SvgResult result = parser.Parse("M0,0 M10,10 M20,20 L5,5", *canvas_);
    EXPECT_EQ(result, SvgResult::SVG_RESULT_OK);
}

/**
 * @tc.name: SvgPathParserStuckNumber_001
 * @tc.desc: Verify parser reports an error when a command is followed by a token strtof cannot consume.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserStuckNumber_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M!", *canvas_), SvgResult::SVG_RESULT_ERROR);
    EXPECT_EQ(parser.Parse("M0,0 L!", *canvas_), SvgResult::SVG_RESULT_ERROR);
    EXPECT_EQ(parser.Parse("M0,0 L10,10 .", *canvas_), SvgResult::SVG_RESULT_ERROR);
}

/**
 * @tc.name: SvgPathParserTrailingNumberAfterClose_001
 * @tc.desc: Verify parser terminates on numbers after Z/z (Z takes no parameters).
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserTrailingNumberAfterClose_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M0,0 L10,10 Z 5", *canvas_), SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(parser.Parse("M0,0 L10,10 z 5 6", *canvas_), SvgResult::SVG_RESULT_OK);
}

/**
 * @tc.name: SvgPathParserArcConsumesSevenParams_001
 * @tc.desc: Verify A/a consumes all seven arc parameters so following commands parse correctly.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserArcConsumesSevenParams_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M0,0 A30,30 0 1,1 100,50 Z", *canvas_),
              SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(parser.Parse("M0,0 A30,30 0 1,1 100,50 L200,200 Z", *canvas_),
              SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(parser.Parse("M0,0 A30 30 0 0 1 100 50 A10 10 0 0 1 200 50", *canvas_),
              SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(parser.Parse("m0,0 a30,30 0 1,1 100,50 l10,10 z", *canvas_),
              SvgResult::SVG_RESULT_OK);
}

/**
 * @tc.name: SvgPathParserArcEndPoint_001
 * @tc.desc: Verify the arc approximates a line to its end point and that a following
 *           relative command continues from there.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserArcEndPoint_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M0,0 A30,30 0 1,1 100,50", *canvas_),
              SvgResult::SVG_RESULT_OK);

    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    ASSERT_TRUE(SvgPathPointAt("M0,0 A30,30 0 1,1 100,50", 1.0f, x, y, angle));
    EXPECT_NEAR(x, 100.0f, 0.01f);
    EXPECT_NEAR(y, 50.0f, 0.01f);

    EXPECT_EQ(parser.Parse("M0,0 A30,30 0 1,1 100,50 l10,5", *canvas_),
              SvgResult::SVG_RESULT_OK);
    ASSERT_TRUE(SvgPathPointAt("M0,0 A30,30 0 1,1 100,50 l10,5", 1.0f, x, y, angle));
    EXPECT_NEAR(x, 110.0f, 0.01f);
    EXPECT_NEAR(y, 55.0f, 0.01f);
}

/**
 * @tc.name: SvgPathParserGeometryMoveLine_001
 * @tc.desc: Verify absolute M/L coordinates are recorded in the canvas vertex path.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserGeometryMoveLine_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M10,20 L30,40", *canvas_), SvgResult::SVG_RESULT_OK);
    UICanvasVertices* path = canvas_->GetPath();
    ASSERT_NE(path, nullptr);
    ExpectVertex(path, 0, {PATH_CMD_MOVE_TO, 10.0f, 20.0f});
    ExpectVertex(path, 1, {PATH_CMD_LINE_TO, 30.0f, 40.0f});
}

/**
 * @tc.name: SvgPathParserGeometryRelativeMoveLine_001
 * @tc.desc: Verify relative m/l coordinates are recorded relative to the current point.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserGeometryRelativeMoveLine_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("m10,20 l30,40", *canvas_), SvgResult::SVG_RESULT_OK);
    UICanvasVertices* path = canvas_->GetPath();
    ASSERT_NE(path, nullptr);
    ExpectVertex(path, 0, {PATH_CMD_MOVE_TO, 10.0f, 20.0f});
    ExpectVertex(path, 1, {PATH_CMD_LINE_TO, 40.0f, 60.0f});
}

/**
 * @tc.name: SvgPathParserGeometryCubic_001
 * @tc.desc: Verify C records a CURVE4 segment with the expected control and end points.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserGeometryCubic_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M0,0 C10,10 20,10 30,0", *canvas_), SvgResult::SVG_RESULT_OK);
    UICanvasVertices* path = canvas_->GetPath();
    ASSERT_NE(path, nullptr);
    ExpectVertex(path, 0, {PATH_CMD_MOVE_TO, 0.0f, 0.0f});
    ExpectVertex(path, 1, {PATH_CMD_CURVE4, 10.0f, 10.0f});
    ExpectVertex(path, 2, {PATH_CMD_CURVE4, 20.0f, 10.0f});
    ExpectVertex(path, 3, {PATH_CMD_CURVE4, 30.0f, 0.0f});
}

/**
 * @tc.name: SvgPathParserGeometrySmoothCubic_001
 * @tc.desc: Verify S reflects the previous control point (2*x - ctrlX).
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserGeometrySmoothCubic_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M0,0 C10,10 20,10 30,0 S50,10 60,0", *canvas_), SvgResult::SVG_RESULT_OK);
    UICanvasVertices* path = canvas_->GetPath();
    ASSERT_NE(path, nullptr);
    ExpectVertex(path, 0, {PATH_CMD_MOVE_TO, 0.0f, 0.0f});
    ExpectVertex(path, 1, {PATH_CMD_CURVE4, 10.0f, 10.0f});
    ExpectVertex(path, 2, {PATH_CMD_CURVE4, 20.0f, 10.0f});
    ExpectVertex(path, 3, {PATH_CMD_CURVE4, 30.0f, 0.0f});
    ExpectVertex(path, 4, {PATH_CMD_CURVE4, 40.0f, -10.0f});
    ExpectVertex(path, 5, {PATH_CMD_CURVE4, 50.0f, 10.0f});
    ExpectVertex(path, 6, {PATH_CMD_CURVE4, 60.0f, 0.0f});
}

/**
 * @tc.name: SvgPathParserGeometryQuadratic_001
 * @tc.desc: Verify Q degree-elevates to a cubic CURVE4 segment.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserGeometryQuadratic_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M0,0 Q30,60 60,0", *canvas_), SvgResult::SVG_RESULT_OK);
    UICanvasVertices* path = canvas_->GetPath();
    ASSERT_NE(path, nullptr);
    ExpectVertex(path, 0, {PATH_CMD_MOVE_TO, 0.0f, 0.0f});
    ExpectVertex(path, 1, {PATH_CMD_CURVE4, 20.0f, 40.0f, 0.01f});
    ExpectVertex(path, 2, {PATH_CMD_CURVE4, 40.0f, 40.0f, 0.01f});
    ExpectVertex(path, 3, {PATH_CMD_CURVE4, 60.0f, 0.0f, 0.01f});
}

/**
 * @tc.name: SvgPathParserGeometryArc_001
 * @tc.desc: Verify A subdivides into multiple segments and ends on the target point.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserGeometryArc_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M0,0 A50,50 0 0,1 100,0", *canvas_), SvgResult::SVG_RESULT_OK);
    UICanvasVertices* path = canvas_->GetPath();
    ASSERT_NE(path, nullptr);
    ASSERT_GT(path->GetTotalVertices(), 3u);
    float x = 0.0f;
    float y = 0.0f;
    path->Rewind(path->GetTotalVertices() - 1);
    uint32_t cmd = path->GenerateVertex(&x, &y);
    ASSERT_NE(cmd, PATH_CMD_STOP);
    EXPECT_EQ(cmd, PATH_CMD_CURVE4);
    EXPECT_NEAR(x, 100.0f, 0.5f);
    EXPECT_NEAR(y, 0.0f, 0.5f);
}

/**
 * @tc.name: SvgPathParserGeometryClose_001
 * @tc.desc: Verify Z emits a close polygon command in the vertex path.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserGeometryClose_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M0,0 L10,0 L10,10 Z", *canvas_), SvgResult::SVG_RESULT_OK);
    UICanvasVertices* path = canvas_->GetPath();
    ASSERT_NE(path, nullptr);
    ExpectVertex(path, 0, {PATH_CMD_MOVE_TO, 0.0f, 0.0f});
    ExpectVertex(path, 1, {PATH_CMD_LINE_TO, 10.0f, 0.0f});
    ExpectVertex(path, 2, {PATH_CMD_LINE_TO, 10.0f, 10.0f});
    ExpectVertex(path, 3, {PATH_CMD_END_POLY | PATH_FLAGS_CLOSE, 0.0f, 0.0f});
}

/**
 * @tc.name: SvgPathParserTransformScale_001
 * @tc.desc: Verify parser applies optional transform to all recorded coordinates.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserTransformScale_001, TestSize.Level1)
{
    SvgPathParser parser;
    TransAffine scale = TransAffine::TransAffineScaling(2.0f, 2.0f);
    SvgResult result = parser.Parse("M0,0 L10,0", *canvas_, &scale);
    EXPECT_EQ(result, SvgResult::SVG_RESULT_OK);
    UICanvasVertices* path = canvas_->GetPath();
    ASSERT_NE(path, nullptr);
    ExpectVertex(path, 0, {PATH_CMD_MOVE_TO, 0.0f, 0.0f});
    ExpectVertex(path, 1, {PATH_CMD_LINE_TO, 20.0f, 0.0f});
}

/**
 * @tc.name: SvgPathPointAtNull_001
 * @tc.desc: Verify SvgPathPointAt returns false for null path data.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathPointAtNull_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    EXPECT_FALSE(SvgPathPointAt(nullptr, 0.5f, x, y, angle));
}

/**
 * @tc.name: SvgPathPointAtProgressClamp_001
 * @tc.desc: Verify progress is clamped into [0,1] and maps to start/end points.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathPointAtProgressClamp_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    EXPECT_TRUE(SvgPathPointAt("M0,0 L10,0", -0.5f, x, y, angle));
    EXPECT_NEAR(x, 0.0f, 0.01f);
    EXPECT_NEAR(y, 0.0f, 0.01f);
    EXPECT_TRUE(SvgPathPointAt("M0,0 L10,0", 1.5f, x, y, angle));
    EXPECT_NEAR(x, 10.0f, 0.01f);
    EXPECT_NEAR(y, 0.0f, 0.01f);
}

/**
 * @tc.name: SvgPathPointAtSampling_001
 * @tc.desc: Verify linear path sampling at progress 0, 0.5 and 1.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathPointAtSampling_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    EXPECT_TRUE(SvgPathPointAt("M0,0 L10,0", 0.0f, x, y, angle));
    EXPECT_NEAR(x, 0.0f, 0.01f);
    EXPECT_TRUE(SvgPathPointAt("M0,0 L10,0", 0.5f, x, y, angle));
    EXPECT_NEAR(x, 5.0f, 0.01f);
    EXPECT_NEAR(angle, 0.0f, 0.01f);
    EXPECT_TRUE(SvgPathPointAt("M0,0 L10,0", 1.0f, x, y, angle));
    EXPECT_NEAR(x, 10.0f, 0.01f);
}

/**
 * @tc.name: SvgPathPointAtDegenerate_001
 * @tc.desc: Verify degenerate paths (single point or zero-length) return false.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathPointAtDegenerate_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    EXPECT_FALSE(SvgPathPointAt("M0,0", 0.5f, x, y, angle));
    EXPECT_FALSE(SvgPathPointAt("M0,0 L0,0", 0.5f, x, y, angle));
}

/**
 * @tc.name: SvgPathPointAtUnknownCommand_001
 * @tc.desc: Verify FlattenPath reports failure when the path contains an unknown command.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathPointAtUnknownCommand_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    EXPECT_FALSE(SvgPathPointAt("M0,0 X L10,0", 1.0f, x, y, angle));
}

/**
 * @tc.name: SvgPathParserZeroRadiusArc_001
 * @tc.desc: Verify an arc with a non-positive radius degenerates to a straight line.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserZeroRadiusArc_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M0,0 A0,30 0 0,1 100,50", *canvas_), SvgResult::SVG_RESULT_OK);
    UICanvasVertices* path = canvas_->GetPath();
    ASSERT_NE(path, nullptr);
    ExpectVertex(path, 0, {PATH_CMD_MOVE_TO, 0.0f, 0.0f});
    ExpectVertex(path, 1, {PATH_CMD_LINE_TO, 100.0f, 50.0f});
}

/**
 * @tc.name: SvgPathParserDegenerateArc_001
 * @tc.desc: Verify a self-closing arc (zero delta theta) is handled and clamps segment count.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPathParserTest, SvgPathParserDegenerateArc_001, TestSize.Level1)
{
    SvgPathParser parser;
    EXPECT_EQ(parser.Parse("M0,0 A30,30 0 0,1 0,0", *canvas_), SvgResult::SVG_RESULT_OK);
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    // The arc end point equals the current point, so flattening yields a zero-length polyline.
    // Such a path has no arc length to sample, which SvgPathPointAt reports as a failure.
    EXPECT_FALSE(SvgPathPointAt("M0,0 A30,30 0 0,1 0,0", 1.0f, x, y, angle));
}

} // namespace OHOS
