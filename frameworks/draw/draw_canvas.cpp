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

#include "draw/draw_canvas.h"
#include "common/typed_text.h"
#include "draw/clip_utils.h"
#include "gfx_utils/diagram/depiction/depict_curve.h"
#include "gfx_utils/diagram/spancolorfill/fill_gradient.h"
#include "gfx_utils/diagram/spancolorfill/fill_interpolator.h"

namespace OHOS {
/**
 * Renders monochrome polygon paths and fills
 */
void RenderSolid(const Paint& paint, RasterizerScanlineAntialias& rasterizer, RenderBase& renBase, const bool& isStroke)
{
    GeometryScanline scanline;
    Rgba8T color;
    DrawCanvas::RenderBlendSolid(paint, color, isStroke);
    RenderScanlinesAntiAliasSolid(rasterizer, scanline, renBase, color);
}

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
void DrawCanvas::DoRender(BufferInfo& gfxDstBuffer,
                          void* param,
                          const Paint& paint,
                          const Rect& rect,
                          const Rect& invalidatedArea,
                          const Style& style,
                          const bool& isStroke)
{
    if (param == nullptr) {
        return;
    }
#if defined(GRAPHIC_ENABLE_SHADOW_EFFECT_FLAG) && GRAPHIC_ENABLE_SHADOW_EFFECT_FLAG
    if (paint.HaveShadow()) {
        DrawCanvas::DoDrawShadow(gfxDstBuffer, param, paint, rect, invalidatedArea, style, isStroke);
    }
#endif
    TransAffine transform;
    RenderBuffer renderBuffer;
    InitRenderAndTransform(gfxDstBuffer, renderBuffer, rect, transform, style, paint);

    RasterizerScanlineAntialias rasterizer;
    GeometryScanline scanline;

    PathParam* pathParam = static_cast<PathParam*>(param);
    rasterizer.ClipBox(0, 0, gfxDstBuffer.width, gfxDstBuffer.height);
    SetRasterizer(*pathParam->vertices, paint, rasterizer, transform, isStroke);

    RenderPixfmtRgbaBlend pixFormat(renderBuffer);
    RenderBase renBase(pixFormat);
    FillBase allocator;

    renBase.ResetClipping(true);
    renBase.ClipBox(invalidatedArea.GetLeft(), invalidatedArea.GetTop(), invalidatedArea.GetRight(),
                    invalidatedArea.GetBottom());

    if (paint.GetStyle() == Paint::STROKE_STYLE || paint.GetStyle() == Paint::FILL_STYLE ||
        paint.GetStyle() == Paint::STROKE_FILL_STYLE) {
        RenderSolid(paint, rasterizer, renBase, isStroke);
    }

#if defined(GRAPHIC_ENABLE_GRADIENT_FILL_FLAG) && GRAPHIC_ENABLE_GRADIENT_FILL_FLAG
    if (paint.GetStyle() == Paint::GRADIENT) {
        RenderGradient(paint, rasterizer, transform, renBase, renderBuffer, allocator, invalidatedArea);
    }
#endif
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
    if (paint.GetStyle() == Paint::PATTERN) {
        RenderPattern(paint, pathParam->imageParam, rasterizer, renBase, allocator, rect);
    }
#endif
}

#if defined(GRAPHIC_ENABLE_SHADOW_EFFECT_FLAG) && GRAPHIC_ENABLE_SHADOW_EFFECT_FLAG
void DrawCanvas::DoDrawShadow(BufferInfo& gfxDstBuffer,
                              void* param,
                              const Paint& paint,
                              const Rect& rect,
                              const Rect& invalidatedArea,
                              const Style& style,
                              const bool& isStroke)
{
    if (param == nullptr) {
        return;
    }

    TransAffine transform;
    RenderBuffer renderBuffer;
    DrawCanvas::InitRenderAndTransform(gfxDstBuffer, renderBuffer, rect, transform, style, paint);

    transform.Translate(paint.GetShadowOffsetX(), paint.GetShadowOffsetY());

    RasterizerScanlineAntialias rasterizer;
    GeometryScanline scanline;
    PathParam* pathParam = static_cast<PathParam*>(param);
    rasterizer.ClipBox(0, 0, gfxDstBuffer.width, gfxDstBuffer.height);
    DrawCanvas::SetRasterizer(*pathParam->vertices, paint, rasterizer, transform, isStroke);
    Rect bbox(rasterizer.GetMinX(), rasterizer.GetMinY(), rasterizer.GetMaxX(), rasterizer.GetMaxY());

    RenderPixfmtRgbaBlend pixFormat(renderBuffer);
    RenderBase renBase(pixFormat);
    FillBase allocator;

    renBase.ResetClipping(true);
    renBase.ClipBox(invalidatedArea.GetLeft(), invalidatedArea.GetTop(), invalidatedArea.GetRight(),
                    invalidatedArea.GetBottom());

    Rgba8T shadowColor;
    DrawCanvas::ChangeColor(shadowColor, paint.GetShadowColor(), paint.GetShadowColor().alpha * paint.GetGlobalAlpha());

    RenderScanlinesAntiAliasSolid(rasterizer, scanline, renBase, shadowColor);
#if GRAPHIC_ENABLE_BLUR_EFFECT_FLAG
    bbox.SetLeft(bbox.GetLeft() - paint.GetShadowBlur());
    bbox.SetTop(bbox.GetTop() - paint.GetShadowBlur());
    bbox.SetRight(bbox.GetRight() + paint.GetShadowBlur());
    bbox.SetBottom(bbox.GetBottom() + paint.GetShadowBlur());
    RenderBuffer shadowBuffer;
    RenderPixfmtRgbaBlend pixf2(shadowBuffer);
    Rect shadowRect = {int16_t(bbox.GetLeft()), int16_t(bbox.GetTop()), int16_t(bbox.GetRight()),
                       int16_t(bbox.GetBottom())};
    shadowRect.Intersect(shadowRect, invalidatedArea);
    pixf2.Attach(pixFormat, shadowRect.GetLeft(), shadowRect.GetTop(), shadowRect.GetRight(), shadowRect.GetBottom());
    uint8_t pixelByteSize = DrawUtils::GetPxSizeByColorMode(gfxDstBuffer.mode) >> 3; // 3: Shift right 3 bits

    paint.GetDrawBoxBlur().BoxBlur(pixf2, MATH_UROUND(paint.GetShadowBlur()), pixelByteSize, gfxDstBuffer.stride);

#endif // GRAPHIC_ENABLE_BLUR_EFFECT_FLAG
}
#endif // GRAPHIC_ENABLE_SHADOW_EFFECT_FLAG
#endif // ENABLE_CANVAS_EXTEND

void DrawCanvas::InitRenderAndTransform(BufferInfo& gfxDstBuffer,
                                        RenderBuffer& renderBuffer,
                                        const Rect& rect,
                                        TransAffine& transform,
                                        const Style& style,
                                        const Paint& paint)
{
    int16_t realLeft = rect.GetLeft() + style.paddingLeft_ + style.borderWidth_;
    int16_t realTop = rect.GetTop() + style.paddingTop_ + style.borderWidth_;
    transform.Reset();
    transform *= paint.GetTransAffine();
    transform.Translate(realLeft, realTop);
    renderBuffer.Attach(static_cast<uint8_t*>(gfxDstBuffer.virAddr), gfxDstBuffer.width, gfxDstBuffer.height,
                        gfxDstBuffer.stride);
}

void DrawCanvas::SetRasterizer(UICanvasVertices& vertices,
                               const Paint& paint,
                               RasterizerScanlineAntialias& rasterizer,
                               TransAffine& transform,
                               const bool& isStroke)
{
    DepictCurve canvasPath(vertices);
    if (isStroke) {
#if defined(GRAPHIC_ENABLE_DASH_GENERATE_FLAG) && GRAPHIC_ENABLE_DASH_GENERATE_FLAG
        if (paint.IsLineDash()) {
            using DashStyle = DepictDash;
            using StrokeDashStyle = DepictStroke<DashStyle>;
            using StrokeDashTransform = DepictTransform<StrokeDashStyle>;
            DashStyle dashStyle(canvasPath);
            LineDashStyleCalc(dashStyle, paint);
            StrokeDashStyle strokeDashStyle(dashStyle);
            LineStyleCalc(strokeDashStyle, paint);
            StrokeDashTransform strokeDashTransform(strokeDashStyle, transform);
            rasterizer.Reset();
            rasterizer.AddPath(strokeDashTransform);
            return;
        }
#endif
        using StrokeLineStyle = DepictStroke<DepictCurve>;
        StrokeLineStyle strokeLineStyle(canvasPath);
        LineStyleCalc(strokeLineStyle, paint);

        DepictTransform<StrokeLineStyle> strokeTransform(strokeLineStyle, transform);
        rasterizer.Reset();
        rasterizer.AddPath(strokeTransform);
    } else {
        DepictTransform<DepictCurve> pathTransform(canvasPath, transform);
        rasterizer.Reset();
        rasterizer.AddPath(pathTransform);
    }
}

#if defined(GRAPHIC_ENABLE_GRADIENT_FILL_FLAG) && GRAPHIC_ENABLE_GRADIENT_FILL_FLAG
void DrawCanvas::RenderGradient(const Paint& paint,
                                RasterizerScanlineAntialias& rasterizer,
                                TransAffine& transform,
                                RenderBase& renBase,
                                RenderBuffer& renderBuffer,
                                FillBase& allocator,
                                const Rect& invalidatedArea)
{
    GeometryScanline scanline;

    RenderPixfmtRgbaBlend pixFormatComp(renderBuffer);
    RenderBase m_renBaseComp(pixFormatComp);

    m_renBaseComp.ResetClipping(true);
    m_renBaseComp.ClipBox(invalidatedArea.GetLeft(), invalidatedArea.GetTop(), invalidatedArea.GetRight(),
                          invalidatedArea.GetBottom());
    TransAffine gradientMatrix;
    FillInterpolator interpolatorType(gradientMatrix);
    FillGradientLut gradientColorMode;
    BuildGradientColor(paint, gradientColorMode);
    if (paint.GetGradient() == Paint::Linear) {
        float distance = 0;
        BuildLineGradientMatrix(paint, gradientMatrix, transform, distance);
        GradientLinearCalculate gradientLinearCalculate;
        FillGradient span(interpolatorType, gradientLinearCalculate, gradientColorMode, 0, distance);
        RenderScanlinesAntiAlias(rasterizer, scanline, renBase, allocator, span);
    }

    if (paint.GetGradient() == Paint::Radial) {
        Paint::RadialGradientPoint radialPoint = paint.GetRadialGradientPoint();
        float startRadius = 0;
        float endRadius = 0;
        BuildRadialGradientMatrix(paint, gradientMatrix, transform, startRadius, endRadius);
        GradientRadialCalculate gradientRadialCalculate(radialPoint.r1, radialPoint.x0 - radialPoint.x1,
                                                        radialPoint.y0 - radialPoint.y1);
        FillGradient span(interpolatorType, gradientRadialCalculate, gradientColorMode, startRadius, endRadius);
        RenderScanlinesAntiAlias(rasterizer, scanline, renBase, allocator, span);
    }
}

void DrawCanvas::BuildGradientColor(const Paint& paint, FillGradientLut& gradientColorMode)
{
    gradientColorMode.RemoveAll();
    ListNode<Paint::StopAndColor>* iter = paint.getStopAndColor().Begin();
    uint16_t count = 0;
    for (; count < paint.getStopAndColor().Size(); count++) {
        ColorType stopColor = iter->data_.color;
        Rgba8T sRgba8;
        ChangeColor(sRgba8, stopColor, stopColor.alpha * paint.GetGlobalAlpha());
        gradientColorMode.AddColor(iter->data_.stop, sRgba8);
        iter = iter->next_;
    }
#if defined(GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG) && GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG
    gradientColorMode.BuildLutNoSort();
#else
    gradientColorMode.BuildLut();
#endif
}

void DrawCanvas::BuildRadialGradientMatrix(const Paint& paint,
                                           TransAffine& gradientMatrix,
                                           TransAffine& transform,
                                           float& startRadius,
                                           float& endRadius)
{
    Paint::RadialGradientPoint radialPoint = paint.GetRadialGradientPoint();
    gradientMatrix.Reset();
    gradientMatrix *= TransAffine::TransAffineTranslation(radialPoint.x1, radialPoint.y1);
    gradientMatrix *= transform;
    gradientMatrix.Invert();
    startRadius = radialPoint.r0;
    endRadius = radialPoint.r1;
}
#endif // GRAPHIC_ENABLE_GRADIENT_FILL_FLAG

#if defined(GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG) && GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG
void DrawCanvas::RenderGradientFill(BufferInfo& gfxDstBuffer,
                                    UICanvasVertices& vertices,
                                    const Paint& paint,
                                    const Rect& rect,
                                    const Rect& invalidatedArea)
{
    /* Default style: zero padding and border width, so the transform only
     * translates the path to the origin of the target rect. */
    Style style;
    TransAffine transform;
    RenderBuffer renderBuffer;
    InitRenderAndTransform(gfxDstBuffer, renderBuffer, rect, transform, style, paint);

    RasterizerScanlineAntialias rasterizer;
    rasterizer.ClipBox(0, 0, gfxDstBuffer.width, gfxDstBuffer.height);
    SetRasterizer(vertices, paint, rasterizer, transform, false);

    RenderPixfmtRgbaBlend pixFormat(renderBuffer);
    RenderBase renBase(pixFormat);
    FillBase allocator;

    renBase.ResetClipping(true);
    renBase.ClipBox(invalidatedArea.GetLeft(), invalidatedArea.GetTop(), invalidatedArea.GetRight(),
                    invalidatedArea.GetBottom());

    RenderGradient(paint, rasterizer, transform, renBase, renderBuffer, allocator, invalidatedArea);
}
#endif // GRAPHIC_ENABLE_COMPONENT_GRADIENT_FLAG

#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
void DrawCanvas::RenderPattern(const Paint& paint,
                               void* param,
                               RasterizerScanlineAntialias& rasterizer,
                               RenderBase& renBase,
                               FillBase& allocator,
                               const Rect& rect)
{
    if (param == nullptr) {
        return;
    }
    ImageParam* imageParam = static_cast<ImageParam*>(param);
    if (imageParam->image == nullptr) {
        return;
    }
    GeometryScanline scanline;
    FillPatternRgba spanPattern(imageParam->image->GetImageInfo(), paint.GetPatternRepeatMode(), rect.GetLeft(),
                                rect.GetTop());
    RenderScanlinesAntiAlias(rasterizer, scanline, renBase, allocator, spanPattern);
}
#endif
#endif // GRAPHIC_ENABLE_PATTERN_FILL_FLAG

} // namespace OHOS
