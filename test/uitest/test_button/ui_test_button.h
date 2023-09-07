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

#ifndef UI_TEST_BUTTON_H
#define UI_TEST_BUTTON_H

#include "components/ui_button.h"
#include "components/ui_checkbox.h"
#include "components/ui_scroll_view.h"
#include "ui_test.h"

namespace OHOS {
constexpr char* UI_TEST_CHECKBOX_1 = "testCheckBox1";
constexpr char* UI_TEST_CHECKBOX_2 = "testCheckBox2";
constexpr char* UI_TEST_CHECKBOX_3 = "testCheckBox3";
constexpr char* UI_TEST_RADIO_BUTTON_1 = "testRadioButton1";
constexpr char* UI_TEST_RADIO_BUTTON_2 = "testRadioButton2";
constexpr char* UI_TEST_RADIO_BUTTON_3 = "testRadioButton3";
constexpr char* UI_TEST_TOGGLE_BUTTON_1 = "testToggleButton1";
constexpr char* UI_TEST_TOGGLE_BUTTON_2 = "testToggleButton2";
constexpr char* UI_TEST_TOGGLE_BUTTON_3 = "testToggleButton3";
constexpr char* UI_TEST_CHECKBOX_SETIMAGE_1 = "checkBoxSetImage1";
constexpr char* UI_TEST_CHECKBOX_SETIMAGE_2 = "checkBoxSetImage2";
constexpr char* UI_TEST_CHECKBOX_SETIMAGE_3 = "checkBoxSetImage3";
constexpr char* UI_TEST_CHECKBOX_SETIMAGE_4 = "checkBoxSetImage4";
constexpr char* UI_TEST_RADIO_SETIMAGE_1 = "radioButtonSetImage1";
constexpr char* UI_TEST_RADIO_SETIMAGE_2 = "radioButtonSetImage2";
constexpr char* UI_TEST_TOGGLE_SETIMAGE_1 = "toggleButtonSetImage1";
constexpr char* UI_TEST_TOGGLE_SETIMAGE_2 = "toggleButtonSetImage2";
constexpr char* UI_TEST_BUTTON_1 = "enlarge";
constexpr char* UI_TEST_BUTTON_2 = "narrow";
constexpr char* UI_TEST_BUTTON_3 = "goLeft";
constexpr char* UI_TEST_BUTTON_4 = "goRight";
constexpr char* UI_TEST_BUTTON_5 = "goUp";
constexpr char* UI_TEST_BUTTON_6 = "goDown";
constexpr char* UI_TEST_BUTTON_7 = "hide";
constexpr char* UI_TEST_BUTTON_8 = "display";
constexpr char* UI_TEST_BUTTON_9 = "touchable";
constexpr char* UI_TEST_BUTTON_10 = "untouchable";
constexpr char* UI_TEST_BUTTON_11 = "white";
constexpr char* UI_TEST_BUTTON_12 = "red";
constexpr char* UI_TEST_BUTTON_13 = "resetColor";
constexpr char* UI_TEST_BUTTON_14 = "resetAll";
constexpr char* UI_TEST_BUTTON_15 = "animation";
constexpr char* UI_TEST_BUTTON_16 = "noAnimation";
constexpr char* UI_TEST_BUTTON_17 = "buttonWithImage";

class UITestBUTTON : public UITest {
public:
    UITestBUTTON() {}
    ~UITestBUTTON() {}
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;

    /**
     * @brief Test Checkbox Function
     */
    void UIKitCheckBoxTest001();

    /**
     * @brief Test Checkbox's SetImage Function
     */
    void UIKitCheckBoxTest002() const;

    /**
     * @brief Test Radiobutton Function
     */
    void UIKitRadioButtonTest001();

    /**
     * @brief Test Radiobutton's SetImage Function
     */
    void UIKitRadioButtonTest002() const;

    /**
     * @brief Test Togglebutton Function
     */
    void UIKitToggleButtonTest001();

    /**
     * @brief Test Togglebutton's SetImage Function
     */
    void UIKitToggleButtonTest002();

    /**
     * @brief Test button Function
     */
    void UIKitButtonTest001();

private:
    void UIKitButtonTest002(UIScrollView* container, UIButton* button);
    UIViewGroup* CreateButtonGroup(int16_t posX, int16_t posY, int16_t width, int16_t height,
                                    UICheckBox::OnChangeListener** listener,
                                    const char* id = nullptr,
                                    UIViewType type = UI_CHECK_BOX,
                                    const char* name = "aa");
    void TearDownListeners001();
    void TearDownListeners002();
#if defined(DEFAULT_ANIMATION) && DEFAULT_ANIMATION
    void EnableAnimator(UIScrollView* container, UIButton* button);
#endif
    void InitButtonListeners001(UIButton* button);
    void InitButtonListeners002(UIButton* button);
    static constexpr int16_t CHANGE_SIZE = 10;
    UIScrollView* container_ = nullptr;

    UICheckBox::OnChangeListener* checkBoxChangeListener_ = nullptr;
    UICheckBox::OnChangeListener* checkBoxChangeListener1_ = nullptr;
    UICheckBox::OnChangeListener* checkBoxChangeListener2_ = nullptr;
    UICheckBox::OnChangeListener* radioChangeListener_ = nullptr;
    UICheckBox::OnChangeListener* radioChangeListener1_ = nullptr;
    UICheckBox::OnChangeListener* radioChangeListener2_ = nullptr;
    UICheckBox::OnChangeListener* toggleChangeListener_ = nullptr;
    UICheckBox::OnChangeListener* toggleChangeListener1_ = nullptr;
    UICheckBox::OnChangeListener* toggleChangeListener2_ = nullptr;

    UIView::OnClickListener* clickEnableVisiableListener_ = nullptr;
    UIView::OnClickListener* clickDisableVisiableListener_ = nullptr;
    UIView::OnClickListener* clickEnableTouchableListener_ = nullptr;
    UIView::OnClickListener* clickDisableTouchableListener_ = nullptr;
    UIView::OnClickListener* clickColorToWhiteListener_ = nullptr;
    UIView::OnClickListener* clickColorToRedListener_ = nullptr;
    UIView::OnClickListener* clickRevetColorListener_ = nullptr;
    UIView::OnClickListener* clickRevetToOriginListener_ = nullptr;
    UIView::OnClickListener* clickBigListener_ = nullptr;
    UIView::OnClickListener* clickLeftListener_ = nullptr;
    UIView::OnClickListener* clickRightListener_ = nullptr;
    UIView::OnClickListener* clickUpListener_ = nullptr;
    UIView::OnClickListener* clickDownListener_ = nullptr;
    UIView::OnClickListener* clickSmallListener_ = nullptr;
    UIView::OnClickListener* enableAnimationListener_ = nullptr;
    UIView::OnClickListener* disableAnimationListener_ = nullptr;
};
} // namespace OHOS
#endif // UI_TEST_BUTTON_H
