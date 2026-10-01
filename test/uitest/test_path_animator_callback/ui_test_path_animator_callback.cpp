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

#include "ui_test_path_animator_callback.h"

#include "common/screen.h"
#include "test_resource_config.h"

namespace OHOS {
namespace {
const int16_t BUTTON_WIDTH = BUTTON_WIDHT3;
const int16_t BUTTON_HEIGHT = BUTTON_HEIGHT3;
const int16_t TARGET_SIZE = 30;
const int16_t TARGET_START_X = 20;
const int16_t TARGET_START_Y = 60;
const int16_t BUTTON_START_X = 240;
const int16_t BUTTON_START_Y = 280;
const int16_t BUTTON_Y_GAP = 60;
const int16_t BUTTON_INDEX_2 = 2;
const int16_t BUTTON_INDEX_3 = 3;
const int16_t ANIM_LINE_DURATION = 2000;
const int16_t ANIM_RECT_DURATION = 3000;
const int16_t ANIM_BEZIER_DURATION = 2000;
const int16_t POSTION_48 = 48;
const int16_t POSTION_100 = 100;
const int16_t POSTION_200 = 200;
const int16_t POSTION_300 = 300;
const int16_t POSTION_228 = 228;
const int16_t POSTION_260 = 260;
const int16_t POSTION_320 = 320;
const int16_t POSTION_380 = 380;
const int16_t POSTION_440 = 440;
} // namespace

void UITestPathAnimatorCallback::SetUp()
{
    if (container_ == nullptr) {
        container_ = new UIScrollView();
        container_->Resize(Screen::GetInstance().GetWidth(), Screen::GetInstance().GetHeight() - BACK_BUTTON_HEIGHT);

        UILabel* label = new UILabel();
        container_->Add(label);
        label->SetPosition(TEXT_DISTANCE_TO_LEFT_SIDE, TEXT_DISTANCE_TO_TOP_SIDE, POSTION_228, POSTION_48);
        label->SetText("路径动画");
        label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, FONT_DEFAULT_SIZE);

        lineTarget_ = new UIView();
        lineTarget_->SetPosition(TARGET_START_X, TARGET_START_Y, TARGET_SIZE, TARGET_SIZE);
        lineTarget_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Red().full);
        container_->Add(lineTarget_);

        rectTarget_ = new UIView();
        rectTarget_->SetPosition(TARGET_START_X, TARGET_START_Y + POSTION_100, TARGET_SIZE, TARGET_SIZE);
        rectTarget_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Green().full);
        container_->Add(rectTarget_);

        bezierTarget_ = new UIView();
        bezierTarget_->SetPosition(TARGET_START_X, TARGET_START_Y + POSTION_200, TARGET_SIZE, TARGET_SIZE);
        bezierTarget_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Blue().full);
        container_->Add(bezierTarget_);

        autoRotateTarget_ = new UIView();
        autoRotateTarget_->SetPosition(TARGET_START_X, TARGET_START_Y + POSTION_300, TARGET_SIZE, TARGET_SIZE);
        autoRotateTarget_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Yellow().full);
        container_->Add(autoRotateTarget_);
    }
}

void UITestPathAnimatorCallback::TearDown()
{
    StopAllAnimators();
    DeleteChildren(container_);
    container_ = nullptr;
    lineTarget_ = nullptr;
    rectTarget_ = nullptr;
    bezierTarget_ = nullptr;
    autoRotateTarget_ = nullptr;
    lineBtn_ = nullptr;
    rectBtn_ = nullptr;
    bezierBtn_ = nullptr;
    autoRotateBtn_ = nullptr;
}

const UIView* UITestPathAnimatorCallback::GetTestView()
{
    UIKitPathAnimatorTestLine001();
    UIKitPathAnimatorTestRect002();
    UIKitPathAnimatorTestBezier003();
    UIKitPathAnimatorTestAutoRotate004();
    return container_;
}

void UITestPathAnimatorCallback::SetUpButton(UILabelButton* btn, const char* title, int16_t x, int16_t y)
{
    if (btn == nullptr) {
        return;
    }
    container_->Add(btn);
    btn->SetPosition(x, y, BUTTON_WIDTH, BUTTON_HEIGHT);
    btn->SetText(title);
    btn->SetFont(DEFAULT_VECTOR_FONT_FILENAME, BUTTON_LABEL_SIZE);
    btn->SetOnClickListener(this);
    btn->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_STYLE_BORDER_RADIUS_VALUE, UIButton::RELEASED);
    btn->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_STYLE_BORDER_RADIUS_VALUE, UIButton::PRESSED);
    btn->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_STYLE_BORDER_RADIUS_VALUE, UIButton::INACTIVE);
    btn->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::RELEASED);
    btn->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::PRESSED);
    btn->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::INACTIVE);
    container_->Invalidate();
}

void UITestPathAnimatorCallback::UIKitPathAnimatorTestLine001()
{
    lineBtn_ = new UILabelButton();
    lineBtn_->SetViewId("path_anim_line_btn");
    SetUpButton(lineBtn_, "直线动画", BUTTON_START_X, BUTTON_START_Y);
}

void UITestPathAnimatorCallback::UIKitPathAnimatorTestRect002()
{
    rectBtn_ = new UILabelButton();
    rectBtn_->SetViewId("path_anim_rect_btn");
    SetUpButton(rectBtn_, "矩形动画", BUTTON_START_X, BUTTON_START_Y + BUTTON_Y_GAP);
}

void UITestPathAnimatorCallback::UIKitPathAnimatorTestBezier003()
{
    bezierBtn_ = new UILabelButton();
    bezierBtn_->SetViewId("path_anim_bezier_btn");
    SetUpButton(bezierBtn_, "贝塞尔动画", BUTTON_START_X, BUTTON_START_Y + BUTTON_INDEX_2 * BUTTON_Y_GAP);
}

void UITestPathAnimatorCallback::UIKitPathAnimatorTestAutoRotate004()
{
    autoRotateBtn_ = new UILabelButton();
    autoRotateBtn_->SetViewId("path_anim_autorotate_btn");
    SetUpButton(autoRotateBtn_, "跟随旋转", BUTTON_START_X, BUTTON_START_Y + BUTTON_INDEX_3 * BUTTON_Y_GAP);
}

bool UITestPathAnimatorCallback::OnClick(UIView& view, const ClickEvent& event)
{
    if (&view == lineBtn_) {
        StartLineAnim();
    } else if (&view == rectBtn_) {
        StartRectAnim();
    } else if (&view == bezierBtn_) {
        StartBezierAnim();
    } else if (&view == autoRotateBtn_) {
        StartAutoRotateAnim();
    }
    container_->Invalidate();
    return true;
}

void UITestPathAnimatorCallback::StartLineAnim()
{
    if (lineAnimator_ != nullptr) {
        lineAnimator_->Stop();
        delete lineAnimator_;
        lineAnimator_ = nullptr;
    }
    lineCallback_.SetDuration(ANIM_LINE_DURATION);
    lineCallback_.SetEasingFunc(EasingEquation::LinearEaseNone);
    lineCallback_.SetPathString("path(\"M 0 0 L 240 0\")");
    lineAnimator_ = new Animator(&lineCallback_, lineTarget_, ANIM_LINE_DURATION, false);
    lineAnimator_->Start();
}

void UITestPathAnimatorCallback::StartRectAnim()
{
    if (rectAnimator_ != nullptr) {
        rectAnimator_->Stop();
        delete rectAnimator_;
        rectAnimator_ = nullptr;
    }
    rectCallback_.SetDuration(ANIM_RECT_DURATION);
    rectCallback_.SetEasingFunc(EasingEquation::LinearEaseNone);
    rectCallback_.SetPathString("path(\"M 0 0 L 240 0 L 240 30 L 0 30 Z\")");
    rectAnimator_ = new Animator(&rectCallback_, rectTarget_, ANIM_RECT_DURATION, true);
    rectAnimator_->Start();
}

void UITestPathAnimatorCallback::StartBezierAnim()
{
    if (bezierAnimator_ != nullptr) {
        bezierAnimator_->Stop();
        delete bezierAnimator_;
        bezierAnimator_ = nullptr;
    }
    bezierCallback_.SetDuration(ANIM_BEZIER_DURATION);
    bezierCallback_.SetEasingFunc(EasingEquation::LinearEaseNone);
    bezierCallback_.SetPathString("path(\"M 0 0 Q 120 -40 240 0\")");
    bezierAnimator_ = new Animator(&bezierCallback_, bezierTarget_, ANIM_BEZIER_DURATION, true);
    bezierAnimator_->Start();
}

void UITestPathAnimatorCallback::StartAutoRotateAnim()
{
    if (autoRotateAnimator_ != nullptr) {
        autoRotateAnimator_->Stop();
        delete autoRotateAnimator_;
        autoRotateAnimator_ = nullptr;
    }
    autoRotateCallback_.SetDuration(ANIM_BEZIER_DURATION);
    autoRotateCallback_.SetEasingFunc(EasingEquation::LinearEaseNone);
    autoRotateCallback_.SetPathString("path(\"M 0 0 Q 120 -40 240 0\")");
    autoRotateCallback_.SetAutoRotate(0); // follow the path tangent (offset-rotate:auto alike)
    autoRotateAnimator_ = new Animator(&autoRotateCallback_, autoRotateTarget_, ANIM_BEZIER_DURATION, true);
    autoRotateAnimator_->Start();
}

void UITestPathAnimatorCallback::StopAllAnimators()
{
    if (lineAnimator_ != nullptr) {
        lineAnimator_->Stop();
        delete lineAnimator_;
        lineAnimator_ = nullptr;
    }
    if (rectAnimator_ != nullptr) {
        rectAnimator_->Stop();
        delete rectAnimator_;
        rectAnimator_ = nullptr;
    }
    if (bezierAnimator_ != nullptr) {
        bezierAnimator_->Stop();
        delete bezierAnimator_;
        bezierAnimator_ = nullptr;
    }
    if (autoRotateAnimator_ != nullptr) {
        autoRotateAnimator_->Stop();
        delete autoRotateAnimator_;
        autoRotateAnimator_ = nullptr;
    }
}
} // namespace OHOS
