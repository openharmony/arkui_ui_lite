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

#include "svg/svg_paint_servers.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_document.h"
#include "svg/svg_color_util.h"
#include <cmath>
#include <cstring>

namespace OHOS {

using SvgAttributeParser::MATRIX_INDEX_A;
using SvgAttributeParser::MATRIX_INDEX_D;

namespace {
constexpr uint8_t SPREAD_CYCLES = 4;
constexpr float PERCENT_SCALE = 100.0f;
constexpr uint8_t MAX_STOP_COUNT = 16;
constexpr uint8_t SPREAD_CYCLE_PARITY = 2;

float ComputeSpreadOffset(SvgSpreadMethod spread, uint8_t cycle, uint8_t totalCycles, float offset)
{
    float base = (static_cast<float>(cycle) + offset) / static_cast<float>(totalCycles);
    if (spread == SVG_SPREAD_REPEAT) {
        return base;
    }
    if ((cycle % SPREAD_CYCLE_PARITY) == 0) {
        return base;
    }
    return (static_cast<float>(cycle + 1) - offset) / static_cast<float>(totalCycles);
}

float ClampOffset(float offset)
{
    // NaN and non-positive offsets are clamped to 0 so invalid/missing stops do not
    // produce reversed gradient color positions.
    if (std::isnan(offset) || offset <= 0.0f) {
        return 0.0f;
    }
    if (offset >= 1.0f) {
        return 1.0f;
    }
    return offset;
}

float ParsePercentValue(const char* value)
{
    if (value == nullptr) {
        return 0.0f;
    }
    float v = SvgAttributeParser::ParseFloat(value);
    if (strchr(value, '%') != nullptr) {
        v /= PERCENT_SCALE;
    }
    return v;
}

float ParseGradientCoord(const char* value)
{
    return ParsePercentValue(value);
}

float ParseStopOffset(const char* value)
{
    return ClampOffset(ParsePercentValue(value));
}

void TransformPoint(float& x, float& y, const TransAffine& matrix)
{
    matrix.Transform(&x, &y);
}

float MatrixScale(const TransAffine& matrix)
{
    const float* d = matrix.GetData();
    return sqrtf(d[MATRIX_INDEX_A] * d[MATRIX_INDEX_A] + d[MATRIX_INDEX_D] * d[MATRIX_INDEX_D]);
}

} // namespace

// ---------- SvgStopResource ----------

SvgStopResource::~SvgStopResource()
{
}

SvgElementBase* SvgStopResource::Clone() const
{
    SvgStopResource* clone = new SvgStopResource();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->offset_ = offset_;
    clone->color_ = color_;
    clone->opacity_ = opacity_;
    return clone;
}

bool SvgStopResource::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "id") == 0) {
        SetIdAndRegister(value);
        return true;
    }
    if (strcmp(name, "offset") == 0) {
        offset_ = ParseStopOffset(value);
        return true;
    }
    if (strcmp(name, "stop-color") == 0) {
        color_ = SvgAttributeParser::ParseColor(value);
        return true;
    }
    if (strcmp(name, "stop-opacity") == 0) {
        opacity_ = SvgAttributeParser::ParseOpacity(value);
        return true;
    }
    return false;
}

void SvgStopResource::AppendChild(SvgElementBase* child)
{
    // <stop> does not accept child elements; ignore any nested content.
    (void)child;
}

// ---------- SvgGradientResourceBase ----------

SvgGradientResourceBase::~SvgGradientResourceBase()
{
    while (!children_.IsEmpty()) {
        SvgElementBase* child = children_.Front();
        children_.PopFront();
        UiDelete(child);
    }
}

void SvgGradientResourceBase::AppendChild(SvgElementBase* child)
{
    if (child == nullptr) {
        return;
    }
    if (child->GetOwner() != nullptr && child->GetOwner() != this) {
        return;
    }
    SvgElementCategory category = child->GetCategory();
    if (category == SVG_CATEGORY_RESOURCE || category == SVG_CATEGORY_GENERIC) {
        child->SetOwner(this);
        children_.PushBack(child);
        if (doc_ != nullptr) {
            child->OnDocumentAttached(doc_);
        }
    } else {
        UiDelete(child);
    }
}

void SvgGradientResourceBase::OnDocumentAttached(SvgDocument* doc)
{
    SvgElementBase::OnDocumentAttached(doc);
    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End(); node = node->next_) {
        if (node->data_ != nullptr) {
            node->data_->OnDocumentAttached(doc);
        }
    }
}

void SvgGradientResourceBase::AddStopsToPaint(Paint& paint) const
{
    uint8_t count = 0;
    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End() && count < MAX_STOP_COUNT; node = node->next_) {
        if (node->data_ == nullptr) {
            continue;
        }
        SvgStopResource* stop = dynamic_cast<SvgStopResource*>(node->data_);
        if (stop == nullptr) {
            continue;
        }
        paint.addColorStop(ClampOffset(stop->GetOffset()),
                           SvgApplyOpacity(stop->GetColor(), stop->GetOpacity()));
        count++;
    }
}

void SvgGradientResourceBase::AddSpreadStops(Paint& paint) const
{
    if (spread_ == SVG_SPREAD_PAD) {
        return;
    }
    float offsets[MAX_STOP_COUNT];
    ColorType colors[MAX_STOP_COUNT];
    uint8_t count = 0;
    CollectStops(offsets, colors, count);
    if (count == 0) {
        return;
    }
    for (uint8_t cycle = 0; cycle < SPREAD_CYCLES; cycle++) {
        for (uint8_t i = 0; i < count; i++) {
            float t = ComputeSpreadOffset(spread_, cycle, SPREAD_CYCLES, offsets[i]);
            paint.addColorStop(t, colors[i]);
        }
    }
}

void SvgGradientResourceBase::CollectStops(float* offsets, ColorType* colors, uint8_t& count) const
{
    count = 0;
    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End() && count < MAX_STOP_COUNT; node = node->next_) {
        if (node->data_ == nullptr) {
            continue;
        }
        SvgStopResource* stop = dynamic_cast<SvgStopResource*>(node->data_);
        if (stop == nullptr) {
            continue;
        }
        offsets[count] = ClampOffset(stop->GetOffset());
        colors[count] = SvgApplyOpacity(stop->GetColor(), stop->GetOpacity());
        count++;
    }
}

void SvgGradientResourceBase::CloneChildren(SvgGradientResourceBase* dest) const
{
    if (dest == nullptr) {
        return;
    }
    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End(); node = node->next_) {
        if (node->data_ == nullptr) {
            continue;
        }
        SvgElementBase* childClone = node->data_->Clone();
        if (childClone != nullptr) {
            dest->AppendChild(childClone);
        }
    }
}

// ---------- SvgLinearGradientResource ----------

SvgLinearGradientResource::~SvgLinearGradientResource() {}

bool SvgLinearGradientResource::ParseCoordAttribute(const char* name, const char* value)
{
    if (strcmp(name, "x1") == 0) {
        x1_ = ParseGradientCoord(value);
        return true;
    }
    if (strcmp(name, "y1") == 0) {
        y1_ = ParseGradientCoord(value);
        return true;
    }
    if (strcmp(name, "x2") == 0) {
        x2_ = ParseGradientCoord(value);
        return true;
    }
    if (strcmp(name, "y2") == 0) {
        y2_ = ParseGradientCoord(value);
        return true;
    }
    return false;
}

bool SvgLinearGradientResource::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "id") == 0) {
        SetIdAndRegister(value);
        return true;
    }
    if (ParseCoordAttribute(name, value)) {
        return true;
    }
    if (strcmp(name, "gradientUnits") == 0) {
        if (strcmp(value, "objectBoundingBox") == 0) {
            units_ = SVG_GRADIENT_OBJECT_BOUNDING_BOX;
        } else {
            units_ = SVG_GRADIENT_USER_SPACE_ON_USE;
        }
        return true;
    }
    if (strcmp(name, "gradientTransform") == 0) {
        if (SvgAttributeParser::ParseTransform(value, gradientTransform_)) {
            hasGradientTransform_ = true;
        }
        return true;
    }
    if (strcmp(name, "spreadMethod") == 0) {
        if (strcmp(value, "reflect") == 0) {
            spread_ = SVG_SPREAD_REFLECT;
        } else if (strcmp(value, "repeat") == 0) {
            spread_ = SVG_SPREAD_REPEAT;
        } else {
            spread_ = SVG_SPREAD_PAD;
        }
        return true;
    }
    return false;
}

SvgElementBase* SvgLinearGradientResource::Clone() const
{
    SvgLinearGradientResource* clone = new SvgLinearGradientResource();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->x1_ = x1_;
    clone->y1_ = y1_;
    clone->x2_ = x2_;
    clone->y2_ = y2_;
    clone->units_ = units_;
    clone->gradientTransform_ = gradientTransform_;
    clone->hasGradientTransform_ = hasGradientTransform_;
    clone->spread_ = spread_;
    CloneChildren(clone);
    return clone;
}

void SvgLinearGradientResource::ResolveCoordinates(const Rect& bounds, float& x1, float& y1,
                                                   float& x2, float& y2) const
{
    x1 = x1_;
    y1 = y1_;
    x2 = x2_;
    y2 = y2_;
    if (units_ != SVG_GRADIENT_OBJECT_BOUNDING_BOX) {
        return;
    }
    float w = static_cast<float>(bounds.GetWidth());
    float h = static_cast<float>(bounds.GetHeight());
    float left = static_cast<float>(bounds.GetLeft());
    float top = static_cast<float>(bounds.GetTop());
    x1 = left + x1 * w;
    y1 = top + y1 * h;
    x2 = left + x2 * w;
    y2 = top + y2 * h;
}

void SvgLinearGradientResource::ApplyTransform(float& x1, float& y1, float& x2, float& y2) const
{
    if (!hasGradientTransform_) {
        return;
    }
    TransformPoint(x1, y1, gradientTransform_);
    TransformPoint(x2, y2, gradientTransform_);
}

void SvgLinearGradientResource::ApplyToPaint(Paint& paint,
                                             const Rect& localBounds,
                                             const TransAffine* localToRecordSpace,
                                             bool isFill) const
{
    (void)isFill;
    if (children_.IsEmpty()) {
        return;
    }
    float x1 = 0.0f;
    float y1 = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;
    ResolveCoordinates(localBounds, x1, y1, x2, y2);
    ApplyTransform(x1, y1, x2, y2);
    if (localToRecordSpace != nullptr) {
        TransformPoint(x1, y1, *localToRecordSpace);
        TransformPoint(x2, y2, *localToRecordSpace);
    }
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    if (units_ == SVG_GRADIENT_OBJECT_BOUNDING_BOX && localToRecordSpace != nullptr) {
        float recordScale = MatrixScale(*localToRecordSpace);
        scaleX = static_cast<float>(localBounds.GetWidth()) * recordScale;
        scaleY = static_cast<float>(localBounds.GetHeight()) * recordScale;
    }
    if (spread_ == SVG_SPREAD_PAD) {
        paint.ClearColorStops();
        paint.createLinearGradient(x1, y1, x2, y2);
        AddStopsToPaint(paint);
    } else {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float ex = x1 + dx * static_cast<float>(SPREAD_CYCLES);
        float ey = y1 + dy * static_cast<float>(SPREAD_CYCLES);
        paint.ClearColorStops();
        paint.createLinearGradient(x1, y1, ex, ey);
        AddSpreadStops(paint);
    }
#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
    paint.SetLinearGradientScale(scaleX, scaleY);
#endif
    paint.SetStyle(Paint::GRADIENT);
}

// ---------- SvgRadialGradientResource ----------

SvgRadialGradientResource::~SvgRadialGradientResource() {}

bool SvgRadialGradientResource::ParseCoordAttribute(const char* name, const char* value)
{
    if (strcmp(name, "cx") == 0) {
        cx_ = ParseGradientCoord(value);
        return true;
    }
    if (strcmp(name, "cy") == 0) {
        cy_ = ParseGradientCoord(value);
        return true;
    }
    if (strcmp(name, "r") == 0) {
        r_ = ParseGradientCoord(value);
        return true;
    }
    if (strcmp(name, "fx") == 0) {
        fx_ = ParseGradientCoord(value);
        hasFx_ = true;
        return true;
    }
    if (strcmp(name, "fy") == 0) {
        fy_ = ParseGradientCoord(value);
        hasFy_ = true;
        return true;
    }
    return false;
}

bool SvgRadialGradientResource::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "id") == 0) {
        SetIdAndRegister(value);
        return true;
    }
    if (ParseCoordAttribute(name, value)) {
        return true;
    }
    if (strcmp(name, "gradientUnits") == 0) {
        if (strcmp(value, "objectBoundingBox") == 0) {
            units_ = SVG_GRADIENT_OBJECT_BOUNDING_BOX;
        } else {
            units_ = SVG_GRADIENT_USER_SPACE_ON_USE;
        }
        return true;
    }
    if (strcmp(name, "gradientTransform") == 0) {
        if (SvgAttributeParser::ParseTransform(value, gradientTransform_)) {
            hasGradientTransform_ = true;
        }
        return true;
    }
    if (strcmp(name, "spreadMethod") == 0) {
        if (strcmp(value, "reflect") == 0) {
            spread_ = SVG_SPREAD_REFLECT;
        } else if (strcmp(value, "repeat") == 0) {
            spread_ = SVG_SPREAD_REPEAT;
        } else {
            spread_ = SVG_SPREAD_PAD;
        }
        return true;
    }
    return false;
}

SvgElementBase* SvgRadialGradientResource::Clone() const
{
    SvgRadialGradientResource* clone = new SvgRadialGradientResource();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->cx_ = cx_;
    clone->cy_ = cy_;
    clone->r_ = r_;
    clone->fx_ = fx_;
    clone->fy_ = fy_;
    clone->hasFx_ = hasFx_;
    clone->hasFy_ = hasFy_;
    clone->units_ = units_;
    clone->gradientTransform_ = gradientTransform_;
    clone->hasGradientTransform_ = hasGradientTransform_;
    clone->spread_ = spread_;
    CloneChildren(clone);
    return clone;
}

void SvgRadialGradientResource::ResolveCoordinates(const Rect& bounds, float& cx, float& cy, float& r,
                                                   float& fx, float& fy) const
{
    cx = cx_;
    cy = cy_;
    r = r_;
    fx = hasFx_ ? fx_ : cx_;
    fy = hasFy_ ? fy_ : cy_;
    if (units_ != SVG_GRADIENT_OBJECT_BOUNDING_BOX) {
        return;
    }
    float w = static_cast<float>(bounds.GetWidth());
    float h = static_cast<float>(bounds.GetHeight());
    float left = static_cast<float>(bounds.GetLeft());
    float top = static_cast<float>(bounds.GetTop());
    cx = left + cx * w;
    cy = top + cy * h;
    fx = left + fx * w;
    fy = top + fy * h;
    // r is kept as a fraction of the unit square here; the ellipse mapping is
    // applied later via Paint::SetRadialGradientScale(scaleX, scaleY).
    if (r <= 0.0f) {
        r = 1.0f;
    }
}

void SvgRadialGradientResource::ApplyTransform(float& cx, float& cy, float& r, float& fx, float& fy) const
{
    if (!hasGradientTransform_) {
        return;
    }
    TransformPoint(cx, cy, gradientTransform_);
    TransformPoint(fx, fy, gradientTransform_);
    r *= MatrixScale(gradientTransform_);
}

void SvgRadialGradientResource::ApplyToPaint(Paint& paint,
                                             const Rect& localBounds,
                                             const TransAffine* localToRecordSpace,
                                             bool isFill) const
{
    (void)isFill;
    if (children_.IsEmpty()) {
        return;
    }
    float cx = 0.0f;
    float cy = 0.0f;
    float r = 1.0f;
    float fx = 0.0f;
    float fy = 0.0f;
    ResolveCoordinates(localBounds, cx, cy, r, fx, fy);
    ApplyTransform(cx, cy, r, fx, fy);
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    if (localToRecordSpace != nullptr) {
        TransformPoint(cx, cy, *localToRecordSpace);
        TransformPoint(fx, fy, *localToRecordSpace);
        if (units_ != SVG_GRADIENT_OBJECT_BOUNDING_BOX) {
            r *= MatrixScale(*localToRecordSpace);
        }
    }
    if (units_ == SVG_GRADIENT_OBJECT_BOUNDING_BOX) {
        scaleX = static_cast<float>(localBounds.GetWidth());
        scaleY = static_cast<float>(localBounds.GetHeight());
        if (localToRecordSpace != nullptr) {
            float recordScale = MatrixScale(*localToRecordSpace);
            scaleX *= recordScale;
            scaleY *= recordScale;
        }
    }
    if (spread_ == SVG_SPREAD_PAD) {
        paint.ClearColorStops();
        paint.createRadialGradient(fx, fy, 0.0f, cx, cy, r);
        AddStopsToPaint(paint);
    } else {
        float er = r * static_cast<float>(SPREAD_CYCLES);
        paint.ClearColorStops();
        paint.createRadialGradient(fx, fy, 0.0f, cx, cy, er);
        AddSpreadStops(paint);
    }
#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
    paint.SetRadialGradientScale(scaleX, scaleY);
#endif
    paint.SetStyle(Paint::GRADIENT);
}

// ---------- SvgSolidColorResource ----------

bool SvgSolidColorResource::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "id") == 0) {
        SetIdAndRegister(value);
        return true;
    }
    if (strcmp(name, "solid-color") == 0) {
        color_ = SvgAttributeParser::ParseColor(value);
        return true;
    }
    if (strcmp(name, "solid-opacity") == 0) {
        opacity_ = SvgAttributeParser::ParseOpacity(value);
        return true;
    }
    return false;
}

void SvgSolidColorResource::AppendChild(SvgElementBase* child)
{
    (void)child;
}

SvgElementBase* SvgSolidColorResource::Clone() const
{
    SvgSolidColorResource* clone = new SvgSolidColorResource();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->color_ = color_;
    clone->opacity_ = opacity_;
    return clone;
}

void SvgSolidColorResource::ApplyToPaint(Paint& paint,
                                         const Rect& localBounds,
                                         const TransAffine* localToRecordSpace,
                                         bool isFill) const
{
    (void)localBounds;
    (void)localToRecordSpace;
    ColorType c = SvgApplyOpacity(color_, opacity_);
    if (isFill) {
        paint.SetFillColor(c);
        Paint::PaintStyle style = paint.GetStyle();
        if (style == Paint::STROKE_STYLE) {
            paint.SetStyle(Paint::STROKE_FILL_STYLE);
        } else {
            paint.SetStyle(Paint::FILL_STYLE);
        }
    } else {
        paint.SetStrokeColor(c);
        Paint::PaintStyle style = paint.GetStyle();
        if (style == Paint::FILL_STYLE) {
            paint.SetStyle(Paint::STROKE_FILL_STYLE);
        } else {
            paint.SetStyle(Paint::STROKE_STYLE);
        }
    }
}

// ---------- SvgDefsResource ----------

SvgDefsResource::~SvgDefsResource()
{
    while (!children_.IsEmpty()) {
        SvgElementBase* child = children_.Front();
        children_.PopFront();
        UiDelete(child);
    }
}

bool SvgDefsResource::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "id") == 0) {
        SetIdAndRegister(value);
        return true;
    }
    return false;
}

void SvgDefsResource::AppendChild(SvgElementBase* child)
{
    if (child == nullptr) {
        return;
    }
    if (child->GetOwner() != nullptr && child->GetOwner() != this) {
        return;
    }
    child->SetOwner(this);
    children_.PushBack(child);
    if (doc_ == nullptr) {
        return;
    }
    child->OnDocumentAttached(doc_);
}

void SvgDefsResource::OnDocumentAttached(SvgDocument* doc)
{
    SvgElementBase::OnDocumentAttached(doc);
    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End(); node = node->next_) {
        if (node->data_ == nullptr) {
            continue;
        }
        node->data_->OnDocumentAttached(doc);
    }
}

SvgElementBase* SvgDefsResource::Clone() const
{
    SvgDefsResource* clone = new SvgDefsResource();
    if (clone == nullptr) {
        return nullptr;
    }
    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End(); node = node->next_) {
        if (node->data_ == nullptr) {
            continue;
        }
        SvgElementBase* childClone = node->data_->Clone();
        if (childClone != nullptr) {
            clone->AppendChild(childClone);
        }
    }
    return clone;
}

} // namespace OHOS
