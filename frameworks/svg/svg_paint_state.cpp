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

#include "svg/svg_paint_state.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_color_util.h"
#include "svg/svg_string_util.h"
#include <cstring>

namespace OHOS {

using SvgAttributeParser::MATRIX_INDEX_A;
using SvgAttributeParser::MATRIX_INDEX_B;
using SvgAttributeParser::MATRIX_INDEX_C;
using SvgAttributeParser::MATRIX_INDEX_D;
using SvgAttributeParser::MATRIX_INDEX_E;
using SvgAttributeParser::MATRIX_INDEX_F;

namespace {

constexpr uint8_t URL_PREFIX_LEN = 4;
constexpr const char* URL_PREFIX = "url(";
constexpr const char* VALUE_INHERIT = "inherit";
constexpr const char* VALUE_CURRENT_COLOR = "currentColor";
constexpr uint8_t TEXT_ANCHOR_START = 0;
constexpr uint8_t TEXT_ANCHOR_MIDDLE = 1;
constexpr uint8_t TEXT_ANCHOR_END = 2;
// The graphics Paint default has a white stroke of width 2. This is not an SVG
// stroke inheritance, so it is used to distinguish an untouched default paint.
constexpr uint16_t DEFAULT_PAINT_STROKE_WIDTH = 2;
constexpr uint8_t HEX_SHORT_LEN = 3;
constexpr uint8_t HEX_SHORT_ALPHA_LEN = 4;
constexpr uint8_t HEX_LONG_LEN = 6;
constexpr uint8_t HEX_LONG_ALPHA_LEN = 8;

void ApplyGradientUrl(SvgPaintState& state, const char* value, void (SvgPaintState::*setter)(const char*))
{
    char id[SvgAttributeParser::SVG_URL_ID_LEN] = { 0 };
    if (!SvgAttributeParser::ParseUrlReference(value, id, sizeof(id))) {
        return;
    }
    (state.*setter)(id);
}

void ApplyPaintColor(SvgPaintState& state, const char* value, bool isFill)
{
    if (value == nullptr || value[0] == '\0') {
        return;
    }
    if (strcmp(value, VALUE_INHERIT) == 0 || strcmp(value, VALUE_CURRENT_COLOR) == 0) {
        return;
    }
    if (value[0] == 'u' && strncmp(value, URL_PREFIX, URL_PREFIX_LEN) == 0) {
        auto setter = isFill ? &SvgPaintState::SetFillGradient : &SvgPaintState::SetStrokeGradient;
        ApplyGradientUrl(state, value, setter);
        return;
    }
    // SVG/CSS: an invalid color value is ignored. The parser cannot distinguish
    // a malformed hex like #12345 (it returns transparent), so reject those
    // lengths here before they overwrite an inherited or default fill/stroke.
    if (value[0] == '#') {
        size_t len = strlen(value + 1);
        if (len != HEX_SHORT_LEN && len != HEX_SHORT_ALPHA_LEN &&
            len != HEX_LONG_LEN && len != HEX_LONG_ALPHA_LEN) {
            return;
        }
    }
    if (isFill) {
        state.SetFill(SvgAttributeParser::ParseColor(value));
    } else {
        state.SetStroke(SvgAttributeParser::ParseColor(value));
    }
}

} // namespace

SvgPaintState::SvgPaintState()
    : flags_(0), fillColor_({}), strokeColor_({}), strokeWidth_(0), opacity_(OPA_OPAQUE),
      fillGradientId_(nullptr), strokeGradientId_(nullptr),
      lineCap_(LineCap::BUTT_CAP), lineJoin_(LineJoin::ROUND_JOIN), miterLimit_(0.0f),
      dashArray_(nullptr), dashCount_(0), dashOffset_(0.0f),
      fillOpacity_(OPA_OPAQUE), strokeOpacity_(OPA_OPAQUE), visible_(true), textAnchor_(0),
      fillRule_(FILL_NON_ZERO) {}

SvgPaintState::~SvgPaintState()
{
    delete[] fillGradientId_;
    delete[] strokeGradientId_;
    delete[] dashArray_;
    fillGradientId_ = nullptr;
    strokeGradientId_ = nullptr;
    dashArray_ = nullptr;
}

void SvgPaintState::SetFill(const ColorType& color)
{
    fillColor_ = color;
    flags_ |= SVG_PAINT_FILL;
    flags_ &= ~SVG_PAINT_FILL_GRADIENT;
    delete[] fillGradientId_;
    fillGradientId_ = nullptr;
}

void SvgPaintState::SetStroke(const ColorType& color)
{
    strokeColor_ = color;
    flags_ |= SVG_PAINT_STROKE;
    flags_ &= ~SVG_PAINT_STROKE_GRADIENT;
    delete[] strokeGradientId_;
    strokeGradientId_ = nullptr;
}

void SvgPaintState::SetStrokeWidth(uint16_t width)
{
    strokeWidth_ = width;
    flags_ |= SVG_PAINT_STROKE_WIDTH;
}

void SvgPaintState::SetOpacity(uint8_t opacity)
{
    opacity_ = opacity;
    flags_ |= SVG_PAINT_OPACITY;
}

void SvgPaintState::SetTransform(const TransAffine& transform)
{
    transform_ = transform;
    flags_ |= SVG_PAINT_TRANSFORM;
}

void SvgPaintState::SetFillGradient(const char* id)
{
    delete[] fillGradientId_;
    fillGradientId_ = CopyStringWithLimit(id, SVG_MAX_ATTR_LEN);
    flags_ |= SVG_PAINT_FILL_GRADIENT;
    flags_ &= ~SVG_PAINT_FILL;
}

void SvgPaintState::SetStrokeGradient(const char* id)
{
    delete[] strokeGradientId_;
    strokeGradientId_ = CopyStringWithLimit(id, SVG_MAX_ATTR_LEN);
    flags_ |= SVG_PAINT_STROKE_GRADIENT;
    flags_ &= ~SVG_PAINT_STROKE;
}

void SvgPaintState::SetLineCap(LineCap lineCap)
{
    lineCap_ = lineCap;
    flags_ |= SVG_PAINT_STROKE_LINECAP;
}

void SvgPaintState::SetLineJoin(LineJoin lineJoin)
{
    lineJoin_ = lineJoin;
    flags_ |= SVG_PAINT_STROKE_LINEJOIN;
}

void SvgPaintState::SetMiterLimit(float miterLimit)
{
    miterLimit_ = miterLimit;
    flags_ |= SVG_PAINT_STROKE_MITERLIMIT;
}

void SvgPaintState::SetLineDash(const float* dashArray, uint32_t dashCount)
{
    delete[] dashArray_;
    dashArray_ = nullptr;
    dashCount_ = 0;
    flags_ &= ~(SVG_PAINT_STROKE_DASHARRAY | SVG_PAINT_STROKE_DASHARRAY_NONE);
    if (dashArray == nullptr || dashCount == 0) {
        // Explicit "none" or empty array disables any inherited dash pattern.
        flags_ |= SVG_PAINT_STROKE_DASHARRAY_NONE;
        return;
    }
    dashCount_ = dashCount;
    dashArray_ = new float[dashCount_];
    if (dashArray_ != nullptr) {
        for (uint32_t i = 0; i < dashCount_; i++) {
            dashArray_[i] = dashArray[i];
        }
        flags_ |= SVG_PAINT_STROKE_DASHARRAY;
    } else {
        dashCount_ = 0;
    }
}

void SvgPaintState::SetLineDashOffset(float offset)
{
    dashOffset_ = offset;
    flags_ |= SVG_PAINT_STROKE_DASHOFFSET;
}

void SvgPaintState::SetFillOpacity(uint8_t opacity)
{
    fillOpacity_ = opacity;
    flags_ |= SVG_PAINT_FILL_OPACITY;
}

void SvgPaintState::SetStrokeOpacity(uint8_t opacity)
{
    strokeOpacity_ = opacity;
    flags_ |= SVG_PAINT_STROKE_OPACITY;
}

void SvgPaintState::SetVisible(bool visible)
{
    visible_ = visible;
    flags_ |= SVG_PAINT_VISIBILITY;
}

void SvgPaintState::SetTextAnchor(uint8_t anchor)
{
    textAnchor_ = anchor;
    flags_ |= SVG_PAINT_TEXT_ANCHOR;
}

namespace {

struct PaintAttrEntry {
    const char* name;
    bool (SvgPaintState::*setter)(const char*);
};

} // namespace

bool SvgPaintState::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    static const PaintAttrEntry entries[] = {
        { "fill",            &SvgPaintState::SetFillAttribute },
        { "stroke",          &SvgPaintState::SetStrokeAttribute },
        { "fill-rule",       &SvgPaintState::SetFillRuleAttribute },
        { "stroke-width",    &SvgPaintState::SetStrokeWidthAttribute },
        { "opacity",         &SvgPaintState::SetOpacityAttribute },
        { "fill-opacity",    &SvgPaintState::SetFillOpacityAttribute },
        { "stroke-opacity",  &SvgPaintState::SetStrokeOpacityAttribute },
        { "stroke-linecap",  &SvgPaintState::SetLineCapAttribute },
        { "stroke-linejoin", &SvgPaintState::SetLineJoinAttribute },
        { "stroke-miterlimit", &SvgPaintState::SetMiterLimitAttribute },
        { "stroke-dasharray",  &SvgPaintState::SetDashArrayAttribute },
        { "stroke-dashoffset", &SvgPaintState::SetDashOffsetAttribute },
        { "visibility",      &SvgPaintState::SetVisibilityAttribute },
        { "text-anchor",     &SvgPaintState::SetTextAnchorAttribute },
        { "transform",       &SvgPaintState::SetTransformAttribute },
    };
    for (const auto& entry : entries) {
        if (strcmp(name, entry.name) == 0) {
            return (this->*entry.setter)(value);
        }
    }
    return false;
}

bool SvgPaintState::SetFillAttribute(const char* value)
{
    ApplyPaintColor(*this, value, true);
    return true;
}

bool SvgPaintState::SetStrokeAttribute(const char* value)
{
    ApplyPaintColor(*this, value, false);
    return true;
}

bool SvgPaintState::SetFillRuleAttribute(const char* value)
{
    if (strcmp(value, "evenodd") == 0) {
        fillRule_ = FILL_EVEN_ODD;
    } else {
        fillRule_ = FILL_NON_ZERO;
    }
    return true;
}

bool SvgPaintState::SetStrokeWidthAttribute(const char* value)
{
    // stroke-width initial value is 1; an invalid value must fall back to 1, not 0
    // (which would drop the stroke entirely).
    int16_t width = SvgAttributeParser::ParseLength(value, 1);
    SetStrokeWidth(static_cast<uint16_t>((width < 0) ? 0 : width));
    return true;
}

bool SvgPaintState::SetOpacityAttribute(const char* value)
{
    SetOpacity(SvgAttributeParser::ParseOpacity(value));
    return true;
}

bool SvgPaintState::SetFillOpacityAttribute(const char* value)
{
    SetFillOpacity(SvgAttributeParser::ParseOpacity(value));
    return true;
}

bool SvgPaintState::SetStrokeOpacityAttribute(const char* value)
{
    SetStrokeOpacity(SvgAttributeParser::ParseOpacity(value));
    return true;
}

bool SvgPaintState::SetLineCapAttribute(const char* value)
{
    if (strcmp(value, "round") == 0) {
        SetLineCap(LineCap::ROUND_CAP);
    } else if (strcmp(value, "square") == 0) {
        SetLineCap(LineCap::SQUARE_CAP);
    } else {
        SetLineCap(LineCap::BUTT_CAP);
    }
    return true;
}

bool SvgPaintState::SetLineJoinAttribute(const char* value)
{
    if (strcmp(value, "round") == 0) {
        SetLineJoin(LineJoin::ROUND_JOIN);
    } else if (strcmp(value, "bevel") == 0) {
        SetLineJoin(LineJoin::BEVEL_JOIN);
    } else {
        SetLineJoin(LineJoin::MITER_JOIN);
    }
    return true;
}

bool SvgPaintState::SetMiterLimitAttribute(const char* value)
{
    SetMiterLimit(SvgAttributeParser::ParseFloat(value));
    return true;
}

bool SvgPaintState::SetDashArrayAttribute(const char* value)
{
    float* dashes = nullptr;
    uint32_t count = SvgAttributeParser::ParseDashArray(value, dashes);
    SetLineDash(dashes, count);
    delete[] dashes;
    return true;
}

bool SvgPaintState::SetDashOffsetAttribute(const char* value)
{
    SetLineDashOffset(SvgAttributeParser::ParseFloat(value));
    return true;
}

bool SvgPaintState::SetVisibilityAttribute(const char* value)
{
    SetVisible((strcmp(value, "hidden") != 0) && (strcmp(value, "collapse") != 0));
    return true;
}

bool SvgPaintState::SetTextAnchorAttribute(const char* value)
{
    if (strcmp(value, "middle") == 0) {
        SetTextAnchor(TEXT_ANCHOR_MIDDLE);
    } else if (strcmp(value, "end") == 0) {
        SetTextAnchor(TEXT_ANCHOR_END);
    } else {
        SetTextAnchor(TEXT_ANCHOR_START);
    }
    return true;
}

bool SvgPaintState::SetTransformAttribute(const char* value)
{
    TransAffine matrix;
    if (SvgAttributeParser::ParseTransform(value, matrix)) {
        SetTransform(matrix);
    } else {
        // SVG/CSS: an invalid transform value is ignored, so behave as if no
        // transform were specified.
        SetTransform(TransAffine());
    }
    return true;
}

void SvgPaintState::CopyFrom(const SvgPaintState& other)
{
    if (this == &other) {
        return;
    }
    flags_ = other.flags_;
    fillColor_ = other.fillColor_;
    strokeColor_ = other.strokeColor_;
    strokeWidth_ = other.strokeWidth_;
    opacity_ = other.opacity_;
    transform_ = other.transform_;
    delete[] fillGradientId_;
    fillGradientId_ = nullptr;
    fillGradientId_ = CopyStringWithLimit(other.fillGradientId_, SVG_MAX_ATTR_LEN);
    delete[] strokeGradientId_;
    strokeGradientId_ = nullptr;
    strokeGradientId_ = CopyStringWithLimit(other.strokeGradientId_, SVG_MAX_ATTR_LEN);
    lineCap_ = other.lineCap_;
    lineJoin_ = other.lineJoin_;
    miterLimit_ = other.miterLimit_;
    dashOffset_ = other.dashOffset_;
    fillOpacity_ = other.fillOpacity_;
    strokeOpacity_ = other.strokeOpacity_;
    visible_ = other.visible_;
    textAnchor_ = other.textAnchor_;
    delete[] dashArray_;
    dashArray_ = nullptr;
    dashCount_ = other.dashCount_;
    if (other.dashArray_ != nullptr && dashCount_ > 0) {
        dashArray_ = new float[dashCount_];
        if (dashArray_ != nullptr) {
            for (uint32_t i = 0; i < dashCount_; i++) {
                dashArray_[i] = other.dashArray_[i];
            }
        } else {
            dashCount_ = 0;
        }
    }
    fillRule_ = other.fillRule_;
}

void SvgPaintState::ApplyFill(Paint& paint) const
{
    if (flags_ & SVG_PAINT_FILL) {
        paint.SetFillColor(fillColor_);
    }
    if (flags_ & SVG_PAINT_FILL_OPACITY) {
        ColorType color = (flags_ & SVG_PAINT_FILL) ? fillColor_ : paint.GetFillColor();
        paint.SetFillColor(SvgApplyOpacity(color, fillOpacity_));
    }
}

void SvgPaintState::ApplyStrokeAndStyle(Paint& paint) const
{
    if (flags_ & SVG_PAINT_STROKE) {
        paint.SetStrokeColor(strokeColor_);
    }
    if (flags_ & SVG_PAINT_STROKE_OPACITY) {
        ColorType color = (flags_ & SVG_PAINT_STROKE) ? strokeColor_ : paint.GetStrokeColor();
        paint.SetStrokeColor(SvgApplyOpacity(color, strokeOpacity_));
    }
    if (flags_ & SVG_PAINT_STROKE_WIDTH) {
        paint.SetStrokeWidth(strokeWidth_);
    }
#if defined(GRAPHIC_ENABLE_LINECAP_FLAG) && GRAPHIC_ENABLE_LINECAP_FLAG
    if (flags_ & SVG_PAINT_STROKE_LINECAP) {
        paint.SetLineCap(lineCap_);
    }
#endif
#if defined(GRAPHIC_ENABLE_LINEJOIN_FLAG) && GRAPHIC_ENABLE_LINEJOIN_FLAG
    if (flags_ & SVG_PAINT_STROKE_LINEJOIN) {
        paint.SetLineJoin(lineJoin_);
    }
    if (flags_ & SVG_PAINT_STROKE_MITERLIMIT) {
        paint.SetMiterLimit(miterLimit_);
    }
#endif
#if defined(GRAPHIC_ENABLE_DASH_GENERATE_FLAG) && GRAPHIC_ENABLE_DASH_GENERATE_FLAG
    if (flags_ & SVG_PAINT_STROKE_DASHARRAY) {
        paint.SetLineDash(dashArray_, dashCount_);
    } else if (flags_ & SVG_PAINT_STROKE_DASHARRAY_NONE) {
        paint.SetLineDash(nullptr, 0);
    }
    if ((flags_ & SVG_PAINT_STROKE_DASHOFFSET) && dashCount_ > 0) {
        // Only apply an offset when a dash pattern exists. The underlying Paint
        // marks dash mode active in SetLineDashOffset even if no array is set,
        // which can lead to a null dereference on consumers that inspect the flag
        // without also checking the dash count.
        paint.SetLineDashOffset(dashOffset_);
    }
#endif
}

void SvgPaintState::ApplyOpacityAndVisibility(Paint& paint) const
{
    if (flags_ & SVG_PAINT_OPACITY) {
        uint16_t parentOpa = paint.GetOpacity();
        uint16_t combined = (parentOpa * opacity_) / OPA_OPAQUE;
        paint.SetOpacity(static_cast<uint8_t>(combined));
    }
    if (flags_ & SVG_PAINT_VISIBILITY) {
        if (!visible_) {
            paint.SetOpacity(0);
        }
    }
}

void SvgPaintState::ApplyTransform(Paint& paint) const
{
    if (flags_ & SVG_PAINT_TRANSFORM) {
        TransAffine matrix = paint.GetTransAffine();
        // SVG composes a node's local transform onto the inherited CTM by
        // post-multiplication (CTM * T); operator*= would pre-multiply (T * CTM).
        SvgAttributeParser::PostMultiply(matrix, transform_);
        const float* d = matrix.GetData();
        paint.SetTransform(d[MATRIX_INDEX_A], d[MATRIX_INDEX_B], d[MATRIX_INDEX_D], d[MATRIX_INDEX_E],
                           SvgAttributeParser::ClampToInt16(d[MATRIX_INDEX_C]),
                           SvgAttributeParser::ClampToInt16(d[MATRIX_INDEX_F]));
    }
}

Paint SvgPaintState::Apply(const Paint& parentPaint) const
{
    Paint paint = parentPaint;
    ApplyFill(paint);
    ApplyStrokeAndStyle(paint);
    ApplyOpacityAndVisibility(paint);
    ApplyTransform(paint);
    paint.SetFillingRule(fillRule_);

    // SVG paint inheritance: only enable fill/stroke in the style when the
    // corresponding color was explicitly set on this node. The inherited style
    // already carries the parent's effective fill/stroke state; adding bits here
    // preserves inherited behaviour while preventing the root's default white
    // stroke from appearing on shapes that never asked for one.
    Paint::PaintStyle inheritedStyle = paint.GetStyle();
    bool hasFill = (inheritedStyle == Paint::FILL_STYLE ||
                    inheritedStyle == Paint::STROKE_FILL_STYLE ||
                    inheritedStyle == Paint::GRADIENT ||
                    inheritedStyle == Paint::PATTERN);
    bool hasStroke = (inheritedStyle == Paint::STROKE_STYLE ||
                      inheritedStyle == Paint::STROKE_FILL_STYLE);
    if (flags_ & SVG_PAINT_FILL) {
        hasFill = true;
    }
    if (flags_ & SVG_PAINT_FILL_GRADIENT) {
        hasFill = true;
    }
    if (flags_ & SVG_PAINT_STROKE) {
        hasStroke = true;
    }
    if (flags_ & SVG_PAINT_STROKE_GRADIENT) {
        hasStroke = true;
    }
    // A freshly constructed Paint has STROKE_FILL_STYLE with a white stroke of
    // width 2 and a black fill. That is the graphics default, not an SVG stroke
    // inheritance. The SVG root records this via SVG_PAINT_STROKE_DEFAULT_IGNORED;
    // for callers that bypass the root (e.g. unit tests) fall back to the explicit
    // default-paint helper rather than scattering the color/width checks elsewhere.
    if (hasStroke && !(flags_ & SVG_PAINT_STROKE) &&
        ((flags_ & SVG_PAINT_STROKE_DEFAULT_IGNORED) || IsDefaultParentPaint(paint))) {
        hasStroke = false;
    }
    Paint::PaintStyle newStyle = Paint::STROKE_STYLE;
    if (hasFill && hasStroke) {
        newStyle = Paint::STROKE_FILL_STYLE;
    } else if (hasFill) {
        newStyle = Paint::FILL_STYLE;
    } else if (hasStroke) {
        newStyle = Paint::STROKE_STYLE;
    }
    paint.SetStyle(newStyle);
    return paint;
}

const char* SvgPaintState::GetFillGradientId() const
{
    return fillGradientId_;
}

const char* SvgPaintState::GetStrokeGradientId() const
{
    return strokeGradientId_;
}

bool SvgPaintState::IsDefaultParentPaint(const Paint& paint)
{
    return paint.GetStyle() == Paint::STROKE_FILL_STYLE &&
           paint.GetStrokeWidth() == DEFAULT_PAINT_STROKE_WIDTH &&
           paint.GetStrokeColor().full == Color::White().full &&
           paint.GetFillColor().full == Color::Black().full;
}

void SvgPaintState::SetIgnoreDefaultStroke(bool ignore)
{
    if (ignore) {
        flags_ |= SVG_PAINT_STROKE_DEFAULT_IGNORED;
    } else {
        flags_ &= ~SVG_PAINT_STROKE_DEFAULT_IGNORED;
    }
}

} // namespace OHOS
