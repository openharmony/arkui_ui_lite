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
#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
#include "gfx_utils/diagram/spancolorfill/fill_gradient_svg.h"
#endif
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

#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
void RenderSolidSvg(const Paint& paint, RasterizerScanlineAntialias& rasterizer, RenderBase& renBase,
                    const bool& isStroke)
{
    GeometryScanline scanline;
    Rgba8T color;
    DrawCanvas::RenderBlendSolidSvg(paint, color, isStroke);
    RenderScanlinesAntiAliasSolid(rasterizer, scanline, renBase, color);
}
#endif

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
static void RenderPathPaintStyle(const Paint& paint,
                                 PathParam* pathParam,
                                 RasterizerScanlineAntialias& rasterizer,
                                 TransAffine& transform,
                                 RenderBase& renBase,
                                 RenderBuffer& renderBuffer,
                                 FillBase& allocator,
                                 const Rect& invalidatedArea,
                                 const Rect& rect,
                                 const bool& isStroke)
{
    if (paint.GetStyle() == Paint::STROKE_STYLE || paint.GetStyle() == Paint::FILL_STYLE ||
        paint.GetStyle() == Paint::STROKE_FILL_STYLE) {
#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
        if (pathParam->isSvg) {
            RenderSolidSvg(paint, rasterizer, renBase, isStroke);
        } else {
            RenderSolid(paint, rasterizer, renBase, isStroke);
        }
#else
        RenderSolid(paint, rasterizer, renBase, isStroke);
#endif
    }

#if defined(GRAPHIC_ENABLE_GRADIENT_FILL_FLAG) && GRAPHIC_ENABLE_GRADIENT_FILL_FLAG
    if (paint.GetStyle() == Paint::GRADIENT) {
#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
        if (pathParam->isSvg) {
            DrawCanvas::RenderGradientSvg(paint, rasterizer, transform, renBase, renderBuffer, allocator,
                invalidatedArea);
        } else {
            DrawCanvas::RenderGradient(paint, rasterizer, transform, renBase, renderBuffer, allocator, invalidatedArea);
        }
#else
        DrawCanvas::RenderGradient(paint, rasterizer, transform, renBase, renderBuffer, allocator, invalidatedArea);
#endif
    }
#endif
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
    if (paint.GetStyle() == Paint::PATTERN) {
        DrawCanvas::RenderPattern(paint, pathParam->imageParam, rasterizer, renBase, allocator, rect);
    }
#endif
}

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
#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
    if (pathParam->isSvg && !isStroke) {
        rasterizer.SetFillingRule(paint.GetFillingRule());
    }
#endif

    RenderPixfmtRgbaBlend pixFormat(renderBuffer);
    RenderBase renBase(pixFormat);
    FillBase allocator;

    renBase.ResetClipping(true);
    renBase.ClipBox(invalidatedArea.GetLeft(), invalidatedArea.GetTop(), invalidatedArea.GetRight(),
                    invalidatedArea.GetBottom());

    RenderPathPaintStyle(paint, pathParam, rasterizer, transform, renBase, renderBuffer, allocator,
                         invalidatedArea, rect, isStroke);
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
#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
    if (pathParam->isSvg && !isStroke) {
        rasterizer.SetFillingRule(paint.GetFillingRule());
    }
#endif
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

#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
void DrawCanvas::RenderGradientSvg(const Paint& paint, RasterizerScanlineAntialias& rasterizer, TransAffine& transform,
    RenderBase& renBase, RenderBuffer& renderBuffer, FillBase& allocator, const Rect& invalidatedArea)
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
    BuildGradientColorSvg(paint, gradientColorMode);
    if (paint.GetGradient() == Paint::Linear) {
        float distance = 0;
        BuildLineGradientMatrixSvg(paint, gradientMatrix, transform, distance);
        GradientLinearCalculateSvg gradientLinearCalculate;
        FillGradientSvg span(interpolatorType, gradientLinearCalculate, gradientColorMode, 0, distance);
        RenderScanlinesAntiAlias(rasterizer, scanline, renBase, allocator, span);
    }

    if (paint.GetGradient() == Paint::Radial) {
        Paint::RadialGradientPoint radialPoint = paint.GetRadialGradientPoint();
        float startRadius = 0;
        float endRadius = 0;
        BuildRadialGradientMatrixSvg(paint, gradientMatrix, transform, startRadius, endRadius);
        float scaleX = radialPoint.scaleX;
        float scaleY = radialPoint.scaleY;
        if (scaleX < 1e-6f) {
            scaleX = 1.0f;
        }
        if (scaleY < 1e-6f) {
            scaleY = 1.0f;
        }
        float dx = (radialPoint.x0 - radialPoint.x1) / scaleX;
        float dy = (radialPoint.y0 - radialPoint.y1) / scaleY;
        GradientRadialCalculateSvg gradientRadialCalculate(radialPoint.r1, dx, dy);
        FillGradientSvg span(interpolatorType, gradientRadialCalculate, gradientColorMode, startRadius, endRadius);
        RenderScanlinesAntiAlias(rasterizer, scanline, renBase, allocator, span);
    }
}
#endif

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

#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
void DrawCanvas::BuildGradientColorSvg(const Paint& paint, FillGradientLut& gradientColorMode)
{
    gradientColorMode.RemoveAll();
    ListNode<Paint::StopAndColor>* iter = paint.getStopAndColor().Begin();
    uint16_t count = 0;
    Rgba8T firstColor;
    Rgba8T lastColor;
    float firstOffset = 0.0f;
    bool allSameOffset = true;
    const float offsetEpsilon = 1e-6f;
    for (; count < paint.getStopAndColor().Size(); count++) {
        ColorType stopColor = iter->data_.color;
        Rgba8T sRgba8;
        ChangeColor(sRgba8, stopColor, stopColor.alpha * paint.GetGlobalAlpha());
        if (count == 0) {
            firstColor = sRgba8;
            firstOffset = iter->data_.stop;
        }
        lastColor = sRgba8;
        float offsetDiff = iter->data_.stop - firstOffset;
        if (offsetDiff > offsetEpsilon || offsetDiff < -offsetEpsilon) {
            allSameOffset = false;
        }
        gradientColorMode.AddColor(iter->data_.stop, sRgba8);
        iter = iter->next_;
    }
    /*
     * Degenerate case: all stops collapse to a single offset (e.g. SVG stop offsets
     * clamped to 1.0). FillGradientLut::BuildLut de-duplicates them into one entry
     * and then skips building the lookup table, leaving it uninitialized. Replace
     * them with a solid fill so the LUT is built: a group at the end is preceded by
     * the first stop's color, a group at the start is followed by the last stop's.
     */
    if (count > 0 && allSameOffset) {
        gradientColorMode.RemoveAll();
        Rgba8T solidColor = (firstOffset >= 0.5f) ? firstColor : lastColor;
        gradientColorMode.AddColor(0.0f, solidColor);
        gradientColorMode.AddColor(1.0f, solidColor);
    }
    gradientColorMode.BuildLut();
}
#endif

#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
void DrawCanvas::BuildLineGradientMatrixSvg(const Paint& paint,
                                            TransAffine& gradientMatrix,
                                            TransAffine& transform,
                                            float& distance)
{
    Paint::LinearGradientPoint linearPoint = paint.GetLinearGradientPoint();
    float dx = linearPoint.x1 - linearPoint.x0;
    float dy = linearPoint.y1 - linearPoint.y0;
    /* For SVG objectBoundingBox gradients the endpoints have been mapped to
     * record space. The projection must be performed in the unit gradient
     * coordinate system, so rotate/scale by the inverse bounding-box size. */
    float scaleX = paint.GetLinearGradientScaleX();
    float scaleY = paint.GetLinearGradientScaleY();
    if (scaleX < 1e-6f) {
        scaleX = 1.0f;
    }
    if (scaleY < 1e-6f) {
        scaleY = 1.0f;
    }
    dx /= scaleX;
    dy /= scaleY;
    float angle = FastAtan2F(dy, dx);
    gradientMatrix.Reset();
    gradientMatrix *= TransAffine::TransAffineRotation(angle);
    gradientMatrix *= TransAffine::TransAffineScaling(scaleX, scaleY);
    gradientMatrix *= TransAffine::TransAffineTranslation(linearPoint.x0, linearPoint.y0);
    gradientMatrix *= transform;
    gradientMatrix.Invert();
    distance = Sqrt(dx * dx + dy * dy);
}
#endif

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

#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
void DrawCanvas::BuildRadialGradientMatrixSvg(const Paint& paint,
                                              TransAffine& gradientMatrix,
                                              TransAffine& transform,
                                              float& startRadius,
                                              float& endRadius)
{
    Paint::RadialGradientPoint radialPoint = paint.GetRadialGradientPoint();
    gradientMatrix.Reset();
    gradientMatrix *= TransAffine::TransAffineScaling(radialPoint.scaleX, radialPoint.scaleY);
    gradientMatrix *= TransAffine::TransAffineTranslation(radialPoint.x1, radialPoint.y1);
    gradientMatrix *= transform;
    gradientMatrix.Invert();
    startRadius = radialPoint.r0;
    endRadius = radialPoint.r1;
}
#endif
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

#if defined(FEATURE_COMPONENT_SVG) && FEATURE_COMPONENT_SVG
void DrawCanvas::RenderBlendSolidSvg(const Paint& paint, Rgba8T& color, const bool& isStroke)
{
    if (isStroke) {
        if (paint.GetStyle() == Paint::STROKE_STYLE || paint.GetStyle() == Paint::STROKE_FILL_STYLE) {
            uint16_t blended = static_cast<uint16_t>(paint.GetStrokeColor().alpha) *
                               paint.GetOpacity() / OPA_OPAQUE;
            uint16_t finalAlpha = static_cast<uint16_t>(blended * paint.GetGlobalAlpha());
            ChangeColor(color, paint.GetStrokeColor(),
                        static_cast<uint8_t>(MATH_MIN(finalAlpha, OPA_OPAQUE)));
        }
    } else {
        if (paint.GetStyle() == Paint::FILL_STYLE || paint.GetStyle() == Paint::STROKE_FILL_STYLE) {
            uint16_t blended = static_cast<uint16_t>(paint.GetFillColor().alpha) *
                               paint.GetOpacity() / OPA_OPAQUE;
            uint16_t finalAlpha = static_cast<uint16_t>(blended * paint.GetGlobalAlpha());
            ChangeColor(color, paint.GetFillColor(),
                        static_cast<uint8_t>(MATH_MIN(finalAlpha, OPA_OPAQUE)));
        }
    }
}
#endif

} // namespace OHOS
