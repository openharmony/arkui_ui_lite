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

#ifndef GRAPHIC_LITE_SVG_ANIMATION_H
#define GRAPHIC_LITE_SVG_ANIMATION_H

#include "gfx_utils/list.h"
#include "svg_element_base.h"
#include "svg_types.h"

namespace OHOS {

class SvgPathNode;

enum SvgAnimateTransformType : uint8_t {
    SVG_ANIMATE_TRANSFORM_UNKNOWN = 0,
    SVG_ANIMATE_TRANSFORM_ROTATE,
    SVG_ANIMATE_TRANSFORM_SCALE,
    SVG_ANIMATE_TRANSFORM_TRANSLATE,
};

enum SvgMotionRotateMode : uint8_t {
    SVG_MOTION_ROTATE_NONE = 0,
    SVG_MOTION_ROTATE_AUTO,
    SVG_MOTION_ROTATE_AUTO_REVERSE,
    SVG_MOTION_ROTATE_ANGLE,
};

enum SvgCalcMode : uint8_t {
    SVG_CALC_MODE_LINEAR = 0,
    SVG_CALC_MODE_DISCRETE,
    SVG_CALC_MODE_PACED,
    SVG_CALC_MODE_SPLINE,
};

enum SvgAnimFillMode : uint8_t {
    SVG_ANIM_FILL_REMOVE = 0,
    SVG_ANIM_FILL_FREEZE,
};

enum SvgAnimKind : uint8_t {
    SVG_ANIM_KIND_NUMERIC = 0,
    SVG_ANIM_KIND_TRANSFORM,
    SVG_ANIM_KIND_COLOR,
    SVG_ANIM_KIND_MOTION,
    SVG_ANIM_KIND_SET,
};

class SvgAnimation : public SvgElementBase {
public:
    SvgAnimation() = default;
    ~SvgAnimation() override;

    SvgAnimation(const SvgAnimation&) = delete;
    SvgAnimation& operator=(const SvgAnimation&) = delete;

    SvgElementCategory GetCategory() const override { return SVG_CATEGORY_ANIMATION; }

    bool SetAttribute(const char* name, const char* value) override;

    void AppendChild(SvgElementBase* child) override;

    void SetTarget(SvgElementBase* target) { target_ = target; }

    SvgElementBase* GetTarget() const { return target_; }

    void ResolveTarget();

    // The following raw-pointer getters expose internal data owned by this
    // SvgAnimation instance. Callers must not free or modify them; lifetime
    // is tied to this object.
    const char* GetHref() const { return href_; }

    virtual SvgAnimKind GetAnimKind() const = 0;

    const List<SvgElementBase*>& GetChildren() const { return children_; }

    const char* GetAttributeName() const { return attributeName_; }

    const char* GetFrom() const { return from_; }

    const char* GetTo() const { return to_; }

    const char* GetValues() const { return values_; }

    uint32_t GetDuration() const { return durMs_; }

    const float* GetKeyTimes() const { return keyTimes_; }

    uint32_t GetKeyTimeCount() const { return keyTimeCount_; }

    const float* GetKeySplines() const { return keySplines_; }

    uint32_t GetKeySplineCount() const { return keySplineCount_; }

    SvgCalcMode GetCalcMode() const { return calcMode_; }

    SvgAnimFillMode GetFillMode() const { return fillMode_; }

    uint32_t GetBeginMs() const { return beginMs_; }

    uint32_t GetEndMs() const { return endMs_; }

    float GetRepeatCount() const { return repeatCount_; }

    bool IsIndefinite() const { return indefinite_; }

    bool HasIndefiniteDuration() const { return durIndefinite_; }

    // Set when a numeric list parsed at attribute time (keyTimes / keySplines / keyPoints)
    // contained a non-finite token. Per SVG/SMIL an animation in error has no effect, so
    // SvgAnimator::StartAnimate refuses to build a callback for it.
    bool HasInvalidNumericConfig() const { return numericConfigInvalid_; }

    bool IsAdditiveSum() const { return additiveSum_; }

protected:
    bool SetAnimateAttribute(const char* name, const char* value);

    SvgElementBase* target_ = nullptr;
    char* href_ = nullptr;
    char* attributeName_ = nullptr;
    char* from_ = nullptr;
    char* to_ = nullptr;
    char* values_ = nullptr;
    float* keyTimes_ = nullptr;
    uint32_t keyTimeCount_ = 0;
    float* keySplines_ = nullptr;
    uint32_t keySplineCount_ = 0;
    uint32_t durMs_ = 0;
    bool durIndefinite_ = false;
    bool indefinite_ = false;
    SvgCalcMode calcMode_ = SVG_CALC_MODE_LINEAR;
    SvgAnimFillMode fillMode_ = SVG_ANIM_FILL_REMOVE;
    uint32_t beginMs_ = 0;
    uint32_t endMs_ = 0;
    float repeatCount_ = 1.0f;
    bool numericConfigInvalid_ = false;
    bool additiveSum_ = false;
    List<SvgElementBase*> children_;
};

class SvgAnimate : public SvgAnimation {
public:
    SvgAnimKind GetAnimKind() const override { return SVG_ANIM_KIND_NUMERIC; }
};

class SvgAnimateColor : public SvgAnimation {
public:
    SvgAnimKind GetAnimKind() const override { return SVG_ANIM_KIND_COLOR; }
};

class SvgAnimateTransform : public SvgAnimation {
public:
    bool SetAttribute(const char* name, const char* value) override;

    SvgAnimKind GetAnimKind() const override { return SVG_ANIM_KIND_TRANSFORM; }

    SvgAnimateTransformType GetTransformType() const { return transformType_; }

private:
    SvgAnimateTransformType transformType_ = SVG_ANIMATE_TRANSFORM_UNKNOWN;
};

class SvgAnimateMotion : public SvgAnimation {
public:
    ~SvgAnimateMotion() override;

    bool SetAttribute(const char* name, const char* value) override;

    SvgAnimKind GetAnimKind() const override { return SVG_ANIM_KIND_MOTION; }

    const char* GetPathData() const;

    SvgMotionRotateMode GetRotateMode() const { return rotateMode_; }

    float GetRotateAngle() const { return rotateAngle_; }

    // Returns a pointer to the internal keyPoints array owned by this
    // SvgAnimateMotion. Callers must not free or modify it.
    const float* GetKeyPoints() const { return keyPoints_; }

    uint32_t GetKeyPointCount() const { return keyPointCount_; }

    void SetPathNode(const SvgPathNode* pathNode) { pathNode_ = pathNode; }

private:
    char* path_ = nullptr;
    const SvgPathNode* pathNode_ = nullptr;
    SvgMotionRotateMode rotateMode_ = SVG_MOTION_ROTATE_NONE;
    float rotateAngle_ = 0.0f;
    float* keyPoints_ = nullptr;
    uint32_t keyPointCount_ = 0;
};

class SvgSet : public SvgAnimation {
public:
    SvgAnimKind GetAnimKind() const override { return SVG_ANIM_KIND_SET; }
};

class SvgMPath : public SvgElementBase {
public:
    ~SvgMPath() override;

    SvgElementCategory GetCategory() const override { return SVG_CATEGORY_ANIMATION; }

    bool SetAttribute(const char* name, const char* value) override;

    void AppendChild(SvgElementBase* child) override
    {
        (void)child; // <mpath> does not accept children; ignore silently.
    }

    const char* GetHref() const { return href_; }

private:
    char* href_ = nullptr;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_ANIMATION_H
