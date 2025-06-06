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

#ifndef UI_TEST_DIGITAL_CLOCK_H
#define UI_TEST_DIGITAL_CLOCK_H

#include "components/ui_scroll_view.h"
#include "font/ui_font.h"
#include "ui_test.h"

namespace OHOS {
class UITestDigitalClock : public UITest {
public:
    UITestDigitalClock() {}
    ~UITestDigitalClock() {}
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;

    void UIKitUIDigitalClockTestDisplay001();
    void UIKitUIDigitalClockTestDisplay002();
    void UIKitUIDigitalClockTestSetColor();
    void UIKitUIDigitalClockTestDisplayMode001();
    void UIKitUIDigitalClockTestDisplayMode002();
    void UIKitUIDigitalClockTestDisplayMode003();
    void UIKitUIDigitalClockTestDisplayMode004();
    void UIKitUIDigitalClockTestDisplayLeadingZero();
    void UIKitUIDigitalClockTestSetOpacity();

private:
    UIScrollView* container_ = nullptr;
    void InnerTestTitle(const char* title);
};
} // namespace OHOS
#endif // UI_TEST_DIGITAL_CLOCK_H
