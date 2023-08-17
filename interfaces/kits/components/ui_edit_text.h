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

/**
 * @addtogroup UI_Components
 * @{
 *
 * @brief Defines UI components such as buttons, texts, images, lists, and progress bars.
 *
 */

/**
 * @file ui_edit_text.h
 *
 * @brief Declares a <b>UIEditText</b> class that represents a input edit view.
 *
 */

#ifndef GRAPHIC_LITE_UI_EDIT_TEXT
#define GRAPHIC_LITE_UI_EDIT_TEXT

#include "animator/animator.h"
#include "common/text.h"
#include "components/ui_view.h"

namespace OHOS {
/**
 * @brief Defines the functions for presenting a edit text in a specified area, setting the style and background color,
 *        and setting the display mode such as text and password type.
 */
class UIEditText : public UIView {
public:
    /**
     * @brief Defines a value change event listener. You need to register this listener with the view to listen to
     *        value change events.
     */
    class OnChangeListener : public HeapBase {
    public:
        /**
         * @brief Called when edit text value changed.
         *
         * @param view Indicates the UIEditView.
         * @param value Indicates the changed value.
         */
        virtual void OnChange(UIView& view, const char* value) {}

        /**
         * @brief A destructor used to delete the <b>OnChangeListener</b> instance.
         */
        virtual ~OnChangeListener() {}
    };

    /**
     * @brief A constructor used to create a <b>UIEditText</b> instance.
     */
    UIEditText();

    /**
     * @brief A destructor used to delete the <b>UIEditText</b> instance.
     */
    virtual ~UIEditText();

    /**
     * @brief Obtains the view type.
     *
     * @return Returns <b>UI_EDIT_TEXT</b>, as defined in {@link UIViewType}.
     */
    UIViewType GetViewType() const override
    {
        return UI_EDIT_TEXT;
    }

    /**
     * @brief   Executes the press event action
     *
     * @param   [in] event   The press event, contain press position.
     * @return Returns <b>true</b> if the event is consumed; returns <b>false</b> otherwise.
     */
    bool OnPressEvent(const PressEvent& event) override;

    /**
     * @brief   Executes the long press event action
     *
     * @param   [in] event   The long press event, contain press position.
     * @return Returns <b>true</b> if the event is consumed; returns <b>false</b> otherwise.
     */
    bool OnLongPressEvent(const LongPressEvent& event) override;

    /**
     * @brief Sets the view style.
     *
     * @param style Indicates the view style.
     */
    void SetStyle(Style& style) override;

    /**
     * @brief Sets a style.
     *
     * @param key Indicates the key of the style to set.
     * @param value Indicates the value matching the key.
     */
    void SetStyle(uint8_t key, int64_t value) override;

    /**
     * @brief Checks whether this view needs to be covered before drawing it.
     *
     * @param invalidatedArea Indicates the area to draw.
     * @return Returns <b>true</b> if this view needs to be covered; returns <b> false</b> otherwise.
     */
    bool OnPreDraw(Rect& invalidatedArea) const override
    {
        return false;
    }

    bool OnDragEvent(const DragEvent& event) override;

    /**
     * @brief Draws this view.
     *
     * @param invalidatedArea Indicates the area to draw.
     */
    void OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea) override;

    void Focus() override;

    void Blur() override;

    /**
     * @brief Sets the text content.
     *
     * @param text Indicates the pointer to the text content.
     */
    void SetText(const char* text);

    /**
     * @brief Obtains the input text of this view.
     *
     * @return Returns the text.
     */
    const char* GetText();

    /**
     * @brief Sets the placeholder content.
     *
     * @param placeholder Indicates the pointer to the placeholder content.
     */
    void SetPlaceholder(const char* placeholder);

    /**
     * @brief Sets the vaule listener for this view.
     *        The listener is triggered to invoke the callback function when the value changed.
     *
     * @param listener Indicates the listener to set. For details, see {@link OnChangeListener}.
     */
    const char* GetPlaceholder();

    /**
     * @brief Set max length of the input text.
     *
     * @param maxLength max length size.
     */
    void SetMaxLength(uint16_t maxLength);

    /**
     * @brief Get max length of the input text.
     *
     * @return return the length size.
     */
    uint16_t GetMaxLength();

    /**
     * @brief Set the input type.
     *
     * @param type the input type, such as text or password.
     */
    void SetInputType(InputType type);

    /**
     * @brief Get the input type.
     *
     * @param type the input type, such as text or password.
     */
    InputType GetInputType()
    {
        return inputType_;
    }

    /**
     * @brief Sets the color for this text.
     *
     * @param color Indicates the text color to set.
     */
    void SetTextColor(ColorType color)
    {
        useTextColor_ = true;
        textColor_ = color;
    }

    /**
     * @brief Obtains the color of this text.
     *
     * @return Returns the text color.
     */
    ColorType GetTextColor() const
    {
        return useTextColor_ ? textColor_ : GetStyleConst().textColor_;
    }

    /**
     * @brief Sets the color of the palceholder for this text.
     *
     * @param color Indicates the palceholder color to set.
     */
    void SetPlaceholderColor(ColorType color)
    {
        placeholderColor_ = color;
    }

    /**
     * @brief Obtains the palceholder color of this text.
     *
     * @return Returns the palceholder color.
     */
    ColorType GetPlaceholderColor() const
    {
        return placeholderColor_;
    }

    /**
     * @brief Sets the cursor color.
     *
     * @param color Indicates the cursor color to set.
     */
    void SetCursorColor(ColorType color)
    {
        cursorColor_ = color;
    }

    /**
     * @brief Obtains the cursor color.
     *
     * @return Returns the cursor color.
     */
    ColorType GetCursorColor() const
    {
        return cursorColor_;
    }

    /**
     * @brief Sets the font ID for this view.
     *
     * @param fontId Indicates the font ID composed of font name and size.
     */
    void SetFontId(uint16_t fontId);

    /**
     * @brief Obtains the font ID composed of font name and size.
     *
     * @return Returns the front ID of this view.
     */
    uint8_t GetFontId()
    {
        InitText();
        return inputText_->GetFontId();
    }

    /**
     * @brief Sets the font for this viewview.
     *
     * @param name Indicates the pointer to the font name.
     * @param size Indicates the font size to set.
     */
    void SetFont(const char* name, uint8_t size);

    /**
     * @brief Obtains the width of this text.
     *
     * @return Returns the text width.
     */
    uint16_t GetTextWidth();

    /**
     * @brief Obtains the height of this text.
     *
     * @return Returns the text height.
     */
    uint16_t GetTextHeight();

    void ReMeasure() override;

    /**
     * @brief Insert the text passed from the input method.
     *
     * @param text the text input by the user passed form input method.
     */
    virtual void InsertText(std::string text);

    /**
     * @brief Delete the input text from backward.
     *
     * @param length the length of charactor to delete.
     */
    virtual void DeleteBackward(uint32_t length);

    /**
     * @brief Sets the vaule listener for this view.
     *        The listener is triggered to invoke the callback function when the value changed.
     *
     * @param listener Indicates the listener to set. For details, see {@link OnChangeListener}.
     */
    void SetOnChangeListener(OnChangeListener* onChangeListener)
    {
        onChangeListener_ = onChangeListener;
    }

    /**
     * @brief Obtains the vaule change event listener for the view.
     *
     * @return Returns the vaule change event listener.
     */
    OnChangeListener*& GetOnChangeListener()
    {
        return onChangeListener_;
    }

    /**
     * @brief set the cursor index.
     */
    void SetCursorIndex(uint16_t cursorIndex);

    /**
     * @brief Is it focused or not.
     */
    bool GetIsFocus()
    {
        return isFocused_;
    }

    uint16_t GetCursorIndex();

protected:
    virtual void InitText();
    virtual void UpdateExtraOffsetX(const uint16_t firstVisibleIndex,
                                    const uint16_t lastVisibleIndex);
    virtual uint16_t GetFirstVisibleIndex();
    virtual uint16_t GetLastVisibleIndex();
    virtual uint16_t GetTextLength();
    virtual void UpdateInsertDeletedOffset();
    virtual void UpdateOffsetX();
    virtual uint16_t GetTextWidthByCursorIndex(const uint16_t cursorIndex);

    void SetText(std::string text);
    void InsertTextByCursorIndex(std::string text);
    void UpdateOffsetBySetCursorIndex();
    void UpdateOffsetByInputType();

    Text* inputText_;
    Text* placeholderText_;
    int16_t offsetX_;
    uint16_t cursorIndex_;
    uint16_t deleteTextWidth_;
    uint16_t insertTextWidth_;

    enum UpdateOffsetState : uint8_t {
        UPDATE_OFFSET_UNKNOW = 0,
        UPDATE_OFFSET_INTERFACE,
        UPDATE_OFFSET_PRESS_EVENT,
        UPDATE_OFFSET_DELETE,
        UPDATE_OFFSET_INSERT,
        UPDATE_OFFSET_SET_CURSOR,
        UPDATE_OFFSET_INPUT_TYPE
    };

    UpdateOffsetState offsetState_;
private:
    friend class CursorAnimator;

    void RemeasureForMarquee(int16_t textWidth);
    void UpdateInnerText();
    void CheckValueChange(std::string text);
    void RefreshText();
    void UpdateTextString(std::string text);
    void CalculatedCursorPos(bool drawPlaceholder);
    void DealPressEvents(bool longPressEvent, const Event &event);
    std::string GetInnerText();
    std::string GetInnerPassword();
    void DrawCursor(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, bool drawPlaceholder);

    bool needRefresh_;
    bool useTextColor_;
    bool isFocused_;
    bool drawCursor_;
    uint16_t maxLength_;
    uint16_t placeholderEllipsisIndex_;
    int16_t cursorPosX_;
    ColorType textColor_;
    ColorType placeholderColor_;
    ColorType cursorColor_;
    OnChangeListener* onChangeListener_;
    std::string textStr_;
    std::string passwordStr_;
    Animator* cursorAnimator_;
    InputType inputType_ = InputType::TEXT_TYPE;
};
} // namespace OHOS
#endif // GRAPHIC_LITE_UI_EDIT_TEXT
