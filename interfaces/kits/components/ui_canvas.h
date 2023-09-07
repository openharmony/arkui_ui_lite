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

/**
 * @addtogroup UI_Components
 * @{
 *
 * @brief Defines UI components such as buttons, texts, images, lists, and progress bars.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file ui_canvas.h
 *
 * @brief Defines the attributes of the canvas component and provides functions for drawing rectangles,
 *        circles, and others.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef GRAPHIC_LITE_UI_CANVAS_H
#define GRAPHIC_LITE_UI_CANVAS_H

#include "common/image.h"
#include "components/ui_label.h"
#include "gfx_utils/diagram/depiction/depict_curve.h"
#include "gfx_utils/diagram/depiction/depict_dash.h"
#include "gfx_utils/diagram/depiction/depict_stroke.h"
#include "gfx_utils/diagram/imagefilter/filter_blur.h"
#include "gfx_utils/diagram/rasterizer/rasterizer_scanline_antialias.h"
#include "gfx_utils/diagram/scanline/geometry_scanline.h"
#include "gfx_utils/diagram/spancolorfill/fill_base.h"
#include "gfx_utils/diagram/spancolorfill/fill_gradient.h"
#include "gfx_utils/diagram/spancolorfill/fill_gradient_lut.h"
#include "gfx_utils/diagram/spancolorfill/fill_interpolator.h"
#include "gfx_utils/diagram/spancolorfill/fill_pattern_rgba.h"
#include "gfx_utils/diagram/vertexprimitive/geometry_path_storage.h"
#include "ui_extend_image_view.h"
#include "gfx_utils/file.h"
#include "gfx_utils/list.h"
#include "gfx_utils/diagram/common/paint.h"

namespace OHOS {
class RenderBase;

/**
 * @brief Defines a canvas, which is used to draw multiple types of 2D graphs.
 *
 * @since 1.0
 * @version 1.0
 */
class UICanvas : public UIView {
public:
    /**
     * @brief A constructor used to create a <b>UICanvas</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    UICanvas() : startPoint_({0, 0}), vertices_(nullptr), path_(nullptr) {}

    /**
     * @brief A destructor used to delete the <b>UICanvas</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    virtual ~UICanvas();

    /**
     * @brief Obtains the view type.
     *
     * @return Returns the view type. For details, see {@link UIViewType}.
     * @since 1.0
     * @version 1.0
     */
    UIViewType GetViewType() const override
    {
        return UI_CANVAS;
    }

    /**
     * @brief Clears the entire canvas.
     *
     * @since 1.0
     * @version 1.0
     */
    void Clear();

    /**
     * @brief Sets the coordinates of the start point for drawing a line. For example, if <b>startPoint</b> is
     *        set to {50, 50}, the line is drawn from this set of coordinates on the canvas.
     *
     * @param startPoint Indicates the coordinates of the start point.
     * @see GetStartPosition
     * @since 1.0
     * @version 1.0
     */
    void SetStartPosition(const Point& startPoint)
    {
        startPoint_ = startPoint;
    }

    /**
     * @brief Obtains the coordinates of the start point of a line.
     *
     * @return Returns the coordinates of the start point.
     * @see SetStartPosition
     * @since 1.0
     * @version 1.0
     */
    const Point& GetStartPosition() const
    {
        return startPoint_;
    }

    /**
     * @brief Draws a straight line.
     *
     * If {@link SetStartPosition} is not used to set the coordinates of the start point of the line, the drawing
     * starts from the end point of the last line.
     *
     * @param endPoint Indicates the end point of the straight line.
     * @param paint    Indicates the straight line style. For details, see {@link Paint}.
     * @since 1.0
     * @version 1.0
     */
    void DrawLine(const Point& endPoint, const Paint& paint);

    /**
     * @brief Draws a straight line from the coordinates of the start point.
     *
     * @param startPoint Indicates the coordinates of the start point.
     * @param endPoint   Indicates the coordinates of the end point.
     * @param paint      Indicates the straight line style. For details, see {@link Paint}.
     * @since 1.0
     * @version 1.0
     */
    void DrawLine(const Point& startPoint, const Point& endPoint, const Paint& paint);

    /**
     * @brief Draws a cubic Bezier curve.
     *
     * If {@link SetStartPosition} is not used to set the coordinates of the start point of the curve,
     * the drawing starts from the end point of the last line.
     * Currently, the opacity cannot be set, and the maximum line width is <b>3</b>.
     *
     * @param control1 Indicates the coordinates of the first control point of the cubic Bezier curve.
     * @param control2 Indicates the coordinates of the second control point of the cubic Bezier curve.
     * @param endPoint Indicates the coordinates of the end point of the cubic Bezier curve.
     * @param paint    Indicates the curve style. For details, see {@link Paint}.
     * @since 1.0
     * @version 1.0
     */
    void DrawCurve(const Point& control1, const Point& control2, const Point& endPoint, const Paint& paint);

    /**
     * @brief Draws a cubic Bezier curve from the start point coordinates.
     *
     * Currently, the opacity cannot be set, and the maximum line width is <b>3</b>.
     *
     * @param startPoint Indicates the coordinates of the start point of the cubic Bezier curve.
     * @param control1   Indicates the coordinates of the first control point of the cubic Bezier curve.
     * @param control2   Indicates the coordinates of the second control point of the cubic Bezier curve.
     * @param endPoint   Indicates the coordinates of the end point of the cubic Bezier curve.
     * @param paint      Indicates the curve style. For details, see {@link Paint}.
     * @since 1.0
     * @version 1.0
     */
    void DrawCurve(const Point& startPoint, const Point& control1, const Point& control2,
                   const Point& endPoint, const Paint& paint);

    /**
     * @brief Draws a rectangle.
     *
     * @param startPoint Indicates the coordinates of the point at the upper left corner of the rectangle.
     * @param height     Indicates the height of the rectangle.
     * @param width      Indicates the width of the rectangle.
     * @param paint      Indicates the rectangle style. For details, see {@link Paint}.
     * @since 1.0
     * @version 1.0
     */
    void DrawRect(const Point& startPoint, int16_t height, int16_t width, const Paint& paint);

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    /**
     * @brief Draws a rectangular path with no fill.
     * @param startPoint starting point
     * @param height
     * @param width
     * @param paint paint brush
     */
    void StrokeRect(const Point& startPoint, int16_t height, int16_t width, const Paint& paint);

    /**
     * @brief Clears pixels within a rectangle
     * @param startPoint starting point
     * @param height
     * @param width
     */
    void ClearRect(const Point& startPoint, int16_t height, int16_t width);
#endif

    /**
     * @brief Draws a circle.
     *
     * @param center Indicates the coordinates of the circle center.
     * @param radius Indicates the radius of the circle.
     * @param paint  Indicates the circle style. For details, see {@link Paint}.
     * @since 1.0
     * @version 1.0
     */
    void DrawCircle(const Point& center, uint16_t radius, const Paint& paint);

    /**
     * @brief Draws a sector.
     *
     * When the start angle is smaller than the end angle, the sector is drawn clockwise.
     * Otherwise, the sector is drawn counterclockwise.
     *
     * @param center     Indicates the coordinates of the sector's center.
     * @param radius     Indicates the radius of the sector.
     * @param startAngle Indicates the start angle of the sector. Value <b>0</b> indicates the 12-o'clock direction,
     *                   and <b>90</b> indicates the 3-o'clock direction.
     * @param endAngle   Indicates the end angle of the sector. Value <b>0</b> indicates the 12-o'clock direction,
     *                   and <b>90</b> indicates the 3-o'clock direction.
     * @param paint      Indicates the sector style. For details, see {@link Paint}.
     * @since 1.0
     * @version 1.0
     */
    void DrawSector(const Point& center, uint16_t radius, int16_t startAngle, int16_t endAngle, const Paint& paint);

    /**
     * @brief Draws an arc.
     *
     * Only stroke is supported. \n
     * When the start angle is smaller than the end angle, the sector is drawn clockwise.
     * Otherwise, the sector is drawn counterclockwise. \n
    *
     * @param center     Indicates the coordinates of the arc's center.
     * @param radius     Indicates the radius of the arc.
     * @param startAngle Indicates the start angle of the arc. Value <b>0</b> indicates the 12-o'clock direction,
     *                   and <b>90</b> indicates the 3-o'clock direction.
     * @param endAngle   Indicates the end angle of the arc. Value <b>0</b> indicates the 12-o'clock direction,
     *                   and <b>90</b> indicates the 3-o'clock direction.
     * @param paint      Indicates the arc style. For details, see {@link Paint}.
     * @since 1.0
     * @version 1.0
     */
    void DrawArc(const Point& center, uint16_t radius, int16_t startAngle, int16_t endAngle, const Paint& paint);

#if defined(GRAPHIC_ENABLE_DRAW_IMAGE_FLAG) && GRAPHIC_ENABLE_DRAW_IMAGE_FLAG
    /**
     * @brief Draws an image.
     *
     * @param startPoint Indicates the coordinates of the start point.
     * @param image      Indicates the pointer to the image source.
     * @param paint      Indicates the image style. For details, see {@link Paint}.
     * @since 1.0
     * @version 1.0
     */
    void DrawImage(const Point& startPoint, const char* image, const Paint& paint);

    void DrawImage(const Point& startPoint, const char* image, const Paint& paint, int16_t width, int16_t height);
#endif
    /**
     * @brief Defines the font style.
     */
    struct FontStyle : public HeapBase {
        /** Text direction. For details, see {@link UITextLanguageDirect}. */
        UITextLanguageDirect direct;
        /** Text alignment mode. For details, see {@link UITextLanguageAlignment}. */
        UITextLanguageAlignment align;
        /** Font size */
        uint8_t fontSize;
        /** Letter-spacing */
        int16_t letterSpace;
        /** Font name */
        const char* fontName;
    };

    struct DrawCmd : public HeapBase {
        Paint paint;
        void* param;
        void (*DrawGraphics)(BufferInfo&, void*, const Paint&, const Rect&, const Rect&, const Style&);
        void (*DeleteParam)(void*);
    };

    /**
     * @brief Draws text.
     *
     * Only fill is supported. \n
     * If the text length exceeds the value of <b>maxWidth</b>, the text will be truncated. \n
     *
     * @param startPoint Indicates the coordinates of the start point.
     * @param text       Indicates the pointer to the text content.
     * @param maxWidth   Indicates the maximum width of the text that can be displayed. If the maximum width is
     *                   exceeded, the text is truncated.
     * @param fontStyle  Indicates the text layout and font style. For details, see {@link FontStyle}.
     * @param paint      Indicates the text style. For details, see {@link Paint}.
     * @since 1.0
     * @version 1.0
     */
    void DrawLabel(const Point& startPoint, const char* text, uint16_t maxWidth, const FontStyle& fontStyle,
                   const Paint& paint);

    /**
     * @brief Creates a path.
     *
     * A round corner can be used to join two lines. Currently, miter and bevel joints are not supported.
     * To draw this path, you need to call {@link DrawPath}.
     *
     * @since 3.0
     * @version 5.0
     */
    void BeginPath();

    /**
     * @brief Moves the start point of this path to a specified point.
     *
     * @param point Indicates the specified point to move to.
     * @since 3.0
     * @version 5.0
     */
    void MoveTo(const Point& point);

    /**
     * @brief Creates a straight line from the end point of this path to a specified point.
     *
     * @param point Indicates the coordinates of the specified point.
     * @since 3.0
     * @version 5.0
     */
    void LineTo(const Point& point);

    /**
     * @brief Creates an arc path.
     *
     * @param center     Indicates the coordinates of the arc's center point.
     * @param radius     Indicates the radius of the arc.
     * @param startAngle Indicates the start angle of the arc.
     *                   The value <b>0</b> indicates the 12-o'clock direction,
     *                   and <b>90</b> indicates the 3-o'clock direction.
     * @param endAngle   Indicates the end angle of the arc.
     *                   The value <b>0</b> indicates the 12-o'clock direction,
     *                   and <b>90</b> indicates the 3-o'clock direction.
     * @since 3.0
     * @version 5.0
     */
    void ArcTo(const Point& center, uint16_t radius, int16_t startAngle, int16_t endAngle);

    /**
     * @brief Creates a rectangular path.
     *
     * @param point  Indicates the coordinates of the rectangle's upper left corner.
     * @param height Indicates the height of the rectangle.
     * @param width  Indicates the width of the rectangle.
     * @since 3.0
     * @version 5.0
     */
    void AddRect(const Point& point, int16_t height, int16_t width);

    /**
     * @brief Closes this path.
     *
     * @since 3.0
     * @version 5.0
     */
    void ClosePath();

    /**
     * @brief Draws this path.
     *
     * @param paint Indicates the path style. For details, see {@link Paint}.
     * @since 3.0
     * @version 5.0
     */
    void DrawPath(const Paint& paint);

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    /**
     * @brief Fill polygon path
     * @param paint fill paint
     * @since 3.0
     * @version 5.0
     */
    void FillPath(const Paint& paint);
#endif

#if defined(GRAPHIC_ENABLE_DRAW_TEXT_FLAG) && GRAPHIC_ENABLE_DRAW_TEXT_FLAG
    /*  Draw text on canvas */
    void StrokeText(const char* text, const Point& point, const FontStyle& fontStyle, const Paint& paint);
#endif

    /* Returns an object containing the specified text width */
    Point MeasureText(const char* text, const FontStyle& fontStyle);

    /* Save history status */
    void Save(Paint paint)
    {
        paintStack_.PushBack(paint);
    }

    /**
     * @brief Restore to last historical state
     * @return Returns the last paint value.
     * @since 3.0
     * @version 5.0
     */
    Paint Restore()
    {
        Paint paint;
        if (paintStack_.IsEmpty()) {
            return paint;
        }
        paint = paintStack_.Back();
        paintStack_.PopBack();
        return paint;
    }

#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    void OnBlendDraw(BufferInfo& gfxDstBuffer, const Rect& trunc);
#endif

    void OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea) override;

    static void BlendRaster(const Paint& paint,
                        void* param,
                        RasterizerScanlineAntialias& blendRasterizer,
                        RasterizerScanlineAntialias& rasterizer,
                        RenderBase& renBase,
                        TransAffine& transform,
                        SpanBase& spanGen,
                        const Rect& rect,
                        bool isStroke);
    static void DeleteImageParam(void* param);
    static void DeletePathParam(void* param);
protected:
    constexpr static uint8_t MAX_CURVE_WIDTH = 3;

    struct LineParam : public HeapBase {
        Point start;
        Point end;
    };

    struct CurveParam : public HeapBase {
        Point start;
        Point control1;
        Point control2;
        Point end;
    };

    struct RectParam : public HeapBase {
        Point start;
        int16_t height;
        int16_t width;
    };

    struct CircleParam : public HeapBase {
        Point center;
        uint16_t radius;
    };

    struct ArcParam : public HeapBase {
        Point center;
        uint16_t radius;
        int16_t startAngle;
        int16_t endAngle;
    };

    enum PathCmd {
        CMD_MOVE_TO,
        CMD_LINE_TO,
        CMD_ARC,
        CMD_CLOSE,
    };

    class UICanvasPath : public HeapBase {
    public:
        UICanvasPath() : startPos_({ 0, 0 }), strokeCount_(0) {};
        ~UICanvasPath();
        List<Point> points_;
        List<PathCmd> cmd_;
        List<ArcParam> arcParam_;
        Point startPos_;
        uint16_t strokeCount_;
    };

#if (!(defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND))
    struct PathParam : public HeapBase {
        UICanvasPath* path;
        uint16_t count;
    };

    struct ImageParam : public HeapBase {
        Point start;
        uint16_t height;
        uint16_t width;
        Image* image;
    };
#endif

    struct TextParam : public HeapBase {
        const char* text;
        Point position;
        Color32 fontColor;
        uint8_t fontOpa;
        FontStyle fontStyle;
        Text* textComment;
        TextParam() : text(nullptr), position({0, 0}), fontOpa(0)
        {
            fontColor.full = 0;
            fontStyle.direct = UITextLanguageDirect::TEXT_DIRECT_LTR;
            fontStyle.align = UITextLanguageAlignment::TEXT_ALIGNMENT_LEFT;
            fontStyle.fontSize = 0;
            fontStyle.letterSpace = 0;
            fontStyle.fontName = nullptr;
            textComment = new Text;
        }

        ~TextParam()
        {
            if (textComment) {
                delete textComment;
                textComment = nullptr;
            }
        };
    };

    Point startPoint_;
    UICanvasVertices* vertices_;
    UICanvasPath* path_;
    List<DrawCmd> drawCmdList_;
    // Save historical modification information of paint
    List<Paint> paintStack_;
    static BufferInfo* gfxMapBuffer_;

    static void DeleteLineParam(void* param)
    {
        LineParam* lineParam = static_cast<LineParam*>(param);
        delete lineParam;
    }

    static void DeleteCurveParam(void* param)
    {
        CurveParam* curveParam = static_cast<CurveParam*>(param);
        delete curveParam;
    }

    static void DeleteRectParam(void* param)
    {
        RectParam* rectParam = static_cast<RectParam*>(param);
        delete rectParam;
    }

    static void DeleteCircleParam(void* param)
    {
        CircleParam* circleParam = static_cast<CircleParam*>(param);
        delete circleParam;
    }

    static void DeleteArcParam(void* param)
    {
        ArcParam* arcParam = static_cast<ArcParam*>(param);
        delete arcParam;
    }

    static void DeleteLabel(void* param)
    {
        UILabel* label = static_cast<UILabel*>(param);
        delete label;
    }

    static void DeleteImageView(void* param)
    {
        UIExtendImageView* imageView = static_cast<UIExtendImageView*>(param);
        delete imageView;
    }

    static void DeleteTextParam(void* param)
    {
        TextParam* textParam = static_cast<TextParam*>(param);
        delete textParam;
    }

    static void DoDrawLine(BufferInfo& gfxDstBuffer,
                           void* param,
                           const Paint& paint,
                           const Rect& rect,
                           const Rect& invalidatedArea,
                           const Style& style);
    static void DoDrawCurve(BufferInfo& gfxDstBuffer,
                            void* param,
                            const Paint& paint,
                            const Rect& rect,
                            const Rect& invalidatedArea,
                            const Style& style);
    static void DoDrawRect(BufferInfo& gfxDstBuffer,
                           void* param,
                           const Paint& paint,
                           const Rect& rect,
                           const Rect& invalidatedArea,
                           const Style& style);
    static void DoFillRect(BufferInfo& gfxDstBuffer,
                           void* param,
                           const Paint& paint,
                           const Rect& rect,
                           const Rect& invalidatedArea,
                           const Style& style);
    static void DoDrawCircle(BufferInfo& gfxDstBuffer,
                             void* param,
                             const Paint& paint,
                             const Rect& rect,
                             const Rect& invalidatedArea,
                             const Style& style);
    static void DoDrawArc(BufferInfo& gfxDstBuffer,
                          void* param,
                          const Paint& paint,
                          const Rect& rect,
                          const Rect& invalidatedArea,
                          const Style& style);
#if defined(GRAPHIC_ENABLE_DRAW_IMAGE_FLAG) && GRAPHIC_ENABLE_DRAW_IMAGE_FLAG
    static void DoDrawImage(BufferInfo& gfxDstBuffer,
                            void* param,
                            const Paint& paint,
                            const Rect& rect,
                            const Rect& invalidatedArea,
                            const Style& style);
#endif

    static void DoDrawLabel(BufferInfo& gfxDstBuffer,
                            void* param,
                            const Paint& paint,
                            const Rect& rect,
                            const Rect& invalidatedArea,
                            const Style& style);
    static void DoDrawPath(BufferInfo& gfxDstBuffer,
                           void* param,
                           const Paint& paint,
                           const Rect& rect,
                           const Rect& invalidatedArea,
                           const Style& style);
    static void GetAbsolutePosition(const Point& prePoint, const Rect& rect, const Style& style, Point& point);
    static void DoDrawLineJoin(BufferInfo& gfxDstBuffer,
                               const Point& center,
                               const Rect& invalidatedArea,
                               const Paint& paint);

    static void DoFillPath(BufferInfo& gfxDstBuffer,
                           void* param,
                           const Paint& paint,
                           const Rect& rect,
                           const Rect& invalidatedArea,
                           const Style& style);

    static void  BlitMapBuffer(BufferInfo &gfxDstBuffer, BufferInfo& gfxMapBuffer,
                              Rect& textRect, TransformMap& transMap, const Rect& invalidatedArea);

#if defined(GRAPHIC_ENABLE_DRAW_TEXT_FLAG) && GRAPHIC_ENABLE_DRAW_TEXT_FLAG
    static void DoDrawText(BufferInfo& gfxDstBuffer, void* param, const Paint& paint, const Rect& rect,
                           const Rect& invalidatedArea, const Style& style);
#endif
    /**
     * Assembly parameter setting lineweight，LineCap，LineJoin
     */
    template <class LineStyle>
    static void LineStyleCalc(DepictStroke<LineStyle>& strokeLineStyle, const Paint& paint)
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

    static bool IsSoild(const Paint& paint)
    {
        if (paint.GetStyle() == Paint::STROKE_STYLE ||
           paint.GetStyle() == Paint::FILL_STYLE ||
           paint.GetStyle() == Paint::STROKE_FILL_STYLE) {
            return true;
        }
        return false;
    }

    void DrawRectSetCmd(const Point& startPoint, int16_t height, int16_t width, const Paint& paint,
                        Paint::PaintStyle paintStyle);
    static void InitGfxMapBuffer(const BufferInfo& srcBuff, const Rect& rect);
    static BufferInfo* UpdateMapBufferInfo(const BufferInfo& srcBuff, const Rect& rect);
    static void DestroyMapBufferInfo();
    void SetArcParamInfo(const Point& center, uint16_t radius, int16_t startAngle, int16_t endAngle);
#if defined(ENABLE_CANVAS_EXTEND) && ENABLE_CANVAS_EXTEND
    void SetDrawLinePath(const Point& startPoint, int16_t height, int16_t width, const Paint& paint);
#endif
};
} // namespace OHOS
#endif // GRAPHIC_LITE_UI_CANVAS_H
