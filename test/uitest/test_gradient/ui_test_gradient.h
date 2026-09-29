/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef UI_TEST_GRADIENT_H
#define UI_TEST_GRADIENT_H

#include "ui_test.h"
#include "components/ui_scroll_view.h"
#include "components/ui_view.h"
#include "gfx_utils/gradient_info.h"

namespace OHOS {
class UITestGradient : public UITest {
public:
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;

private:
    void UIKitGradientTestTwoColor001();
    void UIKitGradientTestThreeColor002();
    void UIKitGradientTestFiveColor003();
    void UIKitGradientTestDirectionRight004();
    void UIKitGradientTestDirectionLeft005();
    void UIKitGradientTestDirectionBottom006();
    void UIKitGradientTestDirectionBottomRight007();
    void UIKitGradientTestButtonMultiColor008();

    UIView* CreateGradientView(int16_t x, int16_t y, int16_t w, int16_t h, GradientInfo* info);
    void AddLabel(const char* text, int16_t y);

    UIScrollView* container_ = nullptr;
};
} // namespace OHOS

#endif // UI_TEST_GRADIENT_H
