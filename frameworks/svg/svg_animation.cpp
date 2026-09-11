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

#include "svg/svg_animation.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_document.h"
#include "svg/svg_shape_nodes.h"
#include "svg/svg_string_util.h"
#include <cmath>
#include <cstring>

namespace OHOS {

namespace {

constexpr uint32_t MS_PER_SECOND = 1000;
constexpr uint32_t MAX_ANIM_STRING_LEN = 4096;

char* CopyString(const char* src)
{
    return CopyStringWithLimit(src, MAX_ANIM_STRING_LEN);
}

uint32_t ParseDurationMs(const char* value)
{
    if (value == nullptr) {
        return 0;
    }
    float duration = SvgAttributeParser::ParseFloat(value);
    if (!isfinite(duration) || duration <= 0) {
        return 0;
    }
    if (strstr(value, "ms") != nullptr) {
        if (duration > static_cast<float>(UINT32_MAX)) {
            return UINT32_MAX;
        }
        return static_cast<uint32_t>(duration);
    }
    constexpr uint32_t MAX_SECONDS = UINT32_MAX / MS_PER_SECOND;
    if (duration > static_cast<float>(MAX_SECONDS)) {
        return UINT32_MAX;
    }
    return static_cast<uint32_t>(duration * MS_PER_SECOND);
}

bool SetStringAttr(const char* name, const char* value, const char* attr, char*& dest)
{
    if (strcmp(name, attr) != 0) {
        return false;
    }
    delete[] dest;
    dest = CopyString(value);
    return true;
}

// Destination of a semicolon-separated float list attribute: the array, its element count,
// and the flag that records a rejected list (see SvgAnimation::HasInvalidNumericConfig).
struct FloatListTarget {
    float** values;
    uint32_t* count;
    bool* invalid;
};

bool SetFloatArrayAttr(const char* name, const char* value, const char* attr, const FloatListTarget& target)
{
    if (strcmp(name, attr) != 0) {
        return false;
    }
    delete[] *target.values;
    *target.values = nullptr;
    *target.count = SvgAttributeParser::ParseSemicolonFloats(value, *target.values, target.invalid);
    return true;
}

bool SetClockAttr(const char* name, const char* value, const char* attr, uint32_t& dest)
{
    if (strcmp(name, attr) != 0) {
        return false;
    }
    dest = SvgAttributeParser::ParseClockValue(value);
    return true;
}

bool SetDurationAttr(const char* name, const char* value, bool& indefinite, uint32_t& dest)
{
    if (strcmp(name, "dur") != 0) {
        return false;
    }
    indefinite = (strcmp(value, "indefinite") == 0);
    dest = indefinite ? 0 : ParseDurationMs(value);
    return true;
}

bool SetRepeatCountAttr(const char* name, const char* value, bool& indefinite, float& dest)
{
    if (strcmp(name, "repeatCount") != 0) {
        return false;
    }
    indefinite = (strcmp(value, "indefinite") == 0);
    if (!indefinite) {
        dest = SvgAttributeParser::ParseFloat(value);
        if (!isfinite(dest) || dest < 0.0f) {
            dest = 1.0f;
        }
    }
    return true;
}

bool SetCalcModeAttr(const char* name, const char* value, SvgCalcMode& dest)
{
    if (strcmp(name, "calcMode") != 0) {
        return false;
    }
    if (strcmp(value, "discrete") == 0) {
        dest = SVG_CALC_MODE_DISCRETE;
    } else if (strcmp(value, "paced") == 0) {
        dest = SVG_CALC_MODE_PACED;
    } else if (strcmp(value, "spline") == 0) {
        dest = SVG_CALC_MODE_SPLINE;
    } else {
        dest = SVG_CALC_MODE_LINEAR;
    }
    return true;
}

bool SetFillModeAttr(const char* name, const char* value, SvgAnimFillMode& dest)
{
    if (strcmp(name, "fill") != 0) {
        return false;
    }
    dest = (strcmp(value, "freeze") == 0) ? SVG_ANIM_FILL_FREEZE : SVG_ANIM_FILL_REMOVE;
    return true;
}

bool SetAdditiveAttr(const char* name, const char* value, bool& dest)
{
    if (strcmp(name, "additive") != 0) {
        return false;
    }
    dest = (strcmp(value, "sum") == 0);
    return true;
}

} // namespace

SvgAnimation::~SvgAnimation()
{
    delete[] href_;
    href_ = nullptr;
    delete[] attributeName_;
    attributeName_ = nullptr;
    delete[] from_;
    from_ = nullptr;
    delete[] to_;
    to_ = nullptr;
    delete[] values_;
    values_ = nullptr;
    delete[] keyTimes_;
    keyTimes_ = nullptr;
    delete[] keySplines_;
    keySplines_ = nullptr;
    while (!children_.IsEmpty()) {
        SvgElementBase* child = children_.Front();
        children_.PopFront();
        UiDelete(child);
    }
}

void SvgAnimation::ResolveTarget()
{
    if (href_ == nullptr || doc_ == nullptr) {
        return;
    }
    SvgElementBase* target = doc_->GetResource(href_);
    if (target != nullptr) {
        SetTarget(target);
    }
}

bool SvgAnimation::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "id") == 0) {
        SetIdAndRegister(value);
        return true;
    }
    if (strcmp(name, "href") == 0 || strcmp(name, "xlink:href") == 0) {
        delete[] href_;
        href_ = CopyString((value[0] == '#') ? (value + 1) : value);
        return true;
    }
    return SetAnimateAttribute(name, value);
}

bool SvgAnimation::SetAnimateAttribute(const char* name, const char* value)
{
    return SetStringAttr(name, value, "attributeName", attributeName_) ||
           SetStringAttr(name, value, "from", from_) ||
           SetStringAttr(name, value, "to", to_) ||
           SetStringAttr(name, value, "values", values_) ||
           SetFloatArrayAttr(name, value, "keyTimes",
                             { &keyTimes_, &keyTimeCount_, &numericConfigInvalid_ }) ||
           SetFloatArrayAttr(name, value, "keySplines",
                             { &keySplines_, &keySplineCount_, &numericConfigInvalid_ }) ||
           SetCalcModeAttr(name, value, calcMode_) ||
           SetFillModeAttr(name, value, fillMode_) ||
           SetAdditiveAttr(name, value, additiveSum_) ||
           SetClockAttr(name, value, "begin", beginMs_) ||
           SetClockAttr(name, value, "end", endMs_) ||
           SetDurationAttr(name, value, durIndefinite_, durMs_) ||
           SetRepeatCountAttr(name, value, indefinite_, repeatCount_);
}

void SvgAnimation::AppendChild(SvgElementBase* child)
{
    if (child == nullptr) {
        return;
    }
    if (child->GetOwner() != nullptr && child->GetOwner() != this) {
        return;
    }
    child->SetOwner(this);
    children_.PushBack(child);
    if (doc_ != nullptr) {
        child->OnDocumentAttached(doc_);
    }
}

bool SvgAnimateTransform::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (SvgAnimation::SetAnimateAttribute(name, value)) {
        return true;
    }
    if (strcmp(name, "type") == 0) {
        if (strcmp(value, "rotate") == 0) {
            transformType_ = SVG_ANIMATE_TRANSFORM_ROTATE;
        } else if (strcmp(value, "scale") == 0) {
            transformType_ = SVG_ANIMATE_TRANSFORM_SCALE;
        } else if (strcmp(value, "translate") == 0) {
            transformType_ = SVG_ANIMATE_TRANSFORM_TRANSLATE;
        } else {
            transformType_ = SVG_ANIMATE_TRANSFORM_UNKNOWN;
        }
        return true;
    }
    return false;
}

SvgAnimateMotion::~SvgAnimateMotion()
{
    delete[] path_;
    path_ = nullptr;
    delete[] keyPoints_;
    keyPoints_ = nullptr;
}

bool SvgAnimateMotion::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (SvgAnimation::SetAnimateAttribute(name, value)) {
        return true;
    }
    if (strcmp(name, "path") == 0) {
        delete[] path_;
        path_ = CopyString(value);
        return true;
    } else if (strcmp(name, "rotate") == 0) {
        if (strcmp(value, "auto") == 0) {
            rotateMode_ = SVG_MOTION_ROTATE_AUTO;
        } else if (strcmp(value, "auto-reverse") == 0) {
            rotateMode_ = SVG_MOTION_ROTATE_AUTO_REVERSE;
        } else {
            rotateMode_ = SVG_MOTION_ROTATE_ANGLE;
            rotateAngle_ = SvgAttributeParser::ParseFloat(value);
        }
        return true;
    } else if (strcmp(name, "keyPoints") == 0) {
        delete[] keyPoints_;
        keyPoints_ = nullptr;
        keyPointCount_ = SvgAttributeParser::ParseSemicolonFloats(value, keyPoints_, &numericConfigInvalid_);
        return true;
    }
    return false;
}

const char* SvgAnimateMotion::GetPathData() const
{
    if (pathNode_ != nullptr) {
        return pathNode_->GetPathData();
    }
    return path_;
}

SvgMPath::~SvgMPath()
{
    delete[] href_;
    href_ = nullptr;
}

bool SvgMPath::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "xlink:href") == 0 || strcmp(name, "href") == 0) {
        delete[] href_;
        href_ = CopyString((value[0] == '#') ? (value + 1) : value);
        return true;
    }
    return false;
}

} // namespace OHOS
