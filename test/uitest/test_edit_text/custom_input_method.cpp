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

#include "custom_input_method.h"

#include <string>

#include "common/input_method_manager.h"
#include "components/root_view.h"
#include "components/ui_checkbox.h"
#include "components/ui_edit_text.h"
#include "components/ui_label.h"
#include "font/ui_font.h"
#include "ui_test.h"
#include "graphic_timer.h"

namespace OHOS {
namespace {
struct KeyInfo {
    uint8_t keyCount;
    uint8_t row1Count;
    uint8_t row2Count;
    uint8_t row3Count;
    uint8_t row4Count;
    const char* keyName[32]; // 32: max count of keys
};

const KeyInfo LOW_CASE_KEY_IOFO = {
    32, 10, 19, 28, 32,
    {
        "q",     "w",     "e", "r",     "t", "y", "u", "i", "o",   "p", // 10
        "a",     "s",     "d", "f",     "g", "h", "j", "k", "l",        // 9
        "shift", "z",     "x", "c",     "v", "b", "n", "m", "del",      // 9
        "123",   "space", ".", "return"                                 // 4
    }
};

const KeyInfo UPPER_CASE_KEY_IOFO = {
    32, 10, 19, 28, 32,
    {
        "Q",     "W",     "E", "R",     "T", "Y", "U", "I", "O",   "P", // 10
        "A",     "S",     "D", "F",     "G", "H", "J", "K", "L",        // 9
        "shift", "Z",     "X", "C",     "V", "B", "N", "M", "del",      // 9
        "123",   "space", ".", "return"                                 // 4
    }
};

const KeyInfo NUM_KEY_IOFO = {
    31, 10, 20, 27, 31,
    {
        "1",   "2",     "3", "4",     "5", "6", "7",   "8", "9", "0",  // 10
        "-",   "/",     ":", ";",     "(", ")", "$",   "&", "@", "\"", // 10
        "#+=", ".",     ",", "?",     "!", "'", "del",                 // 7
        "ABC", "space", ".", "return"                                  // 4
    }
};

const KeyInfo SYMBOL_KEY_IOFO = {
    31, 10, 20, 27, 31,
    {
        "[",   "]",     "{", "}",     "#", "%", "^",   "*", "+", "=", // 10
        "_",   "\\",    "|", "~",     "<", ">", "￥",  "€", "£", "•", // 10
        "123", ".",     ",", "?",     "!", "'", "del",                // 7
        "ABC", "space", ".", "return"                                 // 4
    }
};

const int16_t KEYBOARD_WIDTH = 580;
const int16_t KEYBOARD_HEIGHT = 320;
const int16_t KEY_WIDTH = 50;
const int16_t KEY_HEIGHT = 65;
const int16_t FUNC_KEY_WIDTH = 65;
const int16_t SPACE_KEY_WIDTH = 260;
const int16_t RETURN_KEY_WIDTH = 120;
const int16_t KEY_BORDER_RADIUS = 10;
const int16_t KEY_MARGIN_LEFT = 7;

KeyInfo GetKeyInfo(KeyboardType type)
{
    KeyInfo keyInfo;
    switch (type) {
        case KeyboardType::LOW_CASE:
            keyInfo = LOW_CASE_KEY_IOFO;
            break;
        case KeyboardType::UPPER_CASE:
            keyInfo = UPPER_CASE_KEY_IOFO;
            break;
        case KeyboardType::NUMBER:
            keyInfo = NUM_KEY_IOFO;
            break;
        case KeyboardType::SYMBOL:
            keyInfo = SYMBOL_KEY_IOFO;
            break;
        default:
            keyInfo = LOW_CASE_KEY_IOFO;
            break;
    }

    return keyInfo;
}
} // namespace

void CustomInputMethod::OnShow(InputMethodManager::InputMethodParam& param)
{
    bool needFocus = editView_ == nullptr ? true : false;
    SetupView(keyboardType_);

    // update value
    editView_->SetText(param.text.c_str());
    editView_->SetInputType(param.inputType);
    if (param.view != nullptr) {
        UIEditText* paramView = reinterpret_cast<UIEditText*>(param.view);
        editView_->SetPlaceholder(paramView->GetPlaceholder());
        editView_->SetPlaceholderColor(paramView->GetPlaceholderColor());
        editView_->SetCursorColor(paramView->GetCursorColor());
        editView_->SetCursorIndex(paramView->GetCursorIndex());
        editView_->SetTextColor(paramView->GetTextColor());
        editView_->SetMaxLength(paramView->GetMaxLength());
    }
    if (needFocus) {
        editView_->RequestFocus();
    }

    container_->Invalidate();

    // keyboard show callback
    InputMethodManager::GetInstance().OnKeyboardShow();
}

void CustomInputMethod::OnHide()
{
    editView_->Blur();
    TearDownView();

    // keyboard show callback
    InputMethodManager::GetInstance().OnKeyboardHide();
}

void CustomInputMethod::SetupView(KeyboardType type)
{
    if (container_ == nullptr) {
        container_ = new UIViewGroup();
        container_->Resize(KEYBOARD_WIDTH, 650); // 650: height
        container_->SetPosition(350, 100);       // 350: position x, 100: position y
        container_->SetStyle(STYLE_BORDER_COLOR, Color::White().full);
        // add on rootview
        RootView::GetInstance()->Add(container_);
    }

    if (editView_ != nullptr) {
        container_->Remove(editView_);
        delete editView_;
    }
    editView_ = new UIEditTextEx();
    container_->Add(editView_);
    editView_->Resize(250, 40); // 250: width, 40: height
    editView_->SetPosition(0, 0);
    editView_->SetViewId("Input_edit_text_view");

    if (inputTypeBtn_ == nullptr) {
        inputTypeBtn_ = new UILabelButton();
        container_->Add(inputTypeBtn_);
        inputTypeBtn_->Resize(100, 40); // 100: width, 40: height
        inputTypeBtn_->SetText("toggle");
        inputTypeBtn_->SetViewId(UI_TEST_KEY_INPUT);
        inputTypeBtn_->LayoutRightToSibling("Input_edit_text_view", 10); // 10: offset
        inputTypeBtn_->SetOnClickListener(this);
        inputTypeBtn_->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE,
                                        UIButton::RELEASED);
        inputTypeBtn_->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::PRESSED);
    }

    SetupKeyboard(type);
}

void CustomInputMethod::SetupKeyboard(KeyboardType type)
{
    UIViewGroup* keyboard = new UIViewGroup();
    int16_t rowHeight = KEYBOARD_HEIGHT / 4; // 4: row number
    keyboard->Resize(KEYBOARD_WIDTH, KEYBOARD_HEIGHT);
    keyboard->SetPosition(0, 45); // 45: position y
    keyboard->SetStyle(STYLE_BACKGROUND_COLOR, Color::Gray().full);
    keyboard->SetViewId("keyboard");
    container_->Add(keyboard);
    FlexLayout* row1 = SetupKeyRow("row1", KEYBOARD_WIDTH, rowHeight);
    row1->SetPosition(0, 0);
    keyboard->Add(row1);

    FlexLayout* row2 = SetupKeyRow("row2", KEYBOARD_WIDTH, rowHeight);
    keyboard->Add(row2);
    row2->LayoutBottomToSibling("row1");

    FlexLayout* row3 = SetupKeyRow("row3", KEYBOARD_WIDTH, rowHeight);
    keyboard->Add(row3);
    row3->LayoutBottomToSibling("row2");

    FlexLayout* row4 = SetupKeyRow("row4", KEYBOARD_WIDTH, rowHeight);
    keyboard->Add(row4);
    row4->LayoutBottomToSibling("row3");

    KeyInfo keyInfo = GetKeyInfo(type);
    for (int16_t i = 0; i < keyInfo.keyCount; i++) {
        UILabelButton* key = SetupButton(keyInfo.keyName[i]);
        if (i < keyInfo.row1Count) {
            row1->Add(key);
        } else if (i < keyInfo.row2Count) {
            row2->Add(key);
        } else if (i < keyInfo.row3Count) {
            row3->Add(key);
        } else if (i < keyInfo.row4Count) {
            row4->Add(key);
        }
    }

    row1->LayoutChildren();
    row2->LayoutChildren();
    row3->LayoutChildren();
    row4->LayoutChildren();
    keyboard->Invalidate();
}

FlexLayout* CustomInputMethod::SetupKeyRow(const char* name, int16_t width, int16_t height)
{
    FlexLayout* row = new FlexLayout();
    row->Resize(width, height);
    row->SetViewId(name);
    row->SetMajorAxisAlign(ALIGN_CENTER);
    row->SetStyle(STYLE_BACKGROUND_OPA, OPA_TRANSPARENT);
    return row;
}

UILabelButton* CustomInputMethod::SetupButton(const char* title)
{
    UILabelButton* keyBtn = new UILabelButton();
    keyBtn->SetText(title);
    keyBtn->SetViewId(title);
    keyBtn->SetFont(DEFAULT_VECTOR_FONT_FILENAME, BUTTON_LABEL_SIZE);
    keyBtn->SetStyle(STYLE_MARGIN_LEFT, KEY_MARGIN_LEFT);
    keyBtn->SetOnClickListener(this);
    int16_t radius = KEY_BORDER_RADIUS;
    keyBtn->SetStyleForState(STYLE_BORDER_RADIUS, radius, UIButton::RELEASED);
    keyBtn->SetStyleForState(STYLE_BORDER_RADIUS, radius, UIButton::PRESSED);
    keyBtn->SetStyleForState(STYLE_BORDER_RADIUS, radius, UIButton::INACTIVE);
    keyBtn->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::RELEASED);
    keyBtn->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::PRESSED);
    keyBtn->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::INACTIVE);
    keyBtn->Resize(KEY_WIDTH, KEY_HEIGHT);

    keyBtn->SetOnLongPressListener(this);
    keyBtn->SetOnTouchListener(this);

    if ((strcmp(title, "shift") == 0) || (strcmp(title, "del") == 0) || (strcmp(title, "123") == 0) ||
        (strcmp(title, "ABC") == 0) || (strcmp(title, "#+=") == 0)) {
        keyBtn->Resize(FUNC_KEY_WIDTH, KEY_HEIGHT);
    } else if (strcmp(title, "space") == 0) {
        keyBtn->Resize(SPACE_KEY_WIDTH, KEY_HEIGHT);
    } else if (strcmp(title, "return") == 0) {
        keyBtn->Resize(RETURN_KEY_WIDTH, KEY_HEIGHT);
    }
    return keyBtn;
}

void CustomInputMethod::TearDownView()
{
    UITest::DeleteChildren(container_);
    container_ = nullptr;
    editView_ = nullptr;
}

bool CustomInputMethod::OnClick(UIView& view, const ClickEvent& event)
{
    if (inputTypeBtn_ == &view) {
        InputType type = editView_->GetInputType();
        if (type == InputType::TEXT_TYPE) {
            editView_->SetInputType(InputType::PASSWORD_TYPE);
            InputMethodManager::GetInstance().SetInputType(InputType::PASSWORD_TYPE);
        } else {
            editView_->SetInputType(InputType::TEXT_TYPE);
            InputMethodManager::GetInstance().SetInputType(InputType::TEXT_TYPE);
        }
    } else {
        DealKeyEvent(view);
    }

    return true;
}

void CustomInputMethod::DealKeyEvent(UIView& view)
{
    const char* key = reinterpret_cast<UILabelButton*>(&view)->GetText();
    if (key == nullptr) {
        return;
    }

    if (editView_ == nullptr) {
        return;
    }

    if (editView_->GetIsFocus()) {
        InputMethodManager::GetInstance().SetCursorIndex(editView_->GetCursorIndex());
    } else {
        editView_->SetCursorIndex(InputMethodManager::GetInstance().GetCursorIndex());
    }

    if (strcmp(key, "shift") == 0) {
        if (keyboardType_ == KeyboardType::LOW_CASE) {
            keyboardType_ = KeyboardType::UPPER_CASE;
        } else {
            keyboardType_ = KeyboardType::LOW_CASE;
        }
        ChangeKeyboard(keyboardType_);
    } else if (strcmp(key, "123") == 0) {
        keyboardType_ = KeyboardType::NUMBER;
        ChangeKeyboard(keyboardType_);
    } else if (strcmp(key, "ABC") == 0) {
        keyboardType_ = KeyboardType::LOW_CASE;
        ChangeKeyboard(keyboardType_);
    } else if (strcmp(key, "#+=") == 0) {
        keyboardType_ = KeyboardType::SYMBOL;
        ChangeKeyboard(keyboardType_);
    } else if (strcmp(key, "del") == 0) {
        InputMethodManager::GetInstance().DeleteBackward(1);
        editView_->DeleteBackward(1);
    } else if (strcmp(key, "space") == 0) {
        InputMethodManager::GetInstance().InsertText(" ");
        editView_->InsertText(" ");
    } else if (strcmp(key, "return") == 0) {
        // do nothing
    } else {
        InputMethodManager::GetInstance().InsertText(key);
        editView_->InsertText(key);
    }
}

bool CustomInputMethod::OnLongPress(UIView &view, const LongPressEvent &event)
{
    longPressed_ = true;
    if (inputTypeBtn_ == &view) {
        InputType type = editView_->GetInputType();
        if (type == InputType::TEXT_TYPE) {
            editView_->SetInputType(InputType::PASSWORD_TYPE);
            InputMethodManager::GetInstance().SetInputType(InputType::PASSWORD_TYPE);
        } else {
            editView_->SetInputType(InputType::TEXT_TYPE);
            InputMethodManager::GetInstance().SetInputType(InputType::TEXT_TYPE);
        }
    } else {
        key_ = reinterpret_cast<UILabelButton*>(&view)->GetText();
        timer_.Start();
    }
    return true;
}

bool CustomInputMethod::OnRelease(UIView& view, const ReleaseEvent& event)
{
    if (longPressed_) {
        longPressed_ = false;
        timer_.Stop();
    }
    return true;
}

void CustomInputMethod::DealLongPressKeyEvent()
{
    if (key_ == nullptr) {
        return;
    }

    if (strcmp(key_, "del") == 0) {
        InputMethodManager::GetInstance().DeleteBackward(1);
        editView_->DeleteBackward(1);
    } else if (strcmp(key_, "space") == 0) {
        InputMethodManager::GetInstance().InsertText(" ");
        editView_->InsertText(" ");
    } else if (strcmp(key_, "return") == 0 || strcmp(key_, "123") == 0 || strcmp(key_, "ABC") == 0
               || strcmp(key_, "#+=") == 0 || strcmp(key_, "shift") == 0) {
        // do nothing
    } else {
        InputMethodManager::GetInstance().InsertText(key_);
        editView_->InsertText(key_);
    }

    if (!longPressed_) {
        timer_.Stop();
    } else {
        timer_.Start();
    }
}

void CustomInputMethod::ChangeKeyboard(KeyboardType type)
{
    UIView* keyboardTmp = container_->GetChildById("keyboard");
    SetupKeyboard(type);
    container_->Remove(keyboardTmp);
    UITest::DeleteChildren(keyboardTmp);
}
} // namespace OHOS
