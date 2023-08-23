/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
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

#include "draw/clip_utils.h"
#include "draw/draw_arc.h"
#include "draw/draw_image.h"
#include "gfx_utils/graphic_log.h"
#include "render/render_buffer.h"
#include "render/render_pixfmt_rgba_blend.h"
#include "render/render_scanline.h"
#if ( (defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND) )
#include "draw/draw_canvas.h"
#endif

namespace OHOS {
UICanvas::UICanvasPath::~UICanvasPath()
{
    points_.Clear();
    cmd_.Clear();
    arcParam_.Clear();
}

BufferInfo* UICanvas::gfxMapBuffer_ = nullptr;

void RenderSolid(const Paint& paint,
                 RasterizerScanlineAntialias& rasterizer,
                 RenderBase& renBase,
                 const bool& isStroke);

void UICanvas::BeginPath()
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    /* If the previous path is not added to the drawing linked list, it should be destroyed directly. */
    if (vertices_ != nullptr && vertices_->GetTotalVertices() == 0) {
        delete vertices_;
        vertices_ = nullptr;
    }

    vertices_ = new UICanvasVertices();
    if (vertices_ == nullptr) {
        GRAPHIC_LOGE("new UICanvasVertices fail");
        return;
    }
#else
    if (path_ != nullptr && path_->strokeCount_ == 0) {
        delete path_;
        path_ = nullptr;
    }

    path_ = new UICanvasPath();
    if (path_ == nullptr) {
        GRAPHIC_LOGE("new UICanvasPath fail");
        return;
    }
#endif
}

void UICanvas::MoveTo(const Point& point)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    if (vertices_ == nullptr) {
        return;
    }
    vertices_->MoveTo(point.x, point.y);
#else
    if (path_ == nullptr) {
        return;
    }

    path_->startPos_ = point;
    /* If the previous command is also CMD_MOVE_TO, the previous command is overwritten. */
    if ((path_->cmd_.Size() != 0) && (path_->cmd_.Tail()->data_ == CMD_MOVE_TO)) {
        path_->points_.Tail()->data_ = point;
        return;
    }
    path_->points_.PushBack(point);
    path_->cmd_.PushBack(CMD_MOVE_TO);
#endif
}

void UICanvas::LineTo(const Point& point)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    if (vertices_ == nullptr) {
        return;
    }
    vertices_->LineTo(point.x, point.y);
#else
    if (path_ == nullptr) {
        return;
    }

    path_->points_.PushBack(point);
    if (path_->cmd_.Size() == 0) {
        path_->startPos_ = point;
        path_->cmd_.PushBack(CMD_MOVE_TO);
    } else {
        path_->cmd_.PushBack(CMD_LINE_TO);
    }
#endif
}

void UICanvas::ArcTo(const Point& center, uint16_t radius, int16_t startAngle, int16_t endAngle)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    if (vertices_ == nullptr || startAngle == endAngle) {
        return;
    }
    float sinma = radius * Sin(startAngle);
    float cosma = radius * Sin(QUARTER_IN_DEGREE - startAngle);
    if (vertices_->GetTotalVertices() != 0) {
        vertices_->LineTo(float(center.x + sinma), float(center.y - cosma));
    } else {
        vertices_->MoveTo(float(center.x + sinma), float(center.y - cosma));
    }
    if (MATH_ABS(startAngle - endAngle) < CIRCLE_IN_DEGREE) {
        sinma = radius * Sin(endAngle);
        cosma = radius * Sin(QUARTER_IN_DEGREE - endAngle);
    }
    int16_t angle = endAngle - startAngle;
    bool largeArcFlag = false;
    if (angle > SEMICIRCLE_IN_DEGREE || angle <= 0) {
        largeArcFlag = true;
    }
    vertices_->ArcTo(radius, radius, angle, largeArcFlag, 1, float(center.x + sinma), float(center.y - cosma));
#else
    if (path_ == nullptr) {
        return;
    }

    /*
     * If there is no command before CMD_ARC, only the arc is drawn. If there is a command in front of
     * CMD_ARC, the start point of arc must be connected to the end point of the path.
     */
    float sinma = radius * Sin(startAngle);
    float cosma = radius * Sin(QUARTER_IN_DEGREE - startAngle);
    if (path_->cmd_.Size() != 0) {
        path_->points_.PushBack({MATH_ROUND(center.x + sinma), MATH_ROUND(center.y - cosma)});
        path_->cmd_.PushBack(CMD_LINE_TO);
    } else {
        path_->startPos_ = {MATH_ROUND(center.x + sinma), MATH_ROUND(center.y - cosma)};
    }

    /* If the ARC scan range exceeds 360 degrees, the end point of the path is the position of the start angle. */
    if (MATH_ABS(startAngle - endAngle) < CIRCLE_IN_DEGREE) {
        sinma = radius * Sin(endAngle);
        cosma = radius * Sin(QUARTER_IN_DEGREE - endAngle);
    }
    path_->points_.PushBack({MATH_ROUND(center.x + sinma), MATH_ROUND(center.y - cosma)});
    path_->cmd_.PushBack(CMD_ARC);

    SetArcParamInfo(center, radius, startAngle, endAngle);
#endif
}

void UICanvas::SetArcParamInfo(const Point& center, uint16_t radius, int16_t startAngle, int16_t endAngle)
{
    int16_t start;
    int16_t end;
    if (startAngle > endAngle) {
        start = endAngle;
        end = startAngle;
    } else {
        start = startAngle;
        end = endAngle;
    }

    DrawArc::GetInstance()->GetDrawRange(start, end);
    ArcParam param;
    param.center = center;
    param.radius = radius;
    param.startAngle = start;
    param.endAngle = end;
    path_->arcParam_.PushBack(param);
}

void UICanvas::AddRect(const Point& point, int16_t height, int16_t width)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    if (vertices_ == nullptr) {
        return;
    }

    int16_t right = static_cast<int16_t>(point.x + width);
    int16_t bottom = point.y + height;

    float fRight = static_cast<float>(width) + static_cast<float>(point.x);
    float fBottom = static_cast<float>(height) + static_cast<float>(point.y);
    const int16_t setup = 3;
    if (fRight > INT16_MAX) {
        right += setup;
    }
    if (fBottom > INT16_MAX) {
        bottom += setup;
    }

    MoveTo(point);
    LineTo({right, point.y});
    LineTo({right, bottom});
    LineTo({point.x, bottom});
#else
    if (path_ == nullptr) {
        return;
    }

    MoveTo(point);
    LineTo({static_cast<int16_t>(point.x + width), point.y});
    LineTo({static_cast<int16_t>(point.x + width), static_cast<int16_t>(point.y + height)});
    LineTo({point.x, static_cast<int16_t>(point.y + height)});

#endif
    ClosePath();
}

void UICanvas::ClosePath()
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    if (vertices_ == nullptr) {
        return;
    }
    vertices_->ClosePolygon();
#else
    if ((path_ == nullptr) || (path_->cmd_.Size() == 0)) {
        return;
    }

    path_->points_.PushBack(path_->startPos_);
    path_->cmd_.PushBack(CMD_CLOSE);
#endif
}

UICanvas::~UICanvas()
{
    if ((path_ != nullptr) && (path_->strokeCount_ == 0)) {
        delete path_;
        path_ = nullptr;
    }
    void* param = nullptr;
    ListNode<DrawCmd>* curDraw = drawCmdList_.Begin();
    for (; curDraw != drawCmdList_.End(); curDraw = curDraw->next_) {
        param = curDraw->data_.param;
        curDraw->data_.DeleteParam(param);
        curDraw->data_.param = nullptr;
    }
    drawCmdList_.Clear();
    if (vertices_ != nullptr) {
        delete vertices_;
        vertices_ = nullptr;
    }
    DestroyMapBufferInfo();
}

void UICanvas::Clear()
{
    if ((path_ != nullptr) && (path_->strokeCount_ == 0)) {
        delete path_;
        path_ = nullptr;
    }
    void* param = nullptr;
    ListNode<DrawCmd>* curDraw = drawCmdList_.Begin();
    for (; curDraw != drawCmdList_.End(); curDraw = curDraw->next_) {
        param = curDraw->data_.param;
        curDraw->data_.DeleteParam(param);
        curDraw->data_.param = nullptr;
    }
    drawCmdList_.Clear();
    if (vertices_ != nullptr) {
        delete vertices_;
        vertices_ = nullptr;
    }
    Invalidate();
}

void UICanvas::DeleteImageParam(void* param)
{
    ImageParam* imageParam = static_cast<ImageParam*>(param);
    if (imageParam->image != nullptr) {
        delete imageParam->image;
        imageParam->image = nullptr;
    }
    delete imageParam;
    imageParam = nullptr;
}

void UICanvas::DeletePathParam(void* param)
{
    PathParam* pathParam = static_cast<PathParam*>(param);
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    if (pathParam->vertices != nullptr) {
        pathParam->vertices->FreeAll();
        pathParam->vertices = nullptr;
    }
    if (pathParam->imageParam != nullptr) {
        DeleteImageParam(pathParam->imageParam);
    }
#else
    pathParam->path->strokeCount_--;
    if (pathParam->path->strokeCount_ == 0) {
        delete pathParam->path;
    }
#endif
    delete pathParam;
    pathParam = nullptr;
}

void UICanvas::DrawLine(const Point& endPoint, const Paint& paint)
{
    DrawLine(startPoint_, endPoint, paint);
}

void UICanvas::DrawLine(const Point& startPoint, const Point& endPoint, const Paint& paint)
{
    LineParam* lineParam = new LineParam;
    if (lineParam == nullptr) {
        GRAPHIC_LOGE("new LineParam fail");
        return;
    }
    lineParam->start = startPoint;
    lineParam->end = endPoint;

    DrawCmd cmd;
    cmd.paint = paint;
    cmd.param = lineParam;
    cmd.DeleteParam = DeleteLineParam;
    cmd.DrawGraphics = DoDrawLine;
    drawCmdList_.PushBack(cmd);

    Invalidate();
    SetStartPosition(endPoint);
}

void UICanvas::DrawCurve(const Point& control1, const Point& control2, const Point& endPoint, const Paint& paint)
{
    DrawCurve(startPoint_, control1, control2, endPoint, paint);
}

void UICanvas::DrawCurve(const Point& startPoint,
                         const Point& control1,
                         const Point& control2,
                         const Point& endPoint,
                         const Paint& paint)
{
    CurveParam* curveParam = new CurveParam;
    if (curveParam == nullptr) {
        GRAPHIC_LOGE("new CurveParam fail");
        return;
    }
    curveParam->start = startPoint;
    curveParam->control1 = control1;
    curveParam->control2 = control2;
    curveParam->end = endPoint;

    DrawCmd cmd;
    cmd.paint = paint;
    if (paint.GetStrokeWidth() > MAX_CURVE_WIDTH) {
        cmd.paint.SetStrokeWidth(MAX_CURVE_WIDTH);
    }
    cmd.param = curveParam;
    cmd.DeleteParam = DeleteCurveParam;
    cmd.DrawGraphics = DoDrawCurve;
    drawCmdList_.PushBack(cmd);

    Invalidate();
    SetStartPosition(endPoint);
}

void UICanvas::DrawRect(const Point& startPoint, int16_t height, int16_t width, const Paint& paint)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    SetDrawLinePath(startPoint, height, width, paint);
#else
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::STROKE_STYLE) {
        RectParam* rectParam = new RectParam;
        if (rectParam == nullptr) {
            GRAPHIC_LOGE("new RectParam fail");
            return;
        }
        rectParam->start = startPoint;
        rectParam->height = height;
        rectParam->width = width;

        DrawCmd cmd;
        cmd.paint = paint;
        cmd.param = rectParam;
        cmd.DeleteParam = DeleteRectParam;
        cmd.DrawGraphics = DoDrawRect;
        drawCmdList_.PushBack(cmd);
    }

    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::FILL_STYLE) {
        RectParam* rectParam = new RectParam;
        if (rectParam == nullptr) {
            GRAPHIC_LOGE("new RectParam fail");
            return;
        }
        rectParam->start = startPoint;
        rectParam->height = height;
        rectParam->width = width;

        DrawCmd cmd;
        cmd.paint = paint;
        cmd.param = rectParam;
        cmd.DeleteParam = DeleteRectParam;
        cmd.DrawGraphics = DoFillRect;
        drawCmdList_.PushBack(cmd);
    }
#endif
    Invalidate();
}

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
void UICanvas::SetDrawLinePath(const Point& startPoint, int16_t height, int16_t width, const Paint& paint)
{
    if (!paint.GetChangeFlag()) {
        if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::STROKE_STYLE) {
            DrawRectSetCmd(startPoint, height, width, paint, Paint::PaintStyle::STROKE_STYLE);
        }

        if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::FILL_STYLE) {
            DrawRectSetCmd(startPoint, height, width, paint, Paint::PaintStyle::FILL_STYLE);
        }
    } else {
        BeginPath();
        MoveTo(startPoint);
        LineTo({static_cast<int16_t>(startPoint.x + width), startPoint.y});
        LineTo({static_cast<int16_t>(startPoint.x + width), static_cast<int16_t>(startPoint.y + height)});
        LineTo({startPoint.x, static_cast<int16_t>(startPoint.y + height)});
        ClosePath();
        FillPath(paint);
        DrawPath(paint);
    }
}
#endif

void UICanvas::DrawRectSetCmd(const Point& startPoint, int16_t height, int16_t width, const Paint& paint,
                              Paint::PaintStyle paintStyle)
{
    RectParam* rectParam = new RectParam;
    if (rectParam == nullptr) {
        GRAPHIC_LOGE("new RectParam fail");
        return;
    }
    rectParam->start = startPoint;
    rectParam->height = height;
    rectParam->width = width;

    DrawCmd cmd;
    cmd.paint = paint;
    cmd.param = rectParam;
    cmd.DeleteParam = DeleteRectParam;
    if (paintStyle == Paint::PaintStyle::STROKE_STYLE) {
        cmd.DrawGraphics = DoDrawRect;
    } else if (paintStyle == Paint::PaintStyle::FILL_STYLE) {
        cmd.DrawGraphics = DoFillRect;
    }
    drawCmdList_.PushBack(cmd);
}

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
void UICanvas::StrokeRect(const Point& startPoint, int16_t height, int16_t width, const Paint& paint)
{
    if (!paint.GetChangeFlag()) {
        RectParam* rectParam = new RectParam;
        if (rectParam == nullptr) {
            GRAPHIC_LOGE("new RectParam fail");
            return;
        }
        rectParam->start = startPoint;
        rectParam->height = height;
        rectParam->width = width;

        DrawCmd cmd;
        cmd.paint = paint;
        cmd.param = rectParam;
        cmd.DeleteParam = DeleteRectParam;
        cmd.DrawGraphics = DoDrawRect;
        drawCmdList_.PushBack(cmd);
    } else {
        BeginPath();
        MoveTo(startPoint);
        LineTo({static_cast<int16_t>(startPoint.x + width), startPoint.y});
        LineTo({static_cast<int16_t>(startPoint.x + width), static_cast<int16_t>(startPoint.y + height)});
        LineTo({startPoint.x, static_cast<int16_t>(startPoint.y + height)});
        ClosePath();
        DrawPath(paint);
    }
    SetStartPosition(startPoint);
}

void UICanvas::ClearRect(const Point& startPoint, int16_t height, int16_t width)
{
    Paint paint;
    paint.SetFillColor(this->style_->bgColor_);
    paint.SetStyle(Paint::FILL_STYLE);
    BeginPath();
    MoveTo(startPoint);
    LineTo({static_cast<int16_t>(startPoint.x + width), startPoint.y});
    LineTo({static_cast<int16_t>(startPoint.x + width), static_cast<int16_t>(startPoint.y + height)});
    LineTo({startPoint.x, static_cast<int16_t>(startPoint.y + height)});
    ClosePath();
    FillPath(paint);
}
#endif

void UICanvas::DrawCircle(const Point& center, uint16_t radius, const Paint& paint)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    if (paint.GetChangeFlag()) {
#if defined(GRAPHIC_ENABLE_BEZIER_ARC_FLAG) && GRAPHIC_ENABLE_BEZIER_ARC_FLAG
        if (vertices_ == nullptr) {
            vertices_ = new UICanvasVertices();
        }
        vertices_->RemoveAll();
        BezierArc arc(center.x, center.y, radius, radius, 0, TWO_TIMES * PI);
        vertices_->ConcatPath(arc, 0);
        vertices_->ClosePolygon();
        if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::STROKE_STYLE) {
            DrawPath(paint);
        }
        if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::FILL_STYLE) {
            FillPath(paint);
        }
#endif
    } else {
        CircleParam* circleParam = new CircleParam;
        if (circleParam == nullptr) {
            GRAPHIC_LOGE("new CircleParam fail");
            return;
        }
        circleParam->center = center;
        circleParam->radius = radius;

        DrawCmd cmd;
        cmd.paint = paint;
        cmd.param = circleParam;
        cmd.DeleteParam = DeleteCircleParam;
        cmd.DrawGraphics = DoDrawCircle;
        drawCmdList_.PushBack(cmd);
    }
#else
    CircleParam* circleParam = new CircleParam;
    if (circleParam == nullptr) {
        GRAPHIC_LOGE("new CircleParam fail");
        return;
    }
    circleParam->center = center;
    circleParam->radius = radius;

    DrawCmd cmd;
    cmd.paint = paint;
    cmd.param = circleParam;
    cmd.DeleteParam = DeleteCircleParam;
    cmd.DrawGraphics = DoDrawCircle;
    drawCmdList_.PushBack(cmd);
#endif
    Invalidate();
}

void UICanvas::DrawSector(const Point& center,
                          uint16_t radius,
                          int16_t startAngle,
                          int16_t endAngle,
                          const Paint& paint)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    BeginPath();
    MoveTo(center);
    ArcTo(center, radius, startAngle, endAngle);
    ClosePath();
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::STROKE_STYLE) {
        DrawPath(paint);
    }
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::FILL_STYLE) {
        FillPath(paint);
    }
#else
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::FILL_STYLE) {
        Paint innerPaint = paint;
        innerPaint.SetStyle(Paint::PaintStyle::STROKE_STYLE);
        innerPaint.SetStrokeWidth(radius);
        innerPaint.SetStrokeColor(paint.GetFillColor());
        radius >>= 1;
        DrawArc(center, radius, startAngle, endAngle, innerPaint);
    }
#endif
}

void UICanvas::DrawArc(const Point& center, uint16_t radius, int16_t startAngle,
                       int16_t endAngle, const Paint& paint)
{
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::STROKE_STYLE) {
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
        if (paint.GetChangeFlag()) {
            ArcTo(center, radius, startAngle, endAngle);
            DrawPath(paint);
        } else
#endif
        {
            ArcParam* arcParam = new ArcParam;
            if (arcParam == nullptr) {
                GRAPHIC_LOGE("new ArcParam fail");
                return;
            }
            arcParam->center = center;
            arcParam->radius = radius;

            int16_t start;
            int16_t end;
            if (startAngle > endAngle) {
                start = endAngle;
                end = startAngle;
            } else {
                start = startAngle;
                end = endAngle;
            }

            DrawArc::GetInstance()->GetDrawRange(start, end);
            arcParam->startAngle = start;
            arcParam->endAngle = end;

            DrawCmd cmd;
            cmd.paint = paint;
            cmd.param = arcParam;
            cmd.DeleteParam = DeleteArcParam;
            cmd.DrawGraphics = DoDrawArc;
            drawCmdList_.PushBack(cmd);
        }
        Invalidate();
    }
}

void UICanvas::DrawLabel(const Point& startPoint,
                         const char* text,
                         uint16_t maxWidth,
                         const FontStyle& fontStyle,
                         const Paint& paint)
{
    if (text == nullptr) {
        return;
    }
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::FILL_STYLE) {
        UILabel* label = new UILabel();
        if (label == nullptr) {
            GRAPHIC_LOGE("new UILabel fail");
            return;
        }
        label->SetLineBreakMode(UILabel::LINE_BREAK_CLIP);
        label->SetPosition(startPoint.x, startPoint.y);
        label->SetWidth(maxWidth);
        label->SetHeight(GetHeight());
        label->SetText(text);
        label->SetFont(fontStyle.fontName, fontStyle.fontSize);
        label->SetAlign(fontStyle.align);
        label->SetDirect(fontStyle.direct);
        label->SetStyle(STYLE_LETTER_SPACE, fontStyle.letterSpace);
        label->SetStyle(STYLE_TEXT_COLOR, paint.GetFillColor().full);
        label->SetStyle(STYLE_TEXT_OPA, paint.GetOpacity());

        DrawCmd cmd;
        cmd.param = label;
        cmd.DeleteParam = DeleteLabel;
        cmd.DrawGraphics = DoDrawLabel;
        drawCmdList_.PushBack(cmd);

        Invalidate();
    }
}
#if defined(GRAPHIC_ENABLE_DRAW_IMAGE_FLAG) && GRAPHIC_ENABLE_DRAW_IMAGE_FLAG
void UICanvas::DrawImage(const Point &startPoint, const char* image, const Paint& paint)
{
    if (image == nullptr) {
        return;
    }
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::FILL_STYLE) {
        UIExtendImageView* imageView = new UIExtendImageView();
        if (imageView == nullptr) {
            GRAPHIC_LOGE("new UIImageView fail");
            return;
        }
        imageView->SetCanvas(this);
        imageView->SetPosition(startPoint.x, startPoint.y);
        imageView->SetSrc(image);

        DrawCmd cmd;
        cmd.paint = paint;
        cmd.param = imageView;
        cmd.DeleteParam = DeleteImageView;
        cmd.DrawGraphics = DoDrawImage;
        drawCmdList_.PushBack(cmd);

        Invalidate();
        SetStartPosition(startPoint);
    }
}

void UICanvas::DrawImage(const Point& startPoint, const char* image,
                         const Paint& paint, int16_t width, int16_t height)
{
    if (image == nullptr) {
        return;
    }
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::FILL_STYLE) {
        UIExtendImageView* imageView = new UIExtendImageView();
        if (imageView == nullptr) {
            GRAPHIC_LOGE("new UIImageView fail");
            return;
        }
        imageView->SetCanvas(this);
        imageView->SetPosition(startPoint.x, startPoint.y);
        imageView->SetSrc(image);
        float scaleX = 1.0;
        float scaleY = 1.0;
        if (width > 0 && imageView->GetWidth() > 0) {
            scaleX = (float)width / (float)imageView->GetWidth();
        }
        if (height > 0 && imageView->GetHeight() > 0) {
            scaleY = (float)height / (float)imageView->GetHeight();
        }
        DrawCmd cmd;
        cmd.paint = paint;
        cmd.paint.Scale(scaleX, scaleY);
        cmd.param = imageView;
        cmd.DeleteParam = DeleteImageView;
        cmd.DrawGraphics = DoDrawImage;
        drawCmdList_.PushBack(cmd);

        Invalidate();
        SetStartPosition(startPoint);
    }

    Invalidate();
    SetStartPosition(startPoint);
}
#endif

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
void SetImageParamInfo(ImageParam* imageParam, const Paint& paint, PathParam* pathParam)
{
    imageParam->image->SetSrc(paint.GetPatternImage());
    ImageHeader header = {0};
    imageParam->image->GetHeader(header);
    imageParam->start = {0, 0};
    imageParam->height = header.height;
    imageParam->width = header.width;

    pathParam->imageParam = imageParam;
}
#endif
#endif

void UICanvas::DrawPath(const Paint& paint)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    if (vertices_ == nullptr) {
        return;
    }

    PathParam* pathParam = new PathParam;
    if (pathParam == nullptr) {
        GRAPHIC_LOGE("new LineParam fail");
        return;
    }

    pathParam->vertices = vertices_;
    pathParam->isStroke = true;
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
    if (paint.GetStyle() == Paint::PATTERN) {
        ImageParam* imageParam = new ImageParam;
        if (imageParam == nullptr) {
            GRAPHIC_LOGE("new ImageParam fail");
            return;
        }

        imageParam->image = new Image();
        if (imageParam->image == nullptr) {
            delete imageParam;
            imageParam = nullptr;
            return;
        }

        SetImageParamInfo(imageParam, paint, pathParam);
    }
#endif
#else
    if ((path_ == nullptr) || (path_->cmd_.Size() == 0)) {
        return;
    }

    path_->strokeCount_++;
    PathParam* pathParam = new PathParam;
    if (pathParam == nullptr) {
        GRAPHIC_LOGE("new PathParam fail");
        return;
    }
    pathParam->path = path_;
    pathParam->count = path_->cmd_.Size();
#endif
    DrawCmd cmd;
    cmd.paint = paint;
    cmd.param = pathParam;
    cmd.DeleteParam = DeletePathParam;
    cmd.DrawGraphics = DoDrawPath;
    drawCmdList_.PushBack(cmd);
    Invalidate();
}

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
void UICanvas::FillPath(const Paint& paint)
{
    if (vertices_ == nullptr) {
        return;
    }

    PathParam* pathParam = new PathParam;
    if (pathParam == nullptr) {
        GRAPHIC_LOGE("new LineParam fail");
        return;
    }

    pathParam->vertices = vertices_;
    pathParam->isStroke = false;
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
    if (paint.GetStyle() == Paint::PATTERN) {
        ImageParam* imageParam = new ImageParam;
        if (imageParam == nullptr) {
            GRAPHIC_LOGE("new ImageParam fail");
            return;
        }

        imageParam->image = new Image();
        if (imageParam->image == nullptr) {
            delete imageParam;
            imageParam = nullptr;
            return;
        }

        SetImageParamInfo(imageParam, paint, pathParam);
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

void UICanvas::OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    Rect rect = GetOrigRect();
    BaseGfxEngine::GetInstance()->DrawRect(gfxDstBuffer, rect, invalidatedArea, *style_, opaScale_);

    void* param = nullptr;
    ListNode<DrawCmd>* curDraw = drawCmdList_.Begin();
    Rect coords = GetOrigRect();
    Rect trunc = invalidatedArea;
    if (!trunc.Intersect(trunc, coords)) {
        return;
    }
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    bool haveComposite = false;
    for (; curDraw != drawCmdList_.End(); curDraw = curDraw->next_) {
        if (curDraw->data_.paint.HaveComposite()) {
            haveComposite = true;
            break;
        }
    }

    if (haveComposite) {
        OnBlendDraw(gfxDstBuffer, trunc);
    } else
#endif
    {
        curDraw = drawCmdList_.Begin();
        for (; curDraw != drawCmdList_.End(); curDraw = curDraw->next_) {
            param = curDraw->data_.param;
            curDraw->data_.DrawGraphics(gfxDstBuffer, param, curDraw->data_.paint, rect, trunc, *style_);
        }
    }
}

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
void OnBlendDrawPattern(ListNode<UICanvas::DrawCmd>* curDraw,
                        UICanvas::DrawCmd& drawCmd,
                        Rect& rect,
                        const Rect& trunc,
                        RasterizerScanlineAntialias& blendRasterizer,
                        RasterizerScanlineAntialias& rasterizer,
                        RenderBase& renBase,
                        TransAffine& transform,
                        PathParam* pathParamBlend)
{
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
    if (curDraw->data_.paint.GetStyle() == Paint::PATTERN) {
        if (curDraw->data_.param == nullptr) {
            return;
        }
        PathParam* pathParam = static_cast<PathParam*>(curDraw->data_.param);
        ImageParam* imageParam = static_cast<ImageParam*>(pathParam->imageParam);
        if (imageParam->image == nullptr) {
            return;
        }
        FillPatternRgba spanPattern(imageParam->image->GetImageInfo(), curDraw->data_.paint.GetPatternRepeatMode(),
                                    rect.GetLeft(), rect.GetTop());
        UICanvas::BlendRaster(drawCmd.paint, drawCmd.param, blendRasterizer, rasterizer, renBase, transform,
                              spanPattern, trunc, pathParamBlend->isStroke);
    }
#endif
}

void OnBlendDrawGradient(ListNode<UICanvas::DrawCmd>* curDraw,
                         UICanvas::DrawCmd& drawCmd,
                         const Rect& trunc,
                         RasterizerScanlineAntialias& blendRasterizer,
                         RasterizerScanlineAntialias& rasterizer,
                         RenderBase& renBase,
                         TransAffine& transform,
                         PathParam* pathParamBlend)
{
#if defined(GRAPHIC_ENABLE_GRADIENT_FILL_FLAG) && GRAPHIC_ENABLE_GRADIENT_FILL_FLAG
    if (curDraw->data_.paint.GetStyle() == Paint::GRADIENT) {
        TransAffine gradientMatrix;
        FillInterpolator interpolatorType(gradientMatrix);
        FillGradientLut gradientColorMode;
        DrawCanvas::BuildGradientColor(curDraw->data_.paint, gradientColorMode);
        if (curDraw->data_.paint.GetGradient() == Paint::Linear) {
            float distance = 0;
            DrawCanvas::BuildLineGradientMatrix(drawCmd.paint, gradientMatrix, transform, distance);
            GradientLinearCalculate gradientLinearCalculate;
            FillGradient span(interpolatorType, gradientLinearCalculate, gradientColorMode, 0, distance);
            UICanvas::BlendRaster(drawCmd.paint, drawCmd.param, blendRasterizer, rasterizer, renBase,
                                  transform, span, trunc, pathParamBlend->isStroke);
        }
        if (curDraw->data_.paint.GetGradient() == Paint::Radial) {
            Paint::RadialGradientPoint radialPoint = drawCmd.paint.GetRadialGradientPoint();
            float startRadius = 0;
            float endRadius = 0;
            DrawCanvas::BuildRadialGradientMatrix(drawCmd.paint, gradientMatrix, transform, startRadius, endRadius);
            GradientRadialCalculate gradientRadialCalculate(endRadius, radialPoint.x0 - radialPoint.x1,
                                                            radialPoint.y0 - radialPoint.y1);
            FillGradient span(interpolatorType, gradientRadialCalculate, gradientColorMode, startRadius, endRadius);
            UICanvas::BlendRaster(drawCmd.paint, drawCmd.param, blendRasterizer, rasterizer, renBase,
                                  transform, span, trunc, pathParamBlend->isStroke);
        }
    }
#endif
}

void UICanvas::OnBlendDraw(BufferInfo& gfxDstBuffer, const Rect& trunc)
{
    Rect rect = GetOrigRect();
    RenderBuffer renderBuffer;
    TransAffine transform;
    ListNode<DrawCmd>* curDrawEnd = drawCmdList_.Begin();
    RasterizerScanlineAntialias blendRasterizer;
    DrawCmd drawCmd;
    int count = 0;
    for (; curDrawEnd != drawCmdList_.End(); curDrawEnd = curDrawEnd->next_) {
        if (curDrawEnd->data_.paint.HaveComposite()) {
            drawCmd = curDrawEnd->data_;
            count++;
        }
    }
    if (drawCmd.param == nullptr) {
        return;
    }
    PathParam* pathParamBlend = static_cast<PathParam*>(drawCmd.param);
    ListNode<DrawCmd>* curDraw = drawCmdList_.Begin();
    DrawCanvas::InitRenderAndTransform(gfxDstBuffer, renderBuffer, rect, transform, *style_, curDraw->data_.paint);
    DrawCanvas::SetRasterizer(*pathParamBlend->vertices, drawCmd.paint, blendRasterizer, transform,
                              pathParamBlend->isStroke);
    RasterizerScanlineAntialias scanline;
    RenderPixfmtRgbaBlend pixFormat(renderBuffer);
    RenderBase renBase(pixFormat);
    renBase.ResetClipping(true);
    renBase.ClipBox(trunc.GetLeft(), trunc.GetTop(), trunc.GetRight(), trunc.GetBottom());
    for (; curDraw != drawCmdList_.End(); curDraw = curDraw->next_) {
        if (curDraw->data_.paint.HaveComposite()) {
            drawCmd = curDraw->data_;
            count--;
        }
        if (count <= 0) {
            continue;
        }
        RasterizerScanlineAntialias rasterizer;
        if (curDraw->data_.param == nullptr) {
            continue;
        }
        PathParam* pathParam = static_cast<PathParam*>(curDraw->data_.param);
#if defined(GRAPHIC_ENABLE_BLUR_EFFECT_FLAG) && GRAPHIC_ENABLE_BLUR_EFFECT_FLAG
        if (curDraw->data_.paint.HaveShadow()) {
            DrawCanvas::DoDrawShadow(gfxDstBuffer, curDraw->data_.param, curDraw->data_.paint, rect, trunc, *style_,
                                     pathParam->isStroke);
        }
#endif
        DrawCanvas::InitRenderAndTransform(gfxDstBuffer, renderBuffer, rect, transform, *style_, curDraw->data_.paint);
        rasterizer.ClipBox(0, 0, gfxDstBuffer.width, gfxDstBuffer.height);
        DrawCanvas::SetRasterizer(*pathParam->vertices, curDraw->data_.paint, rasterizer, transform,
                                  pathParam->isStroke);
        if (IsSoild(curDraw->data_.paint)) {
            Rgba8T color;
            DrawCanvas::RenderBlendSolid(curDraw->data_.paint, color, pathParam->isStroke);
            SpanSoildColor spanSoildColor(color);
            BlendRaster(drawCmd.paint, drawCmd.param, blendRasterizer, rasterizer, renBase, transform,
                        spanSoildColor, rect, pathParamBlend->isStroke);
        }

        OnBlendDrawGradient(curDraw, drawCmd, trunc, blendRasterizer,
                            rasterizer, renBase, transform, pathParamBlend);

        OnBlendDrawPattern(curDraw, drawCmd, rect, trunc, blendRasterizer,
                           rasterizer, renBase, transform, pathParamBlend);
    }
}
#endif

void UICanvas::GetAbsolutePosition(const Point& prePoint, const Rect& rect, const Style& style, Point& point)
{
    point.x = prePoint.x + rect.GetLeft() + style.paddingLeft_ + style.borderWidth_;
    point.y = prePoint.y + rect.GetTop() + style.paddingTop_ + style.borderWidth_;
}

void UICanvas::DoDrawLine(BufferInfo& gfxDstBuffer,
                          void* param,
                          const Paint& paint,
                          const Rect& rect,
                          const Rect& invalidatedArea,
                          const Style& style)
{
    if (param == nullptr) {
        return;
    }
    LineParam* lineParam = static_cast<LineParam*>(param);
    Point start;
    Point end;
    GetAbsolutePosition(lineParam->start, rect, style, start);
    GetAbsolutePosition(lineParam->end, rect, style, end);

    BaseGfxEngine::GetInstance()->DrawLine(gfxDstBuffer, start, end, invalidatedArea, paint.GetStrokeWidth(),
                                           paint.GetStrokeColor(), paint.GetOpacity());
}

void UICanvas::DoDrawCurve(BufferInfo& gfxDstBuffer,
                           void* param,
                           const Paint& paint,
                           const Rect& rect,
                           const Rect& invalidatedArea,
                           const Style& style)
{
    if (param == nullptr) {
        return;
    }
    CurveParam* curveParam = static_cast<CurveParam*>(param);
    Point start;
    Point end;
    Point control1;
    Point control2;
    GetAbsolutePosition(curveParam->start, rect, style, start);
    GetAbsolutePosition(curveParam->end, rect, style, end);
    GetAbsolutePosition(curveParam->control1, rect, style, control1);
    GetAbsolutePosition(curveParam->control2, rect, style, control2);

    BaseGfxEngine::GetInstance()->DrawCubicBezier(gfxDstBuffer,
                                                  start,
                                                  control1,
                                                  control2,
                                                  end,
                                                  invalidatedArea,
                                                  paint.GetStrokeWidth(),
                                                  paint.GetStrokeColor(),
                                                  paint.GetOpacity());
}

void UICanvas::DoDrawRect(BufferInfo& gfxDstBuffer,
                          void* param,
                          const Paint& paint,
                          const Rect& rect,
                          const Rect& invalidatedArea,
                          const Style& style)
{
    if (param == nullptr) {
        return;
    }
    RectParam* rectParam = static_cast<RectParam*>(param);
    Style drawStyle = StyleDefault::GetDefaultStyle();
    drawStyle.bgColor_ = paint.GetStrokeColor();
    drawStyle.bgOpa_ = paint.GetOpacity();
    drawStyle.borderRadius_ = 0;

    int16_t lineWidth = static_cast<int16_t>(paint.GetStrokeWidth());
    Point start;
    GetAbsolutePosition(rectParam->start, rect, style, start);

    int16_t x = start.x - lineWidth / 2; // 2: half
    int16_t y = start.y - lineWidth / 2; // 2: half
    Rect coords;
    BaseGfxEngine* baseGfxEngine = BaseGfxEngine::GetInstance();
    if ((rectParam->height <= lineWidth) || (rectParam->width <= lineWidth)) {
        coords.SetPosition(x, y);
        coords.SetHeight(rectParam->height + lineWidth);
        coords.SetWidth(rectParam->width + lineWidth);
        baseGfxEngine->DrawRect(gfxDstBuffer, coords, invalidatedArea, drawStyle, OPA_OPAQUE);
        return;
    }

    coords.SetPosition(x, y);
    coords.SetHeight(lineWidth);
    coords.SetWidth(rectParam->width);
    baseGfxEngine->DrawRect(gfxDstBuffer, coords, invalidatedArea, drawStyle, OPA_OPAQUE);

    coords.SetPosition(x + rectParam->width, y);
    coords.SetHeight(rectParam->height);
    coords.SetWidth(lineWidth);
    baseGfxEngine->DrawRect(gfxDstBuffer, coords, invalidatedArea, drawStyle, OPA_OPAQUE);

    coords.SetPosition(x, y + lineWidth);
    coords.SetHeight(rectParam->height);
    coords.SetWidth(lineWidth);
    baseGfxEngine->DrawRect(gfxDstBuffer, coords, invalidatedArea, drawStyle, OPA_OPAQUE);

    coords.SetPosition(x + lineWidth, y + rectParam->height);
    coords.SetHeight(lineWidth);
    coords.SetWidth(rectParam->width);
    baseGfxEngine->DrawRect(gfxDstBuffer, coords, invalidatedArea, drawStyle, OPA_OPAQUE);
}

void UICanvas::DoFillRect(BufferInfo& gfxDstBuffer,
                          void* param,
                          const Paint& paint,
                          const Rect& rect,
                          const Rect& invalidatedArea,
                          const Style& style)
{
    if (param == nullptr) {
        return;
    }
    RectParam* rectParam = static_cast<RectParam*>(param);
    uint8_t enableStroke = static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::STROKE_STYLE;
    int16_t lineWidth = enableStroke ? paint.GetStrokeWidth() : 0;
    if ((rectParam->height <= lineWidth) || (rectParam->width <= lineWidth)) {
        return;
    }
    Point start;
    GetAbsolutePosition(rectParam->start, rect, style, start);

    Rect coords;
    coords.SetPosition(start.x + (lineWidth + 1) / 2, start.y + (lineWidth + 1) / 2); // 2: half
    coords.SetHeight(rectParam->height - lineWidth);
    coords.SetWidth(rectParam->width - lineWidth);

    Style drawStyle = StyleDefault::GetDefaultStyle();
    drawStyle.bgColor_ = paint.GetFillColor();
    drawStyle.bgOpa_ = paint.GetOpacity();
    drawStyle.borderRadius_ = 0;
    BaseGfxEngine::GetInstance()->DrawRect(gfxDstBuffer, coords, invalidatedArea, drawStyle, OPA_OPAQUE);
}

void UICanvas::DoDrawCircle(BufferInfo& gfxDstBuffer,
                            void* param,
                            const Paint& paint,
                            const Rect& rect,
                            const Rect& invalidatedArea,
                            const Style& style)
{
    if (param == nullptr) {
        return;
    }
    CircleParam* circleParam = static_cast<CircleParam*>(param);

    Style drawStyle = StyleDefault::GetDefaultStyle();
    drawStyle.lineOpa_ = paint.GetOpacity();

    ArcInfo arcInfo = {{0}};
    arcInfo.imgPos = {0, 0};
    arcInfo.startAngle = 0;
    arcInfo.endAngle = CIRCLE_IN_DEGREE;
    GetAbsolutePosition(circleParam->center, rect, style, arcInfo.center);
    uint8_t enableStroke = static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::STROKE_STYLE;
    uint16_t halfLineWidth = enableStroke ? (paint.GetStrokeWidth() >> 1) : 0;
    BaseGfxEngine* baseGfxEngine = BaseGfxEngine::GetInstance();
    if (static_cast<uint8_t>(paint.GetStyle()) & Paint::PaintStyle::FILL_STYLE) {
        arcInfo.radius = circleParam->radius - halfLineWidth;
        drawStyle.lineWidth_ = arcInfo.radius;
        drawStyle.lineColor_ = paint.GetFillColor();
        baseGfxEngine->DrawArc(gfxDstBuffer, arcInfo, invalidatedArea, drawStyle, OPA_OPAQUE,
                               CapType::CAP_NONE);
    }

    if (enableStroke) {
        arcInfo.radius = circleParam->radius + halfLineWidth - 1;
        drawStyle.lineWidth_ = static_cast<int16_t>(paint.GetStrokeWidth());
        drawStyle.lineColor_ = paint.GetStrokeColor();
        baseGfxEngine->DrawArc(gfxDstBuffer, arcInfo, invalidatedArea, drawStyle, OPA_OPAQUE,
                               CapType::CAP_NONE);
    }
}

void UICanvas::DoDrawArc(BufferInfo& gfxDstBuffer,
                         void* param,
                         const Paint& paint,
                         const Rect& rect,
                         const Rect& invalidatedArea,
                         const Style& style)
{
    if (param == nullptr) {
        return;
    }
    ArcParam* arcParam = static_cast<ArcParam*>(param);

    ArcInfo arcInfo = {{0}};
    arcInfo.imgPos = {0, 0};
    arcInfo.startAngle = arcParam->startAngle;
    arcInfo.endAngle = arcParam->endAngle;
    Style drawStyle = StyleDefault::GetDefaultStyle();
    drawStyle.lineWidth_ = static_cast<int16_t>(paint.GetStrokeWidth());
    drawStyle.lineColor_ = paint.GetStrokeColor();
    drawStyle.lineOpa_ = paint.GetOpacity();
    arcInfo.radius = arcParam->radius + ((paint.GetStrokeWidth() + 1) >> 1);

    GetAbsolutePosition(arcParam->center, rect, style, arcInfo.center);
    BaseGfxEngine::GetInstance()->DrawArc(gfxDstBuffer, arcInfo, invalidatedArea, drawStyle, OPA_OPAQUE,
                                          CapType::CAP_NONE);
}
#if defined(GRAPHIC_ENABLE_DRAW_IMAGE_FLAG) && GRAPHIC_ENABLE_DRAW_IMAGE_FLAG
void UICanvas::DoDrawImage(BufferInfo& gfxDstBuffer,
                           void* param,
                           const Paint& paint,
                           const Rect& rect,
                           const Rect& invalidatedArea,
                           const Style& style)
{
    if (param == nullptr) {
        return;
    }
    UIImageView* imageView = static_cast<UIImageView*>(param);
    Point startPos = {imageView->GetX(), imageView->GetY()};
    Point start;
    GetAbsolutePosition({startPos.x, startPos.y}, rect, style, start);
    imageView->SetPosition(start.x, start.y);
    if (!paint.GetTransAffine().IsIdentity()) {
        float angle = paint.GetRotateAngle();
        imageView->Rotate(MATH_ROUND(angle), Vector2<float>(0, 0));
        imageView->Translate(Vector3<int16_t>(paint.GetTranslateX(), paint.GetTranslateY(), 1));
        Vector2<float> scale(static_cast<float>(paint.GetScaleX()), static_cast<float>(paint.GetScaleY()));
        imageView->Scale(scale, Vector2<float>(0, 0));
    }
    imageView->OnDraw(gfxDstBuffer, invalidatedArea);
    imageView->SetPosition(startPos.x, startPos.y);
}
#endif

void UICanvas::DoDrawLabel(BufferInfo& gfxDstBuffer,
                           void* param,
                           const Paint& paint,
                           const Rect& rect,
                           const Rect& invalidatedArea,
                           const Style& style)
{
    if (param == nullptr) {
        return;
    }
    UILabel* label = static_cast<UILabel*>(param);
    Point startPos = {label->GetX(), label->GetY()};
    Point start;
    GetAbsolutePosition({startPos.x, startPos.y}, rect, style, start);
    label->SetPosition(start.x, start.y);
    label->OnDraw(gfxDstBuffer, invalidatedArea);
    label->SetPosition(startPos.x, startPos.y);
}

void UICanvas::DoDrawLineJoin(BufferInfo& gfxDstBuffer,
                              const Point& center,
                              const Rect& invalidatedArea,
                              const Paint& paint)
{
    ArcInfo arcinfo = {{0}};
    arcinfo.center = center;
    arcinfo.imgPos = {0, 0};
    arcinfo.radius = (paint.GetStrokeWidth() + 1) >> 1;
    arcinfo.startAngle = 0;
    arcinfo.endAngle = CIRCLE_IN_DEGREE;

    Style style;
    style.lineColor_ = paint.GetStrokeColor();
    style.lineWidth_ = static_cast<int16_t>(paint.GetStrokeWidth());
    style.lineOpa_ = OPA_OPAQUE;
    BaseGfxEngine::GetInstance()->DrawArc(gfxDstBuffer, arcinfo, invalidatedArea,
                                          style, OPA_OPAQUE, CapType::CAP_NONE);
}

void UICanvas::DoDrawPath(BufferInfo& gfxDstBuffer,
                          void* param,
                          const Paint& paint,
                          const Rect& rect,
                          const Rect& invalidatedArea,
                          const Style& style)
{
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    BaseGfxEngine::GetInstance()->DrawPath(gfxDstBuffer, param, paint, rect, invalidatedArea, style);
#else
    if (param == nullptr) {
        return;
    }
    PathParam* pathParam = static_cast<PathParam*>(param);
    const UICanvasPath* path = pathParam->path;
    if (path == nullptr) {
        return;
    }
    Point pathEnd = {COORD_MIN, COORD_MIN};

    ListNode<Point>* pointIter = path->points_.Begin();
    ListNode<ArcParam>* arcIter = path->arcParam_.Begin();
    ListNode<PathCmd>* iter = path->cmd_.Begin();
    for (uint16_t i = 0; (i < pathParam->count) && (iter != path->cmd_.End()); i++, iter = iter->next_) {
        switch (iter->data_) {
            case CMD_MOVE_TO: {
                pointIter = pointIter->next_;
                break;
            }
            case CMD_LINE_TO: {
                Point start = pointIter->prev_->data_;
                Point end = pointIter->data_;
                pointIter = pointIter->next_;
                if ((start.x == end.x) && (start.y == end.y)) {
                    break;
                }

                GetAbsolutePosition(start, rect, style, start);
                GetAbsolutePosition(end, rect, style, end);
                BaseGfxEngine::GetInstance()->DrawLine(gfxDstBuffer, start, end, invalidatedArea,
                                                       paint.GetStrokeWidth(), paint.GetStrokeColor(), OPA_OPAQUE);
                if ((pathEnd.x == start.x) && (pathEnd.y == start.y)) {
                    DoDrawLineJoin(gfxDstBuffer, start, invalidatedArea, paint);
                }
                pathEnd = end;
                break;
            }
            case CMD_ARC: {
                ArcInfo arcInfo = {{0}};
                arcInfo.imgPos = Point{0, 0};
                arcInfo.startAngle = arcIter->data_.startAngle;
                arcInfo.endAngle = arcIter->data_.endAngle;
                Style drawStyle = StyleDefault::GetDefaultStyle();
                drawStyle.lineWidth_ = static_cast<int16_t>(paint.GetStrokeWidth());
                drawStyle.lineColor_ = paint.GetStrokeColor();
                drawStyle.lineOpa_ = OPA_OPAQUE;
                arcInfo.radius = arcIter->data_.radius + ((paint.GetStrokeWidth() + 1) >> 1);

                GetAbsolutePosition(arcIter->data_.center, rect, style, arcInfo.center);
                BaseGfxEngine::GetInstance()->DrawArc(gfxDstBuffer, arcInfo, invalidatedArea, drawStyle, OPA_OPAQUE,
                                                      CapType::CAP_NONE);
                if (pointIter != path->points_.Begin()) {
                    DoDrawLineJoin(gfxDstBuffer, pathEnd, invalidatedArea, paint);
                }

                GetAbsolutePosition(pointIter->data_, rect, style, pathEnd);
                pointIter = pointIter->next_;
                arcIter = arcIter->next_;
                break;
            }
            case CMD_CLOSE: {
                Point start = pointIter->prev_->data_;
                Point end = pointIter->data_;
                GetAbsolutePosition(start, rect, style, start);
                GetAbsolutePosition(end, rect, style, end);
                if ((start.x != end.x) || (start.y != end.y)) {
                    BaseGfxEngine::GetInstance()->DrawLine(gfxDstBuffer, start, end, invalidatedArea,
                                                           paint.GetStrokeWidth(), paint.GetStrokeColor(), OPA_OPAQUE);
                    if ((pathEnd.x == start.x) && (pathEnd.y == start.y)) {
                        DoDrawLineJoin(gfxDstBuffer, start, invalidatedArea, paint);
                    }
                    pathEnd = end;
                }

                if ((pathEnd.x == end.x) && (pathEnd.y == end.y)) {
                    DoDrawLineJoin(gfxDstBuffer, end, invalidatedArea, paint);
                }
                pointIter = pointIter->next_;
                break;
            }
            default:
                break;
        }
    }
#endif
}

void UICanvas::DoFillPath(BufferInfo& gfxDstBuffer,
                          void* param,
                          const Paint& paint,
                          const Rect& rect,
                          const Rect& invalidatedArea,
                          const Style& style)
{
    BaseGfxEngine::GetInstance()->FillPath(gfxDstBuffer, param, paint, rect, invalidatedArea, style);
}

#if defined(GRAPHIC_ENABLE_DRAW_TEXT_FLAG) && GRAPHIC_ENABLE_DRAW_TEXT_FLAG
void UICanvas::StrokeText(const char* text, const Point& point, const FontStyle& fontStyle, const Paint& paint)
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
        DrawCmd cmd;
        cmd.param = textParam;
        cmd.DeleteParam = DeleteTextParam;
        cmd.DrawGraphics = DoDrawText;
        cmd.paint = paint;
        drawCmdList_.PushBack(cmd);
        Invalidate();
        SetStartPosition(point);
    }
}
#endif

Point UICanvas::MeasureText(const char* text, const FontStyle& fontStyle)
{
    Text* textCompent = new Text;
    textCompent->SetText(text);
    textCompent->SetFont(fontStyle.fontName, fontStyle.fontSize);
    textCompent->SetDirect(static_cast<UITextLanguageDirect>(fontStyle.direct));
    textCompent->SetAlign(static_cast<UITextLanguageAlignment>(fontStyle.align));
    Style drawStyle;
    drawStyle.SetStyle(STYLE_LETTER_SPACE, fontStyle.letterSpace);
    textCompent->ReMeasureTextSize(this->GetRect(), drawStyle);
    Point textSize = textCompent->GetTextSize();
    delete textCompent;
    return textSize;
}

void UICanvas::BlitMapBuffer(BufferInfo &gfxDstBuffer, BufferInfo& gfxMapBuffer, Rect& textRect,
                             TransformMap& transMap, const Rect& invalidatedArea)
{
    Rect invalidRect = textRect;
    transMap.SetTransMapRect(textRect);
    if (invalidRect.Intersect(invalidRect, transMap.GetBoxRect())) {
        uint8_t pxSize = DrawUtils::GetPxSizeByColorMode(gfxDstBuffer.mode);
        ImageInfo imageInfo;
        imageInfo.header.colorMode = gfxDstBuffer.mode;
        imageInfo.dataSize = gfxMapBuffer.width * gfxMapBuffer.height *
                DrawUtils::GetByteSizeByColorMode(gfxDstBuffer.mode);
        imageInfo.header.width = gfxMapBuffer.width;
        imageInfo.header.height = gfxMapBuffer.height;
        imageInfo.header.reserved = 0;
        uint8_t* addr = reinterpret_cast<uint8_t*>(gfxMapBuffer.virAddr);
        imageInfo.data = addr;
        TransformDataInfo imageTranDataInfo = {imageInfo.header, imageInfo.data, pxSize,
                                               BlurLevel::LEVEL0, TransformAlgorithm::BILINEAR};
        BaseGfxEngine::GetInstance()->DrawTransform(gfxDstBuffer, invalidatedArea, {0, 0}, Color::Black(),
                                                    OPA_OPAQUE, transMap, imageTranDataInfo);
    }
}

#if defined(GRAPHIC_ENABLE_DRAW_TEXT_FLAG) && GRAPHIC_ENABLE_DRAW_TEXT_FLAG
void UICanvas::DoDrawText(BufferInfo& gfxDstBuffer,
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
    text->ReMeasureTextSize(textRect, drawStyle);
    if (text->GetTextSize().x == 0 || text->GetTextSize().y == 0) {
        return;
    }
    textRect.SetWidth(text->GetTextSize().x + 1);
    textRect.SetHeight(text->GetTextSize().y + 1);
    OpacityType opa = DrawUtils::GetMixOpacity(textParam->fontOpa, style.bgOpa_);
    if (!paint.GetTransAffine().IsIdentity()) {
        Rect textImageRect(0, 0, textRect.GetWidth(), textRect.GetHeight());
        BufferInfo* mapBufferInfo = UpdateMapBufferInfo(gfxDstBuffer, textImageRect);
        text->OnDraw(*mapBufferInfo, textImageRect, textImageRect, textImageRect, 0, drawStyle,
                     Text::TEXT_ELLIPSIS_END_INV, opa);
        TransformMap trans;
        trans.SetTransMapRect(textRect);
        trans.Scale(Vector2<float>(static_cast<float>(paint.GetScaleX()), static_cast<float>(paint.GetScaleY())),
                    Vector2<float>(0, 0));
        float angle = paint.GetRotateAngle();
        trans.Rotate(MATH_ROUND(angle),  Vector2<float>(0, 0));
        trans.Translate(Vector2<int16_t>(paint.GetTranslateX(), paint.GetTranslateY()));
        BlitMapBuffer(gfxDstBuffer, *mapBufferInfo, textRect, trans, invalidatedArea);
    } else {
        text->OnDraw(gfxDstBuffer, invalidatedArea, textRect, textRect, 0,
                     drawStyle, Text::TEXT_ELLIPSIS_END_INV, opa);
    }
}
#endif

void UICanvas::InitGfxMapBuffer(const BufferInfo& srcBuff, const Rect& rect)
{
    gfxMapBuffer_ = new BufferInfo();
    gfxMapBuffer_->rect = rect;
    gfxMapBuffer_->mode = srcBuff.mode;
    gfxMapBuffer_->color = srcBuff.color;
    gfxMapBuffer_->width = static_cast<uint16_t>(rect.GetWidth());
    gfxMapBuffer_->height = static_cast<uint16_t>(rect.GetHeight());
    uint8_t destByteSize = DrawUtils::GetByteSizeByColorMode(srcBuff.mode);
    gfxMapBuffer_->stride = static_cast<int32_t>(gfxMapBuffer_->width) * static_cast<int32_t>(destByteSize);
    uint32_t buffSize = gfxMapBuffer_->height * gfxMapBuffer_->stride;
    BaseGfxEngine* baseGfxEngine = BaseGfxEngine::GetInstance();
    gfxMapBuffer_->virAddr = baseGfxEngine->AllocBuffer(buffSize, BUFFER_MAP_SURFACE);
    gfxMapBuffer_->phyAddr = gfxMapBuffer_->virAddr;

    errno_t err = memset_s(gfxMapBuffer_->virAddr, buffSize, 0, buffSize);
    if (err != EOK) {
        baseGfxEngine->FreeBuffer(static_cast<uint8_t*>(gfxMapBuffer_->virAddr), BUFFER_MAP_SURFACE);
        GRAPHIC_LOGE("memset_s gfxMapBuffer_ fail");
        return;
    }
}

BufferInfo* UICanvas::UpdateMapBufferInfo(const BufferInfo& srcBuff, const Rect& rect)
{
    if (gfxMapBuffer_ == nullptr) {
        InitGfxMapBuffer(srcBuff, rect);
        return gfxMapBuffer_;
    }

    if (rect.GetWidth() != gfxMapBuffer_->width ||
        rect.GetHeight() != gfxMapBuffer_->height ||
        srcBuff.mode != gfxMapBuffer_->mode) {
        DestroyMapBufferInfo();
        InitGfxMapBuffer(srcBuff, rect);
    } else {
        uint32_t buffSize = gfxMapBuffer_->height * gfxMapBuffer_->stride;
        errno_t err = memset_s(gfxMapBuffer_->virAddr, buffSize, 0, buffSize);
        if (err != EOK) {
            BaseGfxEngine::GetInstance()->FreeBuffer(static_cast<uint8_t*>(gfxMapBuffer_->virAddr), BUFFER_MAP_SURFACE);
            GRAPHIC_LOGE("memset_s gfxMapBuffer_ fail");
        }
    }
    return gfxMapBuffer_;
}

void UICanvas::DestroyMapBufferInfo()
{
    if (gfxMapBuffer_ != nullptr) {
        BaseGfxEngine::GetInstance()->FreeBuffer(static_cast<uint8_t*>(gfxMapBuffer_->virAddr), BUFFER_MAP_SURFACE);
        gfxMapBuffer_->virAddr = nullptr;
        gfxMapBuffer_->phyAddr = nullptr;
        delete gfxMapBuffer_;
        gfxMapBuffer_ = nullptr;
    }
}

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
void UICanvas::BlendRaster(const Paint& paint,
                           void* param,
                           RasterizerScanlineAntialias& blendRasterizer,
                           RasterizerScanlineAntialias& rasterizer,
                           RenderBase& renBase,
                           TransAffine& transform,
                           SpanBase& spanGen,
                           const Rect& rect,
                           bool isStroke)
{
    TransAffine gradientMatrixBlend;
    GeometryScanline scanline1;
    GeometryScanline scanline2;
    FillBase allocator1;

    if (IsSoild(paint)) {
        Rgba8T blendColor;
        DrawCanvas::RenderBlendSolid(paint, blendColor, isStroke);
        SpanSoildColor spanBlendSoildColor(blendColor);
        BlendScanLine(paint.GetGlobalCompositeOperation(), blendRasterizer, rasterizer,
                      scanline1, scanline2, renBase, allocator1, spanBlendSoildColor, spanGen);
    }
#if defined(GRAPHIC_ENABLE_GRADIENT_FILL_FLAG) && GRAPHIC_ENABLE_GRADIENT_FILL_FLAG
    FillInterpolator interpolatorTypeBlend(gradientMatrixBlend);
    FillGradientLut gradientColorModeBlend;
    if (paint.GetStyle() == Paint::GRADIENT) {
        DrawCanvas::BuildGradientColor(paint, gradientColorModeBlend);
        if (paint.GetGradient() == Paint::Linear) {
            float distance = 0;
            DrawCanvas::BuildLineGradientMatrix(paint, gradientMatrixBlend, transform, distance);
            GradientLinearCalculate gradientLinearCalculate;
            FillGradient span(interpolatorTypeBlend, gradientLinearCalculate,
                                    gradientColorModeBlend, 0, distance);
            BlendScanLine(paint.GetGlobalCompositeOperation(), blendRasterizer, rasterizer,
                          scanline1, scanline2, renBase, allocator1, span, spanGen);
        } else if (paint.GetGradient() == Paint::Radial) {
            Paint::RadialGradientPoint radialPoint = paint.GetRadialGradientPoint();
            float startRadius = 0;
            float endRadius = 0;
            DrawCanvas::BuildRadialGradientMatrix(paint, gradientMatrixBlend, transform, startRadius, endRadius);
            GradientRadialCalculate gradientRadialCalculate(endRadius, radialPoint.x0 - radialPoint.x1,
                                                            radialPoint.y0 - radialPoint.y1);
            FillGradient span(interpolatorTypeBlend, gradientRadialCalculate, gradientColorModeBlend,
                                    startRadius, endRadius);
            BlendScanLine(paint.GetGlobalCompositeOperation(), blendRasterizer, rasterizer,
                          scanline1, scanline2, renBase, allocator1, span, spanGen);
        }
    }
#endif
#if defined(GRAPHIC_ENABLE_PATTERN_FILL_FLAG) && GRAPHIC_ENABLE_PATTERN_FILL_FLAG
    if (paint.GetStyle() == Paint::PATTERN) {
        if (param == nullptr) {
            return;
        }

        PathParam* pathParam = static_cast<PathParam*>(param);

        ImageParam* imageParam = static_cast<ImageParam*>(pathParam->imageParam);

        if (imageParam->image == nullptr) {
            return;
        }
        FillPatternRgba spanPattern(imageParam->image->GetImageInfo(), paint.GetPatternRepeatMode(), rect.GetLeft(),
                                    rect.GetTop());
        BlendScanLine(paint.GetGlobalCompositeOperation(), blendRasterizer, rasterizer,
                      scanline1, scanline2, renBase, allocator1, spanPattern, spanGen);
    }
#endif
}
#endif // ENABLE_CANVAS_EXTEND
} // namespace OHOS
