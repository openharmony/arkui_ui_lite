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

#ifndef TRANSITION_ANIMATOR_CALLBACK_H
#define TRANSITION_ANIMATOR_CALLBACK_H

#include "animator/animator.h"
#include "animator/easing_equation.h"
#include "gfx_utils/color.h"
#include "gfx_utils/graphic_math.h"
#include "gfx_utils/style.h"

namespace OHOS {

/**
 * @brief Applies transition properties for a timeline driven by Animator.
 *
 * Animator owns the lifecycle and timing. This callback owns only transition
 * state and frame rendering, so the same callback can be driven by the native
 * Animator or by an external transition timeline.
 */
class TransitionAnimatorCallback : public AnimatorCallback {
public:
    TransitionAnimatorCallback();
    ~TransitionAnimatorCallback() override = default;

    TransitionAnimatorCallback(const TransitionAnimatorCallback&) = delete;
    TransitionAnimatorCallback& operator=(const TransitionAnimatorCallback&) = delete;
    TransitionAnimatorCallback(TransitionAnimatorCallback&&) = delete;
    TransitionAnimatorCallback& operator=(TransitionAnimatorCallback&&) = delete;

    void Reset();
    void SetDuration(uint32_t duration);
    void SetDelay(uint32_t delay);
    void SetEasingFunc(EasingFunc easing);

    void SetOpacity(uint8_t from, uint8_t to);
    void SetScale(float fromX, float fromY, float toX, float toY,
                  float pivotX = -1.0f, float pivotY = -1.0f);
    void SetRotation(int16_t fromAngle, int16_t toAngle,
                     float pivotX = -1.0f, float pivotY = -1.0f);
    void SetColor(ColorType from, ColorType to);
    void SetPosition(int16_t fromX, int16_t fromY, int16_t toX, int16_t toY);

    bool HasEffects() const
    {
        return (effectMask_ != 0) || ((next_ != nullptr) && next_->HasEffects());
    }

    void Callback(UIView* view) override;

    /**
     * @brief Links the next transition callback in the animation chain.
     *
     * The callback objects are owned by the transition implementation. This
     * class only keeps the link used to calculate the frame timeline.
     */
    void AddTransitionAnimatorCallback(TransitionAnimatorCallback* next, bool sequential);

    uint32_t GetTotalTime() const;

    /**
     * @brief Applies a frame using externally calculated elapsed and duration values.
     */
    void ApplyFrame(UIView* view, uint32_t elapsedTime, uint32_t durationTime);

private:
    void ApplyChainFrame(UIView* view, uint32_t timelineTime);
    void ApplyCurrentFrame(UIView* view, uint32_t elapsedTime, uint32_t durationTime);
    void ApplyOpacity(UIView* view, uint16_t actualTime, uint16_t durationTime);
    void ApplyScale(UIView* view, uint16_t actualTime, uint16_t durationTime);
    void ApplyRotation(UIView* view, uint16_t actualTime, uint16_t durationTime);
    void ApplyColor(UIView* view, uint16_t actualTime, uint16_t durationTime);
    void ApplyPosition(UIView* view, uint16_t actualTime, uint16_t durationTime);

    EasingFunc easing_;
    uint32_t duration_;
    uint32_t delay_;
    uint32_t startTime_;  // animation start timestamp (0 = not started), frame-rate independent
    uint8_t effectMask_;  // bitmask of configured transition effects (EFFECT_OPACITY, EFFECT_SCALE, ...)

    uint8_t opaFrom_;
    uint8_t opaTo_;
    int16_t scaleFromX_;
    int16_t scaleFromY_;
    int16_t scaleToX_;
    int16_t scaleToY_;
    float scalePivotX_;
    float scalePivotY_;
    int16_t rotateFrom_;
    int16_t rotateTo_;
    float rotatePivotX_;
    float rotatePivotY_;
    ColorType colorFrom_;
    ColorType colorTo_;
    int16_t posFromX_;
    int16_t posFromY_;
    int16_t posToX_;
    int16_t posToY_;
    TransitionAnimatorCallback* next_ = nullptr;
    bool nextSequential_ = false;
};
} // namespace OHOS

#endif // TRANSITION_ANIMATOR_CALLBACK_H
