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

#ifndef UI_TEST_TRANSITION_H
#define UI_TEST_TRANSITION_H

#include "animator/view_transition.h"
#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_scroll_view.h"
#include "components/ui_view_group.h"
#include "ui_test.h"

namespace OHOS {
/**
 * @brief Test page for all transition types. The upper section covers the six basic types
 *        (fade / slide / scale) with two panels; the lower section covers the shared-element
 *        type with its own stage so the shared element never appears in the basic scenarios.
 */
class UITestTransition : public UITest, public UIView::OnClickListener {
public:
    UITestTransition() {}
    ~UITestTransition() {}
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;

    bool OnClick(UIView& view, const ClickEvent& event) override;

    void UIKitTransitionTestFade001();
    void UIKitTransitionTestSlideLeft002();
    void UIKitTransitionTestSlideRight003();
    void UIKitTransitionTestSlideUp004();
    void UIKitTransitionTestSlideDown005();
    void UIKitTransitionTestScale006();
    void UIKitTransitionTestSharedElement007();

private:
    void SetUpButton(UILabelButton* btn, const char* title, int16_t x, int16_t y);
    void StartTransition(ViewTransition::Type type);
    void StartSharedElementTransition();
    void StopCurrentTransition();

    UIScrollView* container_ = nullptr;
    // section 1: six basic transition types
    UIViewGroup* stage1_ = nullptr;
    UIView* panelA1_ = nullptr;
    UIView* panelB1_ = nullptr;
    // section 2: shared element transition; the star only lives in this stage
    UIViewGroup* stage2_ = nullptr;
    UIView* panelA2_ = nullptr;
    UIView* panelB2_ = nullptr;
    UIView* sharedStar_ = nullptr;

    UILabelButton* fadeBtn_ = nullptr;
    UILabelButton* slideLeftBtn_ = nullptr;
    UILabelButton* slideRightBtn_ = nullptr;
    UILabelButton* slideUpBtn_ = nullptr;
    UILabelButton* slideDownBtn_ = nullptr;
    UILabelButton* scaleBtn_ = nullptr;
    UILabelButton* sharedBtn_ = nullptr;

    ViewTransition* transition_ = nullptr;
};
} // namespace OHOS

#endif // UI_TEST_TRANSITION_H
