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

#include "animator/transition_animator_callback.h"
#include "graphic_config.h"
#include "hal_tick.h"

namespace OHOS {

namespace {
uint8_t ClampColorComponent(int16_t value)
{
    if (value < 0) {
        return 0;
    }
    return (value > UINT8_MAX) ? UINT8_MAX : static_cast<uint8_t>(value);
}

/* effect masks: kept in the .cpp because they are private implementation details */
constexpr uint8_t EFFECT_OPACITY = 0x01;
constexpr uint8_t EFFECT_SCALE = 0x02;
constexpr uint8_t EFFECT_ROTATION = 0x04;
constexpr uint8_t EFFECT_COLOR = 0x08;
constexpr uint8_t EFFECT_POSITION = 0x10;
constexpr int16_t SCALE_CONVERSION = 256;
} // namespace

TransitionAnimatorCallback::TransitionAnimatorCallback()
    : easing_(EasingEquation::LinearEaseNone), duration_(0), delay_(0), startTime_(0), effectMask_(0), opaFrom_(0),
      opaTo_(0), scaleFromX_(SCALE_CONVERSION), scaleFromY_(SCALE_CONVERSION),
      scaleToX_(SCALE_CONVERSION), scaleToY_(SCALE_CONVERSION), scalePivotX_(-1.0f), scalePivotY_(-1.0f),
      rotateFrom_(0), rotateTo_(0), rotatePivotX_(-1.0f), rotatePivotY_(-1.0f), posFromX_(0), posFromY_(0),
      posToX_(0), posToY_(0)
{
    colorFrom_.full = 0;
    colorTo_.full = 0;
}

void TransitionAnimatorCallback::Reset()
{
    easing_ = EasingEquation::LinearEaseNone;
    duration_ = 0;
    delay_ = 0;
    startTime_ = 0;
    effectMask_ = 0;
    next_ = nullptr;
    nextSequential_ = false;
}

void TransitionAnimatorCallback::SetDuration(uint32_t duration)
{
    duration_ = duration;
    startTime_ = 0;
}

void TransitionAnimatorCallback::SetDelay(uint32_t delay)
{
    delay_ = delay;
    startTime_ = 0;
}

void TransitionAnimatorCallback::SetEasingFunc(EasingFunc easing)
{
    easing_ = (easing != nullptr) ? easing : EasingEquation::LinearEaseNone;
}

void TransitionAnimatorCallback::SetOpacity(uint8_t from, uint8_t to)
{
    opaFrom_ = from;
    opaTo_ = to;
    effectMask_ |= EFFECT_OPACITY;
}

void TransitionAnimatorCallback::SetScale(float fromX, float fromY, float toX, float toY, float pivotX, float pivotY)
{
    scaleFromX_ = static_cast<int16_t>(fromX * SCALE_CONVERSION);
    scaleFromY_ = static_cast<int16_t>(fromY * SCALE_CONVERSION);
    scaleToX_ = static_cast<int16_t>(toX * SCALE_CONVERSION);
    scaleToY_ = static_cast<int16_t>(toY * SCALE_CONVERSION);
    scalePivotX_ = pivotX;
    scalePivotY_ = pivotY;
    effectMask_ |= EFFECT_SCALE;
}

void TransitionAnimatorCallback::SetRotation(int16_t fromAngle, int16_t toAngle, float pivotX, float pivotY)
{
    rotateFrom_ = fromAngle;
    rotateTo_ = toAngle;
    rotatePivotX_ = pivotX;
    rotatePivotY_ = pivotY;
    effectMask_ |= EFFECT_ROTATION;
}

void TransitionAnimatorCallback::SetColor(ColorType from, ColorType to)
{
    colorFrom_ = from;
    colorTo_ = to;
    effectMask_ |= EFFECT_COLOR;
}

void TransitionAnimatorCallback::SetPosition(int16_t fromX, int16_t fromY, int16_t toX, int16_t toY)
{
    posFromX_ = fromX;
    posFromY_ = fromY;
    posToX_ = toX;
    posToY_ = toY;
    effectMask_ |= EFFECT_POSITION;
}

void TransitionAnimatorCallback::AddTransitionAnimatorCallback(TransitionAnimatorCallback* next, bool sequential)
{
    next_ = next;
    nextSequential_ = sequential;
}

uint32_t TransitionAnimatorCallback::GetTotalTime() const
{
    uint32_t currentTime = duration_ + delay_;
    if (next_ == nullptr) {
        return currentTime;
    }
    uint32_t nextTime = next_->GetTotalTime();
    if (nextSequential_) {
        return currentTime + nextTime;
    }
    return (currentTime > nextTime) ? currentTime : nextTime;
}

void TransitionAnimatorCallback::Callback(UIView* view)
{
    if ((view == nullptr) || (duration_ == 0)) {
        return;
    }
    // use HALTick timestamp to calculate actual elapsed time
    // keeping animation duration accurate under frame-rate fluctuation
    uint32_t now = HALTick::GetInstance().GetTime();
    if (startTime_ == 0) {
        startTime_ = now;  // first frame records the start timestamp
    }
    uint32_t currentTime = now - startTime_;  // actual elapsed time
    ApplyFrame(view, currentTime, duration_);
}

void TransitionAnimatorCallback::ApplyFrame(UIView* view, uint32_t elapsedTime, uint32_t durationTime)
{
    if ((next_ != nullptr) || (delay_ != 0)) {
        ApplyChainFrame(view, elapsedTime);
        return;
    }
    ApplyCurrentFrame(view, elapsedTime, durationTime);
}

void TransitionAnimatorCallback::ApplyChainFrame(UIView* view, uint32_t timelineTime)
{
    if (view == nullptr) {
        return;
    }
    if (timelineTime >= delay_) {
        ApplyCurrentFrame(view, timelineTime - delay_, duration_);
    }
    if (next_ != nullptr) {
        if (nextSequential_) {
            uint32_t currentTime = delay_ + duration_;
            if (timelineTime < currentTime) {
                return;
            }
            next_->ApplyChainFrame(view, timelineTime - currentTime);
        } else {
            next_->ApplyChainFrame(view, timelineTime);
        }
    }
}

void TransitionAnimatorCallback::ApplyCurrentFrame(UIView* view, uint32_t elapsedTime, uint32_t durationTime)
{
    if ((view == nullptr) || (durationTime == 0)) {
        return;
    }
    uint16_t actualTime = static_cast<uint16_t>((elapsedTime > UINT16_MAX) ? UINT16_MAX : elapsedTime);
    uint16_t totalTime = static_cast<uint16_t>((durationTime > UINT16_MAX) ? UINT16_MAX : durationTime);
    if (actualTime > totalTime) {
        actualTime = totalTime;
    }
    if (effectMask_ & EFFECT_OPACITY) {
        ApplyOpacity(view, actualTime, totalTime);
    }
    if (effectMask_ & EFFECT_SCALE) {
        ApplyScale(view, actualTime, totalTime);
    }
    if (effectMask_ & EFFECT_ROTATION) {
        ApplyRotation(view, actualTime, totalTime);
    }
    if (effectMask_ & EFFECT_COLOR) {
        ApplyColor(view, actualTime, totalTime);
    }
    if (effectMask_ & EFFECT_POSITION) {
        ApplyPosition(view, actualTime, totalTime);
    }
    view->Invalidate();
}

void TransitionAnimatorCallback::ApplyOpacity(UIView* view, uint16_t actualTime, uint16_t durationTime)
{
    int16_t value = easing_(static_cast<int16_t>(opaFrom_), static_cast<int16_t>(opaTo_), actualTime, durationTime);
    if (value < 0) {
        value = 0;
    } else if (value > OPA_OPAQUE) {
        value = OPA_OPAQUE;
    }
    view->SetOpaScale(static_cast<uint8_t>(value));
}

void TransitionAnimatorCallback::ApplyScale(UIView* view, uint16_t actualTime, uint16_t durationTime)
{
    int16_t sx = easing_(scaleFromX_, scaleToX_, actualTime, durationTime);
    int16_t sy = easing_(scaleFromY_, scaleToY_, actualTime, durationTime);
    float pivotX = (scalePivotX_ < 0.0f) ? (view->GetWidth() / 2.0f) : scalePivotX_;
    float pivotY = (scalePivotY_ < 0.0f) ? (view->GetHeight() / 2.0f) : scalePivotY_;
    view->Scale(Vector2<float>(static_cast<float>(sx) / SCALE_CONVERSION, static_cast<float>(sy) / SCALE_CONVERSION),
                Vector2<float>(pivotX, pivotY));
}

void TransitionAnimatorCallback::ApplyRotation(UIView* view, uint16_t actualTime, uint16_t durationTime)
{
    int16_t angle = easing_(rotateFrom_, rotateTo_, actualTime, durationTime);
    float pivotX = (rotatePivotX_ < 0.0f) ? (view->GetWidth() / 2.0f) : rotatePivotX_;
    float pivotY = (rotatePivotY_ < 0.0f) ? (view->GetHeight() / 2.0f) : rotatePivotY_;
    view->Rotate(angle, Vector2<float>(pivotX, pivotY));
}

void TransitionAnimatorCallback::ApplyColor(UIView* view, uint16_t actualTime, uint16_t durationTime)
{
#if defined(COLOR_DEPTH) && COLOR_DEPTH == 32
    int16_t r = easing_(static_cast<int16_t>(colorFrom_.red), static_cast<int16_t>(colorTo_.red),
                        actualTime, durationTime);
    int16_t g = easing_(static_cast<int16_t>(colorFrom_.green), static_cast<int16_t>(colorTo_.green),
                        actualTime, durationTime);
    int16_t b = easing_(static_cast<int16_t>(colorFrom_.blue), static_cast<int16_t>(colorTo_.blue),
                        actualTime, durationTime);
    int16_t a = easing_(static_cast<int16_t>(colorFrom_.alpha), static_cast<int16_t>(colorTo_.alpha),
                        actualTime, durationTime);
    ColorType cur = Color::GetColorFromRGBA(ClampColorComponent(r), ClampColorComponent(g),
                                            ClampColorComponent(b), ClampColorComponent(a));
    view->SetStyle(STYLE_BACKGROUND_COLOR, cur.full);
#else
    int16_t rFrom = static_cast<int16_t>(colorFrom_.red << 3);
    int16_t gFrom = static_cast<int16_t>(colorFrom_.green << 2);
    int16_t bFrom = static_cast<int16_t>(colorFrom_.blue << 3);
    int16_t rTo = static_cast<int16_t>(colorTo_.red << 3);
    int16_t gTo = static_cast<int16_t>(colorTo_.green << 2);
    int16_t bTo = static_cast<int16_t>(colorTo_.blue << 3);
    int16_t r = easing_(rFrom, rTo, actualTime, durationTime);
    int16_t g = easing_(gFrom, gTo, actualTime, durationTime);
    int16_t b = easing_(bFrom, bTo, actualTime, durationTime);
    ColorType cur = Color::GetColorFromRGB(ClampColorComponent(r), ClampColorComponent(g), ClampColorComponent(b));
    view->SetStyle(STYLE_BACKGROUND_COLOR, cur.full);
#endif
}

void TransitionAnimatorCallback::ApplyPosition(UIView* view, uint16_t actualTime, uint16_t durationTime)
{
    int16_t x = easing_(posFromX_, posToX_, actualTime, durationTime);
    int16_t y = easing_(posFromY_, posToY_, actualTime, durationTime);
    view->SetPosition(x, y);
}

} // namespace OHOS
