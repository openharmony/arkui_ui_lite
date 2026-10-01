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
#include "gfx_utils/diagram/common/paint.h"
#include "svg/svg_animation.h"
#include "svg/svg_shape_nodes.h"

using namespace testing::ext;

namespace OHOS {

class SvgAnimationNodeTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp()
    {
        if (canvas_ == nullptr) {
            canvas_ = new UICanvas();
        }
        if (paint_ == nullptr) {
            paint_ = new Paint();
        }
    }
    void TearDown() {}

    static UICanvas* canvas_;
    static Paint* paint_;
};

UICanvas* SvgAnimationNodeTest::canvas_ = nullptr;
Paint* SvgAnimationNodeTest::paint_ = nullptr;

/**
 * @tc.name: SvgAnimationNodeGetType_001
 * @tc.desc: Verify each animation node returns its expected animation kind.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimationNodeGetType_001, TestSize.Level1)
{
    SvgAnimate animate;
    SvgAnimateColor animateColor;
    SvgAnimateMotion animateMotion;
    SvgAnimateTransform animateTransform;
    SvgSet set;
    SvgMPath mpath;

    EXPECT_EQ(animate.GetAnimKind(), SvgAnimKind::SVG_ANIM_KIND_NUMERIC);
    EXPECT_EQ(animateColor.GetAnimKind(), SvgAnimKind::SVG_ANIM_KIND_COLOR);
    EXPECT_EQ(animateMotion.GetAnimKind(), SvgAnimKind::SVG_ANIM_KIND_MOTION);
    EXPECT_EQ(animateTransform.GetAnimKind(), SvgAnimKind::SVG_ANIM_KIND_TRANSFORM);
    EXPECT_EQ(set.GetAnimKind(), SvgAnimKind::SVG_ANIM_KIND_SET);
    EXPECT_EQ(mpath.GetCategory(), SvgElementCategory::SVG_CATEGORY_ANIMATION);
}

/**
 * @tc.name: SvgAnimationNodeSetAttribute_001
 * @tc.desc: Verify animation nodes accept known attributes and report unknown ones
 *           (so a typo'd SMIL attribute is not silently swallowed).
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimationNodeSetAttribute_001, TestSize.Level1)
{
    SvgAnimate animate;
    SvgAnimateColor animateColor;
    SvgAnimateMotion animateMotion;
    SvgAnimateTransform animateTransform;
    SvgSet set;
    SvgMPath mpath;

    EXPECT_TRUE(animate.SetAttribute("attributeName", "x"));
    EXPECT_TRUE(animateColor.SetAttribute("attributeName", "fill"));
    EXPECT_TRUE(animateMotion.SetAttribute("path", "M0 0L10 10"));
    EXPECT_TRUE(animateTransform.SetAttribute("type", "rotate"));
    EXPECT_TRUE(set.SetAttribute("to", "100"));
    EXPECT_TRUE(mpath.SetAttribute("xlink:href", "#path1"));

    EXPECT_FALSE(animate.SetAttribute("unknown", "value"));
    EXPECT_FALSE(animateColor.SetAttribute("unknown", "value"));
    EXPECT_FALSE(animateMotion.SetAttribute("unknown", "value"));
    EXPECT_FALSE(animateTransform.SetAttribute("unknown", "value"));
    EXPECT_FALSE(set.SetAttribute("unknown", "value"));
    EXPECT_FALSE(mpath.SetAttribute("unknown", "value"));
}

/**
 * @tc.name: SvgAnimationNodeNullAttribute_001
 * @tc.desc: Verify animation nodes reject null attributes consistently with other nodes.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimationNodeNullAttribute_001, TestSize.Level1)
{
    SvgAnimate animate;
    SvgAnimateColor animateColor;
    SvgAnimateMotion animateMotion;
    SvgAnimateTransform animateTransform;
    SvgSet set;
    SvgMPath mpath;

    EXPECT_FALSE(animate.SetAttribute(nullptr, nullptr));
    EXPECT_FALSE(animateColor.SetAttribute(nullptr, nullptr));
    EXPECT_FALSE(animateMotion.SetAttribute(nullptr, nullptr));
    EXPECT_FALSE(animateTransform.SetAttribute(nullptr, nullptr));
    EXPECT_FALSE(set.SetAttribute(nullptr, nullptr));
    EXPECT_FALSE(mpath.SetAttribute(nullptr, nullptr));

    EXPECT_FALSE(animate.SetAttribute("dur", nullptr));
    EXPECT_FALSE(set.SetAttribute(nullptr, "100"));
}

/**
 * @tc.name: SvgAnimationNodeChild_001
 * @tc.desc: Verify animation nodes handle children through the base class.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimationNodeChild_001, TestSize.Level1)
{
    SvgAnimateTransform parent;
    SvgMPath* child = new SvgMPath();
    parent.AppendChild(child);

    EXPECT_EQ(parent.GetChildren().Size(), 1U);
    // child is owned by parent and released in parent's destructor.
}

/**
 * @tc.name: SvgAnimateNodeParse_001
 * @tc.desc: Verify SvgAnimate stores attributeName/from/to/dur/repeatCount.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateNodeParse_001, TestSize.Level1)
{
    SvgAnimate animate;
    EXPECT_EQ(animate.GetAttributeName(), nullptr);
    EXPECT_EQ(animate.GetFrom(), nullptr);
    EXPECT_EQ(animate.GetTo(), nullptr);
    EXPECT_EQ(animate.GetDuration(), 0U);
    EXPECT_FALSE(animate.IsIndefinite());

    EXPECT_TRUE(animate.SetAttribute("attributeName", "opacity"));
    EXPECT_TRUE(animate.SetAttribute("from", "1"));
    EXPECT_TRUE(animate.SetAttribute("to", "0.2"));
    EXPECT_TRUE(animate.SetAttribute("dur", "1.5s"));
    EXPECT_TRUE(animate.SetAttribute("repeatCount", "indefinite"));

    ASSERT_NE(animate.GetAttributeName(), nullptr);
    EXPECT_STREQ(animate.GetAttributeName(), "opacity");
    EXPECT_STREQ(animate.GetFrom(), "1");
    EXPECT_STREQ(animate.GetTo(), "0.2");
    EXPECT_EQ(animate.GetDuration(), 1500U);
    EXPECT_TRUE(animate.IsIndefinite());

    // overwriting an attribute replaces the stored value
    EXPECT_TRUE(animate.SetAttribute("to", "0.5"));
    EXPECT_STREQ(animate.GetTo(), "0.5");
}

/**
 * @tc.name: SvgAnimateNodeDur_001
 * @tc.desc: Verify dur parsing for seconds, milliseconds, bare numbers and invalid input.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateNodeDur_001, TestSize.Level1)
{
    SvgAnimate animate;
    EXPECT_TRUE(animate.SetAttribute("dur", "300ms"));
    EXPECT_EQ(animate.GetDuration(), 300U);

    EXPECT_TRUE(animate.SetAttribute("dur", "2"));
    EXPECT_EQ(animate.GetDuration(), 2000U);

    EXPECT_TRUE(animate.SetAttribute("dur", "0.5s"));
    EXPECT_EQ(animate.GetDuration(), 500U);

    EXPECT_TRUE(animate.SetAttribute("dur", "abc"));
    EXPECT_EQ(animate.GetDuration(), 0U);

    EXPECT_TRUE(animate.SetAttribute("dur", "-1s"));
    EXPECT_EQ(animate.GetDuration(), 0U);
}

/**
 * @tc.name: SvgAnimateNodeDurNaN_003
 * @tc.desc: Verify a NaN/Inf duration is rejected and treated as zero, not cast to an undefined value.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateNodeDurNaN_003, TestSize.Level1)
{
    SvgAnimate animate;
    EXPECT_TRUE(animate.SetAttribute("dur", "nan"));
    EXPECT_EQ(animate.GetDuration(), 0U);
    EXPECT_FALSE(animate.IsIndefinite());

    EXPECT_TRUE(animate.SetAttribute("dur", "inf"));
    EXPECT_EQ(animate.GetDuration(), 0U);
    EXPECT_FALSE(animate.IsIndefinite());
}

/**
 * @tc.name: SvgAnimateNodeRepeatCount_001
 * @tc.desc: Verify repeatCount parsing: only "indefinite" repeats.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateNodeRepeatCount_001, TestSize.Level1)
{
    SvgAnimate animate;
    EXPECT_TRUE(animate.SetAttribute("repeatCount", "3"));
    EXPECT_FALSE(animate.IsIndefinite());

    EXPECT_TRUE(animate.SetAttribute("repeatCount", "indefinite"));
    EXPECT_TRUE(animate.IsIndefinite());

    EXPECT_TRUE(animate.SetAttribute("repeatCount", "1"));
    EXPECT_FALSE(animate.IsIndefinite());
}

/**
 * @tc.name: SvgAnimateTransformType_001
 * @tc.desc: Verify animateTransform parses the type attribute into the enum.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateTransformType_001, TestSize.Level1)
{
    SvgAnimateTransform transform;
    EXPECT_EQ(transform.GetTransformType(), SvgAnimateTransformType::SVG_ANIMATE_TRANSFORM_UNKNOWN);

    EXPECT_TRUE(transform.SetAttribute("type", "rotate"));
    EXPECT_EQ(transform.GetTransformType(), SvgAnimateTransformType::SVG_ANIMATE_TRANSFORM_ROTATE);

    EXPECT_TRUE(transform.SetAttribute("type", "scale"));
    EXPECT_EQ(transform.GetTransformType(), SvgAnimateTransformType::SVG_ANIMATE_TRANSFORM_SCALE);

    EXPECT_TRUE(transform.SetAttribute("type", "translate"));
    EXPECT_EQ(transform.GetTransformType(), SvgAnimateTransformType::SVG_ANIMATE_TRANSFORM_TRANSLATE);

    EXPECT_TRUE(transform.SetAttribute("type", "skewX"));
    EXPECT_EQ(transform.GetTransformType(), SvgAnimateTransformType::SVG_ANIMATE_TRANSFORM_UNKNOWN);

    // animate attributes are parsed through the base class
    EXPECT_TRUE(transform.SetAttribute("dur", "3s"));
    EXPECT_EQ(transform.GetDuration(), 3000U);
}

/**
 * @tc.name: SvgAnimateMotionParse_001
 * @tc.desc: Verify animateMotion parses path and rotate attributes.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateMotionParse_001, TestSize.Level1)
{
    SvgAnimateMotion motion;
    EXPECT_EQ(motion.GetPathData(), nullptr);
    EXPECT_EQ(motion.GetRotateMode(), SvgMotionRotateMode::SVG_MOTION_ROTATE_NONE);

    EXPECT_TRUE(motion.SetAttribute("path", "M0 0 L10 10"));
    EXPECT_STREQ(motion.GetPathData(), "M0 0 L10 10");

    EXPECT_TRUE(motion.SetAttribute("rotate", "auto"));
    EXPECT_EQ(motion.GetRotateMode(), SvgMotionRotateMode::SVG_MOTION_ROTATE_AUTO);

    EXPECT_TRUE(motion.SetAttribute("rotate", "auto-reverse"));
    EXPECT_EQ(motion.GetRotateMode(), SvgMotionRotateMode::SVG_MOTION_ROTATE_AUTO_REVERSE);

    EXPECT_TRUE(motion.SetAttribute("rotate", "45"));
    EXPECT_EQ(motion.GetRotateMode(), SvgMotionRotateMode::SVG_MOTION_ROTATE_ANGLE);
    EXPECT_FLOAT_EQ(motion.GetRotateAngle(), 45.0f);
}

/**
 * @tc.name: SvgAnimateMotionPathNode_001
 * @tc.desc: Verify the bound mpath target takes priority over the path attribute.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateMotionPathNode_001, TestSize.Level1)
{
    SvgAnimateMotion motion;
    motion.SetAttribute("path", "M0 0 L10 10");

    SvgPathNode pathNode;
    pathNode.SetAttribute("d", "M5 5 L50 50");
    motion.SetPathNode(&pathNode);
    EXPECT_STREQ(motion.GetPathData(), "M5 5 L50 50");

    motion.SetPathNode(nullptr);
    EXPECT_STREQ(motion.GetPathData(), "M0 0 L10 10");
}

/**
 * @tc.name: SvgSetNodeParse_001
 * @tc.desc: Verify <set> stores attributeName and to.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgSetNodeParse_001, TestSize.Level1)
{
    SvgSet set;
    EXPECT_TRUE(set.SetAttribute("attributeName", "visibility"));
    EXPECT_TRUE(set.SetAttribute("to", "hidden"));
    EXPECT_STREQ(set.GetAttributeName(), "visibility");
    EXPECT_STREQ(set.GetTo(), "hidden");
}

/**
 * @tc.name: SvgMPathNodeHref_001
 * @tc.desc: Verify <mpath> stores the href with the '#' prefix stripped.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgMPathNodeHref_001, TestSize.Level1)
{
    SvgMPath mpath;
    EXPECT_EQ(mpath.GetHref(), nullptr);

    EXPECT_TRUE(mpath.SetAttribute("xlink:href", "#path1"));
    EXPECT_STREQ(mpath.GetHref(), "path1");

    EXPECT_TRUE(mpath.SetAttribute("href", "path2"));
    EXPECT_STREQ(mpath.GetHref(), "path2");
}

/**
 * @tc.name: SvgAnimateNodeCalcModeAndFill_002
 * @tc.desc: Verify calcMode and fill parsing into the enum values.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateNodeCalcModeAndFill_002, TestSize.Level1)
{
    SvgAnimate animate;
    EXPECT_EQ(animate.GetCalcMode(), SVG_CALC_MODE_LINEAR); // default

    EXPECT_TRUE(animate.SetAttribute("calcMode", "discrete"));
    EXPECT_EQ(animate.GetCalcMode(), SVG_CALC_MODE_DISCRETE);
    EXPECT_TRUE(animate.SetAttribute("calcMode", "paced"));
    EXPECT_EQ(animate.GetCalcMode(), SVG_CALC_MODE_PACED);
    EXPECT_TRUE(animate.SetAttribute("calcMode", "spline"));
    EXPECT_EQ(animate.GetCalcMode(), SVG_CALC_MODE_SPLINE);
    EXPECT_TRUE(animate.SetAttribute("calcMode", "bogus"));
    EXPECT_EQ(animate.GetCalcMode(), SVG_CALC_MODE_LINEAR); // unknown -> linear

    EXPECT_EQ(animate.GetFillMode(), SVG_ANIM_FILL_REMOVE); // default
    EXPECT_TRUE(animate.SetAttribute("fill", "freeze"));
    EXPECT_EQ(animate.GetFillMode(), SVG_ANIM_FILL_FREEZE);
    EXPECT_TRUE(animate.SetAttribute("fill", "remove"));
    EXPECT_EQ(animate.GetFillMode(), SVG_ANIM_FILL_REMOVE);
}

/**
 * @tc.name: SvgAnimateNodeRepeatCountNegative_002
 * @tc.desc: Verify a negative repeatCount falls back to the default value.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateNodeRepeatCountNegative_002, TestSize.Level1)
{
    SvgAnimate animate;
    EXPECT_TRUE(animate.SetAttribute("repeatCount", "-3"));
    EXPECT_FALSE(animate.IsIndefinite());
    EXPECT_FLOAT_EQ(animate.GetRepeatCount(), 1.0f);
}

/**
 * @tc.name: SvgAnimateNodeClockValues_002
 * @tc.desc: Verify begin/end clock attributes are parsed into milliseconds.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateNodeClockValues_002, TestSize.Level1)
{
    SvgAnimate animate;
    EXPECT_EQ(animate.GetBeginMs(), 0U);
    EXPECT_EQ(animate.GetEndMs(), 0U);

    EXPECT_TRUE(animate.SetAttribute("begin", "1s"));
    EXPECT_EQ(animate.GetBeginMs(), 1000U);
    EXPECT_TRUE(animate.SetAttribute("end", "2s"));
    EXPECT_EQ(animate.GetEndMs(), 2000U);
}

/**
 * @tc.name: SvgAnimateMotionKeyPoints_002
 * @tc.desc: Verify animateMotion parses the keyPoints attribute.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimationNodeTest, SvgAnimateMotionKeyPoints_002, TestSize.Level1)
{
    SvgAnimateMotion motion;
    EXPECT_EQ(motion.GetKeyPointCount(), 0U);
    EXPECT_EQ(motion.GetKeyPoints(), nullptr);

    EXPECT_TRUE(motion.SetAttribute("keyPoints", "0;0.5;1"));
    EXPECT_EQ(motion.GetKeyPointCount(), 3U);
    ASSERT_NE(motion.GetKeyPoints(), nullptr);
    EXPECT_FLOAT_EQ(motion.GetKeyPoints()[0], 0.0f);
    EXPECT_FLOAT_EQ(motion.GetKeyPoints()[1], 0.5f);
    EXPECT_FLOAT_EQ(motion.GetKeyPoints()[2], 1.0f);
}
} // namespace OHOS
