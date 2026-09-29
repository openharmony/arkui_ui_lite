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

#include "ui_test_transition.h"
#include "common/screen.h"
#include "test_resource_config.h"

namespace OHOS {
namespace {
const int16_t BUTTON_WIDTH = BUTTON_WIDHT3;
const int16_t BUTTON_HEIGHT = 44;
const int16_t BUTTON_Y_GAP = 8;
const int16_t STAGE_X = 16;
const int16_t STAGE1_Y = 56;
const int16_t STAGE_W = 300;
const int16_t STAGE_H = 170;
const int16_t BUTTON_X = 324;
const int16_t SECTION2_TITLE_Y = 240;
const int16_t SECTION2_TITLE_H = 29;
const int16_t STAGE2_Y = 280;
const int16_t STAR_SMALL_X = 30;
const int16_t STAR_SMALL_Y = 30;
const int16_t STAR_SMALL_SIZE = 40;
// the engine scales around the view center, so the end rect position is the top-left of the
// original-size box: stage center (150, 85) minus half of the original size (40 / 2)
const int16_t STAR_BIG_X = 130;
const int16_t STAR_BIG_Y = 65;
const int16_t STAR_BIG_SIZE = 80;
const int32_t ANIM_DURATION = 600;
// 0-based row offsets of the buttons in the right column (fade is row 0)
const uint8_t BUTTON_ROW_2 = 1;
const uint8_t BUTTON_ROW_3 = 2;
const uint8_t BUTTON_ROW_4 = 3;
const uint8_t BUTTON_ROW_5 = 4;
const uint8_t BUTTON_ROW_6 = 5;
const uint8_t BUTTON_ROW_7 = 6;
const uint8_t SIZE_HALF = 2;

void SetupStagePanel(UIViewGroup* stage, UIView*& panel, uint32_t color, bool visible, const char* labelText)
{
    UIViewGroup* group = new UIViewGroup();
    group->SetPosition(0, 0, STAGE_W, STAGE_H);
    group->SetStyle(STYLE_BACKGROUND_COLOR, color);
    // Keep the panel rectangular. Rounded corners introduce anti-aliased transparent
    // pixels that show the dark stage background through semi-transparent panels
    // during cross-fade/scale, creating a dark outline/rectangle artifact.
    group->SetVisible(visible);

    constexpr int16_t LABEL_SIZE = 60;
    UILabel* label = new UILabel();
    label->SetPosition((STAGE_W - LABEL_SIZE) / SIZE_HALF, (STAGE_H - LABEL_SIZE) / SIZE_HALF, LABEL_SIZE, LABEL_SIZE);
    label->SetText(labelText);
    label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, FONT_DEFAULT_SIZE * SIZE_HALF);
    label->SetStyle(STYLE_TEXT_COLOR, Color::White().full);
    label->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER,
                    UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER);
    group->Add(label);

    panel = group;
    stage->Add(panel);
}

UILabel* SetupTitle(UIViewGroup* container, const char* text, int16_t y, int16_t height)
{
    UILabel* title = new UILabel();
    container->Add(title);
    title->SetPosition(TEXT_DISTANCE_TO_LEFT_SIDE, y, 264, height); // 264: label width
    title->SetText(text);
    title->SetFont(DEFAULT_VECTOR_FONT_FILENAME, FONT_DEFAULT_SIZE);
    return title;
}
} // namespace

void UITestTransition::SetUp()
{
    if (container_ == nullptr) {
        container_ = new UIScrollView();
        container_->Resize(Screen::GetInstance().GetWidth(), Screen::GetInstance().GetHeight() - BACK_BUTTON_HEIGHT);

        SetupTitle(container_, "转场动画", TEXT_DISTANCE_TO_TOP_SIDE, 48); // 48: title height

        // section 1: six basic transition types
        stage1_ = new UIViewGroup();
        stage1_->SetPosition(STAGE_X, STAGE1_Y, STAGE_W, STAGE_H);
        // Match the JS basic_trans demo: the area behind the panels is the black card,
        // not the dark-blue container background.
        stage1_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Black().full);
        container_->Add(stage1_);

        SetupStagePanel(stage1_, panelA1_, Color::GetColorFromRGB(0x29, 0x79, 0xff).full, true, "A");
        SetupStagePanel(stage1_, panelB1_, Color::GetColorFromRGB(0xff, 0x91, 0x00).full, false, "B");

        // section 2: shared element transition
        SetupTitle(container_, "共享元素转场", SECTION2_TITLE_Y, SECTION2_TITLE_H);
        stage2_ = new UIViewGroup();
        stage2_->SetPosition(STAGE_X, STAGE2_Y, STAGE_W, STAGE_H);
        stage2_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Black().full);
        container_->Add(stage2_);

        SetupStagePanel(stage2_, panelA2_, Color::GetColorFromRGB(0xff, 0x55, 0x55).full, true, "A");
        SetupStagePanel(stage2_, panelB2_, Color::GetColorFromRGB(0x55, 0xff, 0x77).full, false, "B");

        sharedStar_ = new UIView();
        sharedStar_->SetPosition(STAR_SMALL_X, STAR_SMALL_Y, STAR_SMALL_SIZE, STAR_SMALL_SIZE);
        sharedStar_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Yellow().full);
        sharedStar_->SetStyle(STYLE_BORDER_RADIUS, STAR_SMALL_SIZE / 2); // 2: half size makes a circle
        stage2_->Add(sharedStar_);
    }
}

void UITestTransition::TearDown()
{
    StopCurrentTransition();
    DeleteChildren(container_);
    container_ = nullptr;
    stage1_ = nullptr;
    panelA1_ = nullptr;
    panelB1_ = nullptr;
    stage2_ = nullptr;
    panelA2_ = nullptr;
    panelB2_ = nullptr;
    sharedStar_ = nullptr;
    fadeBtn_ = nullptr;
    slideLeftBtn_ = nullptr;
    slideRightBtn_ = nullptr;
    slideUpBtn_ = nullptr;
    slideDownBtn_ = nullptr;
    scaleBtn_ = nullptr;
    sharedBtn_ = nullptr;
}

const UIView* UITestTransition::GetTestView()
{
    // GetTestView() may be invoked multiple times by the test framework;
    // create the buttons only once to avoid duplicate widgets and leaks.
    if (fadeBtn_ != nullptr) {
        return container_;
    }

    UIKitTransitionTestFade001();
    UIKitTransitionTestSlideLeft002();
    UIKitTransitionTestSlideRight003();
    UIKitTransitionTestSlideUp004();
    UIKitTransitionTestSlideDown005();
    UIKitTransitionTestScale006();
    UIKitTransitionTestSharedElement007();
    return container_;
}

void UITestTransition::SetUpButton(UILabelButton* btn, const char* title, int16_t x, int16_t y)
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

void UITestTransition::UIKitTransitionTestFade001()
{
    fadeBtn_ = new UILabelButton();
    fadeBtn_->SetViewId("transition_fade_btn");
    SetUpButton(fadeBtn_, "淡入淡出", BUTTON_X, STAGE1_Y);
}

void UITestTransition::UIKitTransitionTestSlideLeft002()
{
    slideLeftBtn_ = new UILabelButton();
    slideLeftBtn_->SetViewId("transition_slide_left_btn");
    SetUpButton(slideLeftBtn_, "左滑", BUTTON_X, STAGE1_Y + BUTTON_ROW_2 * (BUTTON_HEIGHT + BUTTON_Y_GAP));
}

void UITestTransition::UIKitTransitionTestSlideRight003()
{
    slideRightBtn_ = new UILabelButton();
    slideRightBtn_->SetViewId("transition_slide_right_btn");
    SetUpButton(slideRightBtn_, "右滑", BUTTON_X, STAGE1_Y + BUTTON_ROW_3 * (BUTTON_HEIGHT + BUTTON_Y_GAP));
}

void UITestTransition::UIKitTransitionTestSlideUp004()
{
    slideUpBtn_ = new UILabelButton();
    slideUpBtn_->SetViewId("transition_slide_up_btn");
    SetUpButton(slideUpBtn_, "上滑", BUTTON_X, STAGE1_Y + BUTTON_ROW_4 * (BUTTON_HEIGHT + BUTTON_Y_GAP));
}

void UITestTransition::UIKitTransitionTestSlideDown005()
{
    slideDownBtn_ = new UILabelButton();
    slideDownBtn_->SetViewId("transition_slide_down_btn");
    SetUpButton(slideDownBtn_, "下滑", BUTTON_X, STAGE1_Y + BUTTON_ROW_5 * (BUTTON_HEIGHT + BUTTON_Y_GAP));
}

void UITestTransition::UIKitTransitionTestScale006()
{
    scaleBtn_ = new UILabelButton();
    scaleBtn_->SetViewId("transition_scale_btn");
    SetUpButton(scaleBtn_, "缩放", BUTTON_X, STAGE1_Y + BUTTON_ROW_6 * (BUTTON_HEIGHT + BUTTON_Y_GAP));
}

void UITestTransition::UIKitTransitionTestSharedElement007()
{
    sharedBtn_ = new UILabelButton();
    sharedBtn_->SetViewId("transition_shared_element_btn");
    SetUpButton(sharedBtn_, "共享元素", BUTTON_X, STAGE1_Y + BUTTON_ROW_7 * (BUTTON_HEIGHT + BUTTON_Y_GAP));
}

bool UITestTransition::OnClick(UIView& view, const ClickEvent& event)
{
    // Ignore clicks while a transition is running to avoid creating overlapping
    // animations and potential use-after-free/corruption from rapid re-entry.
    if (transition_ != nullptr && transition_->GetState() != Animator::STOP) {
        return true;
    }

    if (&view == fadeBtn_) {
        StartTransition(ViewTransition::TRANSITION_FADE);
    } else if (&view == scaleBtn_) {
        StartTransition(ViewTransition::TRANSITION_SCALE);
    } else if (&view == sharedBtn_) {
        StartSharedElementTransition();
    } else {
        ViewTransition::Type type = ViewTransition::TRANSITION_FADE;
        if (&view == slideLeftBtn_) {
            type = ViewTransition::TRANSITION_SLIDE_LEFT;
        } else if (&view == slideRightBtn_) {
            type = ViewTransition::TRANSITION_SLIDE_RIGHT;
        } else if (&view == slideUpBtn_) {
            type = ViewTransition::TRANSITION_SLIDE_UP;
        } else if (&view == slideDownBtn_) {
            type = ViewTransition::TRANSITION_SLIDE_DOWN;
        } else {
            return true;
        }
        StartTransition(type);
    }

    container_->Invalidate();
    return true;
}

void UITestTransition::StartTransition(ViewTransition::Type type)
{
    StopCurrentTransition();
    transition_ = new ViewTransition();
    if (transition_ == nullptr) {
        return;
    }
    // toggle the two panels: the currently visible one is the outgoing view. Reading the
    // visibility stays correct even when a previous transition was interrupted mid-run,
    // because Stop() restores both panels to their snapshot states first.
    bool panelAVisible = panelA1_->IsVisible();
    transition_->SetOutgoingView(panelAVisible ? panelA1_ : panelB1_);
    transition_->SetIncomingView(panelAVisible ? panelB1_ : panelA1_);
    transition_->SetType(type);
    transition_->SetDuration(ANIM_DURATION);
    transition_->SetEasingFunc(EasingEquation::CubicEaseInOut);
    transition_->Start();
}

void UITestTransition::StartSharedElementTransition()
{
    StopCurrentTransition();
    transition_ = new ViewTransition();
    if (transition_ == nullptr) {
        return;
    }
    bool panelAVisible = panelA2_->IsVisible();
    transition_->SetOutgoingView(panelAVisible ? panelA2_ : panelB2_);
    transition_->SetIncomingView(panelAVisible ? panelB2_ : panelA2_);
    transition_->SetSharedElement(sharedStar_);
    if (panelAVisible) {
        transition_->SetSharedStartRect(STAR_SMALL_X, STAR_SMALL_Y, STAR_SMALL_SIZE, STAR_SMALL_SIZE);
        transition_->SetSharedEndRect(STAR_BIG_X, STAR_BIG_Y, STAR_BIG_SIZE, STAR_BIG_SIZE);
    } else {
        transition_->SetSharedStartRect(STAR_BIG_X, STAR_BIG_Y, STAR_BIG_SIZE, STAR_BIG_SIZE);
        transition_->SetSharedEndRect(STAR_SMALL_X, STAR_SMALL_Y, STAR_SMALL_SIZE, STAR_SMALL_SIZE);
    }
    transition_->SetType(ViewTransition::TRANSITION_SHARED_ELEMENT);
    transition_->SetDuration(ANIM_DURATION);
    transition_->SetEasingFunc(EasingEquation::CubicEaseInOut);
    transition_->Start();
}

void UITestTransition::StopCurrentTransition()
{
    if (transition_ == nullptr) {
        return;
    }
    // InvalidateTargets cancels the transition (restoring snapshots if it is running)
    // and clears the view references before deleting; the transition never owns the views.
    transition_->InvalidateTargets();
    delete transition_;
    transition_ = nullptr;
}
} // namespace OHOS
