/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef GRAPHIC_LITE_DRAW_CANVAS_H
#define GRAPHIC_LITE_DRAW_CANVAS_H

#include "common/image.h"
#include "gfx_utils/diagram/common/paint.h"
#include "gfx_utils/diagram/depiction/depict_dash.h"
#include "gfx_utils/diagram/depiction/depict_stroke.h"
#include "gfx_utils/diagram/rasterizer/rasterizer_scanline_antialias.h"
#include "gfx_utils/diagram/spancolorfill/fill_gradient_lut.h"

namespace OHOS {

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
struct ImageParam : public HeapBase {
    Point start;
    uint16_t height;
    uint16_t width;
    int16_t newWidth;
    int16_t newHeight;
    Image* image;
};

struct PathParam : public HeapBase {
    UICanvasVertices* vertices;
    ImageParam* imageParam = nullptr;
    bool isStroke;
};
#endif

class RenderBuffer;
class RenderBase;
class DrawCanvas : public HeapBase {
public:
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    static void DoRender(BufferInfo& gfxDstBuffer,
                         void* param,
                         const Paint& paint,
                         const Rect& rect,
                         const Rect& invalidatedArea,
                         const Style& style,
                         const bool& isStroke);

#if defined(GRAPHIC_ENABLE_SHADOW_EFFECT_FLAG) && GRAPHIC_ENABLE_SHADOW_EFFECT_FLAG
    static void DoDrawShadow(BufferInfo& gfxDstBuffer,
                             void* param,
                             const Paint& paint,
                             const Rect& rect,
                             const Rect& invalidatedArea,
                             const Style& style,
                             const bool& isStroke);
#endif
#endif
    static void InitRenderAndTransform(BufferInfo& gfxDstBuffer,
                                       RenderBuffer& renderBuffer,
                                       const Rect& rect,
                                       TransAffine& transform,
                                       const Style& style,
                                       const Paint& paint);

    static void SetRasterizer(UICanvasVertices& vertices,
                              const Paint& paint,
                              RasterizerScanlineAntialias& rasterizer,
                              TransAffine& transform,
                              const bool& isStroke);

#if defined(GRAPHIC_ENABLE_GRADIENT_FILL_FLAG) && GRAPHIC_ENABLE_GRADIENT_FILL_FLAG
    /**
     * Render gradient
     */
    static void RenderGradient(const Paint& paint,
                               RasterizerScanlineAntialias& rasterizer,
                               TransAffine& transform,
                               RenderBase& renBase,
                               RenderBuffer& renderBuffer,
                               FillBase& allocator,
                               const Rect& invalidatedArea);

    static void BuildGradientColor(const Paint& paint, FillGradientLut& gradientColorMode);

    static void BuildLineGradientMatrix(const Paint& paint,
                                        TransAffine& gradientMatrix,
                                        TransAffine& transform,
                                        float& distance)
    {
        Paint::LinearGradientPoint linearPoint = paint.GetLinearGradientPoint();
        float angle = FastAtan2F(linearPoint.y1 - linearPoint.y0, linearPoint.x1 - linearPoint.x0);
        gradientMatrix.Reset();
        gradientMatrix *= TransAffine::TransAffineRotation(angle);
        gradientMatrix *= TransAffine::TransAffineTranslation(linearPoint.x0, linearPoint.y0);
        gradientMatrix *= transform;
        gradientMatrix.Invert();
        distance = Sqrt((linearPoint.x1 - linearPoint.x0) * (linearPoint.x1 - linearPoint.x0) +
                        (linearPoint.y1 - linearPoint.y0) * (linearPoint.y1 - linearPoint.y0));
    }

    static void BuildRadialGradientMatrix(const Paint& paint,
                                          TransAffine& gradientMatrix,
                                          TransAffine& transform,
                                          float& startRadius,
                                          float& endRadius);
#endif // GRAPHIC_ENABLE_GRADIENT_FILL_FLAG
#if defined(GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG) && GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG
    /**
     * @brief Renders a gradient-filled closed path into the destination buffer.
     *
     * Encapsulates the whole scanline pipeline (transform, rasterizer, render
     * base and gradient span) so that callers such as the UIView gradient
     * adapter only need to prepare vertices and a gradient paint.
     * Unlike DoRender(), this function does not depend on ENABLE_CANVAS_EXTEND.
     *
     * @param gfxDstBuffer    destination frame buffer
     * @param vertices        closed path to fill, in coordinates relative to
     *                        the origin of @p rect (same space as the paint's
     *                        gradient endpoints)
     * @param paint           paint with style Paint::GRADIENT
     * @param rect            target rect in absolute coordinates
     * @param invalidatedArea dirty area used for clipping
     */
    static void RenderGradientFill(BufferInfo& gfxDstBuffer,
                                   UICanvasVertices& vertices,
                                   const Paint& paint,
                                   const Rect& rect,
                                   const Rect& invalidatedArea);
#endif // GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG

#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    /**
     * Render pattern mode
     */
    static void RenderPattern(const Paint& paint,
                              void* param,
                              RasterizerScanlineAntialias& rasterizer,
                              RenderBase& renBase,
                              FillBase& allocator,
                              const Rect& rect);
#endif
#endif // GRAPHIC_ENABLE_PATTERN_FILL_FLAG

    static void ChangeColor(Rgba8T& color, ColorType colorType, uint8_t alpha)
    {
        color.red = colorType.red;
        color.green = colorType.green;
        color.blue = colorType.blue;
        color.alpha = alpha;
    }

    static void RenderBlendSolid(const Paint& paint, Rgba8T& color, const bool& isStroke)
    {
        if (isStroke) {
            if (paint.GetStyle() == Paint::STROKE_STYLE || paint.GetStyle() == Paint::STROKE_FILL_STYLE) {
                ChangeColor(color, paint.GetStrokeColor(),
                            static_cast<uint8_t>(paint.GetStrokeColor().alpha * paint.GetGlobalAlpha()));
            }
        } else {
            if (paint.GetStyle() == Paint::FILL_STYLE || paint.GetStyle() == Paint::STROKE_FILL_STYLE) {
                ChangeColor(color, paint.GetFillColor(),
                            static_cast<uint8_t>(paint.GetFillColor().alpha * paint.GetGlobalAlpha()));
            }
        }
    }

    /**
     * Assembly parameter setting lineweight，LineCap，LineJoin
     */
    template <class LineStyle> static void LineStyleCalc(DepictStroke<LineStyle>& strokeLineStyle, const Paint& paint)
    {
        strokeLineStyle.SetWidth(paint.GetStrokeWidth()); // Line style related
#if defined(GRAPHIC_ENABLE_LINECAP_FLAG) && GRAPHIC_ENABLE_LINECAP_FLAG
        strokeLineStyle.SetLineCap(paint.GetLineCap());
#endif
#if defined(GRAPHIC_ENABLE_LINEJOIN_FLAG) && GRAPHIC_ENABLE_LINEJOIN_FLAG
        strokeLineStyle.SetLineJoin(paint.GetLineJoin());
        if (paint.GetMiterLimit() > 0) {
            strokeLineStyle.SetMiterLimit(paint.GetMiterLimit());
        }
#endif
    };

#if defined(GRAPHIC_ENABLE_DASH_GENERATE_FLAG) && GRAPHIC_ENABLE_DASH_GENERATE_FLAG
    /**
     * Set linedash style
     */
    static void LineDashStyleCalc(DepictDash& dashStyle, const Paint& paint)
    {
        for (uint32_t i = 0; i < paint.GetLineDashCount(); i += TWO_STEP) {
            dashStyle.AddDash(paint.GetLineDash()[i], paint.GetLineDash()[i + 1]);
        }
        dashStyle.DashStart(paint.GetLineDashOffset());
    };
#endif
};
} // namespace OHOS
#endif // GRAPHIC_LITE_DRAW_CANVAS_H
