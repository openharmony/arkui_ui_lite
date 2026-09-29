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

#ifndef GRAPHIC_LITE_UI_TRANSITION_H
#define GRAPHIC_LITE_UI_TRANSITION_H

#include "animator/animator.h"
#include "animator/easing_equation.h"
#include "gfx_utils/rect.h"

namespace OHOS {

class ViewTransition final : public HeapBase {
public:
    /**
     * @brief Enumerates the transition animation types.
     *
     * @since 1.0
     * @version 1.0
     */
    enum Type : uint8_t {
        /** Fade transition: the outgoing view fades out and the incoming view fades in. */
        TRANSITION_FADE,
        /** Slide-left transition: the outgoing view slides leftwards out of the screen while the incoming
         *  view slides in from the right. */
        TRANSITION_SLIDE_LEFT,
        /** Slide-right transition: the outgoing view slides rightwards out of the screen while the incoming
         *  view slides in from the left. */
        TRANSITION_SLIDE_RIGHT,
        /** Slide-up transition: the outgoing view slides upwards out of the screen while the incoming view
         *  slides in from the bottom. */
        TRANSITION_SLIDE_UP,
        /** Slide-down transition: the outgoing view slides downwards out of the screen while the incoming
         *  view slides in from the top. */
        TRANSITION_SLIDE_DOWN,
        /** Scale transition: the outgoing view stays at full size and fades out while the incoming view
         *  scales up and fades in. */
        TRANSITION_SCALE,
        /** Shared-element transition: a shared element animates from its start rect to its end rect while the
         *  outgoing and incoming views use a cover-fade. */
        TRANSITION_SHARED_ELEMENT,
    };

    /**
     * @brief A default constructor used to create a <b>ViewTransition</b> instance.
     *
     * The default transition type is <b>TRANSITION_FADE</b> and the easing function is linear.
     *
     * @since 1.0
     * @version 1.0
     */
    ViewTransition();

    /**
     * @brief A destructor used to delete the <b>ViewTransition</b> instance.
     *
     * Call {@link Stop}, {@link Cancel}, or {@link InvalidateTargets} before destruction.
     *
     * @since 1.0
     * @version 1.0
     */
    ~ViewTransition() {};

    ViewTransition(const ViewTransition&) = delete;
    ViewTransition& operator=(const ViewTransition&) = delete;

    /**
     * @brief Sets the outgoing view, which is the view leaving the screen during the transition.
     *
     * The transition does not own the view. The caller must ensure the view outlives this transition.
     * Before destroying the view, call {@link InvalidateTargets} to clear the internal reference.
     *
     * @param view Indicates the pointer to the outgoing <b>UIView</b>.
     * @see SetIncomingView
     * @since 1.0
     * @version 1.0
     */
    void SetOutgoingView(UIView* view)
    {
        outView_ = view;
    }

    /**
     * @brief Sets the incoming view, which is the view entering the screen during the transition.
     *
     * The transition does not own the view. The caller must ensure the view outlives this transition.
     * Before destroying the view, call {@link InvalidateTargets} to clear the internal reference.
     *
     * @param view Indicates the pointer to the incoming <b>UIView</b>.
     * @see SetOutgoingView
     * @since 1.0
     * @version 1.0
     */
    void SetIncomingView(UIView* view)
    {
        inView_ = view;
    }

    /**
     * @brief Sets the transition animation type.
     *
     * @param type Indicates the transition type. For details, see {@link Type}.
     * @see Type
     * @since 1.0
     * @version 1.0
     */
    void SetType(Type type)
    {
        type_ = type;
    }

    /**
     * @brief Sets the animation duration of this transition.
     *
     * The actual Animator period is duration + delay. The transition progress is computed only
     * after the delay has elapsed.
     *
     * @param ms Indicates the animation duration, in milliseconds.
     * @since 1.0
     * @version 1.0
     */
    void SetDuration(int32_t ms);

    /**
     * @brief Sets the delay before this transition starts.
     *
     * During the delay period the views remain in their start-of-transition states. The Animator
     * period is extended by this amount.
     *
     * @param ms Indicates the delay, in milliseconds.
     * @since 1.0
     * @version 1.0
     */
    void SetDelay(int32_t ms);

    /**
     * @brief Sets the easing function used to interpolate the transition progress.
     *
     * @param func Indicates the easing function. For details, see {@link EasingFunc}.
     * @since 1.0
     * @version 1.0
     */
    void SetEasingFunc(EasingFunc func)
    {
        easing_ = (func != nullptr) ? func : EasingEquation::LinearEaseNone;
    }

    /**
     * @brief Sets the shared element for the shared-element transition.
     *
     * The shared element animates from the start rect (see {@link SetSharedStartRect}) to the end rect
     * (see {@link SetSharedEndRect}) during the transition.
     *
     * The transition does not own the shared element. The caller must ensure it outlives this
     * transition. Before destroying the shared element, call {@link InvalidateTargets} to clear the
     * internal reference.
     *
     * @param element Indicates the pointer to the shared <b>UIView</b>.
     * @since 1.0
     * @version 1.0
     */
    void SetSharedElement(UIView* element)
    {
        sharedElement_ = element;
    }

    /**
     * @brief Sets the start rectangle of the shared element, in the coordinate system of the shared
     *        element's parent (same as UIView::SetPosition).
     *
     * @param x Indicates the x-coordinate of the start rectangle.
     * @param y Indicates the y-coordinate of the start rectangle.
     * @param w Indicates the width of the start rectangle.
     * @param h Indicates the height of the start rectangle.
     * @see SetSharedEndRect
     * @since 1.0
     * @version 1.0
     */
    void SetSharedStartRect(int16_t x, int16_t y, int16_t w, int16_t h)
    {
        sharedStartRect_.SetPosition(x, y);
        sharedStartRect_.Resize(w, h);
    }

    /**
     * @brief Sets the end rectangle of the shared element, in the coordinate system of the shared
     *        element's parent (same as UIView::SetPosition).
     *
     * @param x Indicates the x-coordinate of the end rectangle.
     * @param y Indicates the y-coordinate of the end rectangle.
     * @param w Indicates the width of the end rectangle.
     * @param h Indicates the height of the end rectangle.
     * @see SetSharedStartRect
     * @since 1.0
     * @version 1.0
     */
    void SetSharedEndRect(int16_t x, int16_t y, int16_t w, int16_t h)
    {
        sharedEndRect_.SetPosition(x, y);
        sharedEndRect_.Resize(w, h);
    }

    /**
     * @brief Starts this transition.
     *
     * @see Stop
     * @since 1.0
     * @version 1.0
     */
    void Start();

    /**
     * @brief Stops and cancels this transition, restoring all views to their snapshot states.
     *
     * This call is equivalent to Cancel(). The transition completion listener is NOT invoked.
     * If the transition has not started or has already finished, this call does nothing.
     *
     * @see Start
     * @since 1.0
     * @version 1.0
     */
    void Stop();

    /**
     * @brief Cancels this transition and restores all the views to their snapshot states.
     *
     * If the transition has not started or has already finished, this call does nothing.
     *
     * @see Start
     * @since 1.0
     * @version 1.0
     */
    void Cancel();

    /**
     * @brief Cancels this transition and clears all the view references held by it.
     *
     * The transition is cancelled first so that any running animator is stopped and the views are
     * restored to their snapshot states. After this call, the outgoing view, incoming view and
     * shared element references are all set to <b>nullptr</b>.
     *
     * @since 1.0
     * @version 1.0
     */
    void InvalidateTargets()
    {
        Cancel();  // stop the animator and restore snapshots before clearing references
        outView_ = nullptr;
        inView_ = nullptr;
        sharedElement_ = nullptr;
    }

    /**
     * @brief Obtains the current state of this transition.
     *
     * @return Returns the current animator state. For details, see {@link Animator::GetState}.
     * @since 1.0
     * @version 1.0
     */
    uint8_t GetState() const
    {
        return animator_.GetState();
    }

    /**
     * @brief Represents the listener that receives transition completion notifications.
     *
     * Override {@link OnTransitionComplete} to perform custom logic when a transition finishes.
     *
     * @since 1.0
     * @version 1.0
     */
    class OnTransitionListener : public HeapBase {
    public:
        /**
         * @brief A default destructor.
         *
         * @since 1.0
         * @version 1.0
         */
        virtual ~OnTransitionListener() {}

        /**
         * @brief Called when this transition completes.
         *
         * The listener is invoked synchronously from the animator callback path. It must not delete
         * the provided <b>ViewTransition</b> instance or any of its target views.
         *
         * @param transition Indicates the <b>ViewTransition</b> instance that has finished.
         * @since 1.0
         * @version 1.0
         */
        virtual void OnTransitionComplete(ViewTransition& transition) {}
    };

    /**
     * @brief Sets the listener that is notified when this transition completes.
     *
     * The listener is owned by the caller; this transition does not delete it. The listener must
     * not delete this transition or its target views inside {@link OnTransitionComplete}.
     *
     * @param listener Indicates the pointer to the {@link OnTransitionListener}.
     * @since 1.0
     * @version 1.0
     */
    void SetOnTransitionListener(OnTransitionListener* listener)
    {
        listener_ = listener;
    }

private:
    class TransitionCallback : public AnimatorCallback {
    public:
        void Callback(UIView* view) override;
        ViewTransition* owner_ = nullptr;
    };

    // snapshot of a view's position/opacity/visibility, taken at transition start
    // and restored on cancel
    struct ViewSnapshot {
        int16_t x = 0;
        int16_t y = 0;
        uint8_t opa = OPA_OPAQUE;
        bool visible = true;
    };

    void DoCallback();
    void PrepareStart();
    void FinishTransition();
    void RestoreSnapshot();
    void HandleSlide(float t);
    void HandleScale(float t);
    void HandleSharedElement(float t);

    // helpers split out of PrepareStart / FinishTransition / DoCallback to keep them small
    void SaveStartState();
    void SetupSlideStartPosition();
    void SetupSharedElement();
    void ScaleSharedElementToRect(const Rect& rect);
    void FinishView(UIView *view, uint8_t opa, bool visible);
    void FinishSharedElement();
    float GetEasedProgress(uint32_t actualTime);
    void DispatchTransitionFrame(float t);

    void ApplyCrossFade(float t);
    void ApplyCoverFadeOpacity(float t);
    void SaveViewSnapshot(UIView *view, ViewSnapshot &snapshot);
    void RestoreViewSnapshot(UIView *view, const ViewSnapshot &snapshot);
    void ResetViewScaleToIdentity(UIView *view);

    TransitionCallback callback_;
    Animator animator_;
    OnTransitionListener* listener_;

    UIView* outView_;
    UIView* inView_;
    Type type_;
    EasingFunc easing_;

    uint32_t duration_;   // real animation duration (ms), excluding delay
    uint32_t delay_;      // delay before animation progress starts (ms)

    UIView* sharedElement_;
    Rect sharedStartRect_;
    Rect sharedEndRect_;

    ViewSnapshot outSnapshot_;
    ViewSnapshot inSnapshot_;
    ViewSnapshot sharedSnapshot_;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_UI_TRANSITION_H
