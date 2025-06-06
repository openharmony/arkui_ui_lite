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
 * @addtogroup UI_Common
 * @{
 *
 * @brief Defines common UI capabilities, such as image and text processing.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file text.h
 *
 * @brief Declares the <b>Text</b> class that provides functions to set basic text attributes, such as the text
 *        direction and alignment mode.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef GRAPHIC_LITE_TEXT_H
#define GRAPHIC_LITE_TEXT_H

#include <cstring>
#include "common/spannable_string.h"
#include "engines/gfx/gfx_engine_manager.h"
#include "font/ui_font_header.h"

#include "gfx_utils/geometry2d.h"
#include "gfx_utils/graphic_types.h"
#include "gfx_utils/list.h"
#include "gfx_utils/style.h"
#include "gfx_utils/vector.h"

namespace OHOS {
namespace {
const std::string PASSWORD_DOT = "*"; // dot for password type
constexpr uint16_t DEFAULT_TEXT_OFFSET = 5;
constexpr uint16_t BOTH_SIDE_TEXT_OFFSET = DEFAULT_TEXT_OFFSET * 2; // 2: left and right space
constexpr uint16_t DEFAULT_CURSOR_OFFSET = 1;
constexpr uint16_t DEFAULT_CURSOR_WIDTH = 2;
} // namespace name

/**
 * @brief Enumerates text alignment modes.
 */
enum UITextLanguageAlignment : uint8_t {
    /** Left-aligned */
    TEXT_ALIGNMENT_LEFT = 0,
    /** Right-aligned */
    TEXT_ALIGNMENT_RIGHT,
    /** Centered */
    TEXT_ALIGNMENT_CENTER,
    /** Top-aligned */
    TEXT_ALIGNMENT_TOP,
    /** Bottom-aligned */
    TEXT_ALIGNMENT_BOTTOM,
};

/**
 * @brief Enumerates text directions.
 */
enum UITextLanguageDirect : uint8_t {
    /** Left-to-right */
    TEXT_DIRECT_LTR = 0,
    /** Right-to-left */
    TEXT_DIRECT_RTL,
    TEXT_DIRECT_MIXED,
};

/**
 * @brief Stores the attribute information about this arc text to draw.
 */
struct ArcTextInfo {
    uint16_t radius;
    float startAngle;
    float endAngle;
    Point arcCenter;
    uint32_t lineStart;
    uint32_t lineEnd;
    UITextLanguageDirect direct;
    bool hasAnimator;
    uint32_t* codePoints;
    uint16_t codePointsNum;
    uint8_t shapingFontId;
};

/**
 * @brief Stores attribute information for this arc text to be drawn.
 */
struct ArcLetterInfo {
    void InitData(uint16_t inFontId, uint8_t inFontSize, uint32_t inLetter, Point inPos,
        int16_t inRotateAngle, ColorType inColor, OpacityType inOpaScale,
        float inStartAngle, float inEndAngle, float inCurrentAngle, uint16_t inRadius,
        bool inCompatibilityMode, bool inDirectFlag, bool inOrientationFlag, bool inHasAnimator)
    {
        fontId = inFontId;
        fontSize = inFontSize;
        letter = inLetter;
        pos = inPos;
        rotateAngle = inRotateAngle;
        color = inColor;
        opaScale = inOpaScale;
        startAngle = inStartAngle;
        endAngle = inEndAngle;
        currentAngle = inCurrentAngle;
        radius = inRadius;
        compatibilityMode = inCompatibilityMode;
        directFlag = inDirectFlag;
        orientationFlag = inOrientationFlag;
        hasAnimator = inHasAnimator;
    }
    uint16_t fontId;
    uint8_t fontSize;
    uint32_t letter;
    Point pos;
    int16_t rotateAngle;
    ColorType color;
    OpacityType opaScale;
    float startAngle;
    float endAngle;
    float currentAngle;
    uint16_t radius;
    bool compatibilityMode;
    bool directFlag;
    bool orientationFlag;
    bool hasAnimator;
};

/**
 * @brief Enumerates text orientations.
 */
enum TextInRange {
    IN_RANGE,
    OUT_RANGE,
    NEED_CLIP
};

/**
 * @brief Enumerates text orientations.
 */
enum class TextOrientation : uint8_t {
    /** Inside */
    INSIDE,
    /** Outside */
    OUTSIDE,
};

struct BackgroundColor : public HeapBase {
    int16_t start;
    int16_t end;
    ColorType backgroundColor;
};

struct ForegroundColor : public HeapBase {
    int16_t start;
    int16_t end;
    ColorType fontColor;
};

struct LineBackgroundColor : public HeapBase {
    int16_t start;
    int16_t end;
    ColorType linebackgroundColor;
};

struct LabelLineInfo;

/**
 * @brief Represents the base class of <b>Text</b>, providing the text attribute setting and text drawing
 *        capabilities for components that require font display.
 *
 * @since 1.0
 * @version 1.0
 */
class Text : public HeapBase {
public:
    /** Invalid value for the ellipsis position */
    static constexpr uint16_t TEXT_ELLIPSIS_END_INV = 0xFFFF;
    static constexpr uint16_t TEXT_ELLIPSIS_UNICODE = 0x2026;

    /**
     * @brief A constructor used to create a <b>Text</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    Text();

    /**
     * @brief A destructor used to delete the <b>Text</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    virtual ~Text();

    /**
     * @brief Sets the content for this text.
     *
     * @param text Indicates the pointer to the text content.
     * @since 1.0
     * @version 1.0
     */
    virtual void SetText(const char* text);

    /**
     * @brief Sets the SpannableString for this text.
     *
     * @param text Indicates the pointer to the text content.
     * @since 1.0
     * @version 1.0
     */
    void SetSpannableString(const SpannableString* spannableString);

    /**
     * @brief Obtains the content of this text.
     *
     * @return Returns the text content.
     * @since 1.0
     * @version 1.0
     */
    const char* GetText() const
    {
        return text_;
    }

    /**
     * @brief Sets the font name and size.
     *
     * @param name Indicates the pointer to the font name.
     * @param size Indicates the font size to set.
     * @since 1.0
     * @version 1.0
     */
    void SetFont(const char* name, uint8_t size);

    static void SetFont(const char* name, uint8_t size, char*& destName, uint8_t& destSize);

    /**
     * @brief Sets the font ID.
     *
     * @param fontId Indicates the font ID to set.
     * @since 1.0
     * @version 1.0
     */
    void SetFontId(uint16_t fontId);

    /**
     * @brief Obtains the font ID.
     *
     * @return Returns the front ID.
     * @since 1.0
     * @version 1.0
     */
    uint16_t GetFontId() const
    {
        return fontId_;
    }

    /**
     * @brief Obtains the font size.
     *
     * @return Returns the front size.
     * @since 1.0
     * @version 1.0
     */
    uint8_t GetFontSize() const
    {
        return fontSize_;
    }

    /**
     * @brief Sets the direction for this text.
     *
     * @param direct Indicates the text direction, as defined in {@link UITextLanguageDirect}.
     * @since 1.0
     * @version 1.0
     */
    void SetDirect(UITextLanguageDirect direct)
    {
        direct_ = direct;
    }

    /**
     * @brief Obtains the direction of this text.
     *
     * @return Returns the text direction, as defined in {@link UITextLanguageDirect}.
     * @since 1.0
     * @version 1.0
     */
    UITextLanguageDirect GetDirect() const
    {
        return static_cast<UITextLanguageDirect>(direct_);
    }

    /**
     * @brief Sets the alignment mode for this text.
     *
     * @param horizontalAlign Indicates the horizontal alignment mode to set,
     *                        which can be {@link TEXT_ALIGNMENT_LEFT},
     *                        {@link TEXT_ALIGNMENT_CENTER}, or {@link TEXT_ALIGNMENT_RIGHT}.
     * @param verticalAlign Indicates the vertical alignment mode to set, which can be
     *                      {@link TEXT_ALIGNMENT_TOP} (default mode), {@link TEXT_ALIGNMENT_CENTER},
     *                      or {@link TEXT_ALIGNMENT_BOTTOM}.
     * @since 1.0
     * @version 1.0
     */
    void SetAlign(UITextLanguageAlignment horizontalAlign, UITextLanguageAlignment verticalAlign = TEXT_ALIGNMENT_TOP)
    {
        if ((horizontalAlign_ != horizontalAlign) || (verticalAlign_ != verticalAlign)) {
            needRefresh_ = true;
            horizontalAlign_ = horizontalAlign;
            verticalAlign_ = verticalAlign;
        }
    }

    /**
     * @brief Obtains the horizontal alignment mode.
     *
     * @return Returns the horizontal alignment mode.
     * @since 1.0
     * @version 1.0
     */
    UITextLanguageAlignment GetHorAlign() const
    {
        return static_cast<UITextLanguageAlignment>(horizontalAlign_);
    }

    /**
     * @brief Obtains the vertical alignment mode.
     *
     * @return Returns the vertical alignment mode.
     * @since 1.0
     * @version 1.0
     */
    UITextLanguageAlignment GetVerAlign() const
    {
        return static_cast<UITextLanguageAlignment>(verticalAlign_);
    }

    /**
     * @brief Obtains the size of this text.
     *
     * @return Returns the text size.
     * @since 1.0
     * @version 1.0
     */
    Point GetTextSize() const
    {
        return textSize_;
    }

    virtual void ReMeasureTextSize(const Rect& textRect, const Style& style);

    void ReMeasureTextWidthInEllipsisMode(const Rect& textRect, const Style& style, uint16_t ellipsisIndex);

    void OnDraw(BufferInfo& gfxDstBuffer,
                const Rect& invalidatedArea,
                const Rect& viewOrigRect,
                const Rect& textRect,
                int16_t offsetX,
                const Style& style,
                uint16_t ellipsisIndex,
                OpacityType opaScale);

    /**
     * @brief Sets whether to adapt the component width to this text.
     *
     * @param expand Specifies whether to adapt the component width to this text. The value <b>true</b> indicates
     *               that the component width will adapt to this text, and <b>false</b> indicates not.
     * @since 1.0
     * @version 1.0
     */
    void SetExpandWidth(bool expand)
    {
        expandWidth_ = expand;
    }

    /**
     * @brief Checks whether the component width adapts to this text.
     *
     * @return Returns <b>true</b> if the component width adapts to this text; returns <b>false</b> otherwise.
     * @since 1.0
     * @version 1.0
     */
    bool IsExpandWidth() const
    {
        return expandWidth_;
    }

    /**
     * @brief Sets whether to adapt the component height to this text.
     *
     * @param expand Specifies whether to adapt the component height to this text. The value <b>true</b> indicates
     *               that the component height will adapt to this text, and <b>false</b> indicates not.
     * @since 1.0
     * @version 1.0
     */
    void SetExpandHeight(bool expand)
    {
        expandHeight_ = expand;
    }

    /**
     * @brief Checks whether the component height adapts to this text.
     *
     * @return Returns <b>true</b> if the component height adapts to this text; returns <b>false</b> otherwise.
     * @since 1.0
     * @version 1.0
     */
    bool IsExpandHeight() const
    {
        return expandHeight_;
    }

    bool IsNeedRefresh() const
    {
        return needRefresh_;
    }

    /**
     * @brief Obtains the index of the character from where text will be replaced by ellipses based on
     *        the text rectangle and style.
     *
     * @param textRect Indicates the text rectangle.
     * @param style Indicates the text style.
     * @since 1.0
     * @version 1.0
     */
    uint16_t GetEllipsisIndex(const Rect& textRect, const Style& style);

    /**
     * @brief Get the GetShapingFontId of text
     *
     * @return Return ShapingFontId
     */
    virtual uint8_t GetShapingFontId() const
    {
        return 0;
    }

    /**
     * @brief Get the GetCodePointNum of text
     *
     * @return Return num of CodePoints
     */
    virtual uint16_t GetCodePointNum() const
    {
        return 0;
    }

    /**
     * @brief Get the GetCodePoints of text
     *
     * @return Return CodePoints of text
     */
    virtual uint32_t* GetCodePoints() const
    {
        return nullptr;
    }

    void SetSupportBaseLine(bool baseLine)
    {
        baseLine_ = baseLine;
    }

    void SetBackgroundColorSpan(ColorType backgroundColor, int16_t start, int16_t end)
    {
        if (spannableString_ == nullptr) {
            spannableString_ = new SpannableString();
        }
        spannableString_->SetBackgroundColor(backgroundColor, (uint16_t)start, (uint16_t)end);
    }

    List<BackgroundColor> GetBackgroundColorSpan()
    {
        return backgroundColor_;
    }

    void SetForegroundColorSpan(ColorType fontColor, int16_t start, int16_t end)
    {
        if (spannableString_ == nullptr) {
            spannableString_ = new SpannableString();
        }
        spannableString_->SetForegroundColor(fontColor, (uint16_t)start, (uint16_t)end);
    }

    List<ForegroundColor> GetForegroundColorSpan()
    {
        return foregroundColor_;
    }

    void SetLineBackgroundSpan(ColorType linebackgroundColor, int16_t start, int16_t end)
    {
        if (spannableString_ == nullptr) {
            spannableString_ = new SpannableString();
        }
        spannableString_->SetLineBackgroundColor(linebackgroundColor, (uint16_t)start, (uint16_t)end);
    }

    List<LineBackgroundColor> GetLineBackgroundSpan()
    {
        return linebackgroundColor_;
    }

    void SetAbsoluteSizeSpan(uint16_t start, uint16_t end, uint8_t size);
    void SetRelativeSizeSpan(uint16_t start, uint16_t end, float size);
    virtual uint16_t GetLetterIndexByLinePosition(const Style& style, int16_t contentWidth,
                                                  const int16_t& posX, int16_t offsetX);
    virtual uint16_t GetPosXByLetterIndex(const Rect& textRect, const Style& style,
                                          uint16_t beginIndex, uint16_t count);

    void SetEliminateTrailingSpaces(bool eliminateTrailingSpaces)
    {
        eliminateTrailingSpaces_ = eliminateTrailingSpaces;
    }

    bool IsEliminateTrailingSpaces()
    {
        return eliminateTrailingSpaces_;
    }
    uint16_t GetSizeSpan()
    {
        return characterSize_;
    }

    /**
     * @brief Get next character full dispaly offset.
     *
     * @param textRect Indicates size of input box.
     * @param style Indicates the style of text.
     * @param beginIndex Indicates index at the beginning of the text.
     * @param num Indicates num of text.
     *
     * @return Return text offset.
     *
     */
    virtual uint16_t GetNextCharacterFullDispalyOffset(const Rect& textRect,
        const Style& style, uint16_t beginIndex, uint16_t num);

    /**
     * @brief Obtains the width of the meta text.
     *
     * @param style Indicates the style of text.
     *
     * @return Return meta text width.
     */
    virtual int16_t GetMetaTextWidth(const Style& style);

protected:
    struct TextLine {
        uint16_t lineBytes;
        uint16_t linePixelWidth;
    };

    /** Maximum number of lines */
    static constexpr uint16_t MAX_LINE_COUNT = 50;
    static TextLine textLine_[MAX_LINE_COUNT];

    static constexpr const char* TEXT_ELLIPSIS = "…";

    virtual uint32_t GetTextStrLen();

    virtual uint32_t
        GetTextLine(uint32_t begin, uint32_t textLen, int16_t width, uint16_t lineNum, uint8_t letterSpace,
                    uint16_t& letterIndex, SpannableString* spannableString, TextLine& textLine);

    virtual uint16_t GetLetterIndexByPosition(const Rect& textRect, const Style& style, const Point& pos);

    virtual void Draw(BufferInfo& gfxDstBuffer,
                      const Rect& mask,
                      const Rect& coords,
                      const Style& style,
                      int16_t offsetX,
                      uint16_t ellipsisIndex,
                      OpacityType opaScale);

    uint16_t GetLine(int16_t width, uint8_t letterSpace, uint16_t ellipsisIndex, uint32_t& maxLineBytes);
    int16_t TextPositionY(const Rect& textRect, int16_t textHeight);
    int16_t LineStartPos(const Rect& textRect, uint16_t lineWidth);
    void DrawEllipsis(BufferInfo& gfxDstBuffer, LabelLineInfo& labelLine, uint16_t& letterIndex);
    uint32_t CalculateLineWithEllipsis(uint32_t begin, uint32_t textLen, int16_t width,
                                       uint8_t letterSpace, uint16_t& lineNum,
                                       uint16_t& letterIndex,
                                       SpannableString* spannableString);
    uint16_t GetSpanFontIdBySize(uint8_t size);
    SpannableString* CreateSpannableString();
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    TextStyle* textStyles_;
#endif
    char* text_;
    uint16_t fontId_;
    uint8_t fontSize_; // Only the vector font library has a valid value.
    Point textSize_;
    bool needRefresh_ : 1;
    bool expandWidth_ : 1;
    bool expandHeight_ : 1;
    bool baseLine_ : 1;
    uint8_t direct_ : 4; // UITextLanguageDirect
    List<BackgroundColor> backgroundColor_;
    List<ForegroundColor> foregroundColor_;
    List<LineBackgroundColor> linebackgroundColor_;
    uint32_t characterSize_;
    SpannableString* spannableString_;
    TextLine preIndexLine_;

private:
    uint8_t horizontalAlign_ : 4; // UITextLanguageAlignment
    uint8_t verticalAlign_ : 4;   // UITextLanguageAlignment
    bool eliminateTrailingSpaces_;
    static constexpr uint8_t FONT_ID_MAX = 0xFF;
#if defined(ENABLE_ICU) && ENABLE_ICU
    void SetLineBytes(uint16_t& lineBytes, uint16_t lineBegin);
#endif
    void CalculatedCurLineHeight(int16_t& lineHeight, int16_t& curLineHeight,
                                 uint16_t fontHeight, const Style& style, uint16_t lineMaxHeight);
    void SetNextLineBegin(const Style& style, uint16_t lineMaxHeight, int16_t& curLineHeight, Point& pos,
                          int16_t& tempLetterIndex, int16_t& lineHeight, uint16_t& lineBegin, uint16_t letterIndex);
    Point GetPos(int16_t& lineHeight, const Style& style, uint16_t& lineCount, const Rect& coords);
};
} // namespace OHOS
#endif // GRAPHIC_LITE_TEXT_H
