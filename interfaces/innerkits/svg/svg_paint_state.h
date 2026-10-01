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

#ifndef GRAPHIC_LITE_SVG_PAINT_STATE_H
#define GRAPHIC_LITE_SVG_PAINT_STATE_H

#include "gfx_utils/diagram/common/paint.h"
#include "gfx_utils/heap_base.h"
#include "gfx_utils/trans_affine.h"
#include "svg_types.h"

namespace OHOS {

class SvgPaintState : public HeapBase {
public:
    SvgPaintState();

    ~SvgPaintState();

    SvgPaintState(const SvgPaintState&) = delete;
    SvgPaintState& operator=(const SvgPaintState&) = delete;

    void SetFill(const ColorType& color);

    ColorType GetFillColor() const
    {
        return fillColor_;
    }

    void SetStroke(const ColorType& color);

    ColorType GetStrokeColor() const
    {
        return strokeColor_;
    }

    void SetStrokeWidth(uint16_t width);

    uint16_t GetStrokeWidth() const
    {
        return strokeWidth_;
    }

    void SetOpacity(uint8_t opacity);

    void SetTransform(const TransAffine& transform);

    void SetFillGradient(const char* id);

    void SetStrokeGradient(const char* id);

    void SetLineCap(LineCap lineCap);

    void SetLineJoin(LineJoin lineJoin);

    void SetMiterLimit(float miterLimit);

    void SetLineDash(const float* dashArray, uint32_t dashCount);

    void SetLineDashOffset(float offset);

    void SetFillOpacity(uint8_t opacity);

    void SetStrokeOpacity(uint8_t opacity);

    void SetVisible(bool visible);

    void SetTextAnchor(uint8_t anchor);

    bool SetAttribute(const char* name, const char* value);

    void CopyFrom(const SvgPaintState& other);

    Paint Apply(const Paint& parentPaint) const;

    // Marks that the inherited paint's default stroke should be ignored unless
    // this node explicitly sets one. Used by the SVG root to avoid relying on
    // graphics-library default color/width values inside Apply.
    void SetIgnoreDefaultStroke(bool ignore);

    // True when paint matches the graphics library's untouched default
    // (STROKE_FILL_STYLE, white stroke of width 2, black fill).
    static bool IsDefaultParentPaint(const Paint& paint);

    const char* GetFillGradientId() const;

    const char* GetStrokeGradientId() const;

    bool IsVisible() const
    {
        return visible_;
    }

    uint8_t GetTextAnchor() const
    {
        return textAnchor_;
    }

    const TransAffine& GetTransform() const
    {
        return transform_;
    }

    bool HasTransform() const
    {
        return (flags_ & SVG_PAINT_TRANSFORM) != 0;
    }

private:
    enum SvgPaintFlag : uint32_t {
        SVG_PAINT_FILL = 0x01,
        SVG_PAINT_STROKE = 0x02,
        SVG_PAINT_STROKE_WIDTH = 0x04,
        SVG_PAINT_OPACITY = 0x08,
        SVG_PAINT_TRANSFORM = 0x10,
        SVG_PAINT_FILL_GRADIENT = 0x20,
        SVG_PAINT_STROKE_GRADIENT = 0x40,
        SVG_PAINT_STROKE_LINECAP = 0x80,
        SVG_PAINT_STROKE_LINEJOIN = 0x100,
        SVG_PAINT_STROKE_MITERLIMIT = 0x200,
        SVG_PAINT_STROKE_DASHARRAY = 0x400,
        SVG_PAINT_STROKE_DASHARRAY_NONE = 0x10000,
        SVG_PAINT_STROKE_DEFAULT_IGNORED = 0x20000,
        SVG_PAINT_STROKE_DASHOFFSET = 0x800,
        SVG_PAINT_FILL_OPACITY = 0x1000,
        SVG_PAINT_STROKE_OPACITY = 0x2000,
        SVG_PAINT_VISIBILITY = 0x4000,
        SVG_PAINT_TEXT_ANCHOR = 0x8000,
    };

    bool SetFillAttribute(const char* value);
    bool SetStrokeAttribute(const char* value);
    bool SetFillRuleAttribute(const char* value);
    bool SetStrokeWidthAttribute(const char* value);
    bool SetOpacityAttribute(const char* value);
    bool SetFillOpacityAttribute(const char* value);
    bool SetStrokeOpacityAttribute(const char* value);
    bool SetLineCapAttribute(const char* value);
    bool SetLineJoinAttribute(const char* value);
    bool SetMiterLimitAttribute(const char* value);
    bool SetDashArrayAttribute(const char* value);
    bool SetDashOffsetAttribute(const char* value);
    bool SetVisibilityAttribute(const char* value);
    bool SetTextAnchorAttribute(const char* value);
    bool SetTransformAttribute(const char* value);

    void ApplyFill(Paint& paint) const;
    void ApplyStrokeAndStyle(Paint& paint) const;
    void ApplyOpacityAndVisibility(Paint& paint) const;
    void ApplyTransform(Paint& paint) const;

    uint32_t flags_;
    ColorType fillColor_;
    ColorType strokeColor_;
    uint16_t strokeWidth_;
    uint8_t opacity_;
    TransAffine transform_;
    char* fillGradientId_;
    char* strokeGradientId_;
    LineCap lineCap_;
    LineJoin lineJoin_;
    float miterLimit_;
    float* dashArray_;
    uint32_t dashCount_;
    float dashOffset_;
    uint8_t fillOpacity_;
    uint8_t strokeOpacity_;
    bool visible_;
    uint8_t textAnchor_;
    FillingRule fillRule_;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_PAINT_STATE_H
