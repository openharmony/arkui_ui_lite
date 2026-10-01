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

#include "svg/svg_text_nodes.h"
#include "common/text.h"
#include "common/typed_text.h"
#include "font/ui_font.h"
#include "font/ui_font_header.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_document.h"
#include <climits>
#include <cmath>
#include <cstring>

namespace OHOS {

namespace {

// Default font used by SVG <text>. This must match a font actually registered at runtime
// via UIFont::RegisterFontInfo() (see GraphicStartUp::InitFontEngine). Changing this to a
// name that is not registered is a no-op: Text::SetFont keeps the previous font id when
// GetFontId() fails to resolve the name. CJK glyphs only render if the registered font's
// cmap contains the corresponding codepoints; a Latin-only registered font makes Chinese
// invisible while ASCII still draws.
constexpr uint16_t MAX_FONT_SIZE = 256;
constexpr uint8_t TEXT_ANCHOR_START = 0;
constexpr uint8_t TEXT_ANCHOR_MIDDLE = 1;
constexpr uint8_t TEXT_ANCHOR_END = 2;
// Rect::GetWidth()/GetHeight() return right-left+1. Using INT16_MAX as a coordinate
// makes the computed extent overflow to a negative value, so ReMeasureTextSize is skipped.
constexpr int16_t MEASURE_RECT_MAX_EXTENT = INT16_MAX - 1;
constexpr int16_t HALF_DIVISOR = 2;
// Metrics of the default SVG font at the given size, in pixels.
// Text sizes are laid out with the baseline at the <text> y coordinate, so the
// ink box is [baseline - ascender, baseline - descender]. FreeType reports the
// descender as a negative offset below the baseline.
// Returns false when the font engine cannot supply metrics (no font registered,
// bitmap-only build, host unit tests); callers then fall back to the previous
// font-size based approximation.
bool GetFontMetrics(uint8_t fontSize, int16_t& ascender, int16_t& descender)
{
    uint16_t fontId = UIFont::GetInstance()->GetFontId(DEFAULT_VECTOR_FONT_FILENAME, fontSize);
    FontHeader header;
    // GetFontHeader returns 0 on success, non-zero when the font engine cannot
    // supply metrics (no font registered, bitmap-only build, host unit tests).
    int8_t ret = UIFont::GetInstance()->GetFontHeader(header, fontId, fontSize);
    if (ret != 0) {
        return false;
    }
    if (header.ascender <= 0) {
        return false;
    }
    ascender = header.ascender;
    descender = header.descender;
    return true;
}

// Ink box for a single line of text whose baseline sits at baselineY.
// Falls back to the font-size sized box when real metrics are unavailable.
void GetTextInkBox(uint8_t fontSize, int16_t baselineY, int16_t& top, int16_t& bottom)
{
    int16_t ascender = static_cast<int16_t>(fontSize);
    int16_t descender = 0;
    GetFontMetrics(fontSize, ascender, descender);
    top = static_cast<int16_t>(baselineY - ascender);
    bottom = static_cast<int16_t>(baselineY - descender);
}

using SvgAttributeParser::MATRIX_INDEX_A;
using SvgAttributeParser::MATRIX_INDEX_D;

// Extract the rotation angle (degrees) carried by an accumulated transform.
// The affine is laid out as [sx shx tx; shy sy ty]. For a (possibly scaled)
// rotation sx = s*cosθ and shy = s*sinθ, so atan2(shy, sx) recovers θ in the
// SVG y-down / clockwise-positive convention. Returns 0 when the matrix is
// effectively a pure translation/identity with no rotation component.
constexpr float RAD_TO_DEG = 180.0f / 3.14159265358979323846f;
constexpr float ROTATION_EPSILON_DEG = 0.01f;
static float GetRecordRotationDeg(const TransAffine& record)
{
    const float* m = record.GetData();
    return std::atan2(m[MATRIX_INDEX_D], m[MATRIX_INDEX_A]) * RAD_TO_DEG;
}

// Ink overhang relative to the text engine's line box / advance box.  For a
// given string, the actual glyph pixels may extend above the ascender, below
// the descender, left of the origin, or right of the final advance.  When a
// rotated SVG <text> bitmap is sized only to the line box, these overhangs are
// clipped; expanding the leaf view and map buffer by these metrics fixes it.
struct TextInkOverhang {
    int16_t left = 0;
    int16_t right = 0;
    int16_t top = 0;
    int16_t bottom = 0;
};

struct GlyphBounds {
    int16_t minLeft = 0;
    int16_t maxTop = 0;
    int16_t maxRight = 0;
    int16_t minBottom = 0;
};

static void InitGlyphBounds(GlyphBounds& bounds, int16_t left, int16_t top, int16_t right, int16_t bottom)
{
    bounds.minLeft = left;
    bounds.maxTop = top;
    bounds.maxRight = right;
    bounds.minBottom = bottom;
}

static void UpdateGlyphBounds(GlyphBounds& bounds, int16_t left, int16_t top, int16_t right, int16_t bottom)
{
    if (left < bounds.minLeft) {
        bounds.minLeft = left;
    }
    if (top > bounds.maxTop) {
        bounds.maxTop = top;
    }
    if (right > bounds.maxRight) {
        bounds.maxRight = right;
    }
    if (bottom < bounds.minBottom) {
        bounds.minBottom = bottom;
    }
}

struct GlyphMetrics {
    int16_t advance = 0;
    int16_t left = 0;
    int16_t top = 0;
    int16_t right = 0;
    int16_t bottom = 0;
    bool hasInk = false;
};

static GlyphMetrics GetGlyphMetrics(uint32_t letter, uint16_t fontId, uint8_t fontSize, int32_t cursor)
{
    GlyphMetrics m;
    UIFont* fontEngine = UIFont::GetInstance();
    if (fontEngine == nullptr) {
        return m;
    }
    GlyphNode glyphNode;
    uint8_t* fontMap = fontEngine->GetBitmap(letter, glyphNode, fontId, fontSize, 0);
    m.advance = static_cast<int16_t>(fontEngine->GetWidth(letter, fontId, fontSize, 0));
    if (fontMap != nullptr) {
        m.hasInk = true;
        m.left = static_cast<int16_t>(cursor) + glyphNode.left;
        m.top = glyphNode.top;
        m.right = static_cast<int16_t>(cursor) + glyphNode.left +
                  static_cast<int16_t>(glyphNode.cols) - 1;
        m.bottom = static_cast<int16_t>(glyphNode.top) -
                   static_cast<int16_t>(glyphNode.rows) + 1;
    }
    return m;
}

static bool MeasureGlyphInkBounds(const char* text, uint16_t fontId, uint8_t fontSize,
                                  GlyphBounds& bounds, int32_t& cursor)
{
    bool hasGlyph = false;
    uint32_t i = 0;
    cursor = 0;
    while (text[i] != '\0') {
        uint32_t letter = TypedText::GetUTF8Next(text, i, i);
        GlyphMetrics m = GetGlyphMetrics(letter, fontId, fontSize, cursor);
        if (m.hasInk) {
            if (!hasGlyph) {
                InitGlyphBounds(bounds, m.left, m.top, m.right, m.bottom);
            } else {
                UpdateGlyphBounds(bounds, m.left, m.top, m.right, m.bottom);
            }
            hasGlyph = true;
        }
        cursor += m.advance;
    }
    return hasGlyph;
}

static TextInkOverhang FinalizeInkOverhang(const FontHeader& header, const GlyphBounds& bounds, int32_t cursor)
{
    TextInkOverhang overhang;
    if (bounds.minLeft < 0) {
        overhang.left = -bounds.minLeft;
    }
    int32_t advanceRight = cursor - 1;
    if (bounds.maxRight > advanceRight) {
        overhang.right = static_cast<int16_t>(bounds.maxRight - advanceRight);
    }
    int16_t topMargin = static_cast<int16_t>(header.ascender) - bounds.maxTop;
    if (topMargin > 0) {
        overhang.top = topMargin;
    }
    int16_t lineBottom = (header.descender < 0) ? -header.descender : 0;
    int16_t inkBottom = -bounds.minBottom;
    if (inkBottom > lineBottom) {
        overhang.bottom = static_cast<int16_t>(inkBottom - lineBottom);
    }
    return overhang;
}

static void ApplyInkOverhang(int16_t& left, int16_t& right, int16_t& top, int16_t& bottom,
                             const TextInkOverhang& overhang)
{
    left -= overhang.left;
    right += overhang.right;
    top += overhang.top;
    bottom += overhang.bottom;
}

static TextInkOverhang GetTextInkOverhang(const char* text, uint8_t fontSize)
{
    TextInkOverhang overhang;
    if (text == nullptr || text[0] == '\0') {
        return overhang;
    }
    UIFont* fontEngine = UIFont::GetInstance();
    uint16_t fontId = fontEngine->GetFontId(DEFAULT_VECTOR_FONT_FILENAME, fontSize);
    FontHeader header;
    if (fontEngine->GetFontHeader(header, fontId, fontSize) != 0) {
        return overhang;
    }
    GlyphBounds bounds;
    int32_t cursor = 0;
    if (!MeasureGlyphInkBounds(text, fontId, fontSize, bounds, cursor)) {
        return overhang;
    }
    return FinalizeInkOverhang(header, bounds, cursor);
}

static bool IsTextRotated(const TransAffine& transform)
{
    return std::fabs(GetRecordRotationDeg(transform)) > ROTATION_EPSILON_DEG;
}

void AdjustTextBoundsForAnchor(uint8_t anchor, int16_t x, int16_t width, int16_t& left, int16_t& right)
{
    left = x;
    right = x + width;
    if (anchor == TEXT_ANCHOR_MIDDLE) {
        int16_t half = width / HALF_DIVISOR;
        left = x - half;
        right = x + half;
    } else if (anchor == TEXT_ANCHOR_END) {
        left = x - width;
        right = x;
    }
}

constexpr uint8_t UTF8_1BYTE_MASK = 0x80;
constexpr uint8_t UTF8_2BYTE_MASK = 0xE0;
constexpr uint8_t UTF8_2BYTE_PREFIX = 0xC0;
constexpr uint8_t UTF8_3BYTE_MASK = 0xF0;
constexpr uint8_t UTF8_3BYTE_PREFIX = 0xE0;
constexpr uint8_t UTF8_4BYTE_MASK = 0xF8;
constexpr uint8_t UTF8_4BYTE_PREFIX = 0xF0;
constexpr uint8_t UTF8_2BYTE_SEQ_LEN = 2;
constexpr uint8_t UTF8_3BYTE_SEQ_LEN = 3;
constexpr uint8_t UTF8_4BYTE_SEQ_LEN = 4;

uint8_t ClampFontSize(int16_t size)
{
    return (size > 0 && size < MAX_FONT_SIZE) ? static_cast<uint8_t>(size) : SVG_DEFAULT_FONT_SIZE;
}

uint8_t ScaleFontSize(uint8_t base, float scale)
{
    float scaled = static_cast<float>(base) * scale;
    if (scaled < 1.0f) {
        return 1;
    }
    if (scaled >= static_cast<float>(MAX_FONT_SIZE)) {
        return static_cast<uint8_t>(MAX_FONT_SIZE - 1);
    }
    return static_cast<uint8_t>(scaled + 0.5f);
}

UICanvas::FontStyle MakeFontStyle(uint8_t fontSize)
{
    UICanvas::FontStyle fontStyle = {};
    fontStyle.direct = UITextLanguageDirect::TEXT_DIRECT_LTR;
    fontStyle.align = UITextLanguageAlignment::TEXT_ALIGNMENT_LEFT;
    fontStyle.fontSize = fontSize;
    fontStyle.letterSpace = 0;
    fontStyle.fontName = DEFAULT_VECTOR_FONT_FILENAME;
    return fontStyle;
}

uint32_t CountUtf8Chars(const char* text)
{
    uint32_t count = 0;
    if (text == nullptr) {
        return 0;
    }
    uint32_t i = 0;
    while (text[i] != '\0') {
        uint32_t tmp = i;
        TypedText::GetUTF8Next(text, tmp, i);
        count++;
    }
    return count;
}

Point MeasureTextSize(const char* text, const UICanvas::FontStyle& fontStyle)
{
    if (text == nullptr || text[0] == '\0') {
        return {0, 0};
    }
    Text measureText;
    measureText.SetText(text);
    measureText.SetFont(fontStyle.fontName, fontStyle.fontSize);
    measureText.SetDirect(static_cast<UITextLanguageDirect>(fontStyle.direct));
    measureText.SetAlign(static_cast<UITextLanguageAlignment>(fontStyle.align));
    Style drawStyle;
    drawStyle.SetStyle(STYLE_LETTER_SPACE, fontStyle.letterSpace);
    // Use MEASURE_RECT_MAX_EXTENT instead of INT16_MAX: Rect::GetWidth() adds 1, so
    // right=INT16_MAX would overflow to a negative width and ReMeasureTextSize would bail out.
    Rect maxRect(0, 0, MEASURE_RECT_MAX_EXTENT, MEASURE_RECT_MAX_EXTENT);
    measureText.ReMeasureTextSize(maxRect, drawStyle);
    Point size = measureText.GetTextSize();
    if (size.x <= 0 || size.y <= 0) {
        int32_t approxWidth = static_cast<int32_t>(CountUtf8Chars(text)) * static_cast<int32_t>(fontStyle.fontSize);
        size.x = (approxWidth > INT16_MAX) ? INT16_MAX : static_cast<int16_t>(approxWidth);
        size.y = static_cast<int16_t>(fontStyle.fontSize);
    }
    return size;
}

struct TextDrawParams {
    float x = 0.0f;
    float y = 0.0f;
    uint8_t fontSize = SVG_DEFAULT_FONT_SIZE;
    uint8_t anchor = TEXT_ANCHOR_START;
    const TransAffine* record = nullptr;
    uint16_t wrapWidth = 0;
};

// Bundle of CTM-derived and font-metric values shared by the rotated and
// axis-aligned text position helpers. Packing them into one argument keeps
// each helper within codecheck function-parameter limits.
struct TextTransformParams {
    const TextDrawParams& drawParams;
    const UICanvas::FontStyle& fontStyle;
    float scaleX;
    float scaleY;
    int16_t ascender;
};

static void DetectTextRotation(const TransAffine* record, float& scaleX, float& scaleY,
                               float& angle, bool& hasRotation)
{
    if (record != nullptr) {
        record->ScalingAbs(&scaleX, &scaleY);
        angle = GetRecordRotationDeg(*record);
        hasRotation = std::fabs(angle) > ROTATION_EPSILON_DEG;
    }
}

static void PrepareRotatedTextPosition(float& x, float& y, const char* text,
                                       const TextTransformParams& params, TextInkOverhang& overhang)
{
    if (params.drawParams.record == nullptr) {
        return;
    }
    if (params.drawParams.anchor != TEXT_ANCHOR_START) {
        Point size = MeasureTextSize(text, params.fontStyle);
        float offset = 0.0f;
        if (params.drawParams.anchor == TEXT_ANCHOR_MIDDLE) {
            offset = static_cast<float>(size.x) / HALF_DIVISOR;
        } else if (params.drawParams.anchor == TEXT_ANCHOR_END) {
            offset = static_cast<float>(size.x);
        }
        x -= (params.scaleX > 0.0f) ? (offset / params.scaleX) : offset;
    }
    // Size the rotated bitmap to the actual glyph ink box, not the larger
    // line box.  This prevents overhanging pixels (e.g. the right side of
    // "group") from being clipped when the text swings into a corner.
    overhang = GetTextInkOverhang(text, params.drawParams.fontSize);
    int16_t inkTop = static_cast<int16_t>(params.ascender - overhang.top);
    int16_t inkLeft = -overhang.left;
    float inkTopUser = (params.scaleY > 0.0f) ? (static_cast<float>(inkTop) / params.scaleY)
                                               : static_cast<float>(inkTop);
    float inkLeftUser = (params.scaleX > 0.0f) ? (static_cast<float>(inkLeft) / params.scaleX)
                                                : static_cast<float>(inkLeft);
    x += inkLeftUser;
    y -= inkTopUser;
    params.drawParams.record->Transform(&x, &y);
}

static void PrepareAxisAlignedTextPosition(float& x, float& y, const char* text,
                                           const TextTransformParams& params)
{
    if (params.drawParams.record != nullptr) {
        params.drawParams.record->Transform(&x, &y);
    }
    if (params.drawParams.anchor != TEXT_ANCHOR_START) {
        Point size = MeasureTextSize(text, params.fontStyle);
        if (params.drawParams.anchor == TEXT_ANCHOR_MIDDLE) {
            x -= static_cast<float>(size.x / HALF_DIVISOR);
        } else if (params.drawParams.anchor == TEXT_ANCHOR_END) {
            x -= static_cast<float>(size.x);
        }
    }
    y -= static_cast<float>(params.ascender);
}

void DrawTextAt(UICanvas& canvas, const char* text, const Paint& paint, const TextDrawParams& params)
{
    if (text == nullptr || text[0] == '\0') {
        return;
    }
    float x = params.x;
    float y = params.y;
    UICanvas::FontStyle fontStyle = MakeFontStyle(params.fontSize);

    // Detect whether the CTM carries a rotation. Axis-aligned text keeps the
    // original baseline-true drawing path so normal SVG text pages are unchanged;
    // rotated text switches to a baseline-aligned bitmap rotation path so the
    // glyphs turn with the group instead of staying upright.
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float angle = 0.0f;
    bool hasRotation = false;
    DetectTextRotation(params.record, scaleX, scaleY, angle, hasRotation);

    // The ascender returned by GetFontMetrics is in screen pixels because
    // params.fontSize has already been scaled by ScaleFontSize. For axis-aligned
    // text the offset is applied in leaf-local screen pixels after the CTM; for
    // rotated text it is applied in user units before the CTM so it rotates with
    // the glyph box.
    int16_t ascender = static_cast<int16_t>(params.fontSize);
    int16_t descender = 0;
    GetFontMetrics(params.fontSize, ascender, descender);

    Paint drawPaint = paint;
    TextInkOverhang overhang;
    TextTransformParams tfParams{params, fontStyle, scaleX, scaleY, ascender};
    if (hasRotation) {
        PrepareRotatedTextPosition(x, y, text, tfParams, overhang);
        drawPaint.Rotate(angle);
    } else {
        PrepareAxisAlignedTextPosition(x, y, text, tfParams);
    }
    Point start = {SvgAttributeParser::ClampToInt16(x), SvgAttributeParser::ClampToInt16(y)};
#if defined(GRAPHIC_ENABLE_DRAW_TEXT_FLAG) && GRAPHIC_ENABLE_DRAW_TEXT_FLAG
    UICanvas::SvgTextDrawInfo svgInfo;
    svgInfo.isSvgText = true;
    svgInfo.inkTopOffset = overhang.top;
    svgInfo.inkLeftOffset = overhang.left;
    svgInfo.inkRightOffset = overhang.right;
    svgInfo.inkBottomOffset = overhang.bottom;
    canvas.StrokeText(text, start, fontStyle, drawPaint, svgInfo);
#else
    uint16_t drawWidth = (params.wrapWidth > 0) ? params.wrapWidth : UINT16_MAX;
    canvas.DrawLabel(start, text, drawWidth, fontStyle, drawPaint);
#endif
}

// Computes the starting position for a <tspan> and updates the shared cursor when
// absolute x/y are present (SVG text flow: absolute coordinates reset the pen).
static void SetupTSpanCursor(const SvgTSpanNode* tspan, TextCursor& cursor, float& localCx, float& localCy)
{
    localCx = cursor.x + static_cast<float>(tspan->GetDx());
    localCy = cursor.y + static_cast<float>(tspan->GetDy());
    if (tspan->HasX()) {
        localCx = static_cast<float>(tspan->GetX() + tspan->GetDx());
        cursor.x = localCx;
    }
    if (tspan->HasY()) {
        localCy = static_cast<float>(tspan->GetY() + tspan->GetDy());
        cursor.y = localCy;
    }
}

// Copies src into dest (replacing any previous content). Returns false on allocation failure.
static bool CopyTextContent(char*& dest, const char* src)
{
    if (src == nullptr) {
        delete[] dest;
        dest = nullptr;
        return true;
    }
    uint32_t len = strlen(src) + 1;
    char* copy = new char[len];
    if (copy == nullptr) {
        return false;
    }
    if (memcpy_s(copy, len, src, len) != EOK) {
        delete[] copy;
        return false;
    }
    delete[] dest;
    dest = copy;
    return true;
}

} // namespace

SvgTSpanNode::~SvgTSpanNode()
{
    delete[] textContent_;
    textContent_ = nullptr;
}

bool SvgTSpanNode::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "id") == 0) {
        SetIdAndRegister(value);
        return true;
    }
    if (strcmp(name, "x") == 0) {
        x_ = SvgAttributeParser::ParseLength(value);
        hasX_ = true;
        return true;
    }
    if (strcmp(name, "y") == 0) {
        y_ = SvgAttributeParser::ParseLength(value);
        hasY_ = true;
        return true;
    }
    if (strcmp(name, "dx") == 0) {
        dx_ = SvgAttributeParser::ParseLength(value);
        return true;
    }
    if (strcmp(name, "dy") == 0) {
        dy_ = SvgAttributeParser::ParseLength(value);
        return true;
    }
    if (strcmp(name, "font-size") == 0) {
        fontSize_ = ClampFontSize(SvgAttributeParser::ParseLength(value));
        return true;
    }
    if (strcmp(name, "value") == 0) {
        SetTextContent(value);
        return true;
    }
    return paintState_.SetAttribute(name, value);
}

void SvgTSpanNode::AppendChild(SvgElementBase* child)
{
    (void)child;
}

SvgElementBase* SvgTSpanNode::Clone() const
{
    SvgTSpanNode* clone = new SvgTSpanNode();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->x_ = x_;
    clone->y_ = y_;
    clone->dx_ = dx_;
    clone->dy_ = dy_;
    clone->hasX_ = hasX_;
    clone->hasY_ = hasY_;
    clone->fontSize_ = fontSize_;
    if (textContent_ != nullptr) {
        clone->SetTextContent(textContent_);
    }
    clone->paintState_.CopyFrom(paintState_);
    return clone;
}

bool SvgTSpanNode::SetTextContent(const char* text)
{
    return CopyTextContent(textContent_, text);
}

SvgTextNode::~SvgTextNode()
{
    delete[] textContent_;
    textContent_ = nullptr;
}

bool SvgTextNode::SetTextContent(const char* text)
{
    bool ok = CopyTextContent(textContent_, text);
    if (ok) {
        RebuildGeometry();
    }
    return ok;
}

Rect SvgTextNode::GetLocalBounds() const
{
    uint8_t size = ClampFontSize(fontSize_);
    int16_t originX = static_cast<int16_t>(x_ + dx_);
    // SVG <text> positions the baseline at y (plus dy), not the box top.
    int16_t baselineY = static_cast<int16_t>(y_ + dy_);
    int16_t top = 0;
    int16_t bottom = 0;
    GetTextInkBox(size, baselineY, top, bottom);
    int16_t left = originX;
    int16_t right = originX;

    int16_t cursorX = originX;
    int16_t cursorY = baselineY;

    bool rotated = IsTextRotated(accumulatedTransform_);
    if (textContent_ != nullptr && textContent_[0] != '\0') {
        Point dim = MeasureTextSize(textContent_, MakeFontStyle(size));
        AdjustTextBoundsForAnchor(paintState_.GetTextAnchor(), originX, dim.x, left, right);
        // Keep the measured box in play as a floor so multi-line results stay covered.
        int16_t measuredBottom = static_cast<int16_t>(top + dim.y);
        if (measuredBottom > bottom) {
            bottom = measuredBottom;
        }
        if (rotated) {
            TextInkOverhang overhang = GetTextInkOverhang(textContent_, size);
            ApplyInkOverhang(left, right, top, bottom, overhang);
        }
        cursorX += dim.x;
    }
    Rect bounds(left, top, right, bottom);

    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End(); node = node->next_) {
        if (node->data_ == nullptr) {
            continue;
        }
        const SvgTSpanNode* tspan = dynamic_cast<const SvgTSpanNode*>(node->data_);
        if (tspan != nullptr) {
            bounds.Join(bounds, MeasureTSpanBounds(tspan, size, cursorX, cursorY));
        }
    }

    return bounds;
}

Rect SvgTextNode::MeasureTSpanBounds(const SvgTSpanNode* tspan, uint8_t size,
                                     int16_t& cursorX, int16_t& cursorY) const
{
    const char* text = tspan->GetTextContent();
    uint8_t tspanSize = ClampFontSize(tspan->GetFontSize());
    Point dim = MeasureTextSize(text, MakeFontStyle(tspanSize));
    int16_t cx = cursorX + tspan->GetDx();
    int16_t cy = cursorY + tspan->GetDy();
    if (tspan->HasX()) {
        cx = static_cast<int16_t>(tspan->GetX() + tspan->GetDx());
        cursorX = cx;
    }
    if (tspan->HasY()) {
        cy = static_cast<int16_t>(tspan->GetY() + tspan->GetDy());
        cursorY = cy;
    }
    int16_t tLeft = cx;
    int16_t tRight = cx + dim.x;
    AdjustTextBoundsForAnchor(tspan->GetPaintState().GetTextAnchor(), cx, dim.x, tLeft, tRight);
    int16_t tTop = 0;
    int16_t tBottom = 0;
    GetTextInkBox(tspanSize, cy, tTop, tBottom);
    int16_t measuredBottom = static_cast<int16_t>(tTop + dim.y);
    if (measuredBottom > tBottom) {
        tBottom = measuredBottom;
    }
    if (IsTextRotated(accumulatedTransform_)) {
        TextInkOverhang overhang = GetTextInkOverhang(text, tspanSize);
        ApplyInkOverhang(tLeft, tRight, tTop, tBottom, overhang);
    }
    cursorX = cx + dim.x;
    cursorY = cy;
    return Rect(tLeft, tTop, tRight, tBottom);
}

void SvgTextNode::AppendChild(SvgElementBase* child)
{
    SvgLeafNode::AppendChild(child);
}

bool SvgTextNode::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "value") == 0) {
        SetTextContent(value);
        return true;
    }
    return SvgLeafNode::SetAttribute(name, value);
}

bool SvgTextNode::SetGeometryAttribute(const char* name, const char* value)
{
    if (strcmp(name, "x") == 0) {
        x_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "y") == 0) {
        y_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "dx") == 0) {
        dx_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "dy") == 0) {
        dy_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "font-size") == 0) {
        fontSize_ = ClampFontSize(SvgAttributeParser::ParseLength(value));
    } else {
        return false;
    }
    return true;
}

void SvgTextNode::RecordGeometry(UICanvas& canvas)
{
    TransAffine record = GetRecordTransform();
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    accumulatedTransform_.ScalingAbs(&scaleX, &scaleY);
    uint8_t size = ScaleFontSize(ClampFontSize(fontSize_), scaleY);

    TextCursor cursor = {static_cast<float>(x_ + dx_), static_cast<float>(y_ + dy_)};

    TextDrawParams params;
    params.x = cursor.x;
    params.y = cursor.y;
    params.fontSize = size;
    params.anchor = paintState_.GetTextAnchor();
    params.record = &record;
    DrawTextAt(canvas, textContent_, inheritedPaint_, params);

    if (textContent_ != nullptr && textContent_[0] != '\0') {
        Point dim = MeasureTextSize(textContent_, MakeFontStyle(size));
        cursor.x += static_cast<float>(dim.x);
    }

    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End(); node = node->next_) {
        if (node->data_ == nullptr) {
            continue;
        }
        SvgTSpanNode* tspan = dynamic_cast<SvgTSpanNode*>(node->data_);
        if (tspan != nullptr) {
            DrawTSpanChild(canvas, record, scaleY, cursor, tspan);
        }
    }
}

void SvgTextNode::DrawTSpanChild(UICanvas& canvas, const TransAffine& record, float scaleY,
                                 TextCursor& cursor, SvgTSpanNode* tspan)
{
    float localCx = 0.0f;
    float localCy = 0.0f;
    SetupTSpanCursor(tspan, cursor, localCx, localCy);
    uint8_t tspanSize = ScaleFontSize(ClampFontSize(tspan->GetFontSize()), scaleY);
    Paint tspanPaint = tspan->GetPaintState().Apply(inheritedPaint_);
    TextDrawParams tspanParams;
    tspanParams.x = localCx;
    tspanParams.y = localCy;
    tspanParams.fontSize = tspanSize;
    tspanParams.anchor = tspan->GetPaintState().GetTextAnchor();
    tspanParams.record = &record;
    DrawTextAt(canvas, tspan->GetTextContent(), tspanPaint, tspanParams);

    const char* text = tspan->GetTextContent();
    if (text != nullptr && text[0] != '\0') {
        Point dim = MeasureTextSize(text, MakeFontStyle(tspanSize));
        cursor.x = localCx + static_cast<float>(dim.x);
        cursor.y = localCy;
    }
}

Rect SvgTextAreaNode::GetLocalBounds() const
{
    if (width_ > 0 && height_ > 0) {
        int16_t left = x_ + dx_;
        int16_t top = 0;
        int16_t bottom = 0;
        GetTextInkBox(ClampFontSize(fontSize_), static_cast<int16_t>(y_ + dy_), top, bottom);
        int16_t right = left + width_ - 1;
        return {left, top, right, static_cast<int16_t>(top + height_ - 1)};
    }
    return SvgTextNode::GetLocalBounds();
}

bool SvgTextAreaNode::SetGeometryAttribute(const char* name, const char* value)
{
    if (strcmp(name, "width") == 0) {
        width_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "height") == 0) {
        height_ = SvgAttributeParser::ParseLength(value);
    } else {
        return SvgTextNode::SetGeometryAttribute(name, value);
    }
    return true;
}

void SvgTextAreaNode::RecordGeometry(UICanvas& canvas)
{
    TransAffine record = GetRecordTransform();
    TextCursor cursor = {static_cast<float>(x_ + dx_), static_cast<float>(y_ + dy_)};
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    accumulatedTransform_.ScalingAbs(&scaleX, &scaleY);
    uint8_t userSize = ClampFontSize(fontSize_);
    uint8_t size = ScaleFontSize(userSize, scaleY);
    uint16_t wrapWidth = (width_ > 0) ? static_cast<uint16_t>(width_) : 0;
    if (textContent_ != nullptr && textContent_[0] != '\0') {
        TextDrawParams params;
        params.x = cursor.x;
        params.y = cursor.y;
        params.fontSize = size;
        params.anchor = paintState_.GetTextAnchor();
        params.record = &record;
        params.wrapWidth = wrapWidth;
        DrawTextAt(canvas, textContent_, inheritedPaint_, params);
    }

    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End(); node = node->next_) {
        if (node->data_ != nullptr) {
            DrawTextAreaChild(canvas, record, scaleY, cursor, wrapWidth, node->data_);
        }
    }
}

void SvgTextAreaNode::DrawTextAreaChild(UICanvas& canvas, const TransAffine& record, float scaleY,
                                        TextCursor& cursor, uint16_t wrapWidth,
                                        SvgElementBase* child)
{
    SvgTSpanNode* tspan = dynamic_cast<SvgTSpanNode*>(child);
    if (tspan != nullptr) {
        float cx = 0.0f;
        float cy = 0.0f;
        SetupTSpanCursor(tspan, cursor, cx, cy);
        uint8_t tspanSize = ScaleFontSize(ClampFontSize(tspan->GetFontSize()), scaleY);
        Paint tspanPaint = tspan->GetPaintState().Apply(inheritedPaint_);
        TextDrawParams tspanParams;
        tspanParams.x = cx;
        tspanParams.y = cy;
        tspanParams.fontSize = tspanSize;
        tspanParams.anchor = paintState_.GetTextAnchor();
        tspanParams.record = &record;
        tspanParams.wrapWidth = wrapWidth;
        DrawTextAt(canvas, tspan->GetTextContent(), tspanPaint, tspanParams);
        return;
    }
    // <textArea> only supports <tspan> children; other child types are ignored.
}

SvgElementBase* SvgTextNode::Clone() const
{
    SvgTextNode* clone = new SvgTextNode();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->x_ = x_;
    clone->y_ = y_;
    clone->dx_ = dx_;
    clone->dy_ = dy_;
    clone->fontSize_ = fontSize_;
    if (textContent_ != nullptr) {
        clone->SetTextContent(textContent_);
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
    clone->CopyStateFrom(*this);
    return clone;
}

SvgElementBase* SvgTextAreaNode::Clone() const
{
    SvgTextAreaNode* clone = new SvgTextAreaNode();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->x_ = x_;
    clone->y_ = y_;
    clone->dx_ = dx_;
    clone->dy_ = dy_;
    clone->fontSize_ = fontSize_;
    clone->width_ = width_;
    clone->height_ = height_;
    if (textContent_ != nullptr) {
        clone->SetTextContent(textContent_);
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
    clone->CopyStateFrom(*this);
    return clone;
}

} // namespace OHOS
