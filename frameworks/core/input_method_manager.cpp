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

#include "common/input_method_manager.h"

#include "components/ui_edit_text.h"
#include "gfx_utils/graphic_log.h"
#include "hal_tick.h"

namespace OHOS {
InputMethodManager& InputMethodManager::GetInstance()
{
    static InputMethodManager InputMethodManager;
    return InputMethodManager;
}

void InputMethodManager::ShowInputMethod(UIView* view)
{
    if (view == nullptr) {
        return;
    }

    if (inputMethodListener_ == nullptr) {
        return;
    }

    inputView_ = view;
    UIViewType viewType = view->GetViewType();
    if (viewType == UI_EDIT_TEXT) {
        UIEditText* tmpView = reinterpret_cast<UIEditText*>(view);
        InputMethodParam param;
        param.view = view;
        param.inputType = tmpView->GetInputType();

        if (tmpView->GetText() != nullptr) {
            param.text = tmpView->GetText();
        }

        inputMethodListener_->OnShow(param);
    }
}

void InputMethodManager::HideInputMethod()
{
    if (inputMethodListener_ == nullptr) {
        return;
    }

    inputMethodListener_->OnHide();
}

void InputMethodManager::SetInputMethodListener(InputMethodListener* listener)
{
    inputMethodListener_ = listener;
}

void InputMethodManager::InsertText(std::string text)
{
    if (inputView_ == nullptr) {
        return;
    }

    UIViewType viewType = inputView_->GetViewType();
    if (viewType == UI_EDIT_TEXT) {
        UIEditText* tmpView = reinterpret_cast<UIEditText*>(inputView_);
        tmpView->InsertText(text);
    }
}

void InputMethodManager::SetCursorIndex(uint16_t cursorIndex_)
{
    UIEditText* tmpView = reinterpret_cast<UIEditText*>(inputView_);
    tmpView->SetCursorIndex(cursorIndex_);
}

uint16_t InputMethodManager::GetCursorIndex()
{
    UIEditText* tmpView = reinterpret_cast<UIEditText*>(inputView_);
    return tmpView->GetCursorIndex();
}

void InputMethodManager::DeleteBackward(uint16_t length)
{
    if (inputView_ == nullptr) {
        return;
    }

    UIViewType viewType = inputView_->GetViewType();
    if (viewType == UI_EDIT_TEXT) {
        UIEditText* tmpView = reinterpret_cast<UIEditText*>(inputView_);
        tmpView->DeleteBackward(length);
    }
}

void InputMethodManager::SetInputType(InputType type)
{
    if (inputView_ == nullptr) {
        return;
    }

    UIViewType viewType = inputView_->GetViewType();
    if (viewType == UI_EDIT_TEXT) {
        UIEditText* tmpView = reinterpret_cast<UIEditText*>(inputView_);
        tmpView->SetInputType(type);
    }
}

void InputMethodManager::OnKeyboardShow() {}

void InputMethodManager::OnKeyboardHide() {}
} // namespace OHOS
