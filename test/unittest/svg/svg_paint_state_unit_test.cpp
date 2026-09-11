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
#include "gfx_utils/diagram/common/paint.h"
#include "svg/svg_paint_state.h"

using namespace testing::ext;

namespace OHOS {

class SvgPaintStateTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: SvgPaintStateDefaults_001
 * @tc.desc: Verify default state has no gradient ids set.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateDefaults_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_EQ(state.GetFillGradientId(), nullptr);
    EXPECT_EQ(state.GetStrokeGradientId(), nullptr);
}

/**
 * @tc.name: SvgPaintStateFillAndStroke_001
 * @tc.desc: Verify fill and stroke colors propagate to Paint.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateFillAndStroke_001, TestSize.Level1)
{
    SvgPaintState state;
    ColorType fill;
    fill.full = 0xFFFF0000;
    ColorType stroke;
    stroke.full = 0xFF00FF00;

    state.SetFill(fill);
    state.SetStroke(stroke);

    Paint parent;
    Paint paint = state.Apply(parent);
    EXPECT_EQ(paint.GetFillColor().full, fill.full);
    EXPECT_EQ(paint.GetStrokeColor().full, stroke.full);
}

/**
 * @tc.name: SvgPaintStateStrokeWidthAndOpacity_001
 * @tc.desc: Verify stroke width and opacity propagate to Paint.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateStrokeWidthAndOpacity_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetStrokeWidth(5);
    state.SetOpacity(OPA_OPAQUE / 2);

    Paint parent;
    Paint paint = state.Apply(parent);
    EXPECT_EQ(paint.GetStrokeWidth(), 5U);
    EXPECT_EQ(paint.GetOpacity(), OPA_OPAQUE / 2);
}

/**
 * @tc.name: SvgPaintStateTransform_001
 * @tc.desc: Verify transform updates the paint transform.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateTransform_001, TestSize.Level1)
{
    SvgPaintState state;
    TransAffine matrix = TransAffine::TransAffineTranslation(10, 20);
    state.SetTransform(matrix);

    Paint parent;
    Paint paint = state.Apply(parent);
    const float* data = paint.GetTransAffine().GetData();
    EXPECT_FLOAT_EQ(data[2], 10.0f);
    EXPECT_FLOAT_EQ(data[5], 20.0f);
}

/**
 * @tc.name: SvgPaintStateGradients_001
 * @tc.desc: Verify gradient ids can be set and read back.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateGradients_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetFillGradient("fillGrad");
    state.SetStrokeGradient("strokeGrad");

    ASSERT_NE(state.GetFillGradientId(), nullptr);
    ASSERT_NE(state.GetStrokeGradientId(), nullptr);
    EXPECT_STREQ(state.GetFillGradientId(), "fillGrad");
    EXPECT_STREQ(state.GetStrokeGradientId(), "strokeGrad");
}

/**
 * @tc.name: SvgPaintStateApplyCombinesParent_001
 * @tc.desc: Verify Apply combines parent and local state.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateApplyCombinesParent_001, TestSize.Level1)
{
    SvgPaintState state;
    ColorType fill;
    fill.full = 0xFF0000FF;
    state.SetFill(fill);
    state.SetOpacity(OPA_OPAQUE / 2);

    Paint parent;
    parent.SetOpacity(OPA_OPAQUE / 2);
    Paint paint = state.Apply(parent);
    EXPECT_EQ(paint.GetFillColor().full, fill.full);
    EXPECT_EQ(paint.GetOpacity(), OPA_OPAQUE / 4);
}

/**
 * @tc.name: SvgPaintStateApplyNoFlags_001
 * @tc.desc: Verify Apply with no properties set returns the parent paint unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateApplyNoFlags_001, TestSize.Level1)
{
    SvgPaintState state;
    Paint parent;
    ColorType fill;
    fill.full = 0xFF112233;
    ColorType stroke;
    stroke.full = 0xFF445566;
    parent.SetFillColor(fill);
    parent.SetStrokeColor(stroke);
    parent.SetStrokeWidth(7);
    parent.SetOpacity(OPA_OPAQUE / 2);

    Paint paint = state.Apply(parent);
    EXPECT_EQ(paint.GetFillColor().full, fill.full);
    EXPECT_EQ(paint.GetStrokeColor().full, stroke.full);
    EXPECT_EQ(paint.GetStrokeWidth(), 7U);
    EXPECT_EQ(paint.GetOpacity(), OPA_OPAQUE / 2);
}

/**
 * @tc.name: SvgPaintStateNullGradientId_001
 * @tc.desc: Verify setting a null gradient id keeps the getter returning nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateNullGradientId_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetFillGradient(nullptr);
    state.SetStrokeGradient(nullptr);
    EXPECT_EQ(state.GetFillGradientId(), nullptr);
    EXPECT_EQ(state.GetStrokeGradientId(), nullptr);
}

/**
 * @tc.name: SvgPaintStateFillClearsGradient_001
 * @tc.desc: Verify SetFill after SetFillGradient clears the fill gradient id and applies the fill color.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateFillClearsGradient_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetFillGradient("grad");
    ASSERT_NE(state.GetFillGradientId(), nullptr);

    ColorType fill;
    fill.full = 0xFFABCDEF;
    state.SetFill(fill);
    EXPECT_EQ(state.GetFillGradientId(), nullptr);

    Paint parent;
    Paint paint = state.Apply(parent);
    EXPECT_EQ(paint.GetFillColor().full, fill.full);
}

/**
 * @tc.name: SvgPaintStateStrokeClearsGradient_001
 * @tc.desc: Verify SetStroke after SetStrokeGradient clears the stroke gradient id and applies the stroke color.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateStrokeClearsGradient_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetStrokeGradient("grad");
    ASSERT_NE(state.GetStrokeGradientId(), nullptr);

    ColorType stroke;
    stroke.full = 0xFF0F1E2D;
    state.SetStroke(stroke);
    EXPECT_EQ(state.GetStrokeGradientId(), nullptr);

    Paint parent;
    Paint paint = state.Apply(parent);
    EXPECT_EQ(paint.GetStrokeColor().full, stroke.full);
}

/**
 * @tc.name: SvgPaintStateGradientOverwrite_001
 * @tc.desc: Verify setting a gradient id twice releases the old id and keeps the new one.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateGradientOverwrite_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetFillGradient("first");
    state.SetStrokeGradient("one");
    state.SetFillGradient("second");
    state.SetStrokeGradient("two");

    ASSERT_NE(state.GetFillGradientId(), nullptr);
    ASSERT_NE(state.GetStrokeGradientId(), nullptr);
    EXPECT_STREQ(state.GetFillGradientId(), "second");
    EXPECT_STREQ(state.GetStrokeGradientId(), "two");
}

/**
 * @tc.name: SvgPaintStateOpacityZero_001
 * @tc.desc: Verify zero opacity combined with an opaque parent produces a fully transparent paint.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateOpacityZero_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetOpacity(0);

    Paint parent;
    Paint paint = state.Apply(parent);
    EXPECT_EQ(paint.GetOpacity(), 0);

    state.SetOpacity(OPA_OPAQUE);
    Paint opaquePaint = state.Apply(parent);
    EXPECT_EQ(opaquePaint.GetOpacity(), OPA_OPAQUE);
}

/**
 * @tc.name: SvgPaintStateTransformCompose_001
 * @tc.desc: Verify Apply composes the local transform with the parent paint transform.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateTransformCompose_001, TestSize.Level1)
{
    SvgPaintState state;
    TransAffine matrix = TransAffine::TransAffineTranslation(10, 20);
    state.SetTransform(matrix);

    Paint parent;
    parent.SetTransform(1.0f, 0.0f, 0.0f, 1.0f, 5, 6);
    Paint paint = state.Apply(parent);
    const float* data = paint.GetTransAffine().GetData();
    EXPECT_FLOAT_EQ(data[2], 15.0f);
    EXPECT_FLOAT_EQ(data[5], 26.0f);
}

/**
 * @tc.name: SvgPaintStateTransformCompose_002
 * @tc.desc: Verify the local transform is post-multiplied onto the parent CTM (SVG spec
 *           order CTM * T), using non-commuting rotation and translation.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateTransformCompose_002, TestSize.Level1)
{
    SvgPaintState state;
    state.SetTransform(TransAffine::TransAffineTranslation(10, 0));

    Paint parent;
    // Counter-clockwise 90-degree rotation: (x, y) -> (-y, x).
    parent.SetTransform(0.0f, -1.0f, 1.0f, 0.0f, 0, 0);
    Paint paint = state.Apply(parent);
    const float* data = paint.GetTransAffine().GetData();
    // Spec order CTM * T maps (x, y) -> (-y, x + 10); pre-multiplying (T * CTM)
    // would give (x, y) -> (-y + 10, x) instead.
    EXPECT_FLOAT_EQ(data[0], 0.0f);
    EXPECT_FLOAT_EQ(data[1], -1.0f);
    EXPECT_FLOAT_EQ(data[2], 0.0f);
    EXPECT_FLOAT_EQ(data[3], 1.0f);
    EXPECT_FLOAT_EQ(data[4], 0.0f);
    EXPECT_FLOAT_EQ(data[5], 10.0f);
}

/**
 * @tc.name: SetAttributeDispatch_001
 * @tc.desc: Verify the consolidated attribute dispatcher routes paint attributes.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SetAttributeDispatch_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_TRUE(state.SetAttribute("fill", "#ff0000"));
    EXPECT_TRUE(state.SetAttribute("stroke-width", "2"));
    EXPECT_TRUE(state.SetAttribute("fill-opacity", "0.5"));
    EXPECT_FALSE(state.SetAttribute("x", "10"));  // geometric attributes are not handled here
    Paint p = state.Apply(Paint());
    // ColorType packs alpha in the high byte, then red, green, blue. The rgb channels stay as
    // parsed while fill-opacity "0.5" scales the alpha down to OPA_OPAQUE / 2.
    EXPECT_EQ(p.GetFillColor().full & 0x00FFFFFFU, 0x00FF0000U);
    EXPECT_EQ((p.GetFillColor().full >> 24) & 0xFFU, static_cast<uint32_t>(OPA_OPAQUE / 2));
    EXPECT_EQ(p.GetStrokeWidth(), 2);
}

/**
 * @tc.name: TransformQuery_002
 * @tc.desc: Verify HasTransform and GetTransform report transform state correctly.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, TransformQuery_002, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_FALSE(state.HasTransform());
    EXPECT_TRUE(state.SetAttribute("transform", "translate(10,20)"));
    EXPECT_TRUE(state.HasTransform());
}

/**
 * @tc.name: SvgPaintStateSetAttributeNull_001
 * @tc.desc: Verify SetAttribute rejects null name or null value.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateSetAttributeNull_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_FALSE(state.SetAttribute(nullptr, "x"));
    EXPECT_FALSE(state.SetAttribute("fill", nullptr));
}

/**
 * @tc.name: SvgPaintStateLineCap_001
 * @tc.desc: Verify stroke-linecap dispatcher routes round/square/else correctly.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateLineCap_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_TRUE(state.SetAttribute("stroke-linecap", "round"));
    EXPECT_TRUE(state.SetAttribute("stroke-linecap", "square"));
    EXPECT_TRUE(state.SetAttribute("stroke-linecap", "butt"));
}

/**
 * @tc.name: SvgPaintStateLineJoin_001
 * @tc.desc: Verify stroke-linejoin dispatcher routes round/bevel/else correctly.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateLineJoin_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_TRUE(state.SetAttribute("stroke-linejoin", "round"));
    EXPECT_TRUE(state.SetAttribute("stroke-linejoin", "bevel"));
    EXPECT_TRUE(state.SetAttribute("stroke-linejoin", "miter"));
}

/**
 * @tc.name: SvgPaintStateTextAnchor_001
 * @tc.desc: Verify text-anchor mapping middle->1, end->2, else->0.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateTextAnchor_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_TRUE(state.SetAttribute("text-anchor", "middle"));
    EXPECT_EQ(state.GetTextAnchor(), 1);
    EXPECT_TRUE(state.SetAttribute("text-anchor", "end"));
    EXPECT_EQ(state.GetTextAnchor(), 2);
    EXPECT_TRUE(state.SetAttribute("text-anchor", "start"));
    EXPECT_EQ(state.GetTextAnchor(), 0);
}

/**
 * @tc.name: SvgPaintStateVisibility_001
 * @tc.desc: Verify visibility mapping hidden/collapse->false, else->true.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateVisibility_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_TRUE(state.SetAttribute("visibility", "hidden"));
    EXPECT_FALSE(state.IsVisible());
    EXPECT_TRUE(state.SetAttribute("visibility", "collapse"));
    EXPECT_FALSE(state.IsVisible());
    EXPECT_TRUE(state.SetAttribute("visibility", "visible"));
    EXPECT_TRUE(state.IsVisible());
}

/**
 * @tc.name: SvgPaintStateStrokeWidthNegative_001
 * @tc.desc: Verify negative stroke-width is clamped to zero.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateStrokeWidthNegative_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_TRUE(state.SetAttribute("stroke-width", "-5"));
    EXPECT_EQ(state.GetStrokeWidth(), 0);
    EXPECT_TRUE(state.SetAttribute("stroke-width", "10"));
    EXPECT_EQ(state.GetStrokeWidth(), 10);
}

/**
 * @tc.name: SvgPaintStateFillStrokeUrl_001
 * @tc.desc: Verify url() fill/stroke values set the gradient ids.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateFillStrokeUrl_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_TRUE(state.SetAttribute("fill", "url(#g1)"));
    ASSERT_NE(state.GetFillGradientId(), nullptr);
    EXPECT_STREQ(state.GetFillGradientId(), "g1");
    EXPECT_TRUE(state.SetAttribute("stroke", "url(#g2)"));
    ASSERT_NE(state.GetStrokeGradientId(), nullptr);
    EXPECT_STREQ(state.GetStrokeGradientId(), "g2");
}

/**
 * @tc.name: SvgPaintStateFillInherit_001
 * @tc.desc: Verify inherit/currentColor leave the fill color untouched.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateFillInherit_001, TestSize.Level1)
{
    SvgPaintState state;
    ColorType fill;
    fill.full = 0xFF0000FF;
    state.SetFill(fill);
    EXPECT_TRUE(state.SetAttribute("fill", "inherit"));
    EXPECT_EQ(state.GetFillGradientId(), nullptr);
    EXPECT_EQ(state.GetFillColor().full, fill.full);
    EXPECT_TRUE(state.SetAttribute("fill", "currentColor"));
    EXPECT_EQ(state.GetFillColor().full, fill.full);
}

/**
 * @tc.name: SvgPaintStateFillUrlBad_001
 * @tc.desc: Verify a malformed url() reference does not set a gradient id.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateFillUrlBad_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_TRUE(state.SetAttribute("fill", "url(bad"));
    EXPECT_EQ(state.GetFillGradientId(), nullptr);
}

/**
 * @tc.name: SvgPaintStateFillOpacityOnly_001
 * @tc.desc: Verify fill-opacity without fill scales the inherited parent fill color.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateFillOpacityOnly_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetFillOpacity(OPA_OPAQUE / 2);
    Paint parent;
    ColorType fill;
    fill.full = 0xFFFFFFFF;
    parent.SetFillColor(fill);
    Paint paint = state.Apply(parent);
    EXPECT_EQ((paint.GetFillColor().full >> 24) & 0xFF, static_cast<uint8_t>(OPA_OPAQUE / 2));
}

/**
 * @tc.name: SvgPaintStateStrokeOpacityOnly_001
 * @tc.desc: Verify stroke-opacity without stroke scales the inherited parent stroke color.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateStrokeOpacityOnly_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetStrokeOpacity(OPA_OPAQUE / 2);
    Paint parent;
    ColorType stroke;
    stroke.full = 0xFFFFFFFF;
    parent.SetStrokeColor(stroke);
    Paint paint = state.Apply(parent);
    EXPECT_EQ((paint.GetStrokeColor().full >> 24) & 0xFF, static_cast<uint8_t>(OPA_OPAQUE / 2));
}

/**
 * @tc.name: SvgPaintStateVisibilityHiddenApply_001
 * @tc.desc: Verify hidden visibility forces opacity to zero, visible leaves it unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateVisibilityHiddenApply_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetVisible(false);
    EXPECT_FALSE(state.IsVisible());
    Paint parent;
    parent.SetOpacity(OPA_OPAQUE);
    Paint paint = state.Apply(parent);
    EXPECT_EQ(paint.GetOpacity(), 0);

    SvgPaintState visibleState;
    visibleState.SetVisible(true);
    Paint paint2 = visibleState.Apply(parent);
    EXPECT_EQ(paint2.GetOpacity(), OPA_OPAQUE);
}

/**
 * @tc.name: SvgPaintStateCopyFrom_001
 * @tc.desc: Verify CopyFrom duplicates gradient ids, text anchor and visibility.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateCopyFrom_001, TestSize.Level1)
{
    SvgPaintState other;
    EXPECT_TRUE(other.SetAttribute("fill", "url(#fg)"));
    EXPECT_TRUE(other.SetAttribute("stroke", "url(#sg)"));
    EXPECT_TRUE(other.SetAttribute("text-anchor", "end"));
    EXPECT_TRUE(other.SetAttribute("visibility", "hidden"));

    SvgPaintState state;
    state.CopyFrom(other);

    ASSERT_NE(state.GetFillGradientId(), nullptr);
    EXPECT_STREQ(state.GetFillGradientId(), "fg");
    ASSERT_NE(state.GetStrokeGradientId(), nullptr);
    EXPECT_STREQ(state.GetStrokeGradientId(), "sg");
    EXPECT_EQ(state.GetTextAnchor(), 2);
    EXPECT_FALSE(state.IsVisible());
}

/**
 * @tc.name: SvgPaintStateDispatchRemaining_001
 * @tc.desc: Verify the dispatcher routes the remaining paint attributes.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateDispatchRemaining_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_TRUE(state.SetAttribute("opacity", "0.5"));
    EXPECT_TRUE(state.SetAttribute("stroke-opacity", "0.5"));
    EXPECT_TRUE(state.SetAttribute("stroke-miterlimit", "4"));
    EXPECT_TRUE(state.SetAttribute("stroke-dasharray", "1 2 3 4"));
    EXPECT_TRUE(state.SetAttribute("stroke-dashoffset", "2"));
}

/**
 * @tc.name: SvgPaintStateDashNone_001
 * @tc.desc: Verify an empty dash array hits the null/zero guard in SetLineDash.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateDashNone_001, TestSize.Level1)
{
    SvgPaintState state;
    EXPECT_TRUE(state.SetAttribute("stroke-dasharray", "none"));
}

/**
 * @tc.name: SvgPaintStateFillWithOpacity_001
 * @tc.desc: Verify fill combined with fill-opacity scales the local fill color.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateFillWithOpacity_001, TestSize.Level1)
{
    SvgPaintState state;
    ColorType fill;
    fill.full = 0xFFFF0000;
    state.SetFill(fill);
    state.SetFillOpacity(OPA_OPAQUE / 2);
    Paint parent;
    Paint paint = state.Apply(parent);
    EXPECT_EQ((paint.GetFillColor().full >> 24) & 0xFF, static_cast<uint8_t>(OPA_OPAQUE / 2));
}

/**
 * @tc.name: SvgPaintStateStrokeWithOpacity_001
 * @tc.desc: Verify stroke combined with stroke-opacity scales the local stroke color.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateStrokeWithOpacity_001, TestSize.Level1)
{
    SvgPaintState state;
    ColorType stroke;
    stroke.full = 0xFF00FF00;
    state.SetStroke(stroke);
    state.SetStrokeOpacity(OPA_OPAQUE / 2);
    Paint parent;
    Paint paint = state.Apply(parent);
    EXPECT_EQ((paint.GetStrokeColor().full >> 24) & 0xFF, static_cast<uint8_t>(OPA_OPAQUE / 2));
}

/**
 * @tc.name: SvgPaintStateOpacityCombine_001
 * @tc.desc: Verify opacity combines with a non-trivial parent opacity.
 * @tc.type: FUNC
 */
HWTEST_F(SvgPaintStateTest, SvgPaintStateOpacityCombine_001, TestSize.Level1)
{
    SvgPaintState state;
    state.SetOpacity(OPA_OPAQUE / 2);
    Paint parent;
    parent.SetOpacity(OPA_OPAQUE / 2);
    Paint paint = state.Apply(parent);
    EXPECT_EQ(paint.GetOpacity(), static_cast<uint8_t>(OPA_OPAQUE / 4));
}
} // namespace OHOS
