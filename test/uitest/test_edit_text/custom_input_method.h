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

#ifndef UI_TEST_CUSTOM_INPUT_METHOD_H
#define UI_TEST_CUSTOM_INPUT_METHOD_H

#include "common/input_method_manager.h"
#include "components/ui_edit_text.h"
#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_scroll_view.h"
#include "layout/flex_layout.h"
#include "ui_test.h"
#include "graphic_timer.h"

namespace OHOS {
constexpr char* UI_TEST_KEY_INPUT = "toggle";
enum class KeyboardType {
    LOW_CASE,
    UPPER_CASE,
    NUMBER,
    SYMBOL
};
class CustomInputMethod : public UIView::OnClickListener,
                          public UIView::OnTouchListener,
                          public UIView::OnLongPressListener,
                          public InputMethodManager::InputMethodListener {
public:
    class UIEditTextEx : public UIEditText {
        // override the view type, so the FocusManager not invoke InputMethod onShow function
        UIViewType GetViewType() const override
        {
            return UI_NUMBER_MAX;
        }
    };

    explicit CustomInputMethod(): timer_(300, TimerMethod, this) {} // 300: time(ms) for add/del one letter
    ~CustomInputMethod() {}

    static void TimerMethod(void* arg)
    {
        CustomInputMethod *customInputMethod  = reinterpret_cast<CustomInputMethod *>(arg);
        customInputMethod->DealLongPressKeyEvent();
    }

    bool OnRelease(UIView& view, const ReleaseEvent& event) override;
    bool OnClick(UIView& view, const ClickEvent& event) override;
    bool OnLongPress(UIView& view, const LongPressEvent& event) override;
    void OnShow(InputMethodManager::InputMethodParam& param) override;
    void OnHide() override;
    void TimerMethod(UIView& view);
    void DealKeyEvent(UIView& view);
    void DealLongPressKeyEvent();

private:
    void SetupView(KeyboardType type);
    void TearDownView();
    UILabelButton* SetupButton(const char* title);
    void SetupKeyboard(KeyboardType type);
    void ChangeKeyboard(KeyboardType type);
    FlexLayout* SetupKeyRow(const char* name, int16_t width, int16_t height);

    UIEditTextEx* editView_ = nullptr;
    UILabelButton* inputTypeBtn_ = nullptr;
    UIViewGroup* container_ = nullptr;
    KeyboardType keyboardType_ = KeyboardType::LOW_CASE;
    GraphicTimer timer_;
    bool longPressed_ = false;
    const char* key_ = nullptr;
};
} // namespace OHOS
#endif // UI_TEST_CUSTOM_INPUT_METHOD_H
