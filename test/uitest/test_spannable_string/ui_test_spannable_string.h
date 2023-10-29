/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef UI_TEST_SPANNABLE_STRING_H
#define UI_TEST_SPANNABLE_STRING_H

#include "components/ui_scroll_view.h"
#include "font/ui_font.h"
#include "ui_test.h"

namespace OHOS {
class UITestSpannableString : public UITest {
public:
    UITestSpannableString() {}
    ~UITestSpannableString() {}
    void SetUp() override;
    void TearDown() override;
    const UIView *GetTestView() override;

    /* *
     * @brief Test singleline Chinese display
     */
    void UIKitSpannableStringTestNormal001();

    /* *
     * @brief Test textStyle display
     */
    void UIKitSpannableStringTestTextStyle001();

    /* *
     * @brief Test textStyle display
     */
    void UIKitSpannableStringTestFontSize001();

    /* *
     * @brief Show an other way
     */
    void UIKitSpannableStringTestFontSize002();

    /* *
     * @brief Verify two fontId can be find.
     */
    void UIKitSpannableStringTestFontId001();

    /* *
     * @brief Test Set FontId
     */
    void UIKitSpannableStringTestFontId002();

private:
    UIScrollView *container_ = nullptr;
    void InnerTestTitle(const char *title);
};
}


#endif
