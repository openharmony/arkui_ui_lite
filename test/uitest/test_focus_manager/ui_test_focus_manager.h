/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#ifndef UI_TEST_FOCUS_MANAGER_H
#define UI_TEST_FOCUS_MANAGER_H

#include "graphic_config.h"
#if defined(ENABLE_FOCUS_MANAGER) && ENABLE_FOCUS_MANAGER
#include "components/ui_label_button.h"
#include "components/ui_scroll_view.h"
#include "dock/focus_manager.h"
#include "ui_test.h"

namespace OHOS {
constexpr char* UI_TEST_TOWRADS_LEFT = "towards_left";
constexpr char* UI_TEST_TOWRADS_RIGHT = "towards_right";
constexpr char* UI_TEST_TOWRADS_UP = "towards_up";
constexpr char* UI_TEST_TOWRADS_DOWN = "towards_down";
constexpr char* UI_TEST_2_FOCUSABLE = "2_focusable";
constexpr char* UI_TEST_2_UNFOCUSABLE = "2_unfocusable";
constexpr char* UI_TEST_4_INTERCEPT = "4_intercept";
constexpr char* UI_TEST_4_CANCEL_INTERCEPT = "4_cancel_intercept";
constexpr char* UI_TEST_RESET_FOCUS = "reset_focus";
constexpr char* UI_TEST_CLEAR_FOCUS = "clear_focus";

class UITestFocusManager : public UITest {
public:
    UITestFocusManager() {}
    ~UITestFocusManager() {}
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;

    void UIKitFocusManagerTest001();

private:
    void CreateLabels(UIViewGroup* viewGroup, UIViewGroup* viewGroup1);
    void InitListeners(UIViewGroup* viewGroup1, UIViewGroup* viewGroup5, UIView* view2);
    void TearDownListeners001();
    UIView* CreateTestUILabel(UIViewGroup* parent, int16_t x, int16_t y,
        const char* text, bool focusable);
    UIViewGroup* CreateTestUIViewGroup(UIViewGroup* parent, bool focusable, bool interceptFocus);
    UILabelButton* SetUpButton(const char* title, int16_t x, int16_t y, UIViewGroup* viewGroup,
        UIView::OnClickListener* listener, const char* id);
    UIScrollView* container_ = nullptr;
    UIView::OnFocusListener* testOnFocusListener_ = nullptr;
    UIView::OnClickListener* requestFocusByDirectionLeftListener_ = nullptr;
    UIView::OnClickListener* requestFocusByDirectionRightListener_ = nullptr;
    UIView::OnClickListener* requestFocusByDirectionUpListener_ = nullptr;
    UIView::OnClickListener* requestFocusByDirectionDownListener_ = nullptr;
    UIView::OnClickListener* setFocusableViewListener_ = nullptr;
    UIView::OnClickListener* setFocusableViewListener1_ = nullptr;
    UIView::OnClickListener* setGroupInterceptListener_ = nullptr;
    UIView::OnClickListener* setGroupInterceptListener1_ = nullptr;
    UIView::OnClickListener* resetFocusListener_ = nullptr;
    UIView::OnClickListener* clearFocusListener_ = nullptr;
};
} // namespace OHOS
#endif
#endif // UI_TEST_FOCUS_MANAGER_H
