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

#ifndef UI_TEST_ARC_LABEL_H
#define UI_TEST_ARC_LABEL_H

#include "components/ui_arc_label.h"
#include "components/ui_label.h"
#include "components/ui_scroll_view.h"
#include "components/ui_label_button.h"
#include "ui_test.h"

namespace OHOS {
class UITestArcLabel : public UITest {
public:
    UITestArcLabel() {}
    ~UITestArcLabel() {}
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;

    /**
     * @brief Test display arc text from 0 to 270 INSIDE
     */
    void UIKitUIArcLabelTestDisplay001();

    /**
     * @brief Test display arc text from 270 to 0 INSIDE
     */
    void UIKitUIArcLabelTestDisplay002();

    /**
     * @brief Test display arc text from 0 to 270 OUTSIDE
     */
    void UIKitUIArcLabelTestDisplay003();

    /**
     * @brief Test display arc text from 270 to 0 OUTSIDE
     */
    void UIKitUIArcLabelTestDisplay004();

    /**
     * @brief Test arc text align LEFT
     */
    void UIKitUIArcLabelTestAlign001();

    /**
     * @brief Test arc text align CENTER
     */
    void UIKitUIArcLabelTestAlign002();

    /**
     * @brief Test arc text align RIGHT
     */
    void UIKitUIArcLabelTestAlign003();

    /**
     * @brief Test display arc text from 0 to 90
     */
    void UIKitUIArcLabelTestIncompatible001();

    /**
     * @brief Test display arc text from 90 to 0
     */
    void UIKitUIArcLabelTestIncompatible002();

    /**
     * @brief Test display arc text from 30 to 260
     */
    void UIKitUIArcLabelTestIncompatible003();

    /**
     * @brief Test display arc text from 260 to 30
     */
    void UIKitUIArcLabelTestIncompatible004();

private:
    UIScrollView* container_ = nullptr;

    void InnerTestTitle(const char* title, int16_t x, int16_t y) const;

    // for direction and orientation
    void TestArcLabelDisplay(const char* title, const int16_t startAngle, const int16_t endAngle,
        const TextOrientation orientation, int16_t x, int16_t y);

    UILabelButton* GetTestUIButton(const char* buttonText, int16_t x, int16_t y, const char* id);
};
} // namespace OHOS
#endif // UI_TEST_ARC_LABEL_H
