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

namespace OHOS {
static constexpr float OPA_OPAQUE_F = static_cast<float>(OPA_OPAQUE);
static constexpr float SCALE_SHRINK = 0.3f;
static constexpr float SCALE_MIN = 0.7f;

// Defense-in-depth: cap duration/delay at a safe upper bound.
// This protects direct C++ callers of ViewTransition::SetDuration/SetDelay and prevents
// uint32 overflow in animator_.SetTime(duration_ + delay_).
static constexpr uint32_t MAX_TRANSITION_DURATION_MS = 10000;
static constexpr uint32_t MAX_TRANSITION_DELAY_MS = 10000;

static uint8_t ClampOpacity(float value)
{
    if (value < 0.0f) {
        return 0;
    }
    if (value > OPA_OPAQUE_F) {
        return OPA_OPAQUE;
    }
    return static_cast<uint8_t>(value);
}

static int16_t ClampToInt16(int32_t value)
{
    if (value > INT16_MAX) {
        return INT16_MAX;
    }
    if (value < INT16_MIN) {
        return INT16_MIN;
    }
    return static_cast<int16_t>(value);
}

ViewTransition::ViewTransition()
    : animator_(&callback_, nullptr, 0, false),
      listener_(nullptr),
      outView_(nullptr),
      inView_(nullptr),
      type_(TRANSITION_FADE),
      easing_(EasingEquation::LinearEaseNone),
      duration_(0),
      delay_(0),
      sharedElement_(nullptr),
      sharedStartRect_(),
      sharedEndRect_(),
      outSnapshot_(),
      inSnapshot_(),
      sharedSnapshot_()
{
    callback_.owner_ = this;
}

void ViewTransition::SetDuration(int32_t ms)
{
    if (ms < 0) {
        duration_ = 0;
    } else if (static_cast<uint32_t>(ms) > MAX_TRANSITION_DURATION_MS) {
        duration_ = MAX_TRANSITION_DURATION_MS;
    } else {
        duration_ = static_cast<uint32_t>(ms);
    }
    animator_.SetTime(duration_ + delay_);
}

void ViewTransition::SetDelay(int32_t ms)
{
    if (ms < 0) {
        delay_ = 0;
    } else if (static_cast<uint32_t>(ms) > MAX_TRANSITION_DELAY_MS) {
        delay_ = MAX_TRANSITION_DELAY_MS;
    } else {
        delay_ = static_cast<uint32_t>(ms);
    }
    animator_.SetTime(duration_ + delay_);
}

void ViewTransition::Start()
{
    if (inView_ == nullptr && outView_ == nullptr) {
        return;
    }
    PrepareStart();
    if (animator_.GetTime() == 0) {
        // period (duration + delay) is 0: jump to the final state directly instead of starting the animator
        FinishTransition();
        return;
    }
    animator_.SetRunTime(0);
    animator_.Start();
}

void ViewTransition::Stop()
{
    Cancel();
}

void ViewTransition::Cancel()
{
    if (animator_.GetState() == Animator::STOP) {
        return;
    }
    animator_.Cancel();
    RestoreSnapshot();
}

void ViewTransition::PrepareStart()
{
    SaveStartState();

    switch (type_) {
        case TRANSITION_FADE:
            if (inView_ != nullptr) {
                inView_->SetOpaScale(0);
            }
            break;
        case TRANSITION_SCALE:
            if (inView_ != nullptr) {
                inView_->SetOpaScale(0);
                int16_t w = inView_->GetWidth();
                int16_t h = inView_->GetHeight();
                if (w > 0 && h > 0) {
                    inView_->Scale(Vector2<float>(SCALE_MIN, SCALE_MIN),
                                   Vector2<float>(w / 2.0f, h / 2.0f));
                }
            }
            if (outView_ != nullptr) {
                int16_t w = outView_->GetWidth();
                int16_t h = outView_->GetHeight();
                if (w > 0 && h > 0) {
                    outView_->Scale(Vector2<float>(1.0f, 1.0f),
                                    Vector2<float>(w / 2.0f, h / 2.0f));
                }
            }
            break;
        case TRANSITION_SLIDE_LEFT:
        case TRANSITION_SLIDE_RIGHT:
        case TRANSITION_SLIDE_UP:
        case TRANSITION_SLIDE_DOWN:
            SetupSlideStartPosition();
            break;
        case TRANSITION_SHARED_ELEMENT:
            if (inView_ != nullptr) {
                inView_->SetOpaScale(0);
            }
            SetupSharedElement();
            break;
        default:
            break;
    }
}

void ViewTransition::SaveStartState()
{
    SaveViewSnapshot(outView_, outSnapshot_);
    SaveViewSnapshot(inView_, inSnapshot_);
}

void ViewTransition::SaveViewSnapshot(UIView *view, ViewSnapshot &snapshot)
{
    if (view == nullptr) {
        return;
    }
    snapshot.x = view->GetX();
    snapshot.y = view->GetY();
    snapshot.opa = view->GetOpaScale();
    snapshot.visible = view->IsVisible();
    // Transition assumes the view is visible while running.
    // snapshot.visible records the original state; RestoreViewSnapshot is responsible for restoring it.
    view->SetVisible(true);
}

void ViewTransition::SetupSlideStartPosition()
{
    if (inView_ == nullptr || outView_ == nullptr) {
        return;
    }
    // slide transitions: the incoming view starts beside the outgoing view in the slide direction
    // compute in int32_t and clamp to int16_t range: a view positioned near the int16_t
    // limit would otherwise wrap the incoming view to the opposite side
    switch (type_) {
        case TRANSITION_SLIDE_LEFT:
            inView_->SetPosition(
                ClampToInt16(static_cast<int32_t>(outSnapshot_.x) + outView_->GetWidth()),
                outSnapshot_.y);
            break;
        case TRANSITION_SLIDE_RIGHT:
            inView_->SetPosition(
                ClampToInt16(static_cast<int32_t>(outSnapshot_.x) - outView_->GetWidth()),
                outSnapshot_.y);
            break;
        case TRANSITION_SLIDE_UP:
            inView_->SetPosition(
                outSnapshot_.x,
                ClampToInt16(static_cast<int32_t>(outSnapshot_.y) + outView_->GetHeight()));
            break;
        case TRANSITION_SLIDE_DOWN:
            inView_->SetPosition(
                outSnapshot_.x,
                ClampToInt16(static_cast<int32_t>(outSnapshot_.y) - outView_->GetHeight()));
            break;
        default:
            break;
    }
}

void ViewTransition::SetupSharedElement()
{
    if (sharedElement_ == nullptr) {
        return;
    }
    SaveViewSnapshot(sharedElement_, sharedSnapshot_);
    sharedElement_->SetPosition(sharedStartRect_.GetX(), sharedStartRect_.GetY());
    sharedElement_->SetOpaScale(OPA_OPAQUE);
    ScaleSharedElementToRect(sharedStartRect_);
}

void ViewTransition::ScaleSharedElementToRect(const Rect& rect)
{
    if (sharedElement_ == nullptr) {
        return;
    }
    int16_t origW = sharedElement_->GetWidth();
    int16_t origH = sharedElement_->GetHeight();
    if (origW > 0 && origH > 0) {
        float sx = static_cast<float>(rect.GetWidth()) / origW;
        float sy = static_cast<float>(rect.GetHeight()) / origH;
        sharedElement_->Scale(Vector2<float>(sx, sy),
                              Vector2<float>(origW / 2.0f, origH / 2.0f));
    }
}

void ViewTransition::FinishTransition()
{
    FinishView(outView_, outSnapshot_.opa, false);
    FinishView(inView_, OPA_OPAQUE, true);
    FinishSharedElement();
    if (listener_ != nullptr) {
        // The listener is caller-owned and is notified synchronously. It must not delete this
        // ViewTransition or its target views; doing so would cause use-after-free because
        // Animator::Run/Stop still hold references to this object after this call returns.
        listener_->OnTransitionComplete(*this);
    }
}

void ViewTransition::FinishView(UIView *view, uint8_t opa, bool visible)
{
    if (view == nullptr) {
        return;
    }
    // slide transitions: move the view to its final position (the outgoing start position).
    // This is only meaningful when there is an outgoing view; without one the view never
    // participated in slide positioning and must not be moved.
    if (outView_ != nullptr) {
        switch (type_) {
            case TRANSITION_SLIDE_LEFT:
            case TRANSITION_SLIDE_RIGHT:
                view->SetX(outSnapshot_.x);
                break;
            case TRANSITION_SLIDE_UP:
            case TRANSITION_SLIDE_DOWN:
                view->SetY(outSnapshot_.y);
                break;
            default:
                break;
        }
    }
    view->SetOpaScale(opa);
    view->SetVisible(visible);
    // reset scale transform to identity to clear any scale applied by this transition
    ResetViewScaleToIdentity(view);
    view->Invalidate();
}

void ViewTransition::FinishSharedElement()
{
    if (sharedElement_ == nullptr) {
        return;
    }
    sharedElement_->SetPosition(sharedEndRect_.GetX(), sharedEndRect_.GetY());
    sharedElement_->SetVisible(true);
    sharedElement_->SetOpaScale(OPA_OPAQUE);
    ScaleSharedElementToRect(sharedEndRect_);
    sharedElement_->Invalidate();
}

void ViewTransition::RestoreSnapshot()
{
    RestoreViewSnapshot(outView_, outSnapshot_);
    RestoreViewSnapshot(inView_, inSnapshot_);
    RestoreViewSnapshot(sharedElement_, sharedSnapshot_);
}

void ViewTransition::RestoreViewSnapshot(UIView *view, const ViewSnapshot &snapshot)
{
    if (view == nullptr) {
        return;
    }
    view->SetPosition(snapshot.x, snapshot.y);
    view->SetOpaScale(snapshot.opa);
    view->SetVisible(snapshot.visible);
    ResetViewScaleToIdentity(view);
    view->Invalidate();
}

void ViewTransition::ResetViewScaleToIdentity(UIView *view)
{
    int16_t w = view->GetWidth();
    int16_t h = view->GetHeight();
    if (w > 0 && h > 0) {
        view->Scale(Vector2<float>(1.0f, 1.0f),
                    Vector2<float>(w / 2.0f, h / 2.0f));
    }
}

void ViewTransition::TransitionCallback::Callback(UIView* view)
{
    (void)view;
    if (owner_ == nullptr) {
        return;
    }
    owner_->DoCallback();
}

void ViewTransition::ApplyCoverFadeOpacity(float t)
{
    // Use a cover-fade curve instead of a standard cross-dissolve. Keeping one of
    // the two views fully opaque at any moment prevents the dark stage background
    // from showing through two semi-transparent panels, which otherwise produces a
    // black/dark intermediate frame during fade transitions.
    uint8_t outOpa = (t < 0.5f) ? OPA_OPAQUE : ClampOpacity(OPA_OPAQUE_F * 2.0f * (1.0f - t));
    uint8_t inOpa  = (t < 0.5f) ? ClampOpacity(OPA_OPAQUE_F * 2.0f * t) : OPA_OPAQUE;
    if (outView_ != nullptr) {
        outView_->SetOpaScale(outOpa);
    }
    if (inView_ != nullptr) {
        inView_->SetOpaScale(inOpa);
    }
}

void ViewTransition::ApplyCrossFade(float t)
{
    ApplyCoverFadeOpacity(t);
}

void ViewTransition::HandleSlide(float t)
{
    if (outView_ == nullptr) {
        return;
    }
    bool horizontal = (type_ == TRANSITION_SLIDE_LEFT || type_ == TRANSITION_SLIDE_RIGHT);
    // LEFT/UP move the outgoing view towards the negative axis, RIGHT/DOWN towards positive
    int16_t sign = (type_ == TRANSITION_SLIDE_LEFT || type_ == TRANSITION_SLIDE_UP) ? -1 : 1;
    int16_t size = horizontal ? outView_->GetWidth() : outView_->GetHeight();
    // compute in int32_t and clamp to int16_t range to avoid overflow when an overshoot
    // easing drives t outside [0, 1] or the view size approaches the int16_t limit
    int32_t offset = static_cast<int32_t>(static_cast<float>(size) * t);
    if (horizontal) {
        int32_t outX = static_cast<int32_t>(outSnapshot_.x) + sign * offset;
        outView_->SetX(ClampToInt16(outX));
    } else {
        int32_t outY = static_cast<int32_t>(outSnapshot_.y) + sign * offset;
        outView_->SetY(ClampToInt16(outY));
    }
    if (inView_ == nullptr) {
        return;
    }
    if (horizontal) {
        int32_t inX = static_cast<int32_t>(outSnapshot_.x) -
                      sign * (static_cast<int32_t>(size) - offset);
        inView_->SetX(ClampToInt16(inX));
    } else {
        int32_t inY = static_cast<int32_t>(outSnapshot_.y) -
                      sign * (static_cast<int32_t>(size) - offset);
        inView_->SetY(ClampToInt16(inY));
    }
}

void ViewTransition::HandleScale(float t)
{
    float inScale = SCALE_MIN + t * SCALE_SHRINK;

    // Use a cover-fade for the two panels: keep one of them fully opaque at all
    // times. The outgoing view stays at full size and acts as the background,
    // while the incoming view scales up on top of it. This avoids the dark
    // mixed-color rectangle that a standard cross-dissolve produces when the
    // semi-transparent incoming view is blended over a fading outgoing view and
    // the dark stage background.
    ApplyCoverFadeOpacity(t);
    if (inView_ != nullptr) {
        float cx = inView_->GetWidth() / 2.0f;
        float cy = inView_->GetHeight() / 2.0f;
        inView_->Scale(Vector2<float>(inScale, inScale),
                       Vector2<float>(cx, cy));
    }
}

void ViewTransition::HandleSharedElement(float t)
{
    // Use a cover-fade for the two panels: keep one of them fully opaque at all
    // times so the stage background is never exposed through two semi-transparent
    // views. This avoids the dark/muddy mixed-color middle frame that a standard
    // cross-dissolve produces over a dark stage background.
    ApplyCoverFadeOpacity(t);
    if (sharedElement_ == nullptr) {
        return;
    }
    float dx = (sharedEndRect_.GetX() - sharedStartRect_.GetX()) * t;
    float dy = (sharedEndRect_.GetY() - sharedStartRect_.GetY()) * t;
    int32_t x = static_cast<int32_t>(sharedStartRect_.GetX()) + static_cast<int32_t>(dx);
    int32_t y = static_cast<int32_t>(sharedStartRect_.GetY()) + static_cast<int32_t>(dy);
    sharedElement_->SetPosition(ClampToInt16(x), ClampToInt16(y));

    int16_t origW = sharedElement_->GetWidth();
    int16_t origH = sharedElement_->GetHeight();
    if (origW > 0 && origH > 0) {
        float sxStart = static_cast<float>(sharedStartRect_.GetWidth()) / origW;
        float syStart = static_cast<float>(sharedStartRect_.GetHeight()) / origH;
        float sxEnd = static_cast<float>(sharedEndRect_.GetWidth()) / origW;
        float syEnd = static_cast<float>(sharedEndRect_.GetHeight()) / origH;
        float sX = sxStart + (sxEnd - sxStart) * t;
        float sY = syStart + (syEnd - syStart) * t;
        float cx = origW / 2.0f;
        float cy = origH / 2.0f;
        sharedElement_->Scale(Vector2<float>(sX, sY),
                              Vector2<float>(cx, cy));
    }
}

void ViewTransition::DoCallback()
{
    if (outView_ == nullptr && inView_ == nullptr) {
        return;
    }
    uint32_t runTime = animator_.GetRunTime();
    // During the delay period keep the start-of-transition state; do not interpolate.
    // The boundary frame (runTime == delay_) belongs to the animation/finish phase,
    // so it must not return here (this also handles duration_ == 0 correctly).
    if (runTime < delay_) {
        return;
    }

    uint32_t actualTime = runTime - delay_;

    // finish on the last frame; keep this check before the easing computation
    // so the normalized easing input can never overflow uint16_t
    if (actualTime >= duration_) {
        FinishTransition();
        return;
    }

    DispatchTransitionFrame(GetEasedProgress(actualTime));

    if (outView_ != nullptr) {
        outView_->Invalidate();
    }
    if (inView_ != nullptr) {
        inView_->Invalidate();
    }
    if (sharedElement_ != nullptr) {
        sharedElement_->Invalidate();
    }
}

float ViewTransition::GetEasedProgress(uint32_t actualTime)
{
    static constexpr int16_t PROGRESS_RANGE = 1024;
    static constexpr uint16_t EASING_DURATION = 1024;
    // normalize the actual running time (excluding delay) to [0, 1024]: the easing
    // functions only depend on the curTime/durationTime ratio, so scaling is behavior-preserving
    uint16_t easedCur = static_cast<uint16_t>(
        (static_cast<uint64_t>(actualTime) * EASING_DURATION) / duration_);
    int16_t easedProgress = easing_(0, PROGRESS_RANGE, easedCur, EASING_DURATION);
    return static_cast<float>(easedProgress) / PROGRESS_RANGE;
}

void ViewTransition::DispatchTransitionFrame(float t)
{
    switch (type_) {
        case TRANSITION_FADE:
            ApplyCrossFade(t);
            break;
        case TRANSITION_SLIDE_LEFT:
        case TRANSITION_SLIDE_RIGHT:
        case TRANSITION_SLIDE_UP:
        case TRANSITION_SLIDE_DOWN:
            HandleSlide(t);
            break;
        case TRANSITION_SCALE:
            HandleScale(t);
            break;
        case TRANSITION_SHARED_ELEMENT:
            HandleSharedElement(t);
            break;
        default:
            break;
    }
}
} // namespace OHOS
