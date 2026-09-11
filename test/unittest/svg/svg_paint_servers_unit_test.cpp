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
#include "gfx_utils/trans_affine.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_paint_servers.h"

using namespace testing::ext;

namespace OHOS {

class SvgPaintServersTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(SvgPaintServersTest, LinearGradientStops_002, TestSize.Level1)
{
    SvgLinearGradientResource grad;
    grad.SetAttribute("x1", "0");
    grad.SetAttribute("y1", "0");
    grad.SetAttribute("x2", "1");
    grad.SetAttribute("y2", "0");
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    SvgStopResource* s1 = new SvgStopResource();
    s1->SetAttribute("offset", "1");
    s1->SetAttribute("stop-color", "#0000ff");
    grad.AppendChild(s0);
    grad.AppendChild(s1);
    Paint paint;
    grad.ApplyToPaint(paint, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paint.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(paint.GetGradient(), Paint::Linear);
    List<Paint::StopAndColor> stops = paint.getStopAndColor();
    EXPECT_EQ(stops.Size(), 2);
    EXPECT_EQ(stops.Head()->data_.stop, 0.0f);
    EXPECT_EQ(stops.Head()->data_.color.full, SvgAttributeParser::ParseColor("#ff0000").full);
    // Read the second stop via Tail(); do not PopFront() the copied list. getStopAndColor()
    // returns a shallow copy whose nodes are shared with the Paint, so removing one would
    // corrupt the Paint's stop list and crash on access.
    EXPECT_EQ(stops.Tail()->data_.stop, 1.0f);
    EXPECT_EQ(stops.Tail()->data_.color.full, SvgAttributeParser::ParseColor("#0000ff").full);
}

HWTEST_F(SvgPaintServersTest, SolidColorFill_003, TestSize.Level1)
{
    SvgSolidColorResource solid;
    solid.SetAttribute("solid-color", "#00ff00");
    solid.SetAttribute("solid-opacity", "0.5");
    Paint paint;
    solid.ApplyToPaint(paint, {0, 0, 10, 10}, nullptr, true);
    EXPECT_EQ(paint.GetStyle(), Paint::FILL_STYLE);
}

HWTEST_F(SvgPaintServersTest, RadialGradientApply_004, TestSize.Level1)
{
    SvgRadialGradientResource grad;
    grad.SetAttribute("cx", "0.5");
    grad.SetAttribute("cy", "0.5");
    grad.SetAttribute("r", "0.5");
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    grad.AppendChild(s0);
    Paint paint;
    grad.ApplyToPaint(paint, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paint.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(paint.GetGradient(), Paint::Radial);
    List<Paint::StopAndColor> stops = paint.getStopAndColor();
    EXPECT_EQ(stops.Size(), 1);
    EXPECT_EQ(stops.Head()->data_.stop, 0.0f);
    EXPECT_EQ(stops.Head()->data_.color.full, SvgAttributeParser::ParseColor("#ff0000").full);
}

HWTEST_F(SvgPaintServersTest, GradientCloneStops_005, TestSize.Level1)
{
    SvgLinearGradientResource grad;
    grad.SetAttribute("x1", "0");
    grad.SetAttribute("y1", "0");
    grad.SetAttribute("x2", "1");
    grad.SetAttribute("y2", "0");
    SvgElementBase* clone = grad.Clone();
    EXPECT_NE(clone, nullptr);
    SvgLinearGradientResource* clonedGrad = dynamic_cast<SvgLinearGradientResource*>(clone);
    EXPECT_NE(clonedGrad, nullptr);
    delete clone;
}

HWTEST_F(SvgPaintServersTest, GradientClonePreservesStops_009, TestSize.Level1)
{
    SvgLinearGradientResource grad;
    grad.SetAttribute("x1", "0");
    grad.SetAttribute("y1", "0");
    grad.SetAttribute("x2", "1");
    grad.SetAttribute("y2", "0");
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    SvgStopResource* s1 = new SvgStopResource();
    s1->SetAttribute("offset", "1");
    s1->SetAttribute("stop-color", "#0000ff");
    grad.AppendChild(s0);
    grad.AppendChild(s1);
    SvgElementBase* clone = grad.Clone();
    EXPECT_NE(clone, nullptr);
    SvgLinearGradientResource* clonedGrad = dynamic_cast<SvgLinearGradientResource*>(clone);
    EXPECT_NE(clonedGrad, nullptr);
    Paint paint;
    clonedGrad->ApplyToPaint(paint, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paint.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(paint.GetGradient(), Paint::Linear);
    List<Paint::StopAndColor> stops = paint.getStopAndColor();
    EXPECT_EQ(stops.Size(), 2);
    EXPECT_EQ(stops.Head()->data_.stop, 0.0f);
    EXPECT_EQ(stops.Head()->data_.color.full, SvgAttributeParser::ParseColor("#ff0000").full);
    // Read the second stop via Tail(); do not PopFront() the copied list (shared nodes).
    EXPECT_EQ(stops.Tail()->data_.stop, 1.0f);
    EXPECT_EQ(stops.Tail()->data_.color.full, SvgAttributeParser::ParseColor("#0000ff").full);
    delete clone;
}

HWTEST_F(SvgPaintServersTest, DefsResourceCategory_006, TestSize.Level1)
{
    SvgDefsResource defs;
    EXPECT_EQ(defs.GetCategory(), SVG_CATEGORY_GENERIC);
}

HWTEST_F(SvgPaintServersTest, StopResourceCategory_007, TestSize.Level1)
{
    SvgStopResource stop;
    EXPECT_EQ(stop.GetCategory(), SVG_CATEGORY_RESOURCE);
}

HWTEST_F(SvgPaintServersTest, LinearGradientGradientUnits_008, TestSize.Level1)
{
    SvgLinearGradientResource grad;
    grad.SetAttribute("gradientUnits", "userSpaceOnUse");
    // A gradient without stops paints nothing per the SVG spec, so give it stops before
    // checking that userSpaceOnUse coordinates still produce a gradient paint.
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    SvgStopResource* s1 = new SvgStopResource();
    s1->SetAttribute("offset", "1");
    s1->SetAttribute("stop-color", "#0000ff");
    grad.AppendChild(s0);
    grad.AppendChild(s1);
    Paint paint;
    grad.ApplyToPaint(paint, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paint.GetStyle(), Paint::GRADIENT);
}

/**
 * @tc.name: SvgStopResourceAttributes_001
 * @tc.desc: Verify stop offset percentage/clamp parsing, color/opacity and Clone.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgStopResourceAttributes_001, TestSize.Level1)
{
    SvgStopResource stop;
    EXPECT_TRUE(stop.SetAttribute("offset", "50%"));
    EXPECT_FLOAT_EQ(stop.GetOffset(), 0.5f);
    EXPECT_TRUE(stop.SetAttribute("offset", "-1"));
    EXPECT_FLOAT_EQ(stop.GetOffset(), 0.0f);
    EXPECT_TRUE(stop.SetAttribute("offset", "2"));
    EXPECT_FLOAT_EQ(stop.GetOffset(), 1.0f);
    EXPECT_TRUE(stop.SetAttribute("stop-color", "#00ff00"));
    EXPECT_EQ(stop.GetColor().full, SvgAttributeParser::ParseColor("#00ff00").full);
    EXPECT_TRUE(stop.SetAttribute("stop-opacity", "0.5"));
    EXPECT_EQ(stop.GetOpacity(), static_cast<uint8_t>(OPA_OPAQUE / 2));
    EXPECT_FALSE(stop.SetAttribute("unknown", "x"));
    EXPECT_FALSE(stop.SetAttribute(nullptr, "x"));
    EXPECT_FALSE(stop.SetAttribute("offset", nullptr));

    SvgElementBase* clone = stop.Clone();
    ASSERT_NE(clone, nullptr);
    SvgStopResource* clonedStop = dynamic_cast<SvgStopResource*>(clone);
    ASSERT_NE(clonedStop, nullptr);
    EXPECT_FLOAT_EQ(clonedStop->GetOffset(), 1.0f);
    EXPECT_EQ(clonedStop->GetColor().full, SvgAttributeParser::ParseColor("#00ff00").full);
    delete clone;
}

/**
 * @tc.name: SvgPaintServersSetAttributeInvalid_001
 * @tc.desc: Verify SetAttribute rejects null name/value and unknown attributes.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgPaintServersSetAttributeInvalid_001, TestSize.Level1)
{
    SvgLinearGradientResource lin;
    EXPECT_FALSE(lin.SetAttribute(nullptr, "x"));
    EXPECT_FALSE(lin.SetAttribute("x1", nullptr));
    EXPECT_FALSE(lin.SetAttribute("unknown", "x"));

    SvgRadialGradientResource rad;
    EXPECT_FALSE(rad.SetAttribute(nullptr, "x"));
    EXPECT_FALSE(rad.SetAttribute("cx", nullptr));
    EXPECT_FALSE(rad.SetAttribute("unknown", "x"));

    SvgSolidColorResource solid;
    EXPECT_FALSE(solid.SetAttribute(nullptr, "x"));
    EXPECT_FALSE(solid.SetAttribute("solid-color", nullptr));
    EXPECT_FALSE(solid.SetAttribute("unknown", "x"));

    SvgDefsResource defs;
    EXPECT_FALSE(defs.SetAttribute(nullptr, "x"));
    EXPECT_FALSE(defs.SetAttribute("id", nullptr));
    EXPECT_FALSE(defs.SetAttribute("unknown", "x"));
}

/**
 * @tc.name: SvgLinearGradientSpread_001
 * @tc.desc: Verify reflect and repeat spread methods build a gradient with stops.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgLinearGradientSpread_001, TestSize.Level1)
{
    SvgLinearGradientResource grad;
    grad.SetAttribute("x1", "0");
    grad.SetAttribute("y1", "0");
    grad.SetAttribute("x2", "1");
    grad.SetAttribute("y2", "0");
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    SvgStopResource* s1 = new SvgStopResource();
    s1->SetAttribute("offset", "1");
    s1->SetAttribute("stop-color", "#0000ff");
    grad.AppendChild(s0);
    grad.AppendChild(s1);

    grad.SetAttribute("spreadMethod", "reflect");
    Paint paintReflect;
    grad.ApplyToPaint(paintReflect, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paintReflect.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(paintReflect.GetGradient(), Paint::Linear);
    EXPECT_GT(paintReflect.getStopAndColor().Size(), 0);

    grad.SetAttribute("spreadMethod", "repeat");
    Paint paintRepeat;
    grad.ApplyToPaint(paintRepeat, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paintRepeat.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(paintRepeat.GetGradient(), Paint::Linear);
    EXPECT_GT(paintRepeat.getStopAndColor().Size(), 0);
}

/**
 * @tc.name: SvgLinearGradientTransformAndCoords_001
 * @tc.desc: Verify percentage coords, explicit gradientUnits and gradientTransform.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgLinearGradientTransformAndCoords_001, TestSize.Level1)
{
    SvgLinearGradientResource grad;
    EXPECT_TRUE(grad.SetAttribute("x1", "50%"));
    EXPECT_TRUE(grad.SetAttribute("y1", "50%"));
    EXPECT_TRUE(grad.SetAttribute("x2", "50%"));
    EXPECT_TRUE(grad.SetAttribute("y2", "50%"));
    EXPECT_TRUE(grad.SetAttribute("gradientUnits", "objectBoundingBox"));
    EXPECT_TRUE(grad.SetAttribute("gradientTransform", "translate(10,20)"));
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    grad.AppendChild(s0);
    Paint paint;
    grad.ApplyToPaint(paint, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paint.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(paint.GetGradient(), Paint::Linear);
}

/**
 * @tc.name: SvgLinearGradientEmpty_001
 * @tc.desc: Verify ApplyToPaint returns early and adds no stops when there are no children.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgLinearGradientEmpty_001, TestSize.Level1)
{
    SvgLinearGradientResource grad;
    Paint paint;
    grad.ApplyToPaint(paint, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paint.getStopAndColor().Size(), 0);
}

/**
 * @tc.name: SvgLinearGradientAppendNull_001
 * @tc.desc: Verify AppendChild ignores a null child without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgLinearGradientAppendNull_001, TestSize.Level1)
{
    SvgLinearGradientResource grad;
    grad.AppendChild(nullptr);
    EXPECT_EQ(grad.GetCategory(), SVG_CATEGORY_RESOURCE);
}

/**
 * @tc.name: SvgRadialGradientSpread_001
 * @tc.desc: Verify radial reflect/repeat spread methods build a radial gradient.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgRadialGradientSpread_001, TestSize.Level1)
{
    SvgRadialGradientResource grad;
    grad.SetAttribute("cx", "0.5");
    grad.SetAttribute("cy", "0.5");
    grad.SetAttribute("r", "0.5");
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    grad.AppendChild(s0);

    grad.SetAttribute("spreadMethod", "reflect");
    Paint paintReflect;
    grad.ApplyToPaint(paintReflect, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paintReflect.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(paintReflect.GetGradient(), Paint::Radial);

    grad.SetAttribute("spreadMethod", "repeat");
    Paint paintRepeat;
    grad.ApplyToPaint(paintRepeat, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paintRepeat.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(paintRepeat.GetGradient(), Paint::Radial);
}

/**
 * @tc.name: SvgRadialGradientClone_001
 * @tc.desc: Verify radial gradient Clone preserves its stops.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgRadialGradientClone_001, TestSize.Level1)
{
    SvgRadialGradientResource grad;
    grad.SetAttribute("cx", "0.5");
    grad.SetAttribute("cy", "0.5");
    grad.SetAttribute("r", "0.5");
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    SvgStopResource* s1 = new SvgStopResource();
    s1->SetAttribute("offset", "1");
    s1->SetAttribute("stop-color", "#0000ff");
    grad.AppendChild(s0);
    grad.AppendChild(s1);
    SvgElementBase* clone = grad.Clone();
    ASSERT_NE(clone, nullptr);
    SvgRadialGradientResource* clonedGrad = dynamic_cast<SvgRadialGradientResource*>(clone);
    ASSERT_NE(clonedGrad, nullptr);
    Paint paint;
    clonedGrad->ApplyToPaint(paint, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paint.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(paint.GetGradient(), Paint::Radial);
    EXPECT_EQ(paint.getStopAndColor().Size(), 2);
    delete clone;
}

/**
 * @tc.name: SvgRadialGradientUserSpace_001
 * @tc.desc: Verify userSpaceOnUse, fx/fy, percent coords and gradientTransform branches.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgRadialGradientUserSpace_001, TestSize.Level1)
{
    SvgRadialGradientResource grad;
    grad.SetAttribute("gradientUnits", "userSpaceOnUse");
    EXPECT_TRUE(grad.SetAttribute("cx", "50%"));
    EXPECT_TRUE(grad.SetAttribute("cy", "50%"));
    EXPECT_TRUE(grad.SetAttribute("r", "50%"));
    EXPECT_TRUE(grad.SetAttribute("fx", "0.3"));
    EXPECT_TRUE(grad.SetAttribute("fy", "0.3"));
    EXPECT_TRUE(grad.SetAttribute("gradientTransform", "scale(2)"));
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    grad.AppendChild(s0);
    Paint paint;
    grad.ApplyToPaint(paint, {0, 0, 100, 100}, nullptr, true);
    EXPECT_EQ(paint.GetStyle(), Paint::GRADIENT);
    EXPECT_EQ(paint.GetGradient(), Paint::Radial);
}

/**
 * @tc.name: SvgSolidColorClone_001
 * @tc.desc: Verify solid-color Clone preserves attributes and still fills.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgSolidColorClone_001, TestSize.Level1)
{
    SvgSolidColorResource solid;
    solid.SetAttribute("solid-color", "#00ff00");
    solid.SetAttribute("solid-opacity", "0.5");
    SvgElementBase* clone = solid.Clone();
    ASSERT_NE(clone, nullptr);
    SvgSolidColorResource* clonedSolid = dynamic_cast<SvgSolidColorResource*>(clone);
    ASSERT_NE(clonedSolid, nullptr);
    Paint paint;
    clonedSolid->ApplyToPaint(paint, {0, 0, 10, 10}, nullptr, true);
    EXPECT_EQ(paint.GetStyle(), Paint::FILL_STYLE);
    EXPECT_EQ(clone->GetCategory(), SVG_CATEGORY_RESOURCE);
    delete clone;
}

/**
 * @tc.name: SvgDefsSetAttribute_001
 * @tc.desc: Verify defs attribute dispatch, null guards and Clone.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SvgDefsSetAttribute_001, TestSize.Level1)
{
    SvgDefsResource defs;
    EXPECT_TRUE(defs.SetAttribute("id", "defs1"));
    EXPECT_FALSE(defs.SetAttribute("unknown", "x"));
    EXPECT_FALSE(defs.SetAttribute(nullptr, "x"));
    EXPECT_FALSE(defs.SetAttribute("id", nullptr));
    defs.AppendChild(nullptr);
    SvgElementBase* clone = defs.Clone();
    ASSERT_NE(clone, nullptr);
    EXPECT_NE(dynamic_cast<SvgDefsResource*>(clone), nullptr);
    delete clone;
}

/**
 * @tc.name: LinearGradientUserSpaceToRecordSpace_010
 * @tc.desc: Verify userSpaceOnUse coordinates are transformed by localToRecordSpace.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, LinearGradientUserSpaceToRecordSpace_010, TestSize.Level1)
{
    SvgLinearGradientResource grad;
    grad.SetAttribute("x1", "10");
    grad.SetAttribute("y1", "20");
    grad.SetAttribute("x2", "110");
    grad.SetAttribute("y2", "120");
    grad.SetAttribute("gradientUnits", "userSpaceOnUse");
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    grad.AppendChild(s0);

    Paint paint;
    TransAffine translate;
    translate.Translate(5.0f, 7.0f);
    grad.ApplyToPaint(paint, {0, 0, 100, 100}, &translate, true);

    auto pt = paint.GetLinearGradientPoint();
    EXPECT_FLOAT_EQ(pt.x0, 15.0f);
    EXPECT_FLOAT_EQ(pt.y0, 27.0f);
    EXPECT_FLOAT_EQ(pt.x1, 115.0f);
    EXPECT_FLOAT_EQ(pt.y1, 127.0f);
}

/**
 * @tc.name: LinearGradientObjectBoundingBox_011
 * @tc.desc: Verify objectBoundingBox coordinates are resolved against local bounds, not device bounds.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, LinearGradientObjectBoundingBox_011, TestSize.Level1)
{
    SvgLinearGradientResource grad;
    grad.SetAttribute("x1", "0");
    grad.SetAttribute("y1", "0");
    grad.SetAttribute("x2", "1");
    grad.SetAttribute("y2", "0");
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    grad.AppendChild(s0);

    Paint paint;
    grad.ApplyToPaint(paint, {10, 20, 110, 220}, nullptr, true);

    auto pt = paint.GetLinearGradientPoint();
    // Rect {10,20,110,220} uses inclusive width/height, so objectBoundingBox x2=1 maps to 111.
    EXPECT_FLOAT_EQ(pt.x0, 10.0f);
    EXPECT_FLOAT_EQ(pt.y0, 20.0f);
    EXPECT_FLOAT_EQ(pt.x1, 111.0f);
    EXPECT_FLOAT_EQ(pt.y1, 20.0f);
}

/**
 * @tc.name: SolidColorStroke_012
 * @tc.desc: Verify solid-color applied as stroke sets stroke color and preserves stroke style.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SolidColorStroke_012, TestSize.Level1)
{
    SvgSolidColorResource solid;
    solid.SetAttribute("solid-color", "#00ff00");
    Paint paint;
    paint.SetStyle(Paint::STROKE_STYLE);
    solid.ApplyToPaint(paint, {0, 0, 10, 10}, nullptr, false);
    EXPECT_EQ(paint.GetStrokeColor().full, SvgAttributeParser::ParseColor("#00ff00").full);
    EXPECT_EQ(paint.GetStyle(), Paint::STROKE_STYLE);
}

/**
 * @tc.name: SolidColorFillEnablesFill_013
 * @tc.desc: Verify solid-color applied as fill on a stroke-only paint enables fill rendering.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, SolidColorFillEnablesFill_013, TestSize.Level1)
{
    SvgSolidColorResource solid;
    solid.SetAttribute("solid-color", "#0000ff");
    Paint paint;
    paint.SetStyle(Paint::STROKE_STYLE);
    solid.ApplyToPaint(paint, {0, 0, 10, 10}, nullptr, true);
    EXPECT_EQ(paint.GetFillColor().full, SvgAttributeParser::ParseColor("#0000ff").full);
    EXPECT_EQ(paint.GetStyle(), Paint::STROKE_FILL_STYLE);
}

/**
 * @tc.name: RadialGradientUserSpaceToRecordSpace_014
 * @tc.desc: Verify userSpaceOnUse radial coordinates are transformed by localToRecordSpace.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintServersTest, RadialGradientUserSpaceToRecordSpace_014, TestSize.Level1)
{
    SvgRadialGradientResource grad;
    grad.SetAttribute("cx", "10");
    grad.SetAttribute("cy", "20");
    grad.SetAttribute("r", "30");
    grad.SetAttribute("fx", "5");
    grad.SetAttribute("fy", "5");
    grad.SetAttribute("gradientUnits", "userSpaceOnUse");
    SvgStopResource* s0 = new SvgStopResource();
    s0->SetAttribute("offset", "0");
    s0->SetAttribute("stop-color", "#ff0000");
    grad.AppendChild(s0);

    Paint paint;
    TransAffine scale;
    scale.Scale(2.0f, 2.0f);
    grad.ApplyToPaint(paint, {0, 0, 100, 100}, &scale, true);

    auto pt = paint.GetRadialGradientPoint();
    EXPECT_FLOAT_EQ(pt.x1, 20.0f);
    EXPECT_FLOAT_EQ(pt.y1, 40.0f);
    EXPECT_FLOAT_EQ(pt.x0, 10.0f);
    EXPECT_FLOAT_EQ(pt.y0, 10.0f);
    EXPECT_FLOAT_EQ(pt.r1, 60.0f);
}

} // namespace OHOS
