/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "animator/animator_manager.h"

#include <climits>
#include <gtest/gtest.h>

#include "animator/easing_equation.h"
#include "common/task_manager.h"
#if GRAPHIC_ENABLE_TRANSITION_ANIM_FLAG
#include "animator/transition_animator_callback.h"
#endif // GRAPHIC_ENABLE_TRANSITION_ANIM_FLAG

using namespace testing::ext;
namespace OHOS {
namespace {
const int16_t START_POS = 0;
const int16_t END_POS = 100;
const uint16_t TIME = 300;
#if GRAPHIC_ENABLE_TRANSITION_ANIM_FLAG
const int16_t TRANSITION_START_X = 10;
const int16_t TRANSITION_START_Y = 20;
const int16_t TRANSITION_END_X = 40;
const int16_t TRANSITION_END_Y = 70;
const int16_t TRANSITION_VIEW_WIDTH = 80;
const int16_t TRANSITION_VIEW_HEIGHT = 60;
const uint8_t TRANSITION_OPA_FROM = 20;
const uint8_t TRANSITION_OPA_TO = 180;
const uint32_t TRANSITION_DURATION = 32;
const uint32_t TRANSITION_DELAY = 16;
const uint32_t TRANSITION_FRAME_TIME = 16;
const int16_t TRANSITION_MIDDLE_X = 25;
const int16_t TRANSITION_MIDDLE_Y = 45;
const int16_t TRANSITION_ROTATE_TO = 90;
const float TRANSITION_SCALE_FROM = 1.0f;
const float TRANSITION_SCALE_TO_X = 2.0f;
const float TRANSITION_SCALE_TO_Y = 1.5f;
const float TRANSITION_HALF = 2.0f;
const float TRANSITION_PIVOT_X = 5.0f;
const float TRANSITION_PIVOT_Y = 6.0f;
const uint8_t TRANSITION_COLOR_FROM_RED = 10;
const uint8_t TRANSITION_COLOR_FROM_GREEN = 20;
const uint8_t TRANSITION_COLOR_FROM_BLUE = 30;
const uint8_t TRANSITION_COLOR_TO_RED = 100;
const uint8_t TRANSITION_COLOR_TO_GREEN = 120;
const uint8_t TRANSITION_COLOR_TO_BLUE = 140;
const uint8_t TRANSITION_SCALE_X_INDEX = 0;
const uint8_t TRANSITION_SCALE_Y_INDEX = 5;

int16_t EasingBelowRange(int16_t startPos, int16_t endPos, uint16_t curTime, uint16_t durationTime)
{
    (void)startPos;
    (void)endPos;
    (void)curTime;
    (void)durationTime;
    return -1;
}

int16_t EasingAboveRange(int16_t startPos, int16_t endPos, uint16_t curTime, uint16_t durationTime)
{
    (void)startPos;
    (void)endPos;
    (void)curTime;
    (void)durationTime;
    return OPA_OPAQUE + 1;
}

#endif // GRAPHIC_ENABLE_TRANSITION_ANIM_FLAG
} // namespace

class TestAnimatorCallback : public AnimatorCallback {
public:
    explicit TestAnimatorCallback(UIView* view) : view_(view), animator_(nullptr) {}

    virtual ~TestAnimatorCallback()
    {
        if (animator_ != nullptr) {
            delete animator_;
            animator_ = nullptr;
        }
    }

    bool Init()
    {
        if (animator_ == nullptr) {
            animator_ = new Animator(this, view_, TIME, false);
            if (animator_ == nullptr) {
                return false;
            }
            return true;
        }
        return false;
    }

    void Callback(UIView* view) override
    {
        int16_t pos = EasingEquation::LinearEaseNone(START_POS, END_POS, animator_->GetRunTime(), animator_->GetTime());
        view_->SetX(pos);
    }

    void OnStop(UIView& view) override
    {
        view_->SetX(END_POS);
    }

    Animator* GetAnimator() const
    {
        return animator_;
    }

protected:
    UIView* view_;
    Animator* animator_;
};

class AnimatorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static Animator* animator_;
};

Animator* AnimatorTest::animator_ = nullptr;

void AnimatorTest::SetUpTestCase(void)
{
    if (animator_ == nullptr) {
        animator_ = new Animator();
    }
}

void AnimatorTest::TearDownTestCase(void)
{
    if (animator_ != nullptr) {
        delete animator_;
        animator_ = nullptr;
    }
}

/**
 * @tc.name: AnimatorGetState_001
 * @tc.desc: Verify Start function, equal.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, AnimatorGetState_001, TestSize.Level1)
{
    if (animator_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    animator_->Start();
    EXPECT_EQ(animator_->GetState(), Animator::START);
    animator_->Stop();
    EXPECT_EQ(animator_->GetState(), Animator::STOP);
    animator_->Pause();
    EXPECT_EQ(animator_->GetState(), Animator::PAUSE);
    animator_->Resume();
    EXPECT_EQ(animator_->GetState(), Animator::START);
}

/**
 * @tc.name: AnimatorSetState_001
 * @tc.desc: Verify SetState function, equal.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, AnimatorSetState_001, TestSize.Level1)
{
    if (animator_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    animator_->SetState(Animator::START);
    EXPECT_EQ(animator_->GetState(), Animator::START);
    animator_->SetState(Animator::STOP);
    EXPECT_EQ(animator_->GetState(), Animator::STOP);
    animator_->SetState(Animator::PAUSE);
    EXPECT_EQ(animator_->GetState(), Animator::PAUSE);
}

/**
 * @tc.name: AnimatorSetTime_001
 * @tc.desc: Verify SetTime function, equal.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, AnimatorSetTime_001, TestSize.Level1)
{
    if (animator_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    animator_->SetTime(TIME);
    EXPECT_EQ(animator_->GetTime(), TIME);
}

/**
 * @tc.name: AnimatorSetRunTime_001
 * @tc.desc: Verify SetRunTime function, equal.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, AnimatorSetRunTime_001, TestSize.Level1)
{
    if (animator_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    animator_->SetRunTime(TIME);
    EXPECT_EQ(animator_->GetRunTime(), TIME);
}

/**
 * @tc.name: AnimatorIsRepeat_001
 * @tc.desc: Verify IsRepeat function, equal.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, AnimatorIsRepeat_001, TestSize.Level1)
{
    if (animator_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    EXPECT_EQ(animator_->IsRepeat(), false);
}

/**
 * @tc.name: AnimatorManagerAddAndRemove_001
 * @tc.desc: Verify AddAndRemove function, equal.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, AnimatorManagerAddAndRemove_001, TestSize.Level0)
{
    UIView* view = new UIView();
    view->SetX(START_POS);
    auto callback = new TestAnimatorCallback(view);
    if (!callback->Init()) {
        EXPECT_NE(0, 0);
        return;
    }
    Animator* animator = callback->GetAnimator();
    AnimatorManager::GetInstance()->Init();
    animator->Start();
    EXPECT_EQ(animator->GetState(), Animator::START);
    TaskManager::GetInstance()->SetTaskRun(true);
    while (1) {
        TaskManager::GetInstance()->TaskHandler();
        if (animator->GetState() == Animator::STOP) {
            break;
        }
    }
    EXPECT_EQ(view->GetX(), END_POS);

    view->SetX(START_POS);
    animator->Start();
    EXPECT_EQ(animator->GetState(), Animator::START);
    for (uint16_t i = 0; i < TIME; i++) {
        TaskManager::GetInstance()->TaskHandler();
    }
    EXPECT_EQ(view->GetX(), START_POS);
    TaskManager::GetInstance()->SetTaskRun(false);
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
    delete callback;
    delete view;
}

#if GRAPHIC_ENABLE_TRANSITION_ANIM_FLAG
/**
 * @tc.name: TransitionAnimatorCallbackApply_001
 * @tc.desc: Verify the callback applies externally supplied progress.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, TransitionAnimatorCallbackApply_001, TestSize.Level1)
{
    UIView view;
    TransitionAnimatorCallback callback;
    view.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_VIEW_WIDTH, TRANSITION_VIEW_HEIGHT);
    callback.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_END_X, TRANSITION_END_Y);
    callback.SetOpacity(TRANSITION_OPA_FROM, TRANSITION_OPA_TO);
    callback.ApplyFrame(&view, TIME, TIME);

    EXPECT_EQ(view.GetX(), TRANSITION_END_X);
    EXPECT_EQ(view.GetY(), TRANSITION_END_Y);
    EXPECT_EQ(view.GetOpaScale(), TRANSITION_OPA_TO);
}

/**
 * @tc.name: AnimatorSetView_001
 * @tc.desc: Verify an animator can replace the view used by its callback.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, AnimatorSetView_001, TestSize.Level1)
{
    UIView view;
    Animator animator;

    EXPECT_EQ(animator.GetView(), nullptr);
    animator.SetView(&view);
    EXPECT_EQ(animator.GetView(), &view);
    animator.SetView(nullptr);
    EXPECT_EQ(animator.GetView(), nullptr);
}

/**
 * @tc.name: TransitionAnimatorCallbackState_001
 * @tc.desc: Verify reset and invalid frame inputs preserve the view state.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, TransitionAnimatorCallbackState_001, TestSize.Level1)
{
    UIView view;
    TransitionAnimatorCallback callback;
    view.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_VIEW_WIDTH, TRANSITION_VIEW_HEIGHT);

    EXPECT_FALSE(callback.HasEffects());
    callback.Callback(nullptr);
    callback.Callback(&view);
    callback.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_END_X, TRANSITION_END_Y);
    EXPECT_TRUE(callback.HasEffects());
    callback.ApplyFrame(nullptr, TRANSITION_DURATION, TRANSITION_DURATION);
    view.SetPosition(TRANSITION_END_X, TRANSITION_END_Y);
    callback.ApplyFrame(&view, 0, TRANSITION_DURATION);
    EXPECT_EQ(view.GetX(), TRANSITION_START_X);
    EXPECT_EQ(view.GetY(), TRANSITION_START_Y);
    callback.ApplyFrame(&view, TRANSITION_DURATION, 0);
    EXPECT_EQ(view.GetX(), TRANSITION_START_X);
    EXPECT_EQ(view.GetY(), TRANSITION_START_Y);

    callback.SetDuration(TRANSITION_DURATION);
    callback.SetDelay(0);
    view.SetPosition(TRANSITION_END_X, TRANSITION_END_Y);
    callback.Callback(&view);
    EXPECT_EQ(view.GetX(), TRANSITION_START_X);
    EXPECT_EQ(view.GetY(), TRANSITION_START_Y);

    callback.Reset();
    EXPECT_FALSE(callback.HasEffects());
    callback.ApplyFrame(&view, TRANSITION_DURATION, TRANSITION_DURATION);
    EXPECT_EQ(view.GetX(), TRANSITION_START_X);
    EXPECT_EQ(view.GetY(), TRANSITION_START_Y);
}

/**
 * @tc.name: TransitionAnimatorCallbackProperties_001
 * @tc.desc: Verify a transition frame applies position, opacity, color, scale, and rotation together.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, TransitionAnimatorCallbackProperties_001, TestSize.Level1)
{
    UIView view;
    TransitionAnimatorCallback callback;
    const ColorType colorFrom =
        Color::GetColorFromRGB(TRANSITION_COLOR_FROM_RED, TRANSITION_COLOR_FROM_GREEN, TRANSITION_COLOR_FROM_BLUE);
    const ColorType colorTo =
        Color::GetColorFromRGB(TRANSITION_COLOR_TO_RED, TRANSITION_COLOR_TO_GREEN, TRANSITION_COLOR_TO_BLUE);
    view.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_VIEW_WIDTH, TRANSITION_VIEW_HEIGHT);
    callback.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_END_X, TRANSITION_END_Y);
    callback.SetOpacity(TRANSITION_OPA_FROM, TRANSITION_OPA_TO);
    callback.SetColor(colorFrom, colorTo);
    callback.SetScale(TRANSITION_SCALE_FROM, TRANSITION_SCALE_FROM, TRANSITION_SCALE_TO_X, TRANSITION_SCALE_TO_Y,
                      TRANSITION_PIVOT_X, TRANSITION_PIVOT_Y);
    callback.SetRotation(0, TRANSITION_ROTATE_TO, TRANSITION_PIVOT_X, TRANSITION_PIVOT_Y);
    callback.ApplyFrame(&view, TRANSITION_DURATION + TRANSITION_FRAME_TIME, TRANSITION_DURATION);

    EXPECT_EQ(view.GetX(), TRANSITION_END_X);
    EXPECT_EQ(view.GetY(), TRANSITION_END_Y);
    EXPECT_EQ(view.GetOpaScale(), TRANSITION_OPA_TO);
    EXPECT_EQ(view.GetStyle(STYLE_BACKGROUND_COLOR), colorTo.full);
    EXPECT_EQ(view.GetTransformMap().GetRotateAngle(), TRANSITION_ROTATE_TO);
    const float *scaleMatrix = view.GetTransformMap().GetScaleMatrix().GetData();
    EXPECT_FLOAT_EQ(scaleMatrix[TRANSITION_SCALE_X_INDEX], TRANSITION_SCALE_TO_X);
    EXPECT_FLOAT_EQ(scaleMatrix[TRANSITION_SCALE_Y_INDEX], TRANSITION_SCALE_TO_Y);
}

/**
 * @tc.name: TransitionAnimatorCallbackDefaultPivot_001
 * @tc.desc: Verify scale and rotation use the view center when no pivot is supplied.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, TransitionAnimatorCallbackDefaultPivot_001, TestSize.Level1)
{
    UIView view;
    TransitionAnimatorCallback callback;
    view.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_VIEW_WIDTH, TRANSITION_VIEW_HEIGHT);
    callback.SetScale(TRANSITION_SCALE_FROM, TRANSITION_SCALE_FROM, TRANSITION_SCALE_TO_X, TRANSITION_SCALE_TO_Y);
    callback.SetRotation(0, TRANSITION_ROTATE_TO);
    callback.ApplyFrame(&view, TRANSITION_DURATION, TRANSITION_DURATION);

    const Vector3<float> &pivot = view.GetTransformMap().GetPivot();
    EXPECT_FLOAT_EQ(pivot.x_, TRANSITION_VIEW_WIDTH / TRANSITION_HALF);
    EXPECT_FLOAT_EQ(pivot.y_, TRANSITION_VIEW_HEIGHT / TRANSITION_HALF);
    EXPECT_EQ(view.GetTransformMap().GetRotateAngle(), TRANSITION_ROTATE_TO);
}

/**
 * @tc.name: TransitionAnimatorCallbackTimeline_001
 * @tc.desc: Verify callback frames honor delay and use linear easing when a null easing is supplied.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, TransitionAnimatorCallbackTimeline_001, TestSize.Level1)
{
    UIView view;
    TransitionAnimatorCallback callback;
    view.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_VIEW_WIDTH, TRANSITION_VIEW_HEIGHT);
    callback.SetDuration(TRANSITION_DURATION);
    callback.SetDelay(TRANSITION_DELAY);
    callback.SetEasingFunc(nullptr);
    callback.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_END_X, TRANSITION_END_Y);

    // 使用 ApplyFrame 并显式传入已过时间，保证单元测试确定性；
    // Callback() 内部读取 HALTick 真实时间，在快速主机上不稳定。
    callback.ApplyFrame(&view, 0, TRANSITION_DURATION);
    EXPECT_EQ(view.GetX(), TRANSITION_START_X);
    EXPECT_EQ(view.GetY(), TRANSITION_START_Y);

    callback.ApplyFrame(&view, TRANSITION_DELAY, TRANSITION_DURATION);
    EXPECT_EQ(view.GetX(), TRANSITION_START_X);
    EXPECT_EQ(view.GetY(), TRANSITION_START_Y);

    callback.ApplyFrame(&view, TRANSITION_DELAY + TRANSITION_DURATION / 2, TRANSITION_DURATION);
    EXPECT_EQ(view.GetX(), TRANSITION_MIDDLE_X);
    EXPECT_EQ(view.GetY(), TRANSITION_MIDDLE_Y);

    callback.ApplyFrame(&view, TRANSITION_DELAY + TRANSITION_DURATION, TRANSITION_DURATION);
    EXPECT_EQ(view.GetX(), TRANSITION_END_X);
    EXPECT_EQ(view.GetY(), TRANSITION_END_Y);
}

/**
 * @tc.name: TransitionAnimatorCallbackSequential_001
 * @tc.desc: Verify sequential callbacks use one continuous timeline.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, TransitionAnimatorCallbackSequential_001, TestSize.Level1)
{
    UIView view;
    TransitionAnimatorCallback first;
    TransitionAnimatorCallback second;
    first.SetDuration(TRANSITION_DURATION);
    first.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_END_X, TRANSITION_END_Y);
    second.SetDuration(TRANSITION_DURATION);
    second.SetPosition(TRANSITION_END_X, TRANSITION_END_Y, TRANSITION_END_X + TRANSITION_START_X,
                      TRANSITION_END_Y + TRANSITION_START_Y);
    first.AddTransitionAnimatorCallback(&second, true);

    EXPECT_EQ(first.GetTotalTime(), TRANSITION_DURATION * 2);
    first.ApplyFrame(&view, TRANSITION_DURATION / 2, first.GetTotalTime());
    EXPECT_EQ(view.GetX(), TRANSITION_MIDDLE_X);
    first.ApplyFrame(&view, TRANSITION_DURATION + TRANSITION_DURATION / 2, first.GetTotalTime());
    EXPECT_EQ(view.GetX(), TRANSITION_END_X + TRANSITION_START_X / 2);
}

/**
 * @tc.name: TransitionAnimatorCallbackParallel_001
 * @tc.desc: Verify parallel callbacks share the same timeline.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, TransitionAnimatorCallbackParallel_001, TestSize.Level1)
{
    UIView view;
    TransitionAnimatorCallback position;
    TransitionAnimatorCallback opacity;
    position.SetDuration(TRANSITION_DURATION);
    position.SetPosition(TRANSITION_START_X, TRANSITION_START_Y, TRANSITION_END_X, TRANSITION_END_Y);
    opacity.SetDuration(TRANSITION_DURATION);
    opacity.SetOpacity(TRANSITION_OPA_FROM, TRANSITION_OPA_TO);
    position.AddTransitionAnimatorCallback(&opacity, false);

    EXPECT_EQ(position.GetTotalTime(), TRANSITION_DURATION);
    position.ApplyFrame(&view, TRANSITION_DURATION, position.GetTotalTime());
    EXPECT_EQ(view.GetX(), TRANSITION_END_X);
    EXPECT_EQ(view.GetOpaScale(), TRANSITION_OPA_TO);
}

/**
 * @tc.name: TransitionAnimatorCallbackValueBoundary_001
 * @tc.desc: Verify opacity and color values produced outside the valid range are clamped.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQM
 */
HWTEST_F(AnimatorTest, TransitionAnimatorCallbackValueBoundary_001, TestSize.Level1)
{
    UIView view;
    TransitionAnimatorCallback callback;
    const ColorType colorFrom =
        Color::GetColorFromRGB(TRANSITION_COLOR_FROM_RED, TRANSITION_COLOR_FROM_GREEN, TRANSITION_COLOR_FROM_BLUE);
    const ColorType colorTo =
        Color::GetColorFromRGB(TRANSITION_COLOR_TO_RED, TRANSITION_COLOR_TO_GREEN, TRANSITION_COLOR_TO_BLUE);
    const ColorType transparentBlack = Color::GetColorFromRGBA(0, 0, 0, 0);
    const ColorType white = Color::White();
    callback.SetOpacity(TRANSITION_OPA_FROM, TRANSITION_OPA_TO);
    callback.SetColor(colorFrom, colorTo);

    callback.SetEasingFunc(EasingBelowRange);
    callback.ApplyFrame(&view, TRANSITION_DURATION, TRANSITION_DURATION);
    EXPECT_EQ(view.GetOpaScale(), 0);
    EXPECT_EQ(view.GetStyle(STYLE_BACKGROUND_COLOR), transparentBlack.full);
    callback.SetEasingFunc(EasingAboveRange);
    callback.ApplyFrame(&view, TRANSITION_DURATION, TRANSITION_DURATION);
    EXPECT_EQ(view.GetOpaScale(), OPA_OPAQUE);
    EXPECT_EQ(view.GetStyle(STYLE_BACKGROUND_COLOR), white.full);
}

#endif // GRAPHIC_ENABLE_TRANSITION_ANIM_FLAG
} // namespace OHOS
