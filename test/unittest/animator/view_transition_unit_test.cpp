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

#include "animator/view_transition.h"

#include <cstdint>
#include <gtest/gtest.h>
#include <unistd.h>

#include "animator/animator_manager.h"
#include "common/task_manager.h"
#include "components/ui_view.h"

using namespace testing::ext;
namespace OHOS {
namespace {
const int16_t OUT_VIEW_X = 10;
const int16_t OUT_VIEW_Y = 20;
const int16_t IN_VIEW_X = 5;
const int16_t IN_VIEW_Y = 6;
const int16_t VIEW_WIDTH = 80;
const int16_t VIEW_HEIGHT = 60;
const int32_t TRANSITION_DURATION = 300;
const int32_t SHORT_DURATION = 50;
const int32_t TRANSITION_DELAY = 1000;
const uint32_t TASK_WAIT_US = 1000;
const uint32_t MAX_TASK_ROUND = 1000;
const uint8_t CUSTOM_OPA = 200;
const int16_t SHARED_ORIG_X = 50;
const int16_t SHARED_ORIG_Y = 60;
const int16_t SHARED_SIZE = 40;
const int16_t SHARED_END_X = 100;
const int16_t SHARED_END_Y = 110;
const int16_t SHARED_END_W = 80;
const int16_t SHARED_END_H = 80;
const float IDENTITY_SCALE = 1.0f;

// Drives the animator frames until the transition stops or the round budget is exhausted,
// so a regression fails the assertions instead of hanging the test runner.
void RunTransitionUntilStop(ViewTransition* transition)
{
    TaskManager::GetInstance()->SetTaskRun(true);
    for (uint32_t i = 0; i < MAX_TASK_ROUND; i++) {
        usleep(TASK_WAIT_US);
        TaskManager::GetInstance()->TaskHandler();
        if ((transition != nullptr) && (transition->GetState() == Animator::STOP)) {
            break;
        }
    }
    TaskManager::GetInstance()->SetTaskRun(false);
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}
} // namespace

class TestTransitionListener : public ViewTransition::OnTransitionListener {
public:
    void OnTransitionComplete(ViewTransition& transition) override
    {
        completeCount_++;
        lastTransition_ = &transition;
    }

    uint8_t completeCount_ = 0;
    ViewTransition* lastTransition_ = nullptr;
};

class ViewTransitionTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: ViewTransitionStartWithoutViews_001
 * @tc.desc: Verify Start is a no-op when neither the outgoing view nor the incoming view is set,
 *           and Stop or Cancel before Start does nothing.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionStartWithoutViews_001, TestSize.Level0)
{
    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    EXPECT_EQ(transition.GetState(), Animator::STOP);
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 0);

    transition.Stop();
    transition.Cancel();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 0);
}

/**
 * @tc.name: ViewTransitionZeroDurationFinish_001
 * @tc.desc: Verify Start jumps to the final state synchronously when the duration is zero,
 *           without starting the animator.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionZeroDurationFinish_001, TestSize.Level0)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetOpaScale(CUSTOM_OPA);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);

    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(listener.lastTransition_, &transition);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(outView.GetOpaScale(), CUSTOM_OPA);
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionZeroDurationWithDelay_001
 * @tc.desc: Verify duration=0 with a positive delay waits for the delay and then performs a
 *           hard cut to the final state (delay-then-hard-cut), instead of getting stuck.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionZeroDurationWithDelay_001, TestSize.Level0)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    inView.SetVisible(false);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(0);
    transition.SetDelay(TRANSITION_DELAY);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetOpaScale(), 0);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(listener.completeCount_, 0);

    // a few frames within the delay period: views must stay at start-of-transition state
    TaskManager::GetInstance()->SetTaskRun(true);
    for (uint32_t i = 0; i < 5; i++) {
        usleep(TASK_WAIT_US);
        TaskManager::GetInstance()->TaskHandler();
    }
    TaskManager::GetInstance()->SetTaskRun(false);
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetOpaScale(), 0);
    EXPECT_EQ(listener.completeCount_, 0);

    // run until the delay elapses and the hard cut finishes
    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(listener.lastTransition_, &transition);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionStopBeforeStart_001
 * @tc.desc: Verify Stop and Cancel do not modify the views when the transition has not started.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionStopBeforeStart_001, TestSize.Level0)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetOpaScale(CUSTOM_OPA);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    transition.Stop();
    transition.Cancel();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 0);
    EXPECT_EQ(outView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(outView.GetY(), OUT_VIEW_Y);
    EXPECT_EQ(outView.GetOpaScale(), CUSTOM_OPA);
    EXPECT_TRUE(outView.IsVisible());
    EXPECT_EQ(inView.GetX(), IN_VIEW_X);
    EXPECT_EQ(inView.GetY(), IN_VIEW_Y);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_TRUE(inView.IsVisible());
}

/**
 * @tc.name: ViewTransitionInvalidateTargets_001
 * @tc.desc: Verify Start is a no-op after InvalidateTargets clears the view references.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionInvalidateTargets_001, TestSize.Level0)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    transition.InvalidateTargets();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 0);
    EXPECT_EQ(outView.GetX(), OUT_VIEW_X);
    EXPECT_TRUE(outView.IsVisible());
    EXPECT_EQ(inView.GetX(), IN_VIEW_X);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
}

/**
 * @tc.name: ViewTransitionFadeTransition_001
 * @tc.desc: Verify the fade transition makes the incoming view fully transparent at start, then fades out the
 *           outgoing view to its snapshot opacity and fades in the incoming view on completion.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionFadeTransition_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetOpaScale(CUSTOM_OPA);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetOpaScale(), 0);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_TRUE(outView.IsVisible());

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(listener.lastTransition_, &transition);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(outView.GetOpaScale(), CUSTOM_OPA);
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionSlideLeftTransition_001
 * @tc.desc: Verify the slide-left transition positions the incoming view on the right side of the
 *           outgoing view at start, and on completion aligns it with the outgoing view position.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSlideLeftTransition_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SLIDE_LEFT);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X + VIEW_WIDTH);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y);
    EXPECT_TRUE(inView.IsVisible());

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_EQ(outView.GetX(), OUT_VIEW_X);
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionSlideRightTransition_001
 * @tc.desc: Verify the slide-right transition positions the incoming view on the left side of the
 *           outgoing view at start, and on completion aligns it with the outgoing view position.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSlideRightTransition_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SLIDE_RIGHT);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X - VIEW_WIDTH);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y);

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(outView.GetX(), OUT_VIEW_X);
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionSlideUpTransition_001
 * @tc.desc: Verify the slide-up transition positions the incoming view below the outgoing view
 *           at start, and on completion aligns it with the outgoing view position.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSlideUpTransition_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SLIDE_UP);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y + VIEW_HEIGHT);

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(outView.GetY(), OUT_VIEW_Y);
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionSlideDownTransition_001
 * @tc.desc: Verify the slide-down transition positions the incoming view above the outgoing view
 *           at start, and on completion aligns it with the outgoing view position.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSlideDownTransition_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SLIDE_DOWN);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y - VIEW_HEIGHT);

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(outView.GetY(), OUT_VIEW_Y);
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionScaleTransition_001
 * @tc.desc: Verify the scale transition makes the incoming view fully transparent at start, and on completion the
 *           incoming view is fully shown and the transforms of both views are reset to identity.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionScaleTransition_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SCALE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetOpaScale(), 0);

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(outView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_FALSE(outView.IsVisible());
    // 0 and 5 are the x and y scale indices of the scale matrix
    const float* inScale = inView.GetTransformMap().GetScaleMatrix().GetData();
    EXPECT_FLOAT_EQ(inScale[0], IDENTITY_SCALE);
    EXPECT_FLOAT_EQ(inScale[5], IDENTITY_SCALE); // 5 : y scale index
    const float* outScale = outView.GetTransformMap().GetScaleMatrix().GetData();
    EXPECT_FLOAT_EQ(outScale[0], IDENTITY_SCALE);
    EXPECT_FLOAT_EQ(outScale[5], IDENTITY_SCALE); // 5 : y scale index
}

/**
 * @tc.name: ViewTransitionSharedElementTransition_001
 * @tc.desc: Verify the shared-element transition moves the shared element to the start rect at
 *           start, and snaps it to the end rect with the end scale on completion.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSharedElementTransition_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    UIView sharedElement;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    sharedElement.SetPosition(SHARED_ORIG_X, SHARED_ORIG_Y, SHARED_SIZE, SHARED_SIZE);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SHARED_ELEMENT);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetSharedElement(&sharedElement);
    transition.SetSharedStartRect(0, 0, SHARED_SIZE, SHARED_SIZE);
    transition.SetSharedEndRect(SHARED_END_X, SHARED_END_Y, SHARED_END_W, SHARED_END_H);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(sharedElement.GetX(), 0);
    EXPECT_EQ(sharedElement.GetY(), 0);
    EXPECT_TRUE(sharedElement.IsVisible());
    EXPECT_EQ(sharedElement.GetOpaScale(), OPA_OPAQUE);
    EXPECT_EQ(inView.GetOpaScale(), 0);

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(sharedElement.GetX(), SHARED_END_X);
    EXPECT_EQ(sharedElement.GetY(), SHARED_END_Y);
    EXPECT_TRUE(sharedElement.IsVisible());
    EXPECT_EQ(sharedElement.GetOpaScale(), OPA_OPAQUE);
    // the end scale is the end rect size over the original size: 80 / 40 = 2
    const float* sharedScale = sharedElement.GetTransformMap().GetScaleMatrix().GetData();
    EXPECT_FLOAT_EQ(sharedScale[0], 2.0f);    // 0 : x scale index
    EXPECT_FLOAT_EQ(sharedScale[5], 2.0f);    // 5 : y scale index
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionSharedElementInitialScale_001
 * @tc.desc: Verify the shared element is scaled to the start rect size immediately at
 *           transition start, before any animation frame runs.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSharedElementInitialScale_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    UIView sharedElement;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    sharedElement.SetPosition(SHARED_ORIG_X, SHARED_ORIG_Y, SHARED_SIZE, SHARED_SIZE);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SHARED_ELEMENT);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetSharedElement(&sharedElement);
    transition.SetSharedStartRect(0, 0, SHARED_SIZE / 2, SHARED_SIZE / 2);
    transition.SetSharedEndRect(SHARED_END_X, SHARED_END_Y, SHARED_END_W, SHARED_END_H);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    // start rect is half the original size, so the initial scale must be 0.5
    const float* startScale = sharedElement.GetTransformMap().GetScaleMatrix().GetData();
    EXPECT_FLOAT_EQ(startScale[0], 0.5f);
    EXPECT_FLOAT_EQ(startScale[5], 0.5f);

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    const float* endScale = sharedElement.GetTransformMap().GetScaleMatrix().GetData();
    EXPECT_FLOAT_EQ(endScale[0], 2.0f);
    EXPECT_FLOAT_EQ(endScale[5], 2.0f);
}

/**
 * @tc.name: ViewTransitionSetEasingFuncNull_001
 * @tc.desc: Verify passing nullptr to SetEasingFunc is silently downgraded to linear easing
 *           so the transition can still run and finish without crashing.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSetEasingFuncNull_001, TestSize.Level0)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);
    transition.SetEasingFunc(nullptr);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionCancelFade_001
 * @tc.desc: Verify Stop cancels a running fade transition, restores both views to their
 *           snapshot states, and does not invoke the completion listener.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionCancelFade_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetOpaScale(CUSTOM_OPA);
    inView.SetVisible(false);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(inView.GetOpaScale(), 0);

    transition.Stop();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 0);
    EXPECT_EQ(outView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(outView.GetY(), OUT_VIEW_Y);
    EXPECT_EQ(outView.GetOpaScale(), CUSTOM_OPA);
    EXPECT_TRUE(outView.IsVisible());
    EXPECT_EQ(inView.GetX(), IN_VIEW_X);
    EXPECT_EQ(inView.GetY(), IN_VIEW_Y);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_FALSE(inView.IsVisible());
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}

/**
 * @tc.name: ViewTransitionCancelSlideLeft_001
 * @tc.desc: Verify Cancel restores the view positions moved by a running slide transition.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionCancelSlideLeft_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SLIDE_LEFT);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X + VIEW_WIDTH);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y);

    transition.Cancel();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 0);
    EXPECT_EQ(inView.GetX(), IN_VIEW_X);
    EXPECT_EQ(inView.GetY(), IN_VIEW_Y);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(outView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(outView.GetY(), OUT_VIEW_Y);
    EXPECT_TRUE(outView.IsVisible());
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}

/**
 * @tc.name: ViewTransitionCancelSharedElement_001
 * @tc.desc: Verify Cancel restores the shared element position, opacity and visibility
 *           modified by a running shared-element transition.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionCancelSharedElement_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    UIView sharedElement;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    sharedElement.SetPosition(SHARED_ORIG_X, SHARED_ORIG_Y, SHARED_SIZE, SHARED_SIZE);
    sharedElement.SetOpaScale(CUSTOM_OPA);
    sharedElement.SetVisible(false);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SHARED_ELEMENT);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetSharedElement(&sharedElement);
    transition.SetSharedStartRect(0, 0, SHARED_SIZE, SHARED_SIZE);
    transition.SetSharedEndRect(SHARED_END_X, SHARED_END_Y, SHARED_END_W, SHARED_END_H);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(sharedElement.GetX(), 0);
    EXPECT_EQ(sharedElement.GetY(), 0);
    EXPECT_TRUE(sharedElement.IsVisible());
    EXPECT_EQ(sharedElement.GetOpaScale(), OPA_OPAQUE);

    transition.Cancel();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 0);
    EXPECT_EQ(sharedElement.GetX(), SHARED_ORIG_X);
    EXPECT_EQ(sharedElement.GetY(), SHARED_ORIG_Y);
    EXPECT_EQ(sharedElement.GetOpaScale(), CUSTOM_OPA);
    EXPECT_FALSE(sharedElement.IsVisible());
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}

/**
 * @tc.name: ViewTransitionCancelScale_001
 * @tc.desc: Verify Cancel restores a running scale transition: the scale transform applied
 *           to both views returns to identity, and position/opacity/visibility snapshots
 *           are restored without invoking the completion listener.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionCancelScale_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    inView.SetVisible(false);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetOpaScale(CUSTOM_OPA);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SCALE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetOpaScale(), 0);
    EXPECT_TRUE(inView.IsVisible());

    // run a few frames so HandleScale applies a non-identity scale to the views
    TaskManager::GetInstance()->SetTaskRun(true);
    for (uint32_t i = 0; i < 5; i++) {
        usleep(TASK_WAIT_US);
        TaskManager::GetInstance()->TaskHandler();
    }
    TaskManager::GetInstance()->SetTaskRun(false);
    EXPECT_EQ(transition.GetState(), Animator::START);
    // inScale = SCALE_MIN + t * SCALE_SHRINK is below 1 once the first frame is applied
    const float* midInScale = inView.GetTransformMap().GetScaleMatrix().GetData();
    EXPECT_LT(midInScale[0], IDENTITY_SCALE); // 0 : x scale index

    transition.Cancel();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 0);
    const float* inScale = inView.GetTransformMap().GetScaleMatrix().GetData();
    EXPECT_FLOAT_EQ(inScale[0], IDENTITY_SCALE);    // 0 : x scale index
    EXPECT_FLOAT_EQ(inScale[5], IDENTITY_SCALE);    // 5 : y scale index
    const float* outScale = outView.GetTransformMap().GetScaleMatrix().GetData();
    EXPECT_FLOAT_EQ(outScale[0], IDENTITY_SCALE);
    EXPECT_FLOAT_EQ(outScale[5], IDENTITY_SCALE);
    EXPECT_EQ(inView.GetX(), IN_VIEW_X);
    EXPECT_EQ(inView.GetY(), IN_VIEW_Y);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_FALSE(inView.IsVisible());
    EXPECT_EQ(outView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(outView.GetY(), OUT_VIEW_Y);
    EXPECT_EQ(outView.GetOpaScale(), CUSTOM_OPA);
    EXPECT_TRUE(outView.IsVisible());
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}

/**
 * @tc.name: ViewTransitionDelayKeepsStartState_001
 * @tc.desc: Verify the views stay in their start-of-transition states during the delay period
 *           and no interpolation happens before the delay elapses.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionDelayKeepsStartState_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(SHORT_DURATION);
    transition.SetDelay(TRANSITION_DELAY);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetOpaScale(), 0);

    TaskManager::GetInstance()->SetTaskRun(true);
    TaskManager::GetInstance()->TaskHandler();
    TaskManager::GetInstance()->SetTaskRun(false);
    // the running time is still within the delay period, so the interpolation must not start
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetOpaScale(), 0);
    EXPECT_EQ(outView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_EQ(listener.completeCount_, 0);

    transition.Cancel();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}

/**
 * @tc.name: ViewTransitionDelayKeepsStartStateSlide_001
 * @tc.desc: Verify a slide transition stays at its start-of-transition positions during
 *           the delay period (no interpolation), and Cancel restores the original ones.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionDelayKeepsStartStateSlide_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SLIDE_LEFT);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(SHORT_DURATION);
    transition.SetDelay(TRANSITION_DELAY);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    // PrepareStart has placed the incoming view beside the outgoing view already
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X + VIEW_WIDTH);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y);

    // a few frames within the delay period: positions must not be interpolated
    TaskManager::GetInstance()->SetTaskRun(true);
    for (uint32_t i = 0; i < 5; i++) {
        usleep(TASK_WAIT_US);
        TaskManager::GetInstance()->TaskHandler();
    }
    TaskManager::GetInstance()->SetTaskRun(false);
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetX(), OUT_VIEW_X + VIEW_WIDTH);
    EXPECT_EQ(inView.GetY(), OUT_VIEW_Y);
    EXPECT_EQ(outView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(outView.GetY(), OUT_VIEW_Y);
    EXPECT_EQ(listener.completeCount_, 0);

    transition.Cancel();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(inView.GetX(), IN_VIEW_X);
    EXPECT_EQ(inView.GetY(), IN_VIEW_Y);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(outView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(outView.GetY(), OUT_VIEW_Y);
    EXPECT_TRUE(outView.IsVisible());
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}

/**
 * @tc.name: ViewTransitionSetEasingFunc_001
 * @tc.desc: Verify the transition completes normally with a non-linear easing function.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSetEasingFunc_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetEasingFunc(EasingEquation::QuadEaseInOut);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionSetEasingFuncCubicOut_001
 * @tc.desc: Verify the transition completes normally with the CubicEaseOut easing function,
 *           which is the parser mapping target of the "ease-out" timing function.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSetEasingFuncCubicOut_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetEasingFunc(EasingEquation::CubicEaseOut);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionOutgoingViewOnly_001
 * @tc.desc: Verify the fade transition runs and completes when only the outgoing view is set.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionOutgoingViewOnly_001, TestSize.Level1)
{
    UIView outView;
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetOpaScale(CUSTOM_OPA);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_TRUE(outView.IsVisible());

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(outView.GetOpaScale(), CUSTOM_OPA);
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionIncomingViewOnly_001
 * @tc.desc: Verify the fade transition runs and completes when only the incoming view is set.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionIncomingViewOnly_001, TestSize.Level1)
{
    UIView inView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetOpaScale(), 0);
    EXPECT_TRUE(inView.IsVisible());

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_TRUE(inView.IsVisible());
}

/**
 * @tc.name: ViewTransitionIncomingOnlySlide_001
 * @tc.desc: Verify a slide transition with only the incoming view does not move the incoming
 *           view to a bogus position when there is no outgoing reference snapshot.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionIncomingOnlySlide_001, TestSize.Level1)
{
    UIView inView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_SLIDE_LEFT);
    transition.SetIncomingView(&inView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(TRANSITION_DURATION);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetX(), IN_VIEW_X);
    EXPECT_EQ(inView.GetY(), IN_VIEW_Y);

    RunTransitionUntilStop(&transition);
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(inView.GetX(), IN_VIEW_X);
    EXPECT_EQ(inView.GetY(), IN_VIEW_Y);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_TRUE(inView.IsVisible());
}

/**
 * @tc.name: ViewTransitionSetDurationNegative_001
 * @tc.desc: Verify SetDuration with a negative value is clamped to 0 and the transition
 *           finishes synchronously, just like duration 0.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSetDurationNegative_001, TestSize.Level0)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetOpaScale(CUSTOM_OPA);
    inView.SetVisible(false);

    ViewTransition transition;
    TestTransitionListener listener;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetOnTransitionListener(&listener);
    transition.SetDuration(-1);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(listener.completeCount_, 1);
    EXPECT_EQ(inView.GetOpaScale(), OPA_OPAQUE);
    EXPECT_TRUE(inView.IsVisible());
    EXPECT_EQ(outView.GetOpaScale(), CUSTOM_OPA);
    EXPECT_FALSE(outView.IsVisible());
}

/**
 * @tc.name: ViewTransitionSetDurationClamped_001
 * @tc.desc: Verify SetDuration with a value exceeding the upper bound is clamped to
 *           MAX_TRANSITION_DURATION_MS and the transition starts normally.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSetDurationClamped_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    inView.SetVisible(false);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    ViewTransition transition;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetDuration(999999);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);

    // Run a few frames; the clamped duration is 10000ms, so it must still be running.
    TaskManager::GetInstance()->SetTaskRun(true);
    for (uint32_t i = 0; i < 5; i++) {
        usleep(TASK_WAIT_US);
        TaskManager::GetInstance()->TaskHandler();
    }
    EXPECT_EQ(transition.GetState(), Animator::START);

    transition.Stop();
    EXPECT_EQ(transition.GetState(), Animator::STOP);
    EXPECT_EQ(outView.GetX(), OUT_VIEW_X);
    EXPECT_EQ(outView.GetY(), OUT_VIEW_Y);
    EXPECT_TRUE(outView.IsVisible());
    EXPECT_EQ(inView.GetX(), IN_VIEW_X);
    EXPECT_EQ(inView.GetY(), IN_VIEW_Y);
    EXPECT_FALSE(inView.IsVisible());

    TaskManager::GetInstance()->SetTaskRun(false);
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}

/**
 * @tc.name: ViewTransitionSetDelayNegative_001
 * @tc.desc: Verify SetDelay with a negative value is clamped to 0 so the transition
 *           progresses immediately without a delay period.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSetDelayNegative_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    inView.SetVisible(false);

    ViewTransition transition;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetDuration(TRANSITION_DURATION);
    transition.SetDelay(-100);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetOpaScale(), 0);

    // Run a few frames; with no delay the incoming view should already be fading in.
    TaskManager::GetInstance()->SetTaskRun(true);
    for (uint32_t i = 0; i < 10; i++) {
        usleep(TASK_WAIT_US);
        TaskManager::GetInstance()->TaskHandler();
    }
    EXPECT_GT(inView.GetOpaScale(), 0);

    transition.Stop();
    TaskManager::GetInstance()->SetTaskRun(false);
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}

/**
 * @tc.name: ViewTransitionSetDelayClamped_001
 * @tc.desc: Verify SetDelay with a value exceeding the upper bound is clamped to
 *           MAX_TRANSITION_DELAY_MS and the transition stays in the delay state
 *           for the initial frames.
 * @tc.type: FUNC
 * @tc.require: issues712
 */
HWTEST_F(ViewTransitionTest, ViewTransitionSetDelayClamped_001, TestSize.Level1)
{
    UIView inView;
    UIView outView;
    inView.SetPosition(IN_VIEW_X, IN_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    outView.SetPosition(OUT_VIEW_X, OUT_VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
    inView.SetVisible(false);

    ViewTransition transition;
    transition.SetType(ViewTransition::TRANSITION_FADE);
    transition.SetIncomingView(&inView);
    transition.SetOutgoingView(&outView);
    transition.SetDuration(SHORT_DURATION);
    transition.SetDelay(999999);

    AnimatorManager::GetInstance()->Init();
    transition.Start();
    EXPECT_EQ(transition.GetState(), Animator::START);
    EXPECT_EQ(inView.GetOpaScale(), 0);

    // Run a few frames; the clamped delay is 10000ms, so the view must still be at start state.
    TaskManager::GetInstance()->SetTaskRun(true);
    for (uint32_t i = 0; i < 5; i++) {
        usleep(TASK_WAIT_US);
        TaskManager::GetInstance()->TaskHandler();
    }
    EXPECT_EQ(inView.GetOpaScale(), 0);

    transition.Stop();
    TaskManager::GetInstance()->SetTaskRun(false);
    TaskManager::GetInstance()->Remove(AnimatorManager::GetInstance());
}
} // namespace OHOS
