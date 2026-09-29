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

#ifndef UI_TEST_TRANSITION_ANIMATOR_H
#define UI_TEST_TRANSITION_ANIMATOR_H

#include "animator/transition_animator_callback.h"
#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_scroll_view.h"
#include "ui_test.h"

namespace OHOS {
class UITestPropertyTransition : public UITest, public UIView::OnClickListener {
public:
    UITestPropertyTransition() = default;
    ~UITestPropertyTransition() override = default;

    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;
    bool OnClick(UIView& view, const ClickEvent& event) override;

private:
    void AddButton(UILabelButton*& button, const char* text, int16_t x, int16_t y);
    void ResetTarget();
    void StopAnimation();
    void StartAnimation();
    void StartOpacityTransition();
    void StartScaleTransition();
    void StartRotationTransition();
    void StartColorTransition();
    void StartPositionTransition();
    void StartBounceTransition();
    void StartSequentialTransition();
    void StartParallelTransition();

    UIScrollView* container_ = nullptr;
    UIViewGroup* stage_ = nullptr;
    UIView* target_ = nullptr;
    UILabelButton* opacityButton_ = nullptr;
    UILabelButton* scaleButton_ = nullptr;
    UILabelButton* rotationButton_ = nullptr;
    UILabelButton* colorButton_ = nullptr;
    UILabelButton* positionButton_ = nullptr;
    UILabelButton* bounceButton_ = nullptr;
    UILabelButton* sequentialButton_ = nullptr;
    UILabelButton* parallelButton_ = nullptr;
    TransitionAnimatorCallback propertyCallback_;
    TransitionAnimatorCallback nextPropertyCallback_;
    AnimatorCallback* animatorCallback_ = nullptr;
};
} // namespace OHOS

#endif // UI_TEST_TRANSITION_ANIMATOR_H
