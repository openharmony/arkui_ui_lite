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

#ifndef GRAPHIC_LITE_SVG_ANIMATOR_H
#define GRAPHIC_LITE_SVG_ANIMATOR_H

#include "animator/animator.h"
#include "gfx_utils/heap_base.h"
#include "gfx_utils/list.h"
#include "gfx_utils/trans_affine.h"
#include "svg_animation.h"

namespace OHOS {

class SvgDocument;

float SvgLerp(float from, float to, float progress);

uint32_t SvgLerpColor(uint32_t from, uint32_t to, float progress);

class SvgAnimatorCallback : public AnimatorCallback {
public:
    SvgAnimatorCallback(SvgElementBase& target, const SvgAnimation& anim, SvgAnimKind kind);

    ~SvgAnimatorCallback() override;

    SvgAnimatorCallback(const SvgAnimatorCallback&) = delete;
    SvgAnimatorCallback& operator=(const SvgAnimatorCallback&) = delete;

    void Callback(UIView* view) override;

    void OnStop(UIView& view) override;

    void SetAnimator(Animator* animator) { animator_ = animator; }

    void ApplyProgress(float progress, UIView& host);

private:
    static constexpr uint8_t MAX_COMPONENTS = 3;

    void ApplyNumeric(float progress);
    void ApplyTransform(float progress);
    void ApplyTransformValues(float progress, float* out);
    void ApplyColor(float progress);
    void ApplyMotion(float progress);
    void ApplySetValue();
    float ApplyNumericValues(float progress);
    uint32_t ApplyColorValues(float progress);
    float ApplyMotionKeyPoints(float progress);

    void InitNumeric(const SvgAnimation& anim, const char* values);
    void InitTransform(const SvgAnimation& anim, const char* values);
    void InitColor(const SvgAnimation& anim, const char* values);
    void InitMotion(const SvgAnimation& anim);
    void InitSet(const SvgAnimation& anim);

    uint32_t GetSegment(float progress, float& localProgress, uint32_t pointCount) const;
    float ApplySplineEase(float localProgress, uint32_t segmentIndex) const;

    SvgElementBase* target_;
    SvgAnimKind kind_;
    char* attributeName_ = nullptr;
    float fromComp_[MAX_COMPONENTS] = { 0.0f };
    float toComp_[MAX_COMPONENTS] = { 0.0f };
    uint8_t componentCount_ = 0;
    SvgAnimateTransformType transformType_ = SVG_ANIMATE_TRANSFORM_UNKNOWN;
    uint32_t fromColor_ = 0;
    uint32_t toColor_ = 0;
    char* motionPath_ = nullptr;
    char* setValue_ = nullptr;
    SvgMotionRotateMode rotateMode_ = SVG_MOTION_ROTATE_NONE;
    float rotateAngle_ = 0.0f;
    Animator* animator_ = nullptr;

    // When true, the animated transform is composed onto the element's base
    // transform (SVG/SMIL additive="sum"). The default false means replace.
    bool additiveSum_ = false;

    float* values_ = nullptr;
    uint32_t valueCount_ = 0;
    // element's static transform captured at init; the animated transform is
    // composed onto it so the base translate/scale is preserved.
    TransAffine baseTransform_;
    uint32_t* colorValues_ = nullptr;
    uint32_t colorValueCount_ = 0;
    float* keyTimes_ = nullptr;
    uint32_t keyTimeCount_ = 0;
    float* keySplines_ = nullptr;
    uint32_t keySplineCount_ = 0;
    float* keyPoints_ = nullptr;
    uint32_t keyPointCount_ = 0;
    SvgCalcMode calcMode_ = SVG_CALC_MODE_LINEAR;
    SvgAnimFillMode fillMode_ = SVG_ANIM_FILL_REMOVE;
    uint32_t beginMs_ = 0;
    uint32_t endMs_ = 0;
    float repeatCount_ = 1.0f;
    bool indefiniteRepeat_ = false;
    float finalProgress_ = 0.0f; // last applied progress; used by OnStop(fill=freeze) to freeze at the current position
};

class SvgAnimator : public HeapBase {
public:
    SvgAnimator() = default;

    ~SvgAnimator();

    void Start(SvgDocument& doc);

    void ApplyInitialStates(SvgDocument& doc, UIView* host);

    void Stop();

    // Freezes the timeline without tearing down entries; UnpauseAnimations resumes from the frozen point.
    void PauseAnimations();

    // Resumes animators previously frozen by PauseAnimations.
    void UnpauseAnimations();

    uint16_t GetAnimatorCount() const { return entries_.Size(); }

private:
    // Ownership of both pointers belongs to SvgAnimator; they are released in
    // SvgAnimator::Stop() via Release().
    struct SvgAnimEntry {
        Animator* animator = nullptr;
        SvgAnimatorCallback* callback = nullptr;

        SvgAnimEntry() = default;
        SvgAnimEntry(Animator* a, SvgAnimatorCallback* c) : animator(a), callback(c) {}

        void Release()
        {
            UiDelete(animator);
            UiDelete(callback);
        }
    };

    void StartAnimate(SvgElementBase& target, const SvgAnimation& anim, SvgAnimKind kind, UIView* host);
    void StartMotion(SvgElementBase& target, SvgAnimateMotion& anim, SvgDocument& doc, UIView* host);
    void ResolveMotionPath(SvgAnimateMotion& anim, SvgDocument& doc);
    static void ApplySet(SvgElementBase& target, const SvgSet& set);

    List<SvgAnimEntry> entries_;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_ANIMATOR_H
