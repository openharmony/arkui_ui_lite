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

#include "svg/svg_animator.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_document.h"
#include "svg/svg_path_parser.h"
#include "svg/svg_shape_nodes.h"
#include "svg/svg_string_util.h"
#include "securec.h"
#include <cmath>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

namespace OHOS {

namespace {

constexpr float DEGREE_PER_RADIAN = 180.0f / static_cast<float>(SVG_PI);
constexpr uint8_t FLOAT_BUF_LEN = 24;
constexpr uint8_t ATTR_BUF_LEN = 128;
constexpr int32_t PERCENT = 100;
constexpr uint32_t COLOR_BUF_LEN = 16;
constexpr uint8_t HEX_COLOR_SHIFT_R = 16;
constexpr uint8_t HEX_COLOR_SHIFT_G = 8;
constexpr uint8_t HEX_COLOR_SHIFT_A = 24;
constexpr uint32_t BYTE_MASK = 0xFF;
constexpr uint8_t ROTATE_COMPONENTS = 3;
constexpr uint8_t SCALE_COMPONENTS = 2;
constexpr uint8_t TRANSLATE_COMPONENTS = 2;
constexpr uint8_t MAX_TRANSFORM_COMPONENTS = 3;
// Upper bound for counting transform keyframes in a `values` list. A values string
// can never carry more floats than characters, so 256 keyframes x 3 components is a
// safe ceiling for SVG animateTransform usage.
constexpr uint32_t MAX_TRANSFORM_VALUE_COUNT = 256;
constexpr uint8_t KEY_SPLINE_COMPONENTS = 4;
constexpr uint8_t SPLINE_NEWTON_ITERATIONS = 8;
constexpr float SPLINE_DERIVATIVE_EPSILON = 1e-6f;
constexpr uint32_t LAST_SEGMENT_INDEX_OFFSET = 2;
constexpr int32_t DECIMAL_BASE = 10;
constexpr uint8_t MAX_COLOR_VALUES = 64;
constexpr uint32_t MAX_ANIM_STRING_LEN = 4096;

// Matrix3 row/column indices used when reading back an SVG affine matrix
// (a c e / b d f / 0 0 1) from a Matrix3<float> produced by TransAffine.
constexpr uint8_t MATRIX3_A_ROW = 0;
constexpr uint8_t MATRIX3_A_COL = 0;
constexpr uint8_t MATRIX3_B_ROW = 1;
constexpr uint8_t MATRIX3_B_COL = 0;
constexpr uint8_t MATRIX3_C_ROW = 0;
constexpr uint8_t MATRIX3_C_COL = 1;
constexpr uint8_t MATRIX3_D_ROW = 1;
constexpr uint8_t MATRIX3_D_COL = 1;
constexpr uint8_t MATRIX3_E_ROW = 0;
constexpr uint8_t MATRIX3_E_COL = 2;
constexpr uint8_t MATRIX3_F_ROW = 1;
constexpr uint8_t MATRIX3_F_COL = 2;

static inline void FormatString(char* buf, uint32_t size, const char* fmt, ...)
{
    if (size == 0) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    if (vsnprintf_s(buf, size, size - 1, fmt, args) < 0) {
        buf[0] = '\0';
    }
    va_end(args);
}

char* CopyString(const char* src)
{
    return CopyStringWithLimit(src, MAX_ANIM_STRING_LEN);
}

void FormatFloat(char* buf, uint32_t size, float value)
{
    float scaled = value * PERCENT;
    int32_t scaledRounded = static_cast<int32_t>(scaled + ((scaled >= 0) ? 0.5f : -0.5f));
    int32_t intPart = scaledRounded / PERCENT;
    int32_t frac = scaledRounded % PERCENT;
    if (frac < 0) {
        frac = -frac;
    }
    bool negativeBelowOne = (scaledRounded < 0) && (intPart == 0);
    if (frac == 0) {
        FormatString(buf, size, "%d", intPart);
    } else if ((frac % DECIMAL_BASE) == 0) {
        // When the rounded value is between -1 and 0, intPart is 0 but the
        // sign must be preserved. Print "-0.x" using only the fractional part.
        if (negativeBelowOne) {
            FormatString(buf, size, "-0.%d", frac / DECIMAL_BASE);
        } else {
            FormatString(buf, size, "%d.%d", intPart, frac / DECIMAL_BASE);
        }
    } else {
        if (negativeBelowOne) {
            FormatString(buf, size, "-0.%02d", frac);
        } else {
            FormatString(buf, size, "%d.%02d", intPart, frac);
        }
    }
}

uint8_t ParseComponents(const char* str, float* out, uint8_t maxCount)
{
    if (str == nullptr || out == nullptr) {
        return 0;
    }
    uint8_t count = 0;
    const char* p = str;
    while (*p != '\0' && count < maxCount) {
        while (*p == ' ' || *p == ',') {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        bool isNumberStart = (*p >= '0' && *p <= '9') || *p == '-' || *p == '+' || *p == '.';
        if (!isNumberStart) {
            break;
        }
        char* end = nullptr;
        float value = strtof(p, &end);
        if (end == p) {
            break;
        }
        // NaN/Inf are not legal SVG <number> tokens. Stop here so a non-finite value never
        // reaches the animated arrays, where it would later be written out as "nan".
        if (!std::isfinite(value)) {
            break;
        }
        out[count] = value;
        p = end;
        count++;
    }
    return count;
}

// Advance past whitespace and the frame separators (',', ';') that join transform keyframes.
static const char* SkipFrameSeparators(const char* p)
{
    while (*p == ' ' || *p == ',' || *p == ';' || *p == '\t' || *p == '\n' || *p == '\r') {
        p++;
    }
    return p;
}

static bool ParseOneTransformComponent(const char*& p, float& out)
{
    while (*p == ' ' || *p == ',') {
        p++;
    }
    if (*p == '\0' || *p == ';') {
        return false;
    }
    bool isNumberStart = (*p >= '0' && *p <= '9') || *p == '-' || *p == '+' || *p == '.';
    if (!isNumberStart) {
        p++;
        return false;
    }
    char* end = nullptr;
    float value = strtof(p, &end);
    if (end == p) {
        return false;
    }
    // Reject non-finite components so a NaN/Inf never enters the transform value frames.
    if (!std::isfinite(value)) {
        return false;
    }
    p = end;
    out = value;
    return true;
}

// Parse a semicolon-separated list of transform keyframes (each keyframe is one
// or more floats). Returns the total number of floats. If out is nullptr, only
// counts. *compPerFrame receives the number of floats in the first keyframe,
// which tells us how many components each transform value carries (e.g. 1 for a
// single-argument scale, 2 for scale(sx,sy), 3 for rotate(angle,cx,cy)).
uint32_t ParseTransformValues(const char* values, float* out, uint32_t maxOut, uint8_t& compPerFrame)
{
    compPerFrame = 0;
    if (values == nullptr) {
        return 0;
    }
    uint32_t total = 0;
    const char* p = values;
    while (*p != '\0' && total < maxOut) {
        p = SkipFrameSeparators(p);
        if (*p == '\0') {
            break;
        }
        uint8_t compInFrame = 0;
        while (*p != '\0' && *p != ';' && total < maxOut && compInFrame < MAX_TRANSFORM_COMPONENTS) {
            float component = 0.0f;
            if (!ParseOneTransformComponent(p, component)) {
                break;
            }
            if (out != nullptr) {
                out[total] = component;
            }
            total++;
            compInFrame++;
        }
        if (compInFrame == 0) {
            break;
        }
        if (compPerFrame == 0) {
            compPerFrame = compInFrame;
        }
    }
    return total;
}

void ParseColorValueList(const char* values, uint32_t*& colors, uint32_t& count)
{
    colors = nullptr;
    count = 0;
    if (values == nullptr) {
        return;
    }
    uint32_t colorBuffer[MAX_COLOR_VALUES];
    const char* p = values;
    while (*p != '\0' && count < MAX_COLOR_VALUES) {
        while (*p != '\0' && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ';')) {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        colorBuffer[count++] = SvgAttributeParser::ParseColor(p).full;
        while (*p != '\0' && *p != ';') {
            p++;
        }
    }
    if (count == 0) {
        return;
    }
    colors = new uint32_t[count];
    if (colors == nullptr) {
        count = 0;
        return;
    }
    for (uint32_t i = 0; i < count; i++) {
        colors[i] = colorBuffer[i];
    }
}

float CubicBezierValue(float t, float c0, float c1, float c2, float c3)
{
    float mt = 1.0f - t;
    return mt * mt * mt * c0 + 3.0f * mt * mt * t * c1 + 3.0f * mt * t * t * c2 + t * t * t * c3;
}

template<typename T>
void CopyArray(const T* src, uint32_t count, T*& dst, uint32_t& dstCount)
{
    dst = nullptr;
    dstCount = 0;
    if (count == 0 || src == nullptr || count > (static_cast<size_t>(-1) / sizeof(T))) {
        return;
    }
    dst = new T[count];
    if (dst != nullptr) {
        for (uint32_t i = 0; i < count; i++) {
            dst[i] = src[i];
        }
        dstCount = count;
    }
}

} // namespace

float SvgLerp(float from, float to, float progress)
{
    if (progress < 0.0f) {
        progress = 0.0f;
    }
    if (progress > 1.0f) {
        progress = 1.0f;
    }
    return from + (to - from) * progress;
}

uint32_t SvgLerpColor(uint32_t from, uint32_t to, float progress)
{
    uint32_t r = static_cast<uint32_t>(
        SvgLerp(static_cast<float>((from >> HEX_COLOR_SHIFT_R) & BYTE_MASK),
                static_cast<float>((to >> HEX_COLOR_SHIFT_R) & BYTE_MASK), progress) + 0.5f);
    uint32_t g = static_cast<uint32_t>(
        SvgLerp(static_cast<float>((from >> HEX_COLOR_SHIFT_G) & BYTE_MASK),
                static_cast<float>((to >> HEX_COLOR_SHIFT_G) & BYTE_MASK), progress) + 0.5f);
    uint32_t b = static_cast<uint32_t>(
        SvgLerp(static_cast<float>(from & BYTE_MASK), static_cast<float>(to & BYTE_MASK), progress) + 0.5f);
    uint32_t a = static_cast<uint32_t>(
        SvgLerp(static_cast<float>((from >> HEX_COLOR_SHIFT_A) & BYTE_MASK),
                static_cast<float>((to >> HEX_COLOR_SHIFT_A) & BYTE_MASK), progress) + 0.5f);
    return (a << HEX_COLOR_SHIFT_A) | (r << HEX_COLOR_SHIFT_R) |
           (g << HEX_COLOR_SHIFT_G) | b;
}

SvgAnimatorCallback::SvgAnimatorCallback(SvgElementBase& target, const SvgAnimation& anim, SvgAnimKind kind)
    : target_(&target), kind_(kind)
{
    attributeName_ = CopyString(anim.GetAttributeName());
    calcMode_ = anim.GetCalcMode();
    fillMode_ = anim.GetFillMode();
    beginMs_ = anim.GetBeginMs();
    endMs_ = anim.GetEndMs();
    repeatCount_ = anim.GetRepeatCount();
    indefiniteRepeat_ = anim.IsIndefinite();
    additiveSum_ = anim.IsAdditiveSum();
    keyTimeCount_ = anim.GetKeyTimeCount();
    CopyArray(anim.GetKeyTimes(), keyTimeCount_, keyTimes_, keyTimeCount_);
    keySplineCount_ = anim.GetKeySplineCount();
    CopyArray(anim.GetKeySplines(), keySplineCount_, keySplines_, keySplineCount_);
    const char* values = anim.GetValues();
    switch (kind) {
        case SVG_ANIM_KIND_NUMERIC:
            InitNumeric(anim, values);
            break;
        case SVG_ANIM_KIND_TRANSFORM:
            InitTransform(anim, values);
            break;
        case SVG_ANIM_KIND_COLOR:
            InitColor(anim, values);
            break;
        case SVG_ANIM_KIND_MOTION:
            InitMotion(anim);
            break;
        case SVG_ANIM_KIND_SET:
            InitSet(anim);
            break;
        default:
            break;
    }
    // Capture the target's static transform for additive="sum" transform
    // animations. When additive is "replace" (the default) this base transform
    // is ignored and the animated value replaces the element's local transform.
    if (kind_ == SVG_ANIM_KIND_TRANSFORM && target_ != nullptr) {
        baseTransform_ = target_->GetTransform();
    }
}

SvgAnimatorCallback::~SvgAnimatorCallback()
{
    delete[] attributeName_;
    attributeName_ = nullptr;
    delete[] motionPath_;
    motionPath_ = nullptr;
    delete[] setValue_;
    setValue_ = nullptr;
    delete[] values_;
    values_ = nullptr;
    delete[] colorValues_;
    colorValues_ = nullptr;
    delete[] keyTimes_;
    keyTimes_ = nullptr;
    delete[] keySplines_;
    keySplines_ = nullptr;
    delete[] keyPoints_;
    keyPoints_ = nullptr;
}

void SvgAnimatorCallback::InitNumeric(const SvgAnimation& anim, const char* values)
{
    componentCount_ = 1;
    (void)ParseComponents(anim.GetFrom(), fromComp_, 1);
    (void)ParseComponents(anim.GetTo(), toComp_, 1);
    if (values != nullptr) {
        valueCount_ = SvgAttributeParser::ParseSemicolonFloats(values, values_);
    }
}

void SvgAnimatorCallback::InitTransform(const SvgAnimation& anim, const char* values)
{
    const SvgAnimateTransform* trans = dynamic_cast<const SvgAnimateTransform*>(&anim);
    if (trans != nullptr) {
        transformType_ = trans->GetTransformType();
    }
    uint8_t fromCount = ParseComponents(anim.GetFrom(), fromComp_, MAX_COMPONENTS);
    uint8_t toCount = ParseComponents(anim.GetTo(), toComp_, MAX_COMPONENTS);
    uint8_t fromToCount = (fromCount > toCount) ? fromCount : toCount;
    if (values == nullptr) {
        componentCount_ = fromToCount;
        return;
    }
    uint8_t compPerFrame = 0;
    // Count with a real upper bound; passing 0 here would skip the loop entirely
    // and silently disable the values path (leaving componentCount_ at 0).
    uint32_t total = ParseTransformValues(values, nullptr, MAX_TRANSFORM_VALUE_COUNT, compPerFrame);
    if (total == 0 || total > MAX_TRANSFORM_VALUE_COUNT) {
        componentCount_ = fromToCount;
        return;
    }
    values_ = new float[total];
    if (values_ == nullptr) {
        componentCount_ = fromToCount;
        return;
    }
    uint8_t comp2 = 0;
    valueCount_ = ParseTransformValues(values, values_, total, comp2);
    componentCount_ = (compPerFrame != 0) ? compPerFrame : comp2;
    if (componentCount_ == 0) {
        componentCount_ = fromToCount;
    }
}

void SvgAnimatorCallback::InitColor(const SvgAnimation& anim, const char* values)
{
    fromColor_ = SvgAttributeParser::ParseColor(anim.GetFrom()).full;
    toColor_ = SvgAttributeParser::ParseColor(anim.GetTo()).full;
    if (values != nullptr) {
        ParseColorValueList(values, colorValues_, colorValueCount_);
    }
}

void SvgAnimatorCallback::InitMotion(const SvgAnimation& anim)
{
    const SvgAnimateMotion* motion = dynamic_cast<const SvgAnimateMotion*>(&anim);
    if (motion == nullptr) {
        return;
    }
    motionPath_ = CopyString(motion->GetPathData());
    rotateMode_ = motion->GetRotateMode();
    rotateAngle_ = motion->GetRotateAngle();
    keyPointCount_ = motion->GetKeyPointCount();
    CopyArray(motion->GetKeyPoints(), keyPointCount_, keyPoints_, keyPointCount_);
}

void SvgAnimatorCallback::InitSet(const SvgAnimation& anim)
{
    setValue_ = CopyString(anim.GetTo());
}

void SvgAnimatorCallback::Callback(UIView* view)
{
    if (view == nullptr || animator_ == nullptr) {
        return;
    }
    uint32_t total = animator_->GetTime();
    if (total == 0) {
        return;
    }
    uint32_t runTime = animator_->GetRunTime();
    if (beginMs_ > runTime) {
        return;
    }
    uint32_t activeTime = runTime - beginMs_;
    if (endMs_ > 0 && activeTime >= endMs_) {
        // The explicit end time truncates the active duration; freeze at the
        // SMIL-correct progress (endMs_ / total) rather than jumping to the
        // end value. This also updates finalProgress_ so OnStop(fill=freeze)
        // lands at the same point.
        float endProgress = static_cast<float>(endMs_) / static_cast<float>(total);
        if (endProgress > 1.0f) {
            endProgress = 1.0f;
        }
        if (endProgress < 0.0f) {
            endProgress = 0.0f;
        }
        ApplyProgress(endProgress, *view);
        finalProgress_ = endProgress;
        animator_->Stop();
        return;
    }
    if (!indefiniteRepeat_ && repeatCount_ > 0.0f) {
        float cycles = static_cast<float>(activeTime) / static_cast<float>(total);
        if (cycles >= repeatCount_) {
            finalProgress_ = repeatCount_ - floorf(repeatCount_);
            if (finalProgress_ <= 0.0f) {
                finalProgress_ = 1.0f;
            }
            animator_->Stop();
            return;
        }
    }
    uint32_t iterationTime = activeTime % total;
    float progress = static_cast<float>(iterationTime) / static_cast<float>(total);
    finalProgress_ = progress;
    ApplyProgress(progress, *view);
}

void SvgAnimatorCallback::OnStop(UIView& view)
{
    if (view.GetParent() == nullptr) {
        // The host view has already been detached from the visual tree (e.g. the
        // whole SVG component is being destroyed). The document/element tree is
        // mid-teardown, so applying the final animation state is unsafe and also
        // unnecessary because the views are about to be freed.
        return;
    }
    if (fillMode_ == SVG_ANIM_FILL_FREEZE) {
        ApplyProgress(finalProgress_, view);
    } else {
        ApplyProgress(0.0f, view);
    }
}

void SvgAnimatorCallback::ApplyProgress(float progress, UIView& host)
{
    if (target_ == nullptr) {
        return;
    }
    if (progress < 0.0f) {
        progress = 0.0f;
    }
    if (progress > 1.0f) {
        progress = 1.0f;
    }
    switch (kind_) {
        case SVG_ANIM_KIND_NUMERIC:
            ApplyNumeric(progress);
            break;
        case SVG_ANIM_KIND_TRANSFORM:
            ApplyTransform(progress);
            break;
        case SVG_ANIM_KIND_COLOR:
            ApplyColor(progress);
            break;
        case SVG_ANIM_KIND_MOTION:
            ApplyMotion(progress);
            break;
        case SVG_ANIM_KIND_SET:
            ApplySetValue();
            break;
        default:
            break;
    }
    host.Invalidate();
}

void SvgAnimatorCallback::ApplySetValue()
{
    if (attributeName_ == nullptr || setValue_ == nullptr) {
        return;
    }
    target_->SetAttribute(attributeName_, setValue_);
}

void SvgAnimatorCallback::ApplyNumeric(float progress)
{
    if (attributeName_ == nullptr) {
        return;
    }
    float value = 0.0f;
    if (valueCount_ > 0 && values_ != nullptr) {
        value = ApplyNumericValues(progress);
    } else {
        value = SvgLerp(fromComp_[0], toComp_[0], progress);
    }
    char buf[FLOAT_BUF_LEN] = { 0 };
    FormatFloat(buf, sizeof(buf), value);
    target_->SetAttribute(attributeName_, buf);
}

float SvgAnimatorCallback::ApplyNumericValues(float progress)
{
    if (valueCount_ == 0 || values_ == nullptr) {
        return 0.0f;
    }
    if (calcMode_ == SVG_CALC_MODE_DISCRETE) {
        uint32_t idx = 0;
        if (keyTimeCount_ > 0 && keyTimes_ != nullptr) {
            float local = 0.0f;
            idx = GetSegment(progress, local, valueCount_);
        } else {
            float scaled = progress * static_cast<float>(valueCount_);
            idx = static_cast<uint32_t>(scaled);
            if (idx >= valueCount_) {
                idx = valueCount_ - 1;
            }
        }
        return values_[idx];
    }
    // SVG_CALC_MODE_PACED is currently treated as linear. Arc-length uniform
    // sampling across the values list is not implemented in this release.
    float local = 0.0f;
    uint32_t idx = GetSegment(progress, local, valueCount_);
    if (calcMode_ == SVG_CALC_MODE_SPLINE) {
        local = ApplySplineEase(local, idx);
    }
    if (idx + 1 >= valueCount_) {
        return values_[valueCount_ - 1];
    }
    return SvgLerp(values_[idx], values_[idx + 1], local);
}

void SvgAnimatorCallback::ApplyColor(float progress)
{
    if (attributeName_ == nullptr) {
        return;
    }
    uint32_t color = 0;
    if (colorValueCount_ > 0 && colorValues_ != nullptr) {
        color = ApplyColorValues(progress);
    } else {
        color = SvgLerpColor(fromColor_, toColor_, progress);
    }
    char buf[COLOR_BUF_LEN] = { 0 };
    FormatString(buf, sizeof(buf), "#%02X%02X%02X%02X",
                    (color >> HEX_COLOR_SHIFT_A) & BYTE_MASK,
                    (color >> HEX_COLOR_SHIFT_R) & BYTE_MASK,
                    (color >> HEX_COLOR_SHIFT_G) & BYTE_MASK,
                    color & BYTE_MASK);
    target_->SetAttribute(attributeName_, buf);
}

uint32_t SvgAnimatorCallback::ApplyColorValues(float progress)
{
    if (colorValueCount_ == 0 || colorValues_ == nullptr) {
        return 0;
    }
    if (calcMode_ == SVG_CALC_MODE_DISCRETE) {
        uint32_t idx = 0;
        if (keyTimeCount_ > 0 && keyTimes_ != nullptr) {
            float local = 0.0f;
            idx = GetSegment(progress, local, colorValueCount_);
        } else {
            float scaled = progress * static_cast<float>(colorValueCount_);
            idx = static_cast<uint32_t>(scaled);
            if (idx >= colorValueCount_) {
                idx = colorValueCount_ - 1;
            }
        }
        return colorValues_[idx];
    }
    float local = 0.0f;
    uint32_t idx = GetSegment(progress, local, colorValueCount_);
    if (calcMode_ == SVG_CALC_MODE_SPLINE) {
        local = ApplySplineEase(local, idx);
    }
    if (idx + 1 >= colorValueCount_) {
        return colorValues_[colorValueCount_ - 1];
    }
    return SvgLerpColor(colorValues_[idx], colorValues_[idx + 1], local);
}

void SvgAnimatorCallback::ApplyMotion(float progress)
{
    if (motionPath_ == nullptr) {
        return;
    }
    float pathProgress = progress;
    if (keyPointCount_ > 0 && keyPoints_ != nullptr) {
        pathProgress = ApplyMotionKeyPoints(progress);
    }
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    if (!SvgPathPointAt(motionPath_, pathProgress, x, y, angle)) {
        return;
    }
    char fx[FLOAT_BUF_LEN] = { 0 };
    char fy[FLOAT_BUF_LEN] = { 0 };
    char fa[FLOAT_BUF_LEN] = { 0 };
    FormatFloat(fx, sizeof(fx), x);
    FormatFloat(fy, sizeof(fy), y);
    char buf[ATTR_BUF_LEN] = { 0 };
    switch (rotateMode_) {
        case SVG_MOTION_ROTATE_AUTO:
            FormatFloat(fa, sizeof(fa), angle);
            FormatString(buf, sizeof(buf), "translate(%s,%s) rotate(%s)", fx, fy, fa);
            break;
        case SVG_MOTION_ROTATE_AUTO_REVERSE:
            FormatFloat(fa, sizeof(fa), angle + SVG_PI * DEGREE_PER_RADIAN);
            FormatString(buf, sizeof(buf), "translate(%s,%s) rotate(%s)", fx, fy, fa);
            break;
        case SVG_MOTION_ROTATE_ANGLE:
            FormatFloat(fa, sizeof(fa), rotateAngle_);
            FormatString(buf, sizeof(buf), "translate(%s,%s) rotate(%s)", fx, fy, fa);
            break;
        default:
            FormatString(buf, sizeof(buf), "translate(%s,%s)", fx, fy);
            break;
    }
    target_->SetAttribute("transform", buf);
}

float SvgAnimatorCallback::ApplyMotionKeyPoints(float progress)
{
    if (keyPointCount_ == 0 || keyPoints_ == nullptr) {
        return progress;
    }
    if (keyPointCount_ == 1) {
        return keyPoints_[0];
    }
    if (calcMode_ == SVG_CALC_MODE_DISCRETE) {
        uint32_t idx = 0;
        if (keyTimeCount_ > 0 && keyTimes_ != nullptr) {
            float local = 0.0f;
            idx = GetSegment(progress, local, keyPointCount_);
        } else {
            float scaled = progress * static_cast<float>(keyPointCount_);
            idx = static_cast<uint32_t>(scaled);
            if (idx >= keyPointCount_) {
                idx = keyPointCount_ - 1;
            }
        }
        return keyPoints_[idx];
    }
    float local = 0.0f;
    uint32_t idx = GetSegment(progress, local, keyPointCount_);
    if (calcMode_ == SVG_CALC_MODE_SPLINE) {
        local = ApplySplineEase(local, idx);
    }
    if (idx + 1 >= keyPointCount_) {
        return keyPoints_[keyPointCount_ - 1];
    }
    return SvgLerp(keyPoints_[idx], keyPoints_[idx + 1], local);
}

uint32_t SvgAnimatorCallback::GetSegment(float progress, float& localProgress, uint32_t pointCount) const
{
    localProgress = 0.0f;
    if (pointCount <= 1) {
        return 0;
    }
    if (keyTimeCount_ > 0 && keyTimes_ != nullptr) {
        uint32_t useCount = keyTimeCount_ < pointCount ? keyTimeCount_ : pointCount;
        if (progress <= keyTimes_[0]) {
            localProgress = 0.0f;
            return 0;
        }
        uint32_t last = useCount - 1;
        if (progress >= keyTimes_[last]) {
            localProgress = 1.0f;
            return useCount > 1 ? (useCount - LAST_SEGMENT_INDEX_OFFSET) : 0;
        }
        for (uint32_t i = 0; i + 1 < useCount; i++) {
            float t0 = keyTimes_[i];
            float t1 = keyTimes_[i + 1];
            if (progress >= t0 && progress < t1) {
                localProgress = (t1 > t0) ? ((progress - t0) / (t1 - t0)) : 0.0f;
                return i;
            }
        }
        return useCount > 1 ? (useCount - LAST_SEGMENT_INDEX_OFFSET) : 0;
    }
    float scaled = progress * static_cast<float>(pointCount - 1);
    uint32_t idx = static_cast<uint32_t>(scaled);
    if (idx >= pointCount - 1) {
        idx = pointCount - LAST_SEGMENT_INDEX_OFFSET;
    }
    localProgress = scaled - static_cast<float>(idx);
    return idx;
}

float SvgAnimatorCallback::ApplySplineEase(float localProgress, uint32_t segmentIndex) const
{
    if (keySplines_ == nullptr || keySplineCount_ < (segmentIndex + 1) * KEY_SPLINE_COMPONENTS) {
        return localProgress;
    }
    const float* s = keySplines_ + segmentIndex * KEY_SPLINE_COMPONENTS;
    float x1 = s[0];
    float y1 = s[1];
    float x2 = s[2];
    float y2 = s[3];
    float t = localProgress;
    for (int i = 0; i < SPLINE_NEWTON_ITERATIONS; i++) {
        float x = CubicBezierValue(t, 0.0f, x1, x2, 1.0f);
        float dx = 3.0f * (1.0f - t) * (1.0f - t) * x1 +
                   6.0f * (1.0f - t) * t * (x2 - x1) +
                   3.0f * t * t * (1.0f - x2);
        if (std::fabs(dx) > SPLINE_DERIVATIVE_EPSILON) {
            t = t - (x - localProgress) / dx;
            if (t < 0.0f) {
                t = 0.0f;
            }
            if (t > 1.0f) {
                t = 1.0f;
            }
        } else {
            break;
        }
    }
    return CubicBezierValue(t, 0.0f, y1, y2, 1.0f);
}

static void FormatTransformString(SvgAnimateTransformType type, uint8_t componentCount,
                                  const float* values, char* buf, uint32_t bufSize)
{
    char valueStr0[FLOAT_BUF_LEN] = { 0 };
    FormatFloat(valueStr0, sizeof(valueStr0), values[0]);
    switch (type) {
        case SVG_ANIMATE_TRANSFORM_ROTATE:
            if (componentCount >= ROTATE_COMPONENTS) {
                char valueStr1[FLOAT_BUF_LEN] = { 0 };
                char valueStr2[FLOAT_BUF_LEN] = { 0 };
                FormatFloat(valueStr1, sizeof(valueStr1), values[1]);
                FormatFloat(valueStr2, sizeof(valueStr2), values[ROTATE_COMPONENTS - 1]);
                FormatString(buf, bufSize, "rotate(%s,%s,%s)", valueStr0, valueStr1, valueStr2);
            } else {
                FormatString(buf, bufSize, "rotate(%s)", valueStr0);
            }
            break;
        case SVG_ANIMATE_TRANSFORM_SCALE:
            if (componentCount >= SCALE_COMPONENTS) {
                char valueStr1[FLOAT_BUF_LEN] = { 0 };
                FormatFloat(valueStr1, sizeof(valueStr1), values[1]);
                FormatString(buf, bufSize, "scale(%s,%s)", valueStr0, valueStr1);
            } else {
                FormatString(buf, bufSize, "scale(%s)", valueStr0);
            }
            break;
        case SVG_ANIMATE_TRANSFORM_TRANSLATE:
            if (componentCount >= TRANSLATE_COMPONENTS) {
                char valueStr1[FLOAT_BUF_LEN] = { 0 };
                FormatFloat(valueStr1, sizeof(valueStr1), values[1]);
                FormatString(buf, bufSize, "translate(%s,%s)", valueStr0, valueStr1);
            } else {
                FormatString(buf, bufSize, "translate(%s)", valueStr0);
            }
            break;
        default:
            buf[0] = '\0';
            break;
    }
}

static void ApplyComposedTransform(SvgElementBase* target, const TransAffine& baseTransform,
                                   const char* transformString)
{
    TransAffine animMat;
    if (SvgAttributeParser::ParseTransform(transformString, animMat)) {
        // SVG/SMIL additive="sum" for animateTransform: the animated transform is
        // applied to the element first, then the element's base transform is applied.
        // TransAffine's operator* is right-multiplication for column vectors, so the
        // product animMat * baseTransform yields the standard base * anim effect.
        Matrix3<float> composed = animMat * baseTransform;
        TransAffine out(composed[MATRIX3_A_ROW][MATRIX3_A_COL],
                       composed[MATRIX3_B_ROW][MATRIX3_B_COL],
                       composed[MATRIX3_C_ROW][MATRIX3_C_COL],
                       composed[MATRIX3_D_ROW][MATRIX3_D_COL],
                       composed[MATRIX3_E_ROW][MATRIX3_E_COL],
                       composed[MATRIX3_F_ROW][MATRIX3_F_COL]);
        target->SetTransform(out);
    } else {
        target->SetTransform(baseTransform);
    }
}

void SvgAnimatorCallback::ApplyTransform(float progress)
{
    if (componentCount_ == 0) {
        return;
    }
    float values[MAX_COMPONENTS] = { 0.0f };
    if (valueCount_ > 0 && values_ != nullptr) {
        ApplyTransformValues(progress, values);
    } else {
        for (uint8_t i = 0; i < componentCount_; i++) {
            values[i] = SvgLerp(fromComp_[i], toComp_[i], progress);
        }
    }
    char buf[ATTR_BUF_LEN] = { 0 };
    FormatTransformString(transformType_, componentCount_, values, buf, sizeof(buf));
    // SVG/SMIL additive semantics: "replace" (default) means the animated value
    // replaces the base value; "sum" means compose the animated transform onto
    // the element's original base transform.
    if (additiveSum_) {
        ApplyComposedTransform(target_, baseTransform_, buf);
    } else {
        ApplyComposedTransform(target_, TransAffine(), buf);
    }
}

void SvgAnimatorCallback::ApplyTransformValues(float progress, float* out)
{
    if (out == nullptr || values_ == nullptr || valueCount_ == 0 || componentCount_ == 0) {
        return;
    }
    uint32_t frameCount = valueCount_ / componentCount_;
    if (frameCount == 0) {
        return;
    }
    if (calcMode_ == SVG_CALC_MODE_DISCRETE) {
        uint32_t idx = 0;
        if (keyTimeCount_ > 0 && keyTimes_ != nullptr) {
            float local = 0.0f;
            idx = GetSegment(progress, local, frameCount);
        } else {
            float scaled = progress * static_cast<float>(frameCount);
            idx = static_cast<uint32_t>(scaled);
            if (idx >= frameCount) {
                idx = frameCount - 1;
            }
        }
        uint32_t base = idx * componentCount_;
        for (uint8_t c = 0; c < componentCount_; c++) {
            out[c] = values_[base + c];
        }
        return;
    }
    float local = 0.0f;
    uint32_t idx = GetSegment(progress, local, frameCount);
    if (calcMode_ == SVG_CALC_MODE_SPLINE) {
        local = ApplySplineEase(local, idx);
    }
    if (idx + 1 >= frameCount) {
        uint32_t base = (frameCount - 1) * componentCount_;
        for (uint8_t c = 0; c < componentCount_; c++) {
            out[c] = values_[base + c];
        }
        return;
    }
    uint32_t base = idx * componentCount_;
    for (uint8_t c = 0; c < componentCount_; c++) {
        float a = values_[base + c];
        float b = values_[base + componentCount_ + c];
        out[c] = SvgLerp(a, b, local);
    }
}

SvgAnimator::~SvgAnimator()
{
    Stop();
}

void SvgAnimator::ApplyInitialStates(SvgDocument& doc, UIView* host)
{
    if (host == nullptr) {
        return;
    }
    const List<SvgElementBase*>& animations = doc.GetAnimations();
    for (ListNode<SvgElementBase*>* node = animations.Begin(); node != animations.End(); node = node->next_) {
        SvgAnimation* anim = dynamic_cast<SvgAnimation*>(node->data_);
        if (anim == nullptr || anim->GetTarget() == nullptr) {
            continue;
        }
        if (anim->GetBeginMs() != 0) {
            continue;
        }
        SvgElementBase* target = anim->GetTarget();
        SvgAnimKind kind = anim->GetAnimKind();
        if (kind == SVG_ANIM_KIND_MOTION) {
            SvgAnimateMotion* motion = dynamic_cast<SvgAnimateMotion*>(anim);
            if (motion != nullptr) {
                ResolveMotionPath(*motion, doc);
            }
        }
        SvgAnimatorCallback* callback = new SvgAnimatorCallback(*target, *anim, kind);
        if (callback != nullptr) {
            callback->ApplyProgress(0.0f, *host);
            UiDelete(callback);
        }
    }
}

void SvgAnimator::Start(SvgDocument& doc)
{
    Stop();
    UIView* host = doc.GetHostView();
    const List<SvgElementBase*>& animations = doc.GetAnimations();
    for (ListNode<SvgElementBase*>* node = animations.Begin(); node != animations.End(); node = node->next_) {
        SvgAnimation* anim = dynamic_cast<SvgAnimation*>(node->data_);
        if (anim == nullptr || anim->GetTarget() == nullptr) {
            continue;
        }
        SvgElementBase* target = anim->GetTarget();
        SvgAnimKind kind = anim->GetAnimKind();
        if (kind == SVG_ANIM_KIND_MOTION) {
            SvgAnimateMotion* motion = dynamic_cast<SvgAnimateMotion*>(anim);
            if (motion != nullptr) {
                StartMotion(*target, *motion, doc, host);
            }
        } else if (kind == SVG_ANIM_KIND_SET) {
            const SvgSet* set = dynamic_cast<const SvgSet*>(anim);
            if (set != nullptr && set->GetBeginMs() == 0) {
                ApplySet(*target, *set);
            } else {
                StartAnimate(*target, *anim, kind, host);
            }
        } else if (kind == SVG_ANIM_KIND_TRANSFORM) {
            const SvgAnimateTransform* trans = dynamic_cast<const SvgAnimateTransform*>(anim);
            if (trans != nullptr && trans->GetTransformType() != SVG_ANIMATE_TRANSFORM_UNKNOWN) {
                StartAnimate(*target, *anim, kind, host);
            }
        } else {
            StartAnimate(*target, *anim, kind, host);
        }
    }
    if (host != nullptr) {
        host->Invalidate();
    }
}

void SvgAnimator::Stop()
{
    ListNode<SvgAnimEntry>* node = entries_.Begin();
    for (; node != entries_.End(); node = node->next_) {
        if (node->data_.animator != nullptr) {
            node->data_.animator->Stop();
        }
        node->data_.Release();
    }
    entries_.Clear();
}

void SvgAnimator::PauseAnimations()
{
    // Only running animators are frozen; already-finished (STOP) entries are left untouched
    // so that UnpauseAnimations does not accidentally revive them.
    ListNode<SvgAnimEntry>* node = entries_.Begin();
    for (; node != entries_.End(); node = node->next_) {
        Animator* animator = node->data_.animator;
        if (animator != nullptr && animator->GetState() == Animator::START) {
            animator->Pause();
        }
    }
}

void SvgAnimator::UnpauseAnimations()
{
    // Resume only the entries that were actually frozen by PauseAnimations.
    ListNode<SvgAnimEntry>* node = entries_.Begin();
    for (; node != entries_.End(); node = node->next_) {
        Animator* animator = node->data_.animator;
        if (animator != nullptr && animator->GetState() == Animator::PAUSE) {
            animator->Resume();
        }
    }
}

// Scans an animation value string for a non-finite numeric token (NaN/Inf).
// Tokens that strtof cannot consume are skipped so that keywords and colors pass through
// untouched; only an actual NaN/Inf literal is treated as an error.
static bool HasNonFiniteToken(const char* str)
{
    if (str == nullptr) {
        return false;
    }
    const char* p = str;
    while (*p != '\0') {
        if (*p == ' ' || *p == ',' || *p == ';' || *p == '\t' || *p == '\n' || *p == '\r') {
            p++;
            continue;
        }
        char* end = nullptr;
        float value = strtof(p, &end);
        if (end == p) {
            // Not a number token (part of a keyword, color, etc.): advance one char.
            p++;
            continue;
        }
        if (!std::isfinite(value)) {
            return true;
        }
        p = end;
    }
    return false;
}

// Reports whether an animation element is in error and therefore has no effect at all.
// Two causes: an incomplete configuration (no duration, no from/to/values, no attributeName),
// and a non-finite number, which is not a legal SVG <number>. In both cases SVG/SMIL requires
// the whole animation to be skipped rather than partially applied. keyTimes/keySplines/keyPoints
// are validated when the attribute is parsed (SvgAnimation::HasInvalidNumericConfig), while
// from/to/values are still raw strings here and so are scanned now.
static bool IsAnimationInError(const SvgAnimation& anim, SvgAnimKind kind, uint32_t duration)
{
    if (duration == 0 && !anim.HasIndefiniteDuration()) {
        return true;
    }
    if (kind != SVG_ANIM_KIND_MOTION && anim.GetFrom() == nullptr && anim.GetTo() == nullptr &&
        anim.GetValues() == nullptr) {
        return true;
    }
    bool needAttrName = (kind == SVG_ANIM_KIND_NUMERIC || kind == SVG_ANIM_KIND_COLOR || kind == SVG_ANIM_KIND_SET);
    if (needAttrName && anim.GetAttributeName() == nullptr) {
        return true;
    }
    if (anim.HasInvalidNumericConfig()) {
        return true;
    }
    bool numericConfig = (kind == SVG_ANIM_KIND_NUMERIC || kind == SVG_ANIM_KIND_TRANSFORM ||
                          kind == SVG_ANIM_KIND_SET);
    return numericConfig && (HasNonFiniteToken(anim.GetFrom()) || HasNonFiniteToken(anim.GetTo()) ||
                             HasNonFiniteToken(anim.GetValues()));
}

void SvgAnimator::StartAnimate(SvgElementBase& target, const SvgAnimation& anim, SvgAnimKind kind, UIView* host)
{
    // Animation requires a host view (document must be attached to a view).
    if (host == nullptr) {
        return;
    }
    uint32_t duration = (kind == SVG_ANIM_KIND_SET) ? anim.GetBeginMs() : anim.GetDuration();
    if (IsAnimationInError(anim, kind, duration)) {
        return;
    }
    SvgAnimatorCallback* callback = new SvgAnimatorCallback(target, anim, kind);
    if (callback == nullptr) {
        return;
    }
    if (anim.GetBeginMs() == 0) {
        callback->ApplyProgress(0.0f, *host);
    }
    if (anim.HasIndefiniteDuration()) {
        callback->ApplyProgress(0.0f, *host);
        UiDelete(callback);
        return;
    }
    bool repeat = anim.IsIndefinite() || anim.GetRepeatCount() > 1.0f;
    Animator* animator = new Animator(callback, host, duration, repeat);
    if (animator == nullptr) {
        UiDelete(callback);
        return;
    }
    callback->SetAnimator(animator);
    SvgAnimEntry entry = { animator, callback };
    entries_.PushBack(entry);
    animator->Start();
}

void SvgAnimator::StartMotion(SvgElementBase& target, SvgAnimateMotion& anim, SvgDocument& doc, UIView* host)
{
    // Animation requires a host view (document must be attached to a view).
    if (host == nullptr) {
        return;
    }
    ResolveMotionPath(anim, doc);
    if (anim.GetPathData() == nullptr) {
        return;
    }
    StartAnimate(target, anim, SVG_ANIM_KIND_MOTION, host);
}

void SvgAnimator::ResolveMotionPath(SvgAnimateMotion& anim, SvgDocument& doc)
{
    const List<SvgElementBase*>& children = anim.GetChildren();
    for (ListNode<SvgElementBase*>* child = children.Begin(); child != children.End(); child = child->next_) {
        SvgMPath* mpath = dynamic_cast<SvgMPath*>(child->data_);
        if (mpath == nullptr) {
            continue;
        }
        const char* href = mpath->GetHref();
        if (href == nullptr) {
            return;
        }
        SvgElementBase* ref = doc.GetResource(href);
        SvgPathNode* pathNode = dynamic_cast<SvgPathNode*>(ref);
        if (pathNode != nullptr) {
            anim.SetPathNode(pathNode);
        }
        return;
    }
}

void SvgAnimator::ApplySet(SvgElementBase& target, const SvgSet& set)
{
    const char* name = set.GetAttributeName();
    const char* to = set.GetTo();
    if (name == nullptr || to == nullptr) {
        return;
    }
    target.SetAttribute(name, to);
}

} // namespace OHOS
