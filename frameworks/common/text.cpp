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

#include "common/text.h"
#include "common/typed_text.h"
#include "draw/draw_label.h"
#include "font/ui_font.h"
#include "font/ui_font_adaptor.h"
#include "font/ui_font_builder.h"
#include "gfx_utils/graphic_log.h"
#include "securec.h"

namespace OHOS {
Text::TextLine Text::textLine_[MAX_LINE_COUNT] = {{0}};

Text::Text()
    : text_(nullptr),
      fontId_(0),
      fontSize_(0),
      textSize_({0, 0}),
      needRefresh_(false),
      expandWidth_(false),
      expandHeight_(false),
      baseLine_(true),
      direct_(TEXT_DIRECT_LTR),
      characterSize_(0),
      spannableString_(nullptr),
      horizontalAlign_(TEXT_ALIGNMENT_LEFT),
      verticalAlign_(TEXT_ALIGNMENT_TOP),
      eliminateTrailingSpaces_(false)
{
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    textStyles_ = nullptr;
#endif
    SetFont(DEFAULT_VECTOR_FONT_FILENAME, DEFAULT_VECTOR_FONT_SIZE);
}

Text::~Text()
{
    if (text_ != nullptr) {
        UIFree(text_);
        text_ = nullptr;
    }
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    if (textStyles_ != nullptr) {
        UIFree(textStyles_);
        textStyles_ = nullptr;
    }
#endif
    if (spannableString_ != nullptr) {
        delete spannableString_;
        spannableString_ = nullptr;
    }
    if (backgroundColor_.Size() > 0) {
        backgroundColor_.Clear();
    }
    if (linebackgroundColor_.Size() > 0) {
        linebackgroundColor_.Clear();
    }
    if (foregroundColor_.Size() > 0) {
        foregroundColor_.Clear();
    }
}

void Text::SetSpannableString(const SpannableString* spannableString)
{
    if (spannableString_ == nullptr) {
        spannableString_ = new SpannableString();
    }
    spannableString_->SetSpannableString(spannableString);
    needRefresh_ = true;
}

void Text::SetText(const char* text)
{
    if (text == nullptr) {
        return;
    }
    uint32_t textLen = static_cast<uint32_t>(strlen(text));
    if (textLen > MAX_TEXT_LENGTH) {
        textLen = MAX_TEXT_LENGTH;
    }
    if (text_ != nullptr) {
        if (strcmp(text, text_) == 0) {
            return;
        }
        UIFree(text_);
        text_ = nullptr;
    }
    text_ = static_cast<char*>(UIMalloc(textLen + 1));
    if (text_ == nullptr) {
        return;
    }
    if (strncpy_s(text_, textLen + 1, text, textLen) != EOK) {
        UIFree(text_);
        text_ = nullptr;
        return;
    }
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    if (textStyles_ != nullptr) {
        UIFree(textStyles_);
        textStyles_ = nullptr;
    }
#endif
    needRefresh_ = true;
}

void Text::SetFont(const char* name, uint8_t size)
{
    if (name == nullptr) {
        return;
    }
    UIFont* font = UIFont::GetInstance();
    if (font->IsVectorFont()) {
        uint16_t fontId = font->GetFontId(name);
        if ((fontId != UIFontBuilder::GetInstance()->GetTotalFontId()) &&
            ((fontId_ != fontId) || (fontSize_ != size))) {
            fontId_ = fontId;
            fontSize_ = size;
            needRefresh_ = true;
        }
    } else {
        uint16_t fontId = font->GetFontId(name, size);
        SetFontId(fontId);
    }
}

void Text::SetFont(const char* name, uint8_t size, char*& destName, uint8_t& destSize)
{
    if (name == nullptr) {
        return;
    }
    uint32_t nameLen = static_cast<uint32_t>(strlen(name));
    if (nameLen > MAX_TEXT_LENGTH) {
        return;
    }
    if (destName != nullptr) {
        if (strcmp(destName, name) == 0) {
            destSize = size;
            return;
        }
        UIFree(destName);
        destName = nullptr;
    }
    if (nameLen != 0) {
        /* one more to store '\0' */
        destName = static_cast<char*>(UIMalloc(++nameLen));
        if (destName == nullptr) {
            return;
        }
        if (memcpy_s(destName, nameLen, name, nameLen) != EOK) {
            UIFree(destName);
            destName = nullptr;
            return;
        }
        destSize = size;
    }
}

void Text::SetFontId(uint16_t fontId)
{
    UIFontBuilder* fontBuilder = UIFontBuilder::GetInstance();
    if (fontId >= fontBuilder->GetTotalFontId()) {
        GRAPHIC_LOGE("Text::SetFontId invalid fontId(%hhd)", fontId);
        return;
    }
    UIFont* font = UIFont::GetInstance();
    if ((fontId_ == fontId) && (fontSize_ != 0) && !font->IsVectorFont()) {
        GRAPHIC_LOGD("Text::SetFontId same font has already set");
        return;
    }

    UITextLanguageFontParam* fontParam = fontBuilder->GetTextLangFontsTable(fontId);
    if (fontParam == nullptr) {
        return;
    }
    if (font->IsVectorFont()) {
        uint16_t fontId = font->GetFontId(fontParam->ttfName);
        if ((fontId != fontBuilder->GetTotalFontId()) && ((fontId_ != fontId) ||
            (fontSize_ != fontParam->size))) {
            fontId_ = fontId;
            fontSize_ = fontParam->size;
            needRefresh_ = true;
        }
    } else {
        fontId_ = fontId;
        fontSize_ = fontParam->size;
        needRefresh_ = true;
    }
}

void Text::ReMeasureTextSize(const Rect& textRect, const Style& style)
{
    if (fontSize_ == 0) {
        return;
    }
    int16_t maxWidth = (expandWidth_ ? COORD_MAX : textRect.GetWidth());
    if (maxWidth > 0) {
        textSize_ = TypedText::GetTextSize(text_, fontId_, fontSize_, style.letterSpace_, style.lineHeight_, maxWidth,
                                           style.lineSpace_, spannableString_, IsEliminateTrailingSpaces());
        if (baseLine_) {
            FontHeader head;
            if (UIFont::GetInstance()->GetFontHeader(head, fontId_, fontSize_) != 0) {
                return;
            }
            textSize_.y += fontSize_ - head.ascender;
        }
    }
}

void Text::ReMeasureTextWidthInEllipsisMode(const Rect& textRect, const Style& style, uint16_t ellipsisIndex)
{
    if (ellipsisIndex != TEXT_ELLIPSIS_END_INV) {
        int16_t lineMaxWidth  = expandWidth_ ? textSize_.x : textRect.GetWidth();
        uint32_t maxLineBytes = 0;
        uint16_t lineCount = GetLine(lineMaxWidth, style.letterSpace_, ellipsisIndex, maxLineBytes);
        if ((lineCount > 0) && (textSize_.x < textLine_[lineCount - 1].linePixelWidth)) {
            textSize_.x = textLine_[lineCount - 1].linePixelWidth;
        }
    }
}

void Text::DrawEllipsis(BufferInfo& gfxDstBuffer, LabelLineInfo& labelLine, uint16_t& letterIndex)
{
    labelLine.offset.x = 0;
    labelLine.text = TEXT_ELLIPSIS;
    labelLine.lineLength = 1;
    labelLine.length = 1;
    DrawLabel::DrawTextOneLine(gfxDstBuffer, labelLine, letterIndex);
}

void Text::OnDraw(BufferInfo& gfxDstBuffer,
                  const Rect& invalidatedArea,
                  const Rect& viewOrigRect,
                  const Rect& textRect,
                  int16_t offsetX,
                  const Style& style,
                  uint16_t ellipsisIndex,
                  OpacityType opaScale)
{
    if ((text_ == nullptr) || (strlen(text_) == 0) || (fontSize_ == 0)) {
        return;
    }
    Rect mask = invalidatedArea;

    if (mask.Intersect(mask, textRect)) {
        Draw(gfxDstBuffer, mask, textRect, style, offsetX, ellipsisIndex, opaScale);
    }
}

void Text::Draw(BufferInfo& gfxDstBuffer,
                const Rect& mask,
                const Rect& coords,
                const Style& style,
                int16_t offsetX,
                uint16_t ellipsisIndex,
                OpacityType opaScale)
{
    Point offset = {offsetX, 0};
    int16_t lineMaxWidth = expandWidth_ ? textSize_.x : coords.GetWidth();
    uint16_t lineBegin = 0;
    uint32_t maxLineBytes = 0;
    uint16_t lineCount = GetLine(lineMaxWidth, style.letterSpace_, ellipsisIndex, maxLineBytes);
    int16_t lineHeight = style.lineHeight_;
    int16_t curLineHeight;
    UIFont* font = UIFont::GetInstance();
    uint16_t fontHeight = font->GetHeight(fontId_, fontSize_);
    uint16_t lineMaxHeight =
        font->GetLineMaxHeight(text_, textLine_[0].lineBytes, fontId_, fontSize_, 0, spannableString_);
    CalculatedCurLineHeight(lineHeight, curLineHeight, fontHeight, style, lineMaxHeight);
    Point pos = GetPos(lineHeight, style, lineCount, coords);
    OpacityType opa = DrawUtils::GetMixOpacity(opaScale, style.textOpa_);
    uint16_t letterIndex = 0;
    for (uint16_t i = 0; i < lineCount; i++) {
        if (pos.y > mask.GetBottom()) {
            return;
        }
        int16_t tempLetterIndex = letterIndex;
        uint16_t lineBytes = textLine_[i].lineBytes;
#if defined(ENABLE_ICU) && ENABLE_ICU
        SetLineBytes(lineBytes, lineBegin);
#endif
        if ((style.lineHeight_ == 0) && (spannableString_ != nullptr)) {
            curLineHeight = font->GetLineMaxHeight(
                &text_[lineBegin], textLine_[i].lineBytes, fontId_, fontSize_,
                tempLetterIndex, spannableString_);
            if (lineCount > 1) {
                curLineHeight += style.lineSpace_;
            }
        } else {
            curLineHeight = lineHeight;
            if (lineCount == 1) {
                curLineHeight -= style.lineSpace_;
            }
        }
        int16_t nextLine = pos.y + curLineHeight;
        if (lineHeight != style.lineHeight_) {
            nextLine -= style.lineSpace_;
        }
        Rect currentMask(mask.GetLeft(), pos.y, mask.GetRight(), pos.y + curLineHeight);
        currentMask.Intersect(currentMask, mask);
        if (nextLine >= mask.GetTop()) {
            pos.x = LineStartPos(coords, textLine_[i].linePixelWidth);
            LabelLineInfo labelLine {pos, offset, currentMask, curLineHeight, lineBytes,
                                     0, opa, style, &text_[lineBegin], lineBytes,
                                     lineBegin, fontId_, fontSize_, 0, static_cast<UITextLanguageDirect>(direct_),
                                     nullptr, baseLine_,
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
                                     textStyles_,
#endif
                                     &backgroundColor_, &foregroundColor_, &linebackgroundColor_, spannableString_, 0};
            uint16_t ellipsisOssetY = DrawLabel::DrawTextOneLine(gfxDstBuffer, labelLine, letterIndex);
            if ((i == (lineCount - 1)) && (ellipsisIndex != TEXT_ELLIPSIS_END_INV)) {
                labelLine.ellipsisOssetY = ellipsisOssetY;
                DrawEllipsis(gfxDstBuffer, labelLine, letterIndex);
            }
        } else {
            letterIndex = TypedText::GetUTF8CharacterSize(text_, lineBegin + lineBytes);
        }
        SetNextLineBegin(style, lineMaxHeight, curLineHeight, pos,
                         tempLetterIndex, lineHeight, lineBegin, i);
    }
}


void Text::CalculatedCurLineHeight(int16_t& lineHeight, int16_t& curLineHeight,
                                   uint16_t fontHeight, const Style& style, uint16_t lineMaxHeight)
{
    if (lineHeight <= 0) {
        lineHeight = fontHeight;
        lineHeight += style.lineSpace_;
    }
    if ((style.lineSpace_ == 0) && (spannableString_ != nullptr)) {
        curLineHeight = lineMaxHeight;
        curLineHeight += style.lineSpace_;
    } else {
        curLineHeight = lineHeight;
    }
}

Point Text::GetPos(int16_t& lineHeight, const Style& style, uint16_t& lineCount, const Rect& coords)
{
    Point pos;
    if (lineHeight == style.lineHeight_) {
        pos.y = TextPositionY(coords, (lineCount * lineHeight));
    } else {
        pos.y = TextPositionY(coords, (lineCount * lineHeight - style.lineSpace_));
    }
    return pos;
}

#if defined(ENABLE_ICU) && ENABLE_ICU
void Text::SetLineBytes(uint16_t& lineBytes, uint16_t lineBegin)
{
    if (this->IsEliminateTrailingSpaces()) {
        int j = lineBytes - 1;
        while (j >= 0 && text_[lineBegin + j] == ' ') {
            --j;
        }
        lineBytes = j + 1;
    }
}
#endif

void Text::SetNextLineBegin(const Style& style, uint16_t lineMaxHeight, int16_t& curLineHeight, Point& pos,
                            int16_t& tempLetterIndex, int16_t& lineHeight, uint16_t& lineBegin, uint16_t letterIndex)
{
    lineBegin += textLine_[letterIndex].lineBytes;
    pos.y += curLineHeight;
}

int16_t Text::TextPositionY(const Rect& textRect, int16_t textHeight)
{
    int16_t yOffset = 0;
    if (!expandHeight_ && (verticalAlign_ != TEXT_ALIGNMENT_TOP) && (textRect.GetHeight() > textHeight)) {
        if (verticalAlign_ == TEXT_ALIGNMENT_CENTER) {
            yOffset = (textRect.GetHeight() - textHeight) >> 1;
        } else if (verticalAlign_ == TEXT_ALIGNMENT_BOTTOM) {
            yOffset = textRect.GetHeight() - textHeight;
        }
    }
    return textRect.GetY() + yOffset;
}

int16_t Text::LineStartPos(const Rect& textRect, uint16_t lineWidth)
{
    int16_t xOffset = 0;
    int16_t rectWidth = textRect.GetWidth();
    if (horizontalAlign_ == TEXT_ALIGNMENT_CENTER) {
        xOffset = (direct_ == TEXT_DIRECT_RTL) ? ((rectWidth + lineWidth + 1) >> 1) : ((rectWidth - lineWidth) >> 1);
    } else if (horizontalAlign_ == TEXT_ALIGNMENT_RIGHT) {
        xOffset = (direct_ == TEXT_DIRECT_RTL) ? rectWidth : (rectWidth - lineWidth);
    } else {
        xOffset = (direct_ == TEXT_DIRECT_RTL) ? rectWidth : 0;
    }
    return textRect.GetX() + xOffset;
}

uint16_t Text::GetLine(int16_t width, uint8_t letterSpace, uint16_t ellipsisIndex, uint32_t& maxLineBytes)
{
    if (text_ == nullptr) {
        return 0;
    }
    uint16_t lineNum = 0;
    uint32_t textLen = GetTextStrLen();
    if ((ellipsisIndex != TEXT_ELLIPSIS_END_INV) && (ellipsisIndex < textLen)) {
        textLen = ellipsisIndex;
    }
    uint32_t begin = 0;
    uint16_t letterIndex = 0;
    while ((begin < textLen) && (text_[begin] != '\0') && (lineNum < MAX_LINE_COUNT)) {
        begin +=
            GetTextLine(begin, textLen, width, lineNum, letterSpace, letterIndex, spannableString_, textLine_[lineNum]);
        if (maxLineBytes < textLine_[lineNum].lineBytes) {
            maxLineBytes = textLine_[lineNum].lineBytes;
        }
        lineNum++;
    }
    if ((lineNum != 0) && (ellipsisIndex != TEXT_ELLIPSIS_END_INV)) {
        uint16_t ellipsisWidth =
            UIFont::GetInstance()->GetWidth(TEXT_ELLIPSIS_UNICODE, fontId_, fontSize_, 0) + letterSpace;
        textLine_[lineNum - 1].linePixelWidth += ellipsisWidth;
        if (textLine_[lineNum - 1].linePixelWidth > width) {
            int16_t newWidth = width - ellipsisWidth;
            maxLineBytes = CalculateLineWithEllipsis(begin, textLen, newWidth, letterSpace, lineNum, letterIndex,
                                                     spannableString_);
            textLine_[lineNum - 1].linePixelWidth += ellipsisWidth;
        }
    }
    return lineNum;
}

uint32_t Text::CalculateLineWithEllipsis(uint32_t begin, uint32_t textLen, int16_t width,
                                         uint8_t letterSpace, uint16_t& lineNum,
                                         uint16_t& letterIndex,
                                         SpannableString* spannableString)
{
    begin -= textLine_[lineNum - 1].lineBytes;
    lineNum--;
    while ((begin < textLen) && (text_[begin] != '\0') && (lineNum < MAX_LINE_COUNT)) {
        begin += GetTextLine(begin, textLen, width, lineNum, letterSpace, letterIndex, spannableString,
            textLine_[lineNum]);
        lineNum++;
    }
    uint32_t maxLineBytes = 0;
    for (uint16_t i = 0; i < lineNum; i++) {
        if (maxLineBytes < textLine_[i].lineBytes) {
            maxLineBytes = textLine_[i].lineBytes;
        }
    }
    return maxLineBytes;
}

uint32_t Text::GetTextStrLen()
{
    return (text_ != nullptr) ? (strlen(text_)) : 0;
}

uint32_t Text::GetTextLine(uint32_t begin, uint32_t textLen, int16_t width, uint16_t lineNum, uint8_t letterSpace,
                           uint16_t& letterIndex, SpannableString* spannableString, TextLine& textLine)
{
    int16_t lineWidth = width;
    int16_t lineHeight = 0;
    uint16_t nextLineBytes = UIFontAdaptor::GetNextLineAndWidth(&text_[begin], fontId_, fontSize_, letterSpace,
                                                                lineWidth, lineHeight, letterIndex, spannableString,
                                                                false, textLen - begin, IsEliminateTrailingSpaces());
    if (nextLineBytes + begin > textLen) {
        nextLineBytes = textLen - begin;
    }
    textLine.lineBytes = nextLineBytes;
    textLine.linePixelWidth = lineWidth;
    return nextLineBytes;
}

uint16_t Text::GetEllipsisIndex(const Rect& textRect, const Style& style)
{
    if (textSize_.y <= textRect.GetHeight()) {
        return TEXT_ELLIPSIS_END_INV;
    }
    UIFont* fontEngine = UIFont::GetInstance();
    int16_t letterWidth = fontEngine->GetWidth(TEXT_ELLIPSIS_UNICODE, fontId_, fontSize_, 0) + style.letterSpace_;
    Point p;
    p.x = textRect.GetWidth() - letterWidth;
    p.y = textRect.GetHeight();
    int16_t height = style.lineHeight_;
    if (height == 0) {
        height = fontEngine->GetHeight(fontId_, fontSize_) + style.lineSpace_;
    }
    if (height) {
        p.y -= p.y % height;
    }
    if (height != style.lineHeight_) {
        p.y -= style.lineSpace_;
    }
    return GetLetterIndexByPosition(textRect, style, p);
}

uint16_t Text::GetLetterIndexByLinePosition(const Style& style, int16_t contentWidth,
                                            const int16_t& posX, int16_t offsetX)
{
    uint16_t letterIndex = 0;
    int16_t width = 0;
    if (direct_ == UITextLanguageDirect::TEXT_DIRECT_LTR) {
        width = posX - offsetX;
    }

    int16_t lineHeight = style.lineHeight_;
    UIFontAdaptor::GetNextLineAndWidth(text_, fontId_, fontSize_, style.letterSpace_,
                                       width, lineHeight, letterIndex, spannableString_,
                                       false, 0xFFFF, IsEliminateTrailingSpaces());
    return letterIndex;
}

uint16_t Text::GetPosXByLetterIndex(const Rect &textRect, const Style &style,
                                    uint16_t beginIndex, uint16_t count)
{
    if (count == 0) {
        return 0;
    }

    int16_t maxWidth = (expandWidth_ ? COORD_MAX : textRect.GetWidth());

    int16_t textWidth = TypedText::GetTextWidth(text_, fontId_, fontSize_, GetTextStrLen(),
        style.letterSpace_, beginIndex, count);

    return static_cast<uint16_t>(textWidth > maxWidth ? maxWidth : textWidth);
}

uint16_t Text::GetLetterIndexByPosition(const Rect& textRect, const Style& style, const Point& pos)
{
    if (text_ == nullptr) {
        return 0;
    }
    uint32_t lineStart = 0;
    uint32_t nextLineStart = 0;
    int16_t lineHeight = style.lineHeight_;
    uint16_t letterHeight = UIFont::GetInstance()->GetHeight(fontId_, fontSize_);
    if (lineHeight == 0) {
        lineHeight = letterHeight + style.lineSpace_;
    }
    uint16_t height = 0;
    if (lineHeight != style.lineHeight_) {
        height = letterHeight;
    } else {
        height = lineHeight;
    }
    int16_t y = 0;
    uint32_t textLen = static_cast<uint32_t>(strlen(text_));
    int16_t width = 0;
    uint16_t letterIndex = 0;
    while ((lineStart < textLen) && (text_[lineStart] != '\0')) {
        width = textRect.GetWidth();
        nextLineStart += UIFontAdaptor::GetNextLineAndWidth(&text_[lineStart], fontId_, fontSize_, style.letterSpace_,
                                                            width, lineHeight, letterIndex, spannableString_,
                                                            false, 0xFFFF, IsEliminateTrailingSpaces());
        if (nextLineStart == 0) {
            break;
        }
        if (pos.y <= y + height) {
            break;
        }
        y += lineHeight;
        lineStart = nextLineStart;
    }
    if (nextLineStart == textLen) {
        return TEXT_ELLIPSIS_END_INV;
    }
    /* Calculate the x coordinate */
    width = pos.x;
    lineStart +=
        UIFontAdaptor::GetNextLineAndWidth(&text_[lineStart], fontId_, fontSize_, style.letterSpace_, width, lineHeight,
                                           letterIndex, spannableString_, true, 0xFFFF, IsEliminateTrailingSpaces());
    return (lineStart < textLen) ? lineStart : TEXT_ELLIPSIS_END_INV;
}

void Text::SetAbsoluteSizeSpan(uint16_t start, uint16_t end, uint8_t size)
{
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
    if (fontId_ == FONT_ID_MAX) {
        return;
    }
#else
    if (fontId_ == UIFontBuilder::GetInstance()->GetBitmapFontIdMax()) {
        return;
    }
#endif
    uint16_t fontId = GetSpanFontIdBySize(size);
#if defined(ENABLE_VECTOR_FONT) && !ENABLE_VECTOR_FONT
    if (fontId == fontId_) {
        return;
    }
#endif
    if (text_ != nullptr && spannableString_ == nullptr) {
        spannableString_ = new SpannableString();
        if (spannableString_ == nullptr) {
            GRAPHIC_LOGE("Text::SetAbsoluteSizeSpan invalid parameter");
            return;
        }
    }
    if (spannableString_ != nullptr) {
        spannableString_->SetFontSize(size, start, end);
        spannableString_->SetFontId(fontId, start, end);
    }
}

void Text::SetRelativeSizeSpan(uint16_t start, uint16_t end, float size)
{
    uint8_t absoluteSize = 0;
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
    absoluteSize = static_cast<uint8_t>(size * fontSize_);
#else
    UITextLanguageFontParam* fontParam = UIFontBuilder::GetInstance()->GetTextLangFontsTable(fontId_);
    if (fontParam == nullptr) {
        GRAPHIC_LOGE("Text::SetRelativeSizeSpan invalid parameter");
        return;
    }
    absoluteSize = static_cast<uint8_t>(size * fontParam->size);
#endif
    SetAbsoluteSizeSpan(start, end, absoluteSize);
}

uint16_t Text::GetSpanFontIdBySize(uint8_t size)
{
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
    return fontId_;
#else
    UIFontBuilder* fontBuilder = UIFontBuilder::GetInstance();
    UITextLanguageFontParam* fontParam = fontBuilder->GetTextLangFontsTable(fontId_);
    if (fontParam == nullptr) {
        return fontId_;
    }

    uint8_t ttfId = fontParam->ttfId;
    for (uint16_t fontId = 0; fontId < fontBuilder->GetTotalFontId(); fontId++) {
        UITextLanguageFontParam* tempFontParam = fontBuilder->GetTextLangFontsTable(fontId);
        if (tempFontParam == nullptr) {
            continue;
        }
        if (ttfId == tempFontParam->ttfId && size == tempFontParam->size) {
            return fontId;
        }
    }
    return fontId_;
#endif
}

uint16_t Text::GetNextCharacterFullDispalyOffset(const Rect& textRect,
    const Style& style, uint16_t beginIndex, uint16_t num)
{
    return GetPosXByLetterIndex(textRect, style, beginIndex, num);
}

int16_t Text::GetMetaTextWidth(const Style& style)
{
    return TypedText::GetTextWidth(text_, GetFontId(), GetFontSize(), strlen(text_), style.letterSpace_);
}
} // namespace OHOS
