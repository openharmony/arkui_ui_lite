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

#ifndef UI_TEST_PATH_ANIMATOR_CALLBACK_H
#define UI_TEST_PATH_ANIMATOR_CALLBACK_H

#include "animator/path_animator_callback.h"
#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_scroll_view.h"
#include "ui_test.h"

namespace OHOS {
class UITestPathAnimatorCallback : public UITest, public UIView::OnClickListener {
public:
    UITestPathAnimatorCallback() {}
    ~UITestPathAnimatorCallback() {}
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;

    bool OnClick(UIView& view, const ClickEvent& event) override;

    void UIKitPathAnimatorTestLine001();
    void UIKitPathAnimatorTestRect002();
    void UIKitPathAnimatorTestBezier003();
    void UIKitPathAnimatorTestAutoRotate004();

private:
    void SetUpButton(UILabelButton* btn, const char* title, int16_t x, int16_t y);
    void StartLineAnim();
    void StartRectAnim();
    void StartBezierAnim();
    void StartAutoRotateAnim();
    void StopAllAnimators();

    UIScrollView* container_ = nullptr;
    UIView* lineTarget_ = nullptr;
    UIView* rectTarget_ = nullptr;
    UIView* bezierTarget_ = nullptr;
    UIView* autoRotateTarget_ = nullptr;

    UILabelButton* lineBtn_ = nullptr;
    UILabelButton* rectBtn_ = nullptr;
    UILabelButton* bezierBtn_ = nullptr;
    UILabelButton* autoRotateBtn_ = nullptr;

    /* path-effect callbacks (value members, configured then injected into Animators);
       the timeline is owned by the Animator instances below (one-way drive) */
    PathAnimatorCallback lineCallback_;
    PathAnimatorCallback rectCallback_;
    PathAnimatorCallback bezierCallback_;
    PathAnimatorCallback autoRotateCallback_;

    Animator* lineAnimator_ = nullptr;
    Animator* rectAnimator_ = nullptr;
    Animator* bezierAnimator_ = nullptr;
    Animator* autoRotateAnimator_ = nullptr;
};
} // namespace OHOS

#endif // UI_TEST_PATH_ANIMATOR_CALLBACK_H
