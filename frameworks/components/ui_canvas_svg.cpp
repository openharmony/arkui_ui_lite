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

#include "components/ui_canvas.h"

#include "draw/draw_arc.h"
#include "gfx_utils/graphic_log.h"
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
#include "draw/draw_canvas.h"
#endif

namespace OHOS {
// Used to preallocate the text measurement buffer in DrawSvgText.
constexpr uint16_t SVG_TEXT_MEASURE_MAX = 128;
// Number of coordinates per vertex in the Bezier arc output (x, y).
constexpr uint8_t VERTEX_COORD_COUNT = 2;

// Number of control points for a cubic Bezier curve (two control points plus end point).
constexpr uint8_t CUBIC_BEZIER_POINT_COUNT = 3;
constexpr uint8_t CUBIC_BEZIER_P0_INDEX = 0;
constexpr uint8_t CUBIC_BEZIER_P1_INDEX = 1;
constexpr uint8_t CUBIC_BEZIER_P2_INDEX = 2;

// Creates an independent copy of the vertex list so that each enqueued draw command owns its
// geometry. FillPath and DrawPath for a single shape both reference the canvas' shared vertices_
// buffer; without a per-command copy, DeletePathParam would double-free that single buffer.
// Each PathParam now holds its own clone and is deleted exactly once.
UICanvasVertices* UICanvas::CloneVertices(UICanvasVertices* src)
{
    if (src == nullptr) {
        return nullptr;
    }
    UICanvasVertices* dst = new UICanvasVertices();
    if (dst == nullptr) {
        return nullptr;
    }
    float x = 0.0f;
    float y = 0.0f;
    uint32_t cmd = 0;
    float curveX[CUBIC_BEZIER_POINT_COUNT] = {0.0f, 0.0f, 0.0f};
    float curveY[CUBIC_BEZIER_POINT_COUNT] = {0.0f, 0.0f, 0.0f};
    int curveCount = 0;
    src->Rewind(0);
    while (!IsStop(cmd = src->GenerateVertex(&x, &y))) {
        switch (cmd) {
            case PATH_CMD_MOVE_TO:
                dst->MoveTo(x, y);
                break;
            case PATH_CMD_LINE_TO:
                dst->LineTo(x, y);
                break;
            case PATH_CMD_CURVE4:
                curveX[curveCount] = x;
                curveY[curveCount] = y;
                curveCount++;
                if (curveCount == CUBIC_BEZIER_POINT_COUNT) {
                    dst->CubicBezierCurve(curveX[CUBIC_BEZIER_P0_INDEX], curveY[CUBIC_BEZIER_P0_INDEX],
                                          curveX[CUBIC_BEZIER_P1_INDEX], curveY[CUBIC_BEZIER_P1_INDEX],
                                          curveX[CUBIC_BEZIER_P2_INDEX], curveY[CUBIC_BEZIER_P2_INDEX]);
                    curveCount = 0;
                }
                break;
            case PATH_CMD_END_POLY:
            case (PATH_CMD_END_POLY | PATH_FLAGS_CLOSE):
                dst->ClosePolygon();
                break;
            default:
                break;
        }
    }
    src->Rewind(0);
    return dst;
}

void UICanvas::CubicBezierTo(const Point& control1, const Point& control2, const Point& endPoint)
{
    if (vertices_ == nullptr) {
        return;
    }
    /* Stored as a real Bezier segment; the rasterizer flattens it adaptively. */
    vertices_->CubicBezierCurve(control1.x, control1.y, control2.x, control2.y, endPoint.x, endPoint.y);
}
void UICanvas::SvgArcTo(const SvgArcArgs& args)
{
    if (vertices_ == nullptr) {
        return;
    }
#if defined(GRAPHIC_ENABLE_BEZIER_ARC_FLAG) && GRAPHIC_ENABLE_BEZIER_ARC_FLAG
    BezierArcSvg bezierArc(args.startX, args.startY, args.rx, args.ry, args.rotation,
                           args.largeArc, args.sweep, args.x, args.y);
    if (!bezierArc.RadiiOK()) {
        vertices_->LineTo(args.x, args.y);
        return;
    }
    if (args.transform != nullptr) {
        float* vertices = bezierArc.GetVertices();
        uint32_t count = bezierArc.GetNumberVertices();
        for (uint32_t i = 0; i < count; i += VERTEX_COORD_COUNT) { // pairs of (x, y)
            args.transform->Transform(&vertices[i], &vertices[i + 1]);
        }
    }
    vertices_->JoinPath(bezierArc);
#else
    vertices_->LineTo(args.x, args.y);
#endif
}

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
void UICanvas::BeginSvgPath()
{
    if (vertices_ != nullptr) {
        delete vertices_;
        vertices_ = nullptr;
    }
    vertices_ = new UICanvasVertices();
    if (vertices_ == nullptr) {
        GRAPHIC_LOGE("new UICanvasVertices fail");
        return;
    }
}
#endif

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
static ImageParam* CreatePatternImageParam(const Paint& paint, PathParam* pathParam)
{
    ImageParam* imageParam = new ImageParam;
    if (imageParam == nullptr) {
        GRAPHIC_LOGE("new ImageParam fail");
        return nullptr;
    }
    imageParam->image = new Image();
    if (imageParam->image == nullptr) {
        delete imageParam;
        return nullptr;
    }
    SetImageParamInfo(imageParam, paint, pathParam);
    return imageParam;
}

static bool AttachPatternImage(PathParam* pathParam, const Paint& paint)
{
    if (paint.GetStyle() != Paint::PATTERN) {
        return true;
    }
    if (CreatePatternImageParam(paint, pathParam) == nullptr) {
        return false;
    }
    return true;
}
#endif
#endif

void UICanvas::DrawPathSvg(const Paint& paint)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    if (vertices_ == nullptr) {
        return;
    }

    PathParam* pathParam = new PathParam;
    if (pathParam == nullptr) {
        GRAPHIC_LOGE("new PathParam fail");
        return;
    }

    pathParam->vertices = CloneVertices(vertices_);
    if (pathParam->vertices == nullptr) {
        GRAPHIC_LOGE("CloneVertices fail");
        delete pathParam;
        return;
    }
    pathParam->isStroke = true;
    pathParam->isSvg = true;
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
    if (!AttachPatternImage(pathParam, paint)) {
        DeletePathParam(pathParam);
        return;
    }
#endif
    DrawCmd cmd;
    cmd.paint = paint;
    cmd.param = pathParam;
    cmd.DeleteParam = DeletePathParam;
    cmd.DrawGraphics = DoDrawPath;
    drawCmdList_.PushBack(cmd);
    Invalidate();
}
#endif
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
void UICanvas::FillPathSvg(const Paint& paint)
{
    if (vertices_ == nullptr) {
        return;
    }

    PathParam* pathParam = new PathParam;
    if (pathParam == nullptr) {
        GRAPHIC_LOGE("new PathParam fail");
        return;
    }

    pathParam->vertices = CloneVertices(vertices_);
    if (pathParam->vertices == nullptr) {
        GRAPHIC_LOGE("CloneVertices fail");
        delete pathParam;
        return;
    }
    pathParam->isStroke = false;
    pathParam->isSvg = true;
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
    if (!AttachPatternImage(pathParam, paint)) {
        DeletePathParam(pathParam);
        return;
    }
#endif
    DrawCmd cmd;
    cmd.paint = paint;
    cmd.param = pathParam;
    cmd.DeleteParam = DeletePathParam;
    cmd.DrawGraphics = DoFillPath;
    drawCmdList_.PushBack(cmd);
    Invalidate();
}
#endif
#if defined(GRAPHIC_ENABLE_DRAW_TEXT_FLAG) && GRAPHIC_ENABLE_DRAW_TEXT_FLAG
void UICanvas::StrokeText(const char* text, const Point& point, const FontStyle& fontStyle, const Paint& paint,
                          const SvgTextDrawInfo& svgInfo)
{
    if (text == nullptr) {
        return;
    }
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::FILL_STYLE) {
        TextParam* textParam = new TextParam;
        if (textParam == nullptr) {
            GRAPHIC_LOGE("new TextParam fail");
            return;
        }
        textParam->text = text;
        textParam->fontStyle = fontStyle;
        textParam->fontOpa = paint.GetOpacity();
        textParam->fontColor = paint.GetFillColor();
        textParam->position = point;
        textParam->svgInfo = svgInfo;
        DrawCmd cmd;
        cmd.param = textParam;
        cmd.DeleteParam = DeleteTextParam;
        cmd.DrawGraphics = DoDrawTextSvg;
        cmd.paint = paint;
        drawCmdList_.PushBack(cmd);
        Invalidate();
        SetStartPosition(point);
    }
}
void UICanvas::DoDrawTextSvg(BufferInfo& gfxDstBuffer,
                             void* param,
                             const Paint& paint,
                             const Rect& rect,
                             const Rect& invalidatedArea,
                             const Style& style)
{
    TextParam* textParam = static_cast<TextParam*>(param);
    if (textParam == nullptr) {
        return;
    }
    if (textParam->fontStyle.fontSize <= 0) {
        return;
    }
    Text* text = textParam->textComment;
    TextDrawSetup setup = PrepareTextDrawSetup(textParam, text, rect, invalidatedArea, style);
    PrepareSvgTextMeasurement(textParam->svgInfo, paint, text, setup.start, setup.textRect);
    text->ReMeasureTextSize(setup.textRect, setup.drawStyle);
    if (text->GetTextSize().x == 0 || text->GetTextSize().y == 0) {
        return;
    }

    OpacityType opa = OPA_OPAQUE;
    TextDrawArgs args{gfxDstBuffer, text, setup.textRect, invalidatedArea, setup.drawStyle, opa};
    DrawSvgText(args, paint, textParam, style);
}
#endif

bool UICanvas::EnqueueCircleCmd(const Point& center, uint16_t radius, const Paint& paint)
{
    CircleParam* circleParam = new CircleParam;
    if (circleParam == nullptr) {
        GRAPHIC_LOGE("new CircleParam fail");
        return false;
    }
    circleParam->center = center;
    circleParam->radius = radius;

    DrawCmd cmd;
    cmd.paint = paint;
    cmd.param = circleParam;
    cmd.DeleteParam = DeleteCircleParam;
    cmd.DrawGraphics = DoDrawCircle;
    drawCmdList_.PushBack(cmd);
    return true;
}

void UICanvas::DrawCircleSvg(const Point& center, uint16_t radius, const Paint& paint)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    if (paint.GetChangeFlag()) {
#if defined(GRAPHIC_ENABLE_BEZIER_ARC_FLAG) && GRAPHIC_ENABLE_BEZIER_ARC_FLAG
        BeginSvgPath();
        BezierArc arc(center.x, center.y, radius, radius, 0, TWO_TIMES * PI);
        vertices_->ConcatPath(arc, 0);
        vertices_->ClosePolygon();
        if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::STROKE_STYLE) {
            DrawPathSvg(paint);
        }
        if (PaintStyleIncludesFill(paint)) {
            FillPathSvg(paint);
        }
#endif
    } else {
        if (!EnqueueCircleCmd(center, radius, paint)) {
            return;
        }
    }
#else
    if (!EnqueueCircleCmd(center, radius, paint)) {
        return;
    }
#endif
    Invalidate();
}

UICanvas::TextDrawSetup UICanvas::PrepareTextDrawSetup(TextParam* textParam, Text* text, const Rect& rect,
                                                       const Rect& invalidatedArea, const Style& style)
{
    text->SetText(textParam->text);
    text->SetFont(textParam->fontStyle.fontName, textParam->fontStyle.fontSize);
    text->SetDirect(static_cast<UITextLanguageDirect>(textParam->fontStyle.direct));
    text->SetAlign(static_cast<UITextLanguageAlignment>(textParam->fontStyle.align));

    Point start;
    Rect textRect = invalidatedArea;
    GetAbsolutePosition(textParam->position, rect, style, start);
    textRect.SetPosition(start.x, start.y);
    Style drawStyle = style;
    drawStyle.textColor_ = textParam->fontColor;
    drawStyle.lineColor_ = textParam->fontColor;
    drawStyle.bgColor_ = textParam->fontColor;
    drawStyle.SetStyle(STYLE_LETTER_SPACE, textParam->fontStyle.letterSpace);
    return {start, textRect, drawStyle};
}
#if defined(GRAPHIC_ENABLE_DRAW_TEXT_FLAG) && GRAPHIC_ENABLE_DRAW_TEXT_FLAG
void UICanvas::PrepareSvgTextMeasurement(const SvgTextDrawInfo& svgInfo, const Paint& paint, Text* text,
                                         const Point& start, Rect& textRect)
{
    if (text == nullptr || !svgInfo.isSvgText || paint.GetTransAffine().IsIdentity()) {
        return;
    }
    // SVG <text> inside a rotated group needs the glyph bitmap to rotate with the
    // group. The Text engine's baseline-true path shifts the ink box and makes the
    // bitmap rotate around the wrong pivot, so disable it only when a rotation is
    // actually present. Axis-aligned SVG text keeps the original baseline behaviour.
    text->SetSupportBaseLine(false);

    // For rotated SVG <text> the leaf bounds passed in invalidatedArea are the axis-aligned
    // bounding box of the rotated ink box.  Measuring the unrotated glyph run against that
    // box clips it to the AABB (e.g. a tall/narrow rect near 90 degrees), which makes the
    // trailing letters disappear after rotation.  Use a large measurement rect so the text
    // engine returns the natural, unrotated line size.
    textRect.SetLeft(start.x);
    textRect.SetTop(start.y);
    textRect.SetWidth(SVG_TEXT_MEASURE_MAX);
    textRect.SetHeight(SVG_TEXT_MEASURE_MAX);
}

void UICanvas::ApplySvgInkOffsetToDrawRect(const SvgTextDrawInfo& svgInfo, const Rect& imageRect, Rect& drawRect)
{
    if (!svgInfo.isSvgText) {
        return;
    }
    int16_t leftOffset = svgInfo.inkLeftOffset;
    int16_t topOffset = svgInfo.inkTopOffset;
    if (leftOffset == 0 && topOffset == 0) {
        return;
    }
    // For rotated SVG <text> the leaf bounds are sized to the actual glyph ink box.
    // Shift the glyph bitmap inside the map buffer so the rotation pivot sits on
    // the real ink top-left and the whole ink is drawn into the buffer.
    drawRect.SetLeft(-leftOffset);
    drawRect.SetRight(imageRect.GetRight() - leftOffset);
    drawRect.SetTop(-topOffset);
    drawRect.SetBottom(imageRect.GetBottom() - topOffset);
}

void UICanvas::UpdateTextRectSize(TextParam* textParam, Text* text, Rect& textRect, OpacityType& opa,
                                  const Style& style)
{
    if (textParam->svgInfo.isSvgText) {
        textRect.SetWidth(text->GetTextSize().x + textParam->svgInfo.inkLeftOffset +
                          textParam->svgInfo.inkRightOffset + 1);
        textRect.SetHeight(text->GetTextSize().y + textParam->svgInfo.inkTopOffset +
                           textParam->svgInfo.inkBottomOffset + 1);
        opa = textParam->fontOpa;
    } else {
        textRect.SetWidth(text->GetTextSize().x + 1);
        textRect.SetHeight(text->GetTextSize().y + 1);
        opa = DrawUtils::GetMixOpacity(textParam->fontOpa, style.bgOpa_);
    }
}

void UICanvas::DrawSvgText(TextDrawArgs& args, const Paint& paint, TextParam* textParam, const Style& style)
{
    UpdateTextRectSize(textParam, args.text, args.textRect, args.opa, style);
    if (!paint.GetTransAffine().IsIdentity()) {
        SvgTextTransformArgs transformArgs{paint, textParam->svgInfo};
        DrawTextWithTransform(args, transformArgs);
    } else {
        args.text->OnDraw(args.gfxDstBuffer, args.invalidatedArea, args.textRect, args.textRect, 0,
                          args.drawStyle, Text::TEXT_ELLIPSIS_END_INV, args.opa);
    }
}

void UICanvas::DrawTextWithTransform(TextDrawArgs& args, const SvgTextTransformArgs& transformArgs)
{
    Rect textImageRect(0, 0, args.textRect.GetWidth(), args.textRect.GetHeight());
    BufferInfo* mapBufferInfo = UpdateMapBufferInfo(args.gfxDstBuffer, textImageRect);
    if (mapBufferInfo == nullptr) {
        return;
    }
    Rect drawRect = textImageRect;
    ApplySvgInkOffsetToDrawRect(transformArgs.svgInfo, textImageRect, drawRect);
    args.text->OnDraw(*mapBufferInfo, textImageRect, textImageRect, drawRect, 0, args.drawStyle,
                      Text::TEXT_ELLIPSIS_END_INV, args.opa);
    TransformMap trans;
    trans.SetTransMapRect(args.textRect);
    trans.Scale(Vector2<float>(static_cast<float>(transformArgs.paint.GetScaleX()),
                               static_cast<float>(transformArgs.paint.GetScaleY())),
                Vector2<float>(0, 0));
    float angle = transformArgs.paint.GetRotateAngle();
    trans.Rotate(MATH_ROUND(angle), Vector2<float>(0, 0));
    trans.Translate(Vector2<int16_t>(transformArgs.paint.GetTranslateX(), transformArgs.paint.GetTranslateY()));
    BlitMapBuffer(args.gfxDstBuffer, *mapBufferInfo, args.textRect, trans, args.invalidatedArea);
}
#endif


bool UICanvas::PaintStyleIncludesFill(const Paint& paint)
{
    Paint::PaintStyle style = paint.GetStyle();
    bool hasFill = (static_cast<uint8_t>(style) & static_cast<uint8_t>(Paint::FILL_STYLE)) != 0;
#if defined(GRAPHIC_ENABLE_GRADIENT_FILL_FLAG) && GRAPHIC_ENABLE_GRADIENT_FILL_FLAG
    hasFill = hasFill || (style == Paint::GRADIENT);
#endif
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
    hasFill = hasFill || (style == Paint::PATTERN);
#endif
    return hasFill;
}

void UICanvas::DrawEllipse(const Point& center, uint16_t radiusX, uint16_t radiusY, const Paint& paint)
{
#if defined(GRAPHIC_ENABLE_BEZIER_ARC_FLAG) && GRAPHIC_ENABLE_BEZIER_ARC_FLAG
    // Uses the same Bezier-arc primitive as DrawCircle but with independent radii, so the
    // ellipse is a continuous curve (no per-vertex miter joins) and stays smooth.
    BeginSvgPath();
    BezierArc arc(center.x, center.y, radiusX, radiusY, 0, TWO_TIMES * PI);
    vertices_->ConcatPath(arc, 0);
    vertices_->ClosePolygon();
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::STROKE_STYLE) {
        DrawPathSvg(paint);
    }
    if (PaintStyleIncludesFill(paint)) {
        FillPathSvg(paint);
    }
#endif
    Invalidate();
}

} // namespace OHOS
