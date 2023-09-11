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

#include "components/ui_edit_text.h"

#include <codecvt>
#include <locale>

#include "font/ui_font.h"
#include "gfx_utils/graphic_log.h"
#include "securec.h"
#include "themes/theme_manager.h"
#include "common/typed_text.h"

namespace OHOS {
class CursorAnimator : public Animator, public AnimatorCallback {
public:
    explicit CursorAnimator(UIEditText* view) : Animator(this, view, 0, true), editText_(view) {}

    virtual ~CursorAnimator() {}

    void Callback(UIView* view) override
    {
        if ((view == nullptr) || (editText_ == nullptr)) {
            return;
        }

        uint32_t curTime = GetRunTime();
        if (curTime == preTime_) {
            return;
        }
        uint32_t duration = (curTime > preTime_) ? (curTime - preTime_) : (UINT32_MAX - preTime_ + curTime);
        if (duration < CURSOR_ANIMATOT_DURATION) {
            return;
        }
        preTime_ = curTime;
        editText_->drawCursor_ = !editText_->drawCursor_;
        view->Invalidate();
    }

    void StartAnimator()
    {
        if (editText_ == nullptr) {
            return;
        }
        Start();
        preTime_ = GetRunTime();
        editText_->drawCursor_ = false;
    }

    void StopAnimator()
    {
        if (editText_ == nullptr) {
            return;
        }
        Stop();
        editText_->drawCursor_ = false;
        editText_->Invalidate();
    }

private:
    uint32_t preTime_ = 0;
    UIEditText* editText_ = nullptr;
    static constexpr uint16_t CURSOR_ANIMATOT_DURATION = 650;
};

UIEditText::UIEditText()
    : inputText_(nullptr),
      placeholderText_(nullptr),
      offsetX_(DEFAULT_TEXT_OFFSET),
      cursorIndex_(0),
      deleteTextWidth_(0),
      insertTextWidth_(0),
      offsetState_(UPDATE_OFFSET_UNKNOW),
      needRefresh_(false),
      useTextColor_(false),
      isFocused_(false),
      drawCursor_(false),
      maxLength_(MAX_TEXT_LENGTH),
      placeholderEllipsisIndex_(Text::TEXT_ELLIPSIS_END_INV),
      cursorPosX_(0),
      textColor_(Color::White()),
      placeholderColor_(Color::Gray()),
      cursorColor_(Color::White()),
      onChangeListener_(nullptr),
      cursorAnimator_(nullptr)
{
    touchable_ = true;
    focusable_ = true;
    Theme* theme = ThemeManager::GetInstance().GetCurrent();
    Style& style = (theme != nullptr) ? (theme->GetEditTextStyle()) : (StyleDefault::GetEditTextStyle());
    UIView::SetStyle(style);
    SetDraggable(true);
}

UIEditText::~UIEditText()
{
    if (cursorAnimator_ != nullptr) {
        delete cursorAnimator_;
        cursorAnimator_ = nullptr;
    }
    if (inputText_ != nullptr) {
        delete inputText_;
        inputText_ = nullptr;
    }
    if (placeholderText_ != nullptr) {
        delete placeholderText_;
        placeholderText_ = nullptr;
    }
}

bool UIEditText::OnPressEvent(const PressEvent& event)
{
    DealPressEvents(false, event);
    return UIView::OnPressEvent(event);
}

bool UIEditText::OnLongPressEvent(const LongPressEvent &event)
{
    DealPressEvents(true, event);
    return UIView::OnLongPressEvent(event);
}

void UIEditText::DealPressEvents(bool longPressEvent, const Event &event)
{
    if (touchable_) {
        if (!longPressEvent) {
            Point pressPos = event.GetCurrentPos();
            pressPos.x = pressPos.x - GetOrigRect().GetX();
            pressPos.y = pressPos.y - GetOrigRect().GetY();
            Style style = GetStyleConst();
            cursorIndex_ = inputText_->GetLetterIndexByLinePosition(style,
                GetContentRect().GetWidth(), pressPos.x, offsetX_);
            offsetState_ = UPDATE_OFFSET_PRESS_EVENT;
            UpdateOffsetX();
        }
        RequestFocus();
        Invalidate();
    }
}

bool UIEditText::OnDragEvent(const DragEvent& event)
{
    DealPressEvents(false, event);
    return UIView::OnDragEvent(event);
}

void UIEditText::Focus()
{
    if (focusable_) {
        if (cursorAnimator_ == nullptr) {
            cursorAnimator_ = new CursorAnimator(this);
        }
        static_cast<CursorAnimator*>(cursorAnimator_)->StartAnimator();
        isFocused_ = true;
    }
    Invalidate();
    UIView::Focus();
}

void UIEditText::Blur()
{
    if (cursorAnimator_ != nullptr) {
        static_cast<CursorAnimator*>(cursorAnimator_)->StopAnimator();
    }
    isFocused_ = false;
    Invalidate();
    UIView::Blur();
}

void UIEditText::InitText()
{
    if (inputText_ == nullptr) {
        inputText_ = new Text();
        inputText_->SetAlign(TEXT_ALIGNMENT_LEFT, TEXT_ALIGNMENT_CENTER);
        inputText_->SetExpandWidth(true);
        inputText_->SetExpandHeight(false);
    }

    if (placeholderText_ == nullptr) {
        placeholderText_ = new Text();
        placeholderText_->SetAlign(TEXT_ALIGNMENT_LEFT, TEXT_ALIGNMENT_CENTER);
        placeholderText_->SetExpandWidth(false);
        placeholderText_->SetExpandHeight(false);
    }
}

void UIEditText::SetStyle(Style& style)
{
    UIView::SetStyle(style);
    RefreshText();
}

void UIEditText::SetStyle(uint8_t key, int64_t value)
{
    UIView::SetStyle(key, value);
    RefreshText();
}

void UIEditText::SetText(const char* text)
{
    InitText();
    if (text == nullptr) {
        return;
    }

    std::string inputText = std::string(text);
    SetText(inputText);
    cursorIndex_ = TypedText::GetUTF8CharacterSize(text, inputText.length());
    offsetState_ = UPDATE_OFFSET_INTERFACE;
}

void UIEditText::SetText(std::string text)
{
    UpdateTextString(text);
    UpdateInnerText();
}

const char* UIEditText::GetText()
{
    return textStr_.c_str();
}

void UIEditText::UpdateTextString(std::string text)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    std::wstring wideText = convert.from_bytes(text);
    uint32_t textLen = wideText.length();
    uint16_t maxLength = GetMaxLength();
    if (textLen > maxLength) {
        textLen = maxLength;
    }
    std::wstring newWideText = wideText.substr(0, textLen);
    std::string newText = convert.to_bytes(newWideText);
    CheckValueChange(newText);
    textStr_ = newText;
    passwordStr_ = std::string(newWideText.length(), *PASSWORD_DOT.c_str());
}

std::string UIEditText::GetInnerText()
{
    return textStr_;
}

std::string UIEditText::GetInnerPassword()
{
    return passwordStr_;
}

void UIEditText::SetPlaceholder(const char* text)
{
    InitText();
    placeholderText_->SetText(text);
    if (placeholderText_->IsNeedRefresh()) {
        RefreshText();
    }
}

const char* UIEditText::GetPlaceholder()
{
    if ((placeholderText_ == nullptr) || placeholderText_->GetText() == nullptr) {
        return "";
    } else {
        return placeholderText_->GetText();
    }
}

void UIEditText::SetFontId(uint16_t fontId)
{
    InitText();
    inputText_->SetFontId(fontId);
    placeholderText_->SetFontId(fontId);
    if (inputText_->IsNeedRefresh()) {
        RefreshText();
    }
}

void UIEditText::SetFont(const char* name, uint8_t size)
{
    InitText();
    inputText_->SetFont(name, size);
    placeholderText_->SetFont(name, size);
    if (inputText_->IsNeedRefresh()) {
        RefreshText();
    }
}

uint16_t UIEditText::GetTextWidth()
{
    InitText();
    if (inputText_->IsNeedRefresh()) {
        ReMeasure();
    }
    return inputText_->GetTextSize().x;
}

uint16_t UIEditText::GetTextHeight()
{
    InitText();
    if (inputText_->IsNeedRefresh()) {
        ReMeasure();
    }
    return inputText_->GetTextSize().y;
}

void UIEditText::RefreshText()
{
    Invalidate();
    placeholderEllipsisIndex_ = Text::TEXT_ELLIPSIS_END_INV;
    if (!needRefresh_) {
        needRefresh_ = true;
    }
}

void UIEditText::ReMeasure()
{
    if (!needRefresh_) {
        return;
    }
    needRefresh_ = false;
    InitText();
    Style style = GetStyleConst();
    style.textColor_ = GetTextColor();
    Rect contentRect = GetContentRect();
    int16_t width = contentRect.GetWidth() - BOTH_SIDE_TEXT_OFFSET;
    contentRect.SetWidth(width > 0 ? width : 0);
    inputText_->ReMeasureTextSize(contentRect, style);
    placeholderText_->ReMeasureTextSize(contentRect, style);
    placeholderEllipsisIndex_ = placeholderText_->GetEllipsisIndex(contentRect, style);
    placeholderText_->ReMeasureTextWidthInEllipsisMode(contentRect, style, placeholderEllipsisIndex_);

    UpdateOffsetX();
}

void UIEditText::UpdateOffsetX()
{
    if (!inputText_) {
        return;
    }

    Rect contentRect = GetContentRect();
    Style style = GetStyleConst();

    uint16_t textLength = GetTextLength();
    uint16_t firstVisibleIndex = GetFirstVisibleIndex();
    uint16_t lastVisibleIndex = GetLastVisibleIndex();
    if (textLength == 0 || (firstVisibleIndex == 0 && lastVisibleIndex == textLength)) {
        offsetX_ = DEFAULT_TEXT_OFFSET;
        offsetState_ = UPDATE_OFFSET_UNKNOW;
        deleteTextWidth_ = 0;
        insertTextWidth_ = 0;
        return;
    }

    switch (offsetState_) {
        case UPDATE_OFFSET_INTERFACE:
            offsetX_ = DEFAULT_TEXT_OFFSET;
            break;
        case UPDATE_OFFSET_PRESS_EVENT:
            UpdateExtraOffsetX(firstVisibleIndex, lastVisibleIndex);
            break;
        case UPDATE_OFFSET_DELETE:
        case UPDATE_OFFSET_INSERT:
            UpdateInsertDeletedOffset();
            break;
        case UPDATE_OFFSET_SET_CURSOR:
            UpdateOffsetBySetCursorIndex();
            break;
        case UPDATE_OFFSET_INPUT_TYPE:
            UpdateOffsetByInputType();
            break;
        default:
            break;
    }
    offsetState_ = UPDATE_OFFSET_UNKNOW;
    deleteTextWidth_ = 0;
    insertTextWidth_ = 0;
}

uint16_t UIEditText::GetFirstVisibleIndex()
{
    if (!inputText_) {
        return 0;
    }
    Rect contentRect = GetContentRect();
    Style style = GetStyleConst();
    uint16_t firstVisibleIndex = 0;
    if (inputText_->GetDirect() == UITextLanguageDirect::TEXT_DIRECT_LTR) {
        if (offsetX_ > 0) {
            return 0;
        }
        // Returns the number of characters that can be skipped by the offset.
        firstVisibleIndex = inputText_->GetLetterIndexByLinePosition(style, contentRect.GetWidth(),
            0, offsetX_) + 1;
    }
    return firstVisibleIndex;
}

uint16_t UIEditText::GetLastVisibleIndex()
{
    if (!inputText_) {
        return 0;
    }
    Rect contentRect = GetContentRect();
    Style style = GetStyleConst();
    uint16_t lastVisibleIndex = 0;
    if (inputText_->GetDirect() == UITextLanguageDirect::TEXT_DIRECT_LTR) {
        lastVisibleIndex = inputText_->GetLetterIndexByLinePosition(style, contentRect.GetWidth(),
            contentRect.GetWidth(), offsetX_);
    }
    return lastVisibleIndex;
}

void UIEditText::UpdateExtraOffsetX(const uint16_t firstVisibleIndex,
                                    const uint16_t lastVisibleIndex)
{
    if (!inputText_) {
        return;
    }
    Rect contentRect = GetContentRect();
    Style style = GetStyleConst();
    uint16_t characterSize = 0;
    uint16_t textLength = GetTextLength();
    if (cursorIndex_ - firstVisibleIndex < 1 && firstVisibleIndex != 0) {
        characterSize = inputText_->GetNextCharacterFullDispalyOffset(contentRect, style, firstVisibleIndex - 1, 1);
        offsetX_ += characterSize + DEFAULT_TEXT_OFFSET;
        if (GetFirstVisibleIndex() == 0) {
            offsetX_ = DEFAULT_TEXT_OFFSET;
        }
    } else if (lastVisibleIndex - cursorIndex_ < 1 && lastVisibleIndex != textLength) {
        characterSize = inputText_->GetNextCharacterFullDispalyOffset(contentRect, style, lastVisibleIndex, 1);
        offsetX_ -= characterSize + DEFAULT_TEXT_OFFSET;
        if (GetLastVisibleIndex() == textLength) {
            offsetX_ = -(GetTextWidth() - GetRect().GetWidth() + BOTH_SIDE_TEXT_OFFSET);
        }
    }
}

uint16_t UIEditText::GetTextWidthByCursorIndex(const uint16_t cursorIndex)
{
    if (!inputText_ || cursorIndex == 0) {
        return 0;
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    std::wstring wideText = convert.from_bytes(textStr_);
    wideText = wideText.substr(0, cursorIndex);
    std::string clipText = convert.to_bytes(wideText);

    return TypedText::GetTextWidth(clipText.c_str(), GetFontId(), inputText_->GetFontSize(),
        clipText.length(), GetStyleConst().letterSpace_);
}

void UIEditText::UpdateInsertDeletedOffset()
{
    if (deleteTextWidth_ > 0) {
        if (offsetX_ + DEFAULT_TEXT_OFFSET < 0) {
            offsetX_ += deleteTextWidth_;
            if (GetFirstVisibleIndex() == 0 && GetLastVisibleIndex() == GetTextLength()) {
                offsetX_ = DEFAULT_TEXT_OFFSET;
            }
        } else {
            offsetX_ = DEFAULT_TEXT_OFFSET;
        }
    } else if (insertTextWidth_ > 0) {
        if (cursorIndex_ == GetTextLength()) {
            offsetX_ = -(GetTextWidth() - GetRect().GetWidth() + BOTH_SIDE_TEXT_OFFSET);
        } else {
            offsetX_ -= insertTextWidth_;
        }
    }
}

void UIEditText::OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    InitText();
    UIView::OnDraw(gfxDstBuffer, invalidatedArea);

    bool drawPlaceholder = false;
    if (inputText_->GetText() != nullptr && strlen(inputText_->GetText()) > 0) {
        Style style = GetStyleConst();
        style.textColor_ = GetTextColor();
        OpacityType opa = GetMixOpaScale();
        inputText_->OnDraw(gfxDstBuffer, invalidatedArea, GetOrigRect(), GetContentRect(), offsetX_, style,
                           Text::TEXT_ELLIPSIS_END_INV, opa);
        drawPlaceholder = false;
    } else {
        Style style = GetStyleConst();
        style.textColor_ = GetPlaceholderColor();
        OpacityType opa = GetMixOpaScale();
        placeholderText_->OnDraw(gfxDstBuffer, invalidatedArea, GetOrigRect(), GetContentRect(), DEFAULT_TEXT_OFFSET,
                                 style, placeholderEllipsisIndex_, opa);
        drawPlaceholder = true;
    }

    DrawCursor(gfxDstBuffer, invalidatedArea, drawPlaceholder);
}

void UIEditText::DrawCursor(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, bool drawPlaceholder)
{
    if (!(isFocused_ && drawCursor_)) {
        return;
    }

    CalculatedCursorPos(drawPlaceholder);

    Style* cursorStyle = new Style();
    cursorStyle->SetStyle(STYLE_BACKGROUND_COLOR, cursorColor_.full);
    cursorStyle->SetStyle(STYLE_BACKGROUND_OPA, OPA_OPAQUE);

    int16_t left = 0;
    UITextLanguageDirect direct = drawPlaceholder ? placeholderText_->GetDirect() : inputText_->GetDirect();
    if (direct == UITextLanguageDirect::TEXT_DIRECT_LTR) {
        left = cursorPosX_ - DEFAULT_CURSOR_OFFSET;
    } else if (direct == UITextLanguageDirect::TEXT_DIRECT_RTL) {
        left = cursorPosX_ + DEFAULT_CURSOR_OFFSET;
    }

    Rect viewRect;
    viewRect.SetLeft(left);
    viewRect.SetTop(GetOrigRect().GetTop() + (GetRect().GetHeight() - inputText_->GetFontSize()) / 2); // 2: harf size
    viewRect.SetHeight(inputText_->GetFontSize());
    viewRect.SetWidth(DEFAULT_CURSOR_WIDTH);

    BaseGfxEngine::GetInstance()->DrawRect(gfxDstBuffer, viewRect, invalidatedArea, *cursorStyle, OPA_OPAQUE);
    delete cursorStyle;
}

void UIEditText::InsertText(std::string text)
{
    InitText();
    InsertTextByCursorIndex(text);
}

void UIEditText::InsertTextByCursorIndex(std::string text)
{
    if (!inputText_) {
        return;
    }
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    std::wstring wideText = convert.from_bytes(textStr_);
    std::wstring insertWText = convert.from_bytes(text);
    uint32_t textLen = wideText.length();
    uint32_t insertWTextLen = insertWText.length();
    if (cursorIndex_ > textLen || insertWTextLen + textLen > maxLength_) {
        return;
    }

    std::wstring newWideText = wideText.insert(cursorIndex_, insertWText);
    std::string newText = convert.to_bytes(newWideText);
    cursorIndex_ += insertWTextLen;
    SetText(newText);

    Style style = GetStyleConst();
    Rect contentRect = GetContentRect();
    cursorPosX_ = inputText_->GetPosXByLetterIndex(contentRect, style, 0, cursorIndex_);
    if (cursorPosX_ >= contentRect.GetWidth() - DEFAULT_TEXT_OFFSET) {
        insertTextWidth_ = inputText_->GetNextCharacterFullDispalyOffset(contentRect,
            style, cursorIndex_ - insertWTextLen, insertWTextLen) + style.letterSpace_;
        offsetState_ = UPDATE_OFFSET_INSERT;
    }

    if (isFocused_) {
        if (cursorAnimator_ != nullptr) {
            static_cast<CursorAnimator*>(cursorAnimator_)->StartAnimator();
        }
    }
}

void UIEditText::CalculatedCursorPos(bool drawPlaceholder)
{
    if (!inputText_ || !placeholderText_) {
        return;
    }

    Style style = GetStyleConst();
    Rect contentRect = GetContentRect();
    UITextLanguageDirect direct = drawPlaceholder ? placeholderText_->GetDirect() : inputText_->GetDirect();
    if (direct == UITextLanguageDirect::TEXT_DIRECT_LTR) {
        cursorPosX_ = contentRect.GetX() + inputText_->GetPosXByLetterIndex(contentRect,
            style, 0, cursorIndex_) + offsetX_;
    } else if (direct == UITextLanguageDirect::TEXT_DIRECT_RTL) {
        cursorPosX_ = GetRect().GetRight() - style.borderWidth_ - style.paddingRight_
            - inputText_->GetPosXByLetterIndex(contentRect, style, 0, cursorIndex_)
            - offsetX_;
    }
}

uint16_t UIEditText::GetCursorIndex()
{
    return cursorIndex_;
}

void UIEditText::SetCursorIndex(uint16_t cursorIndex)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    std::wstring wideText = convert.from_bytes(textStr_);
    if (cursorIndex > wideText.length()) {
        cursorIndex_ = wideText.length();
    } else {
        cursorIndex_ = cursorIndex;
    }

    offsetState_ = UPDATE_OFFSET_SET_CURSOR;
    RefreshText();
    if (cursorAnimator_ != nullptr) {
        static_cast<CursorAnimator*>(cursorAnimator_)->StartAnimator();
    }
}

void UIEditText::UpdateOffsetBySetCursorIndex()
{
    if (!inputText_) {
        return;
    }

    uint16_t textWidth = GetTextWidthByCursorIndex(cursorIndex_);
    Rect contentRect = GetContentRect();
    int16_t newPosX = 0;
    if (inputText_->GetDirect() == UITextLanguageDirect::TEXT_DIRECT_LTR) {
        newPosX = contentRect.GetX() + textWidth + offsetX_;
        if (newPosX <= contentRect.GetX()) {
            offsetX_ = -(textWidth - DEFAULT_TEXT_OFFSET);
        } else if (newPosX >= GetRect().GetRight()) {
            offsetX_ = -(textWidth - GetRect().GetWidth() + BOTH_SIDE_TEXT_OFFSET);
        }
    }
}

void UIEditText::DeleteBackward(uint32_t length)
{
    if ((length == 0) || (textStr_.length() == 0) || (cursorIndex_ == 0)) {
        return;
    }
    if (!inputText_) {
        return;
    }

    std::string newText;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    std::wstring wideText = convert.from_bytes(textStr_);
    if (length > wideText.size()) {
        cursorIndex_ = 0;
    } else {
        uint32_t deleteLength = cursorIndex_ >= length ? length : length - cursorIndex_;
        if (wideText.size() >= cursorIndex_) {
            wideText.erase(cursorIndex_ - deleteLength, deleteLength);
        }
        newText = convert.to_bytes(wideText);
        cursorIndex_ -= deleteLength;
        deleteTextWidth_ = inputText_->GetNextCharacterFullDispalyOffset(GetContentRect(),
            GetStyleConst(), cursorIndex_, deleteLength);
        offsetState_ = UPDATE_OFFSET_DELETE;
    }

    SetText(newText);
    if (cursorAnimator_ != nullptr) {
        static_cast<CursorAnimator*>(cursorAnimator_)->StartAnimator();
    }
}

void UIEditText::UpdateInnerText()
{
    InitText();
    if (inputType_ == InputType::TEXT_TYPE) {
        inputText_->SetText(GetInnerText().c_str());
    } else {
        inputText_->SetText(GetInnerPassword().c_str());
    }
    RefreshText();
}

void UIEditText::SetMaxLength(uint16_t maxLength)
{
    InitText();
    if (maxLength > MAX_TEXT_LENGTH) {
        maxLength = MAX_TEXT_LENGTH;
    }
    maxLength_ = maxLength;
    if (textStr_.length() > maxLength) {
        SetText(textStr_.substr(0, maxLength));
    }
}

uint16_t UIEditText::GetMaxLength()
{
    return maxLength_;
}

void UIEditText::SetInputType(InputType inputType)
{
    if (inputType_ == inputType) {
        return;
    }
    inputType_ = inputType;

    // update view
    UpdateInnerText();

    offsetState_ = UPDATE_OFFSET_INPUT_TYPE;
}

void UIEditText::UpdateOffsetByInputType()
{
    if (!inputText_) {
        return;
    }

    Rect contentRect = GetContentRect();
    if (GetTextWidth() <= contentRect.GetWidth() - DEFAULT_TEXT_OFFSET) {
        offsetX_ = DEFAULT_TEXT_OFFSET;
        return;
    }

    if (cursorIndex_ == GetTextLength()) {
        offsetX_ = -(GetTextWidth() - GetRect().GetWidth() + BOTH_SIDE_TEXT_OFFSET);
        return;
    }

    if (offsetX_ + GetTextWidth() < contentRect.GetWidth() - DEFAULT_TEXT_OFFSET) {
        offsetX_ = contentRect.GetWidth() - DEFAULT_TEXT_OFFSET - GetTextWidth();
        return;
    }

    UpdateOffsetBySetCursorIndex();
}

void UIEditText::CheckValueChange(std::string text)
{
    if (onChangeListener_ == nullptr) {
        return;
    }

    if (textStr_.compare(text) != 0) {
        onChangeListener_->OnChange(*this, text.c_str());
    }
}

uint16_t UIEditText::GetTextLength()
{
    if (inputText_ == nullptr) {
        return 0;
    }
    return TypedText::GetUTF8CharacterSize(inputText_->GetText(),
        std::string(inputText_->GetText()).length());
}
} // namespace OHOS
