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
#include "animator/animator_manager.h"
#include "draw/draw_canvas.h"
#include "gfx_utils/color.h"
#include "svg/svg_animator.h"
#include "svg/svg_animation.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_container_nodes.h"
#include "svg/svg_document.h"
#include "svg/svg_path_parser.h"
#include "svg/svg_shape_nodes.h"
#include "securec.h"
#include <cmath>
#include <cstring>

using namespace testing::ext;

namespace OHOS {

namespace {

constexpr uint8_t RECORD_NAME_LEN = 64;
constexpr uint8_t RECORD_VALUE_LEN = 128;

class RecordingNode : public SvgElementBase {
public:
    SvgElementCategory GetCategory() const override { return SVG_CATEGORY_VIEW; }

    bool SetAttribute(const char* name, const char* value) override
    {
        if (name != nullptr) {
            (void)strncpy_s(lastName_, sizeof(lastName_), name, sizeof(lastName_) - 1);
        }
        if (value != nullptr) {
            (void)strncpy_s(lastValue_, sizeof(lastValue_), value, sizeof(lastValue_) - 1);
        }
        writeCount_++;
        return true;
    }

    void AppendChild(SvgElementBase* child) override
    {
        // Animation children are registered with the document and targeted at this node so the
        // animator manager can drive them; anything else is not supported here and is dropped.
        if (child != nullptr && doc_ != nullptr) {
            SvgAnimation* anim = dynamic_cast<SvgAnimation*>(child);
            if (anim != nullptr) {
                anim->SetTarget(this);
                doc_->RegisterAnimation(child);
                return;
            }
        }
        delete child;
    }

    SvgElementBase* Clone() const override { return nullptr; }

    void SetTransform(const TransAffine& transform) override
    {
        (void)strncpy_s(lastName_, sizeof(lastName_), "transform", sizeof(lastName_) - 1);
        lastTransform_ = transform;
        writeCount_++;
    }

    const TransAffine& GetTransform() const override { return lastTransform_; }

    char lastName_[RECORD_NAME_LEN] = { 0 };
    char lastValue_[RECORD_VALUE_LEN] = { 0 };
    TransAffine lastTransform_;
    uint32_t writeCount_ = 0;
};

SvgAnimate* CreateOpacityAnimate(SvgElementBase* parent)
{
    SvgAnimate* anim = new SvgAnimate();
    anim->SetAttribute("attributeName", "opacity");
    anim->SetAttribute("from", "1");
    anim->SetAttribute("to", "0");
    anim->SetAttribute("dur", "1s");
    anim->SetAttribute("repeatCount", "indefinite");
    parent->AppendChild(anim);
    return anim;
}

class RecordingRect : public SvgRectNode {
public:
    bool SetAttribute(const char* name, const char* value) override
    {
        bool handled = SvgRectNode::SetAttribute(name, value);
        if (name != nullptr) {
            (void)strncpy_s(lastName_, sizeof(lastName_), name, sizeof(lastName_) - 1);
        }
        if (value != nullptr) {
            (void)strncpy_s(lastValue_, sizeof(lastValue_), value, sizeof(lastValue_) - 1);
        }
        if (handled) {
            writeCount_++;
        }
        return handled;
    }

    char lastName_[RECORD_NAME_LEN] = { 0 };
    char lastValue_[RECORD_VALUE_LEN] = { 0 };
    uint32_t writeCount_ = 0;
};

constexpr float TRANSFORM_EPSILON = 0.05f;

static void ExpectPointTransformed(const TransAffine& matrix, float x, float y, float expectedX, float expectedY)
{
    matrix.Transform(&x, &y);
    EXPECT_NEAR(x, expectedX, TRANSFORM_EPSILON);
    EXPECT_NEAR(y, expectedY, TRANSFORM_EPSILON);
}

} // namespace

class SvgAnimatorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: SvgLerpBoundary_001
 * @tc.desc: Verify SvgLerp interpolates and clamps progress to [0, 1].
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgLerpBoundary_001, TestSize.Level1)
{
    EXPECT_FLOAT_EQ(SvgLerp(0.0f, 100.0f, 0.0f), 0.0f);
    EXPECT_FLOAT_EQ(SvgLerp(0.0f, 100.0f, 0.5f), 50.0f);
    EXPECT_FLOAT_EQ(SvgLerp(0.0f, 100.0f, 1.0f), 100.0f);
    EXPECT_FLOAT_EQ(SvgLerp(1.0f, 0.0f, 0.25f), 0.75f);
    EXPECT_FLOAT_EQ(SvgLerp(0.0f, 100.0f, -1.0f), 0.0f);
    EXPECT_FLOAT_EQ(SvgLerp(0.0f, 100.0f, 2.0f), 100.0f);
}

/**
 * @tc.name: SvgLerpColor_001
 * @tc.desc: Verify SvgLerpColor interpolates RGB channels independently.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgLerpColor_001, TestSize.Level1)
{
    EXPECT_EQ(SvgLerpColor(0xFFFF0000, 0xFF0000FF, 0.0f), 0xFFFF0000U);
    EXPECT_EQ(SvgLerpColor(0xFFFF0000, 0xFF0000FF, 1.0f), 0xFF0000FFU);
    EXPECT_EQ(SvgLerpColor(0xFFFF0000, 0xFF0000FF, 0.5f), 0xFF800080U);
    EXPECT_EQ(SvgLerpColor(0xFF000000, 0xFFFFFFFF, 0.5f), 0xFF808080U);
    // progress is clamped
    EXPECT_EQ(SvgLerpColor(0xFFFF0000, 0xFF0000FF, 2.0f), 0xFF0000FFU);
}

/**
 * @tc.name: SvgPathPointAtLine_001
 * @tc.desc: Verify SvgPathPointAt samples a straight line by arc length.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgPathPointAtLine_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    ASSERT_TRUE(SvgPathPointAt("M0 0 L100 0", 0.5f, x, y, angle));
    EXPECT_FLOAT_EQ(x, 50.0f);
    EXPECT_FLOAT_EQ(y, 0.0f);
    EXPECT_FLOAT_EQ(angle, 0.0f);

    ASSERT_TRUE(SvgPathPointAt("M0 0 L100 0", 0.0f, x, y, angle));
    EXPECT_FLOAT_EQ(x, 0.0f);
    ASSERT_TRUE(SvgPathPointAt("M0 0 L100 0", 1.0f, x, y, angle));
    EXPECT_FLOAT_EQ(x, 100.0f);
}

/**
 * @tc.name: SvgPathPointAtPolyline_001
 * @tc.desc: Verify SvgPathPointAt walks multi-segment paths and reports the tangent angle.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgPathPointAtPolyline_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    // total length 100, progress 0.75 lands halfway down the vertical segment
    ASSERT_TRUE(SvgPathPointAt("M0 0 L50 0 L50 50", 0.75f, x, y, angle));
    EXPECT_FLOAT_EQ(x, 50.0f);
    EXPECT_FLOAT_EQ(y, 25.0f);
    EXPECT_FLOAT_EQ(angle, 90.0f);
}

/**
 * @tc.name: SvgPathPointAtCurve_001
 * @tc.desc: Verify SvgPathPointAt flattens cubic curves.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgPathPointAtCurve_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    // degenerate cubic equal to a straight line from (0,0) to (100,0)
    ASSERT_TRUE(SvgPathPointAt("M0 0 C0 0 100 0 100 0", 0.5f, x, y, angle));
    EXPECT_NEAR(x, 50.0f, 1.0f);
    EXPECT_NEAR(y, 0.0f, 1.0f);
}

/**
 * @tc.name: SvgPathPointAtRectangle_001
 * @tc.desc: Verify SvgPathPointAt walks the closed rectangular motion path used in the demo.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgPathPointAtRectangle_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    const char* rectPath = "M30 110 L170 110 L170 140 L30 140 Z";
    // Total length is 340: top 140, right 30, bottom 140, left 30.
    ASSERT_TRUE(SvgPathPointAt(rectPath, 0.25f, x, y, angle));
    EXPECT_FLOAT_EQ(x, 115.0f);
    EXPECT_FLOAT_EQ(y, 110.0f);
    EXPECT_FLOAT_EQ(angle, 0.0f);

    ASSERT_TRUE(SvgPathPointAt(rectPath, 0.5f, x, y, angle));
    EXPECT_FLOAT_EQ(x, 170.0f);
    EXPECT_FLOAT_EQ(y, 140.0f);
    EXPECT_FLOAT_EQ(angle, 90.0f);

    ASSERT_TRUE(SvgPathPointAt(rectPath, 0.75f, x, y, angle));
    EXPECT_FLOAT_EQ(x, 85.0f);
    EXPECT_FLOAT_EQ(y, 140.0f);
    EXPECT_FLOAT_EQ(angle, 180.0f);

    ASSERT_TRUE(SvgPathPointAt(rectPath, 1.0f, x, y, angle));
    EXPECT_FLOAT_EQ(x, 30.0f);
    EXPECT_FLOAT_EQ(y, 110.0f);
}

/**
 * @tc.name: SvgPathPointAtInvalid_001
 * @tc.desc: Verify SvgPathPointAt rejects invalid input and clamps progress.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgPathPointAtInvalid_001, TestSize.Level1)
{
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    EXPECT_FALSE(SvgPathPointAt(nullptr, 0.5f, x, y, angle));
    EXPECT_FALSE(SvgPathPointAt("", 0.5f, x, y, angle));
    EXPECT_FALSE(SvgPathPointAt("M10 10", 0.5f, x, y, angle));

    ASSERT_TRUE(SvgPathPointAt("M0 0 L100 0", -1.0f, x, y, angle));
    EXPECT_FLOAT_EQ(x, 0.0f);
    ASSERT_TRUE(SvgPathPointAt("M0 0 L100 0", 2.0f, x, y, angle));
    EXPECT_FLOAT_EQ(x, 100.0f);
}

/**
 * @tc.name: RenderBlendSolidOpacity_001
 * @tc.desc: Verify path fill/stroke alpha is multiplied by Paint opacity.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, RenderBlendSolidOpacity_001, TestSize.Level1)
{
    Paint paint;
    paint.SetFillColor(Color::GetColorFromRGB(0xFF, 0x44, 0x44));
    paint.SetStrokeColor(Color::GetColorFromRGB(0xFF, 0xFF, 0xFF));
    paint.SetStyle(Paint::STROKE_FILL_STYLE);
    paint.SetOpacity(51); // approximately 0.2

    Rgba8T fillColor;
    DrawCanvas::RenderBlendSolidSvg(paint, fillColor, false);
    EXPECT_EQ(fillColor.alpha, 51U);
    EXPECT_EQ(fillColor.red, 0xFFU);
    EXPECT_EQ(fillColor.green, 0x44U);
    EXPECT_EQ(fillColor.blue, 0x44U);

    Rgba8T strokeColor;
    DrawCanvas::RenderBlendSolidSvg(paint, strokeColor, true);
    EXPECT_EQ(strokeColor.alpha, 51U);
    EXPECT_EQ(strokeColor.red, 0xFFU);
    EXPECT_EQ(strokeColor.green, 0xFFU);
    EXPECT_EQ(strokeColor.blue, 0xFFU);
}

/**
 * @tc.name: SvgAnimatorCallbackNumeric_001
 * @tc.desc: Verify the numeric callback writes the interpolated attribute to the target node.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackNumeric_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimate anim;
    anim.SetAttribute("attributeName", "opacity");
    anim.SetAttribute("from", "1");
    anim.SetAttribute("to", "0");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_NUMERIC);
    callback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "opacity");
    EXPECT_STREQ(target.lastValue_, "0.5");
    EXPECT_EQ(target.writeCount_, 1U);

    callback.ApplyProgress(-1.0f, view);
    EXPECT_STREQ(target.lastValue_, "1");
    callback.ApplyProgress(2.0f, view);
    EXPECT_STREQ(target.lastValue_, "0");
}

/**
 * @tc.name: SvgAnimatorCallbackFillMode_001
 * @tc.desc: Verify OnStop freezes at the current progress for fill="freeze" and resets to the start value
 *           for fill="remove".
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackFillMode_001, TestSize.Level1)
{
    RecordingNode target;
    UIView view;
    UIView parentView;
    view.SetParent(&parentView);

    SvgAnimate freezeAnim;
    freezeAnim.SetAttribute("attributeName", "x");
    freezeAnim.SetAttribute("from", "0");
    freezeAnim.SetAttribute("to", "100");
    freezeAnim.SetAttribute("dur", "1s");
    freezeAnim.SetAttribute("fill", "freeze");
    SvgAnimatorCallback freezeCallback(target, freezeAnim, SVG_ANIM_KIND_NUMERIC);
    Animator freezeAnimator(&freezeCallback, &view, 1000, false);
    freezeCallback.SetAnimator(&freezeAnimator);
    freezeAnimator.SetRunTime(500);
    freezeCallback.Callback(&view);
    EXPECT_STREQ(target.lastName_, "x");
    EXPECT_STREQ(target.lastValue_, "50");
    freezeCallback.OnStop(view);
    EXPECT_STREQ(target.lastValue_, "50");

    SvgAnimate removeAnim;
    removeAnim.SetAttribute("attributeName", "x");
    removeAnim.SetAttribute("from", "0");
    removeAnim.SetAttribute("to", "100");
    removeAnim.SetAttribute("fill", "remove");
    SvgAnimatorCallback removeCallback(target, removeAnim, SVG_ANIM_KIND_NUMERIC);
    removeCallback.OnStop(view);
    EXPECT_STREQ(target.lastValue_, "0");
}

/**
 * @tc.name: SvgAnimatorCallbackTransform_001
 * @tc.desc: Verify the transform callback applies rotate/scale/translate matrices.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackTransform_001, TestSize.Level1)
{
    RecordingNode target;
    UIView view;

    SvgAnimateTransform rotate;
    rotate.SetAttribute("type", "rotate");
    rotate.SetAttribute("from", "0");
    rotate.SetAttribute("to", "360");
    SvgAnimatorCallback rotateCallback(target, rotate, SVG_ANIM_KIND_TRANSFORM);
    rotateCallback.ApplyProgress(0.25f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    ExpectPointTransformed(target.lastTransform_, 1.0f, 0.0f, 0.0f, 1.0f);
    ExpectPointTransformed(target.lastTransform_, 0.0f, 1.0f, -1.0f, 0.0f);

    SvgAnimateTransform translate;
    translate.SetAttribute("type", "translate");
    translate.SetAttribute("from", "0 0");
    translate.SetAttribute("to", "120 40");
    SvgAnimatorCallback translateCallback(target, translate, SVG_ANIM_KIND_TRANSFORM);
    translateCallback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    ExpectPointTransformed(target.lastTransform_, 0.0f, 0.0f, 60.0f, 20.0f);

    SvgAnimateTransform scale;
    scale.SetAttribute("type", "scale");
    scale.SetAttribute("from", "1");
    scale.SetAttribute("to", "0.5");
    SvgAnimatorCallback scaleCallback(target, scale, SVG_ANIM_KIND_TRANSFORM);
    scaleCallback.ApplyProgress(1.0f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    ExpectPointTransformed(target.lastTransform_, 10.0f, 0.0f, 5.0f, 0.0f);
    ExpectPointTransformed(target.lastTransform_, 0.0f, 10.0f, 0.0f, 5.0f);
}

/**
 * @tc.name: SvgAnimatorCallbackTransformMulti_001
 * @tc.desc: Verify the transform callback applies rotate-with-center and two-component scale matrices.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackTransformMulti_001, TestSize.Level1)
{
    RecordingNode target;
    UIView view;

    SvgAnimateTransform rotateCenter;
    rotateCenter.SetAttribute("type", "rotate");
    rotateCenter.SetAttribute("from", "0 10 20");
    rotateCenter.SetAttribute("to", "90 10 20");
    SvgAnimatorCallback rotateCallback(target, rotateCenter, SVG_ANIM_KIND_TRANSFORM);
    rotateCallback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    // 45° rotation around (10, 20): center is fixed, (11, 20) rotates to (10+sqrt(2)/2, 20+sqrt(2)/2).
    ExpectPointTransformed(target.lastTransform_, 10.0f, 20.0f, 10.0f, 20.0f);
    float rad = 45.0f * SVG_PI / 180.0f;
    ExpectPointTransformed(target.lastTransform_, 11.0f, 20.0f, 10.0f + cosf(rad), 20.0f + sinf(rad));

    SvgAnimateTransform scaleXY;
    scaleXY.SetAttribute("type", "scale");
    scaleXY.SetAttribute("from", "1 1");
    scaleXY.SetAttribute("to", "2 0.5");
    SvgAnimatorCallback scaleCallback(target, scaleXY, SVG_ANIM_KIND_TRANSFORM);
    scaleCallback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    ExpectPointTransformed(target.lastTransform_, 10.0f, 0.0f, 15.0f, 0.0f);
    ExpectPointTransformed(target.lastTransform_, 0.0f, 10.0f, 0.0f, 7.5f);
}

/**
 * @tc.name: SvgAnimatorCallbackTransformAdditive_001
 * @tc.desc: Verify additive="sum" composes the animated transform onto the base transform,
 *           while the default additive="replace" replaces the base transform.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackTransformAdditive_001, TestSize.Level1)
{
    RecordingNode target;
    UIView view;

    // Default additive="replace": a base translate is ignored and only the scale is applied.
    target.lastTransform_ = TransAffine::TransAffineTranslation(100.0f, 50.0f);
    SvgAnimateTransform scaleReplace;
    scaleReplace.SetAttribute("type", "scale");
    scaleReplace.SetAttribute("from", "1");
    scaleReplace.SetAttribute("to", "0.5");
    SvgAnimatorCallback replaceCallback(target, scaleReplace, SVG_ANIM_KIND_TRANSFORM);
    replaceCallback.ApplyProgress(1.0f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    ExpectPointTransformed(target.lastTransform_, 10.0f, 0.0f, 5.0f, 0.0f);

    // additive="sum": scale is applied first, then the base translate.
    target.lastTransform_ = TransAffine::TransAffineTranslation(100.0f, 50.0f);
    SvgAnimateTransform scaleSum;
    scaleSum.SetAttribute("type", "scale");
    scaleSum.SetAttribute("from", "1");
    scaleSum.SetAttribute("to", "0.5");
    scaleSum.SetAttribute("additive", "sum");
    SvgAnimatorCallback sumCallback(target, scaleSum, SVG_ANIM_KIND_TRANSFORM);
    sumCallback.ApplyProgress(1.0f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    ExpectPointTransformed(target.lastTransform_, 10.0f, 0.0f, 105.0f, 50.0f);
}

/**
 * @tc.name: SvgAnimatorCallbackColor_001
 * @tc.desc: Verify the color callback writes interpolated hex colors.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackColor_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimateColor anim;
    anim.SetAttribute("attributeName", "fill");
    anim.SetAttribute("from", "#FF0000");
    anim.SetAttribute("to", "#0000FF");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_COLOR);
    callback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "fill");
    EXPECT_STREQ(target.lastValue_, "#FF800080");
}

/**
 * @tc.name: SvgAnimatorCallbackMotion_001
 * @tc.desc: Verify the motion callback writes translate transforms sampled from the path.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackMotion_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimateMotion anim;
    anim.SetAttribute("path", "M0 0 L100 0");
    anim.SetAttribute("dur", "2s");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_MOTION);
    callback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    EXPECT_STREQ(target.lastValue_, "translate(50,0)");
}

/**
 * @tc.name: SvgAnimatorCallbackMotionRotate_001
 * @tc.desc: Verify rotate="auto" of animateMotion puts rotation before translation so the
 *           object rotates around its own origin and is then translated to the path point.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackMotionRotate_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimateMotion anim;
    anim.SetAttribute("path", "M0 0 L0 100");
    anim.SetAttribute("rotate", "auto");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_MOTION);
    callback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    EXPECT_STREQ(target.lastValue_, "translate(0,50) rotate(90)");
}

/**
 * @tc.name: SvgAnimatorCallbackMotionRotateOrder_001
 * @tc.desc: Verify the rotation is applied before the translation for a horizontal auto-rotated motion path.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackMotionRotateOrder_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimateMotion anim;
    anim.SetAttribute("path", "M0 0 L100 0");
    anim.SetAttribute("rotate", "auto");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_MOTION);
    callback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastValue_, "translate(50,0) rotate(0)");
}

/**
 * @tc.name: SvgAnimatorCallbackMotionAutoReverse_001
 * @tc.desc: Verify rotate="auto-reverse" adds 180 degrees to the tangent angle.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackMotionAutoReverse_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimateMotion anim;
    anim.SetAttribute("path", "M0 0 L0 100");
    anim.SetAttribute("rotate", "auto-reverse");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_MOTION);
    callback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    EXPECT_STREQ(target.lastValue_, "translate(0,50) rotate(270)");
}

/**
 * @tc.name: SvgAnimatorCallbackMotionAngle_001
 * @tc.desc: Verify rotate="<angle>" uses the fixed angle regardless of path tangent.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackMotionAngle_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimateMotion anim;
    anim.SetAttribute("path", "M0 0 L100 0");
    anim.SetAttribute("rotate", "45");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_MOTION);
    callback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    EXPECT_STREQ(target.lastValue_, "translate(50,0) rotate(45)");
}

/**
 * @tc.name: SvgAnimatorStartStop_001
 * @tc.desc: Verify Start creates animators for animation nodes and Stop releases them.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorStartStop_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);
    RecordingNode* rect = new RecordingNode();
    root->AppendChild(rect);
    CreateOpacityAnimate(rect);

    SvgAnimator animator;
    UIView view;
    doc.SetHostView(&view);
    animator.Start(doc);
    EXPECT_EQ(animator.GetAnimatorCount(), 1);
    // Start is idempotent: running animations are stopped before restarting.
    animator.Start(doc);
    EXPECT_EQ(animator.GetAnimatorCount(), 1);
    animator.Stop();
    EXPECT_EQ(animator.GetAnimatorCount(), 0);
    // Stop restores the start value.
    EXPECT_STREQ(rect->lastValue_, "1");
}

/**
 * @tc.name: SvgAnimatorStartNoAnimation_001
 * @tc.desc: Verify Start on a DOM without animation nodes is a no-op.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorStartNoAnimation_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);
    root->AppendChild(new SvgRectNode());

    SvgAnimator animator;
    animator.Start(doc);
    EXPECT_EQ(animator.GetAnimatorCount(), 0);
}

/**
 * @tc.name: SvgAnimatorStartSkipsInvalid_001
 * @tc.desc: Verify animations with zero duration, missing from/to or unknown type are skipped.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorStartSkipsInvalid_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);
    RecordingNode* rect = new RecordingNode();
    root->AppendChild(rect);

    // zero duration
    SvgAnimate* zeroDur = new SvgAnimate();
    zeroDur->SetAttribute("attributeName", "opacity");
    zeroDur->SetAttribute("from", "1");
    zeroDur->SetAttribute("to", "0");
    rect->AppendChild(zeroDur);

    // missing from/to
    SvgAnimate* noFromTo = new SvgAnimate();
    noFromTo->SetAttribute("attributeName", "opacity");
    noFromTo->SetAttribute("dur", "1s");
    rect->AppendChild(noFromTo);

    // animateTransform without a valid type
    SvgAnimateTransform* noType = new SvgAnimateTransform();
    noType->SetAttribute("from", "0");
    noType->SetAttribute("to", "360");
    noType->SetAttribute("dur", "1s");
    rect->AppendChild(noType);

    SvgAnimator animator;
    animator.Start(doc);
    EXPECT_EQ(animator.GetAnimatorCount(), 0);
}

/**
 * @tc.name: SvgAnimatorSet_001
 * @tc.desc: Verify <set> applies its value immediately without creating an animator.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorSet_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);
    RecordingNode* rect = new RecordingNode();
    root->AppendChild(rect);

    SvgSet* set = new SvgSet();
    set->SetAttribute("attributeName", "fill");
    set->SetAttribute("to", "red");
    rect->AppendChild(set);

    SvgAnimator animator;
    animator.Start(doc);
    EXPECT_EQ(animator.GetAnimatorCount(), 0);
    EXPECT_STREQ(rect->lastName_, "fill");
    EXPECT_STREQ(rect->lastValue_, "red");
}

/**
 * @tc.name: SvgAnimatorMotion_001
 * @tc.desc: Verify animateMotion with a path attribute creates an animator.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorMotion_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);
    RecordingNode* rect = new RecordingNode();
    root->AppendChild(rect);

    SvgAnimateMotion* motion = new SvgAnimateMotion();
    motion->SetAttribute("path", "M0 0 L100 100");
    motion->SetAttribute("dur", "2s");
    rect->AppendChild(motion);

    // animateMotion without any path is skipped
    SvgAnimateMotion* noPath = new SvgAnimateMotion();
    noPath->SetAttribute("dur", "2s");
    rect->AppendChild(noPath);

    SvgAnimator animator;
    UIView view;
    doc.SetHostView(&view);
    animator.Start(doc);
    EXPECT_EQ(animator.GetAnimatorCount(), 1);
    animator.Stop();
}

/**
 * @tc.name: SvgAnimatorMotionMPath_001
 * @tc.desc: Verify animateMotion resolves its path from a child <mpath> reference.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorMotionMPath_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);
    RecordingNode* rect = new RecordingNode();
    root->AppendChild(rect);

    SvgAnimateMotion* motion = new SvgAnimateMotion();
    motion->SetAttribute("dur", "2s");
    rect->AppendChild(motion);
    SvgMPath* mpath = new SvgMPath();
    mpath->SetAttribute("xlink:href", "#motionPath");
    motion->AppendChild(mpath);

    SvgPathNode pathNode;
    pathNode.SetAttribute("d", "M0 0 L50 50");
    doc.RegisterResource("motionPath", &pathNode);

    SvgAnimator animator;
    UIView view;
    doc.SetHostView(&view);
    animator.Start(doc);
    EXPECT_EQ(animator.GetAnimatorCount(), 1);
    ASSERT_NE(motion->GetPathData(), nullptr);
    EXPECT_STREQ(motion->GetPathData(), "M0 0 L50 50");
    animator.Stop();
}

/**
 * @tc.name: SvgAnimatorCallbackRun_001
 * @tc.desc: Verify Callback drives progress from the bound animator without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackRun_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimate anim;
    anim.SetAttribute("attributeName", "opacity");
    anim.SetAttribute("from", "1");
    anim.SetAttribute("to", "0");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_NUMERIC);
    Animator animator(&callback, &view, 1000, false);
    callback.SetAnimator(&animator);
    animator.SetRunTime(250);
    callback.Callback(&view);
    EXPECT_STREQ(target.lastValue_, "0.75");
    // Callback without an animator or with a zero period is a no-op.
    SvgAnimatorCallback orphan(target, anim, SVG_ANIM_KIND_NUMERIC);
    orphan.Callback(&view);
    Animator zeroPeriod(&callback, &view, 0, false);
    callback.SetAnimator(&zeroPeriod);
    callback.Callback(&view);
    callback.SetAnimator(&animator);
}

/**
 * @tc.name: SvgAnimatorRepeatProgress_001
 * @tc.desc: Verify an indefinite repeated animator wraps progress instead of sticking at the end value.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorRepeatProgress_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimate anim;
    anim.SetAttribute("attributeName", "opacity");
    anim.SetAttribute("from", "0");
    anim.SetAttribute("to", "1");
    anim.SetAttribute("repeatCount", "indefinite");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_NUMERIC);
    Animator animator(&callback, &view, 1000, true);
    callback.SetAnimator(&animator);
    // beyond one period: progress must wrap (2200ms of 1000ms -> 0.2)
    animator.SetRunTime(2200);
    callback.Callback(&view);
    EXPECT_STREQ(target.lastValue_, "0.2");
}

/**
 * @tc.name: SvgAnimatorMotionRotateAutoPosition_001
 * @tc.desc: Verify animateMotion rotate="auto" keeps the shape on the motion point:
 *           the emitted transform must rotate the shape around its own origin first
 *           and translate it to the path point second (translate(...) rotate(...)).
 *           With the opposite order the heading rotation swings the shape around the
 *           viewport origin, so it only stays on the path where the angle is zero.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorMotionRotateAutoPosition_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimateMotion motion;
    motion.SetAttribute("path", "M30 80 L170 80 L170 140 L30 140 Z");
    motion.SetAttribute("rotate", "auto");
    UIView view;
    SvgAnimatorCallback callback(target, motion, SVG_ANIM_KIND_MOTION);

    // Perimeter 400: progress 0.5 is the bottom-right corner (170,140), heading 90 deg.
    callback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "transform");
    TransAffine matrix;
    ASSERT_TRUE(SvgAttributeParser::ParseTransform(target.lastValue_, matrix));
    // Map the shape-local origin (0,0): it must land on the motion point.
    // TransAffine data layout is [a, c, e, b, d, f], so the image of (0,0) is (d[2], d[5]).
    const float* data = matrix.GetData();
    EXPECT_NEAR(data[2], 170.0f, 0.5f);
    EXPECT_NEAR(data[5], 140.0f, 0.5f);

    // Progress 0.75 -> 300 of 400: bottom edge from (170,140) towards (30,140),
    // 300-200=100 -> (70,140), heading 180 deg.
    callback.ApplyProgress(0.75f, view);
    ASSERT_TRUE(SvgAttributeParser::ParseTransform(target.lastValue_, matrix));
    data = matrix.GetData();
    EXPECT_NEAR(data[2], 70.0f, 0.5f);
    EXPECT_NEAR(data[5], 140.0f, 0.5f);

    // Progress 0.875 -> 350 of 400: left edge from (30,140) towards (30,80),
    // 350-340=10 -> (30,130), heading -90 deg.
    callback.ApplyProgress(0.875f, view);
    ASSERT_TRUE(SvgAttributeParser::ParseTransform(target.lastValue_, matrix));
    data = matrix.GetData();
    EXPECT_NEAR(data[2], 30.0f, 0.5f);
    EXPECT_NEAR(data[5], 130.0f, 0.5f);
}

static void BuildStressRectColor(SvgRootNode* root)
{
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("x", "10");
    rect->SetAttribute("y", "10");
    rect->SetAttribute("width", "40");
    rect->SetAttribute("height", "40");
    rect->SetAttribute("fill", "#FF0000");
    root->AppendChild(rect);
    SvgAnimateColor* animColor = new SvgAnimateColor();
    animColor->SetAttribute("attributeName", "fill");
    animColor->SetAttribute("from", "#FF0000");
    animColor->SetAttribute("to", "#0000FF");
    animColor->SetAttribute("dur", "2s");
    animColor->SetAttribute("repeatCount", "indefinite");
    rect->AppendChild(animColor);
}

static void BuildStressSetRect(SvgRootNode* root)
{
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("x", "70");
    rect->SetAttribute("y", "10");
    rect->SetAttribute("width", "40");
    rect->SetAttribute("height", "40");
    rect->SetAttribute("fill", "#44FF44");
    root->AppendChild(rect);
    SvgSet* set = new SvgSet();
    set->SetAttribute("attributeName", "opacity");
    set->SetAttribute("to", "0.3");
    rect->AppendChild(set);
}

static SvgPathNode* BuildStressMotionPath(SvgRootNode* root)
{
    SvgPathNode* motionPath = new SvgPathNode();
    motionPath->SetAttribute("id", "motionPath");
    motionPath->SetAttribute("d", "M30 80 L170 80 L170 140 L30 140 Z");
    motionPath->SetAttribute("fill", "none");
    motionPath->SetAttribute("stroke", "#888888");
    motionPath->SetAttribute("stroke-width", "2");
    root->AppendChild(motionPath);
    return motionPath;
}

static void BuildStressMotionPolygon(SvgRootNode* root, SvgPathNode* motionPath)
{
    SvgLineNode* poly = new SvgLineNode(SvgLineNode::POLYGON);
    poly->SetAttribute("points", "0,-12 8,8 -8,8");
    poly->SetAttribute("fill", "#FFAA00");
    root->AppendChild(poly);
    SvgAnimateMotion* motion = new SvgAnimateMotion();
    motion->SetAttribute("dur", "4s");
    motion->SetAttribute("repeatCount", "indefinite");
    motion->SetAttribute("rotate", "auto");
    poly->AppendChild(motion);
    SvgMPath* mpath = new SvgMPath();
    mpath->SetAttribute("xlink:href", "#motionPath");
    motion->AppendChild(mpath);
}

static void RunAnimatorCycles(SvgAnimator& animator, SvgDocument& doc, uint8_t cycles, uint8_t framesPerCycle)
{
    for (uint8_t cycle = 0; cycle < cycles; cycle++) {
        animator.Start(doc);
        for (uint8_t i = 0; i < framesPerCycle; i++) {
            AnimatorManager::GetInstance()->AnimatorTask();
        }
        animator.Stop();
    }
}

/**
 * @tc.name: SvgAnimatorDemoStress_001
 * @tc.desc: Drive the demo SVG (animateColor/set/animateMotion+mpath) through repeated
 *           start/frame/stop cycles like the setting application does.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorDemoStress_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);
    BuildStressRectColor(root);
    BuildStressSetRect(root);
    SvgPathNode* motionPath = BuildStressMotionPath(root);
    doc.RegisterResource("motionPath", motionPath);
    BuildStressMotionPolygon(root, motionPath);

    SvgAnimator animator;
    UIView view;
    doc.SetHostView(&view);
    RunAnimatorCycles(animator, doc, 3, 120);
    // reload while stopped, then run again
    animator.Start(doc);
    for (uint8_t i = 0; i < 60; i++) {
        AnimatorManager::GetInstance()->AnimatorTask();
    }
    animator.Stop();
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SvgAnimatorApplyInitialStates_001
 * @tc.desc: Verify ApplyInitialStates applies progress=0 for begin=0 animations without
 *           creating any running animators.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorApplyInitialStates_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);

    RecordingRect* setTarget = new RecordingRect();
    root->AppendChild(setTarget);
    SvgSet* set = new SvgSet();
    set->SetAttribute("attributeName", "fill");
    set->SetAttribute("to", "red");
    setTarget->AppendChild(set);

    RecordingRect* numericTarget = new RecordingRect();
    root->AppendChild(numericTarget);
    SvgAnimate* numeric = new SvgAnimate();
    numeric->SetAttribute("attributeName", "opacity");
    numeric->SetAttribute("from", "1");
    numeric->SetAttribute("to", "0");
    numeric->SetAttribute("dur", "1s");
    numericTarget->AppendChild(numeric);

    RecordingRect* motionTarget = new RecordingRect();
    root->AppendChild(motionTarget);
    SvgAnimateMotion* motion = new SvgAnimateMotion();
    motion->SetAttribute("path", "M10 20 L110 20");
    motion->SetAttribute("dur", "2s");
    motionTarget->AppendChild(motion);

    SvgAnimator animator;
    UIView view;
    doc.SetHostView(&view);
    animator.ApplyInitialStates(doc, &view);

    EXPECT_EQ(animator.GetAnimatorCount(), 0U);
    EXPECT_STREQ(setTarget->lastName_, "fill");
    EXPECT_STREQ(setTarget->lastValue_, "red");
    EXPECT_STREQ(numericTarget->lastName_, "opacity");
    EXPECT_STREQ(numericTarget->lastValue_, "1");
    EXPECT_STREQ(motionTarget->lastName_, "transform");
    EXPECT_STREQ(motionTarget->lastValue_, "translate(10,20)");
}

/**
 * @tc.name: SvgAnimatorApplyInitialStatesSkipsNonZeroBegin_001
 * @tc.desc: Verify ApplyInitialStates skips animations whose begin time is not zero.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorApplyInitialStatesSkipsNonZeroBegin_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);

    RecordingRect* target = new RecordingRect();
    root->AppendChild(target);
    SvgSet* set = new SvgSet();
    set->SetAttribute("attributeName", "fill");
    set->SetAttribute("to", "red");
    set->SetAttribute("begin", "1s");
    target->AppendChild(set);

    SvgAnimator animator;
    UIView view;
    doc.SetHostView(&view);
    animator.ApplyInitialStates(doc, &view);

    EXPECT_EQ(animator.GetAnimatorCount(), 0U);
    EXPECT_EQ(target->writeCount_, 0U);
}

/**
 * @tc.name: SvgAnimatorApplyInitialStatesMPath_001
 * @tc.desc: Verify ApplyInitialStates resolves an animateMotion path from a child mpath
 *           reference and applies the start position.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorApplyInitialStatesMPath_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);

    SvgPathNode* pathNode = new SvgPathNode();
    pathNode->SetAttribute("id", "motionPath");
    pathNode->SetAttribute("d", "M30 80 L170 80 L170 140 L30 140 Z");
    root->AppendChild(pathNode);
    doc.RegisterResource("motionPath", pathNode);

    RecordingRect* target = new RecordingRect();
    root->AppendChild(target);
    SvgAnimateMotion* motion = new SvgAnimateMotion();
    motion->SetAttribute("dur", "4s");
    motion->SetAttribute("repeatCount", "indefinite");
    motion->SetAttribute("rotate", "auto");
    target->AppendChild(motion);
    SvgMPath* mpath = new SvgMPath();
    mpath->SetAttribute("xlink:href", "#motionPath");
    motion->AppendChild(mpath);

    SvgAnimator animator;
    UIView view;
    doc.SetHostView(&view);
    animator.ApplyInitialStates(doc, &view);

    EXPECT_EQ(animator.GetAnimatorCount(), 0U);
    EXPECT_STREQ(target->lastName_, "transform");
    EXPECT_STREQ(target->lastValue_, "translate(30,80) rotate(0)");
}

/**
 * @tc.name: SvgAnimatorCallbackEndExpiry_002
 * @tc.desc: Verify Callback freezes at endMs_/total when active time passes end.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackEndExpiry_002, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimate anim;
    anim.SetAttribute("attributeName", "opacity");
    anim.SetAttribute("from", "1");
    anim.SetAttribute("to", "0");
    anim.SetAttribute("begin", "0s");
    anim.SetAttribute("end", "0.5s");
    anim.SetAttribute("dur", "1s");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_NUMERIC);
    Animator animator(&callback, &view, 1000, false);
    callback.SetAnimator(&animator);
    // activeTime 600ms >= endMs 500ms -> freeze at endMs_/total = 0.5
    animator.SetRunTime(600);
    callback.Callback(&view);
    EXPECT_STREQ(target.lastName_, "opacity");
    EXPECT_STREQ(target.lastValue_, "0.5");
}

/**
 * @tc.name: SvgAnimatorCallbackValuesCalcModes_002
 * @tc.desc: Verify multi-value interpolation for calcMode discrete and spline easing.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackValuesCalcModes_002, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimate anim;
    anim.SetAttribute("attributeName", "x");
    anim.SetAttribute("values", "0;50;100");
    anim.SetAttribute("calcMode", "discrete");
    anim.SetAttribute("begin", "0s");
    anim.SetAttribute("dur", "1s");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_NUMERIC);
    callback.ApplyProgress(0.0f, view);
    EXPECT_STREQ(target.lastValue_, "0");
    callback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastValue_, "50");
    callback.ApplyProgress(1.0f, view);
    EXPECT_STREQ(target.lastValue_, "100");

    // spline easing with explicit keyTimes/keySplines exercises GetSegment + ApplySplineEase
    RecordingNode splineTarget;
    SvgAnimate spline;
    spline.SetAttribute("attributeName", "x");
    spline.SetAttribute("values", "0;100");
    spline.SetAttribute("calcMode", "spline");
    spline.SetAttribute("keyTimes", "0;1");
    spline.SetAttribute("keySplines", "0.5;0;0.5;1");
    UIView view2;
    SvgAnimatorCallback splineCallback(splineTarget, spline, SVG_ANIM_KIND_NUMERIC);
    splineCallback.ApplyProgress(0.5f, view2);
    EXPECT_GT(splineTarget.writeCount_, 0U);
    EXPECT_STREQ(splineTarget.lastName_, "x");
    EXPECT_STREQ(splineTarget.lastValue_, "50");
}

/**
 * @tc.name: SvgAnimatorCallbackRepeatCountHalf_001
 * @tc.desc: Verify a fractional repeatCount < 1 stops at the correct intermediate progress.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackRepeatCountHalf_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimate anim;
    anim.SetAttribute("attributeName", "opacity");
    anim.SetAttribute("from", "1");
    anim.SetAttribute("to", "0");
    anim.SetAttribute("dur", "1s");
    anim.SetAttribute("repeatCount", "0.5");
    anim.SetAttribute("fill", "freeze");
    UIView view;
    UIView parentView;
    view.SetParent(&parentView);

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_NUMERIC);
    Animator animator(&callback, &view, 1000, true);
    callback.SetAnimator(&animator);
    // activeTime 600ms gives cycles=0.6 >= 0.5, should stop and freeze at 0.5
    animator.SetRunTime(600);
    callback.Callback(&view);
    callback.OnStop(view);
    EXPECT_STREQ(target.lastName_, "opacity");
    EXPECT_STREQ(target.lastValue_, "0.5");
}

/**
 * @tc.name: SvgAnimatorCallbackIntegerRepeatFreeze_001
 * @tc.desc: Verify integer repeatCount with fill=freeze stops at the end of the last cycle.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorCallbackIntegerRepeatFreeze_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimate anim;
    anim.SetAttribute("attributeName", "x");
    anim.SetAttribute("from", "0");
    anim.SetAttribute("to", "100");
    anim.SetAttribute("dur", "1s");
    anim.SetAttribute("repeatCount", "3");
    anim.SetAttribute("fill", "freeze");
    UIView view;
    UIView parentView;
    view.SetParent(&parentView);

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_NUMERIC);
    Animator animator(&callback, &view, 1000, true);
    callback.SetAnimator(&animator);
    // activeTime 3000ms gives cycles=3.0 >= 3, should stop at finalProgress=1.0
    animator.SetRunTime(3000);
    callback.Callback(&view);
    callback.OnStop(view);
    EXPECT_STREQ(target.lastName_, "x");
    EXPECT_STREQ(target.lastValue_, "100");
}

/**
 * @tc.name: SvgAnimatorColorAlpha_001
 * @tc.desc: Verify color interpolation preserves and outputs the alpha channel.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorColorAlpha_001, TestSize.Level1)
{
    RecordingNode target;
    SvgAnimate anim;
    anim.SetAttribute("attributeName", "fill");
    anim.SetAttribute("from", "#FFFF0000");
    anim.SetAttribute("to", "#0000FF00");
    UIView view;

    SvgAnimatorCallback callback(target, anim, SVG_ANIM_KIND_COLOR);
    callback.ApplyProgress(0.5f, view);
    EXPECT_STREQ(target.lastName_, "fill");
    EXPECT_STREQ(target.lastValue_, "#80808000");
}

/**
 * @tc.name: SvgAnimationDurationOverflow_001
 * @tc.desc: Verify an excessively large duration is clamped to UINT32_MAX.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimationDurationOverflow_001, TestSize.Level1)
{
    SvgAnimate anim;
    anim.SetAttribute("dur", "5000000s");
    EXPECT_EQ(anim.GetDuration(), 4294967295U);
}

/**
 * @tc.name: SvgAnimationRepeatCountNan_001
 * @tc.desc: Verify a non-finite repeatCount falls back to the default value.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimationRepeatCountNan_001, TestSize.Level1)
{
    SvgAnimate anim;
    anim.SetAttribute("repeatCount", "nan");
    EXPECT_FLOAT_EQ(anim.GetRepeatCount(), 1.0f);
}

/**
 * @tc.name: SvgAnimatorPauseUnpause_001
 * @tc.desc: Verify PauseAnimations freezes running animations (the callback stops firing while
 *           paused) and UnpauseAnimations resumes the same entries from the frozen point. Entries
 *           are kept during pause, not torn down like Stop does.
 * @tc.type: FUNC
 */
HWTEST_F(SvgAnimatorTest, SvgAnimatorPauseUnpause_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);
    RecordingNode* rect = new RecordingNode();
    root->AppendChild(rect);
    CreateOpacityAnimate(rect);

    SvgAnimator animator;
    UIView view;
    doc.SetHostView(&view);
    animator.Start(doc);
    ASSERT_EQ(animator.GetAnimatorCount(), 1);
    // Start applies progress 0 (opacity 1) exactly once via StartAnimate.
    EXPECT_EQ(rect->writeCount_, 1U);
    EXPECT_STREQ(rect->lastValue_, "1");

    // Pause freezes: entries are kept, but the manager no longer drives them.
    animator.PauseAnimations();
    EXPECT_EQ(animator.GetAnimatorCount(), 1);
    for (uint8_t i = 0; i < 30; i++) {
        AnimatorManager::GetInstance()->AnimatorTask();
    }
    EXPECT_EQ(rect->writeCount_, 1U);
    EXPECT_STREQ(rect->lastValue_, "1");

    // Unpause resumes: the same entry is driven again (callback fires once per tick).
    animator.UnpauseAnimations();
    EXPECT_EQ(animator.GetAnimatorCount(), 1);
    for (uint8_t i = 0; i < 30; i++) {
        AnimatorManager::GetInstance()->AnimatorTask();
    }
    EXPECT_GT(rect->writeCount_, 1U);

    animator.Stop();
    EXPECT_EQ(animator.GetAnimatorCount(), 0);
}

} // namespace OHOS
