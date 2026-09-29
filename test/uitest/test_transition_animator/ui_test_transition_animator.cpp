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

#include "ui_test_transition_animator.h"
#include "common/screen.h"
#include "test_resource_config.h"

namespace OHOS {
namespace {
constexpr int16_t STAGE_X = 20;
constexpr int16_t STAGE_HORIZONTAL_MARGIN_COUNT = 2;
constexpr int16_t STAGE_Y = 45;
constexpr int16_t STAGE_HEIGHT = 150;
constexpr int16_t TITLE_WIDTH = 360;
constexpr int16_t TITLE_HEIGHT = 40;
constexpr int16_t TARGET_X = 40;
constexpr int16_t TARGET_Y = 45;
constexpr int16_t TARGET_SIZE = 60;
constexpr int16_t TARGET_END_X = 220;
constexpr int16_t ROTATION_END_ANGLE = 90;
constexpr int16_t BUTTON_START_Y = 220;
constexpr int16_t BUTTON_WIDTH = 130;
constexpr int16_t BUTTON_HEIGHT = 45;
constexpr int16_t BUTTON_HORIZONTAL_GAP = 20;
constexpr int16_t BUTTON_VERTICAL_GAP = 15;
constexpr int16_t BUTTON_ROW_POSITION_SECOND = 2;
constexpr int16_t BUTTON_ROW_POSITION_THIRD = 3;
constexpr uint32_t TRANSITION_DURATION = 1000;
constexpr uint32_t SEQUENTIAL_TRANSITION_DURATION = TRANSITION_DURATION / 2;
constexpr uint8_t OPACITY_START = 25;
constexpr uint8_t OPACITY_END = 255;
constexpr float SCALE_START = 1.0f;
constexpr float SCALE_END = 1.5f;

class PropertyTransitionDemoCallback final : public AnimatorCallback {
public:
    PropertyTransitionDemoCallback(TransitionAnimatorCallback& propertyCallback, UIView* view, uint32_t duration)
        : propertyCallback_(propertyCallback), animator_(this, view, duration, false)
    {
    }

    PropertyTransitionDemoCallback(const PropertyTransitionDemoCallback&) = delete;
    PropertyTransitionDemoCallback& operator=(const PropertyTransitionDemoCallback&) = delete;

    ~PropertyTransitionDemoCallback() override
    {
        if (animator_.GetState() != Animator::STOP) {
            animator_.Stop();
        }
    }

    void Start()
    {
        animator_.Start();
    }

    void Callback(UIView* view) override
    {
        if (view == nullptr) {
            return;
        }
        propertyCallback_.ApplyFrame(view, animator_.GetRunTime(), propertyCallback_.GetTotalTime());
    }

private:
    TransitionAnimatorCallback& propertyCallback_;
    Animator animator_;
};
} // namespace

void UITestPropertyTransition::SetUp()
{
    if (container_ != nullptr) {
        return;
    }
    container_ = new UIScrollView();
    container_->Resize(Screen::GetInstance().GetWidth(), Screen::GetInstance().GetHeight() - BACK_BUTTON_HEIGHT);

    UILabel* title = new UILabel();
    title->SetPosition(TEXT_DISTANCE_TO_LEFT_SIDE, 0, TITLE_WIDTH, TITLE_HEIGHT);
    title->SetText("Native Property Transition");
    title->SetFont(DEFAULT_VECTOR_FONT_FILENAME, FONT_DEFAULT_SIZE);
    container_->Add(title);

    stage_ = new UIViewGroup();
    stage_->SetPosition(STAGE_X, STAGE_Y,
                        Screen::GetInstance().GetWidth() - STAGE_X * STAGE_HORIZONTAL_MARGIN_COUNT, STAGE_HEIGHT);
    stage_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Black().full);
    container_->Add(stage_);

    target_ = new UIView();
    stage_->Add(target_);
    ResetTarget();

    const int16_t firstColumn = TEXT_DISTANCE_TO_LEFT_SIDE;
    const int16_t secondColumn = firstColumn + BUTTON_WIDTH + BUTTON_HORIZONTAL_GAP;
    AddButton(opacityButton_, "Opacity", firstColumn, BUTTON_START_Y);
    AddButton(scaleButton_, "Scale", secondColumn, BUTTON_START_Y);
    AddButton(rotationButton_, "Rotation", firstColumn, BUTTON_START_Y + BUTTON_HEIGHT + BUTTON_VERTICAL_GAP);
    AddButton(colorButton_, "Color", secondColumn, BUTTON_START_Y + BUTTON_HEIGHT + BUTTON_VERTICAL_GAP);
    AddButton(positionButton_, "Position", firstColumn,
              BUTTON_START_Y + (BUTTON_HEIGHT + BUTTON_VERTICAL_GAP) * BUTTON_ROW_POSITION_SECOND);
    AddButton(bounceButton_, "Bounce", secondColumn,
              BUTTON_START_Y + (BUTTON_HEIGHT + BUTTON_VERTICAL_GAP) * BUTTON_ROW_POSITION_SECOND);
    AddButton(sequentialButton_, "Sequential", firstColumn,
              BUTTON_START_Y + (BUTTON_HEIGHT + BUTTON_VERTICAL_GAP) * BUTTON_ROW_POSITION_THIRD);
    AddButton(parallelButton_, "Parallel", secondColumn,
              BUTTON_START_Y + (BUTTON_HEIGHT + BUTTON_VERTICAL_GAP) * BUTTON_ROW_POSITION_THIRD);
}

void UITestPropertyTransition::TearDown()
{
    StopAnimation();
    DeleteChildren(container_);
    container_ = nullptr;
    stage_ = nullptr;
    target_ = nullptr;
    opacityButton_ = nullptr;
    scaleButton_ = nullptr;
    rotationButton_ = nullptr;
    colorButton_ = nullptr;
    positionButton_ = nullptr;
    bounceButton_ = nullptr;
    sequentialButton_ = nullptr;
    parallelButton_ = nullptr;
}

const UIView* UITestPropertyTransition::GetTestView()
{
    return container_;
}

void UITestPropertyTransition::AddButton(UILabelButton*& button, const char* text, int16_t x, int16_t y)
{
    button = new UILabelButton();
    button->SetPosition(x, y, BUTTON_WIDTH, BUTTON_HEIGHT);
    button->SetText(text);
    button->SetFont(DEFAULT_VECTOR_FONT_FILENAME, BUTTON_LABEL_SIZE);
    button->SetOnClickListener(this);
    button->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_STYLE_BORDER_RADIUS_VALUE, UIButton::RELEASED);
    button->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::RELEASED);
    container_->Add(button);
}

void UITestPropertyTransition::ResetTarget()
{
    if (target_ == nullptr) {
        return;
    }
    target_->SetPosition(TARGET_X, TARGET_Y, TARGET_SIZE, TARGET_SIZE);
    target_->SetOpaScale(OPACITY_END);
    target_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Red().full);
    TransformMap& transformMap = target_->GetTransformMap();
    transformMap.SetPolygon(Polygon(Rect(0, 0, 0, 0)));
    target_->Invalidate();
}

void UITestPropertyTransition::StopAnimation()
{
    delete animatorCallback_;
    animatorCallback_ = nullptr;
}

void UITestPropertyTransition::StartAnimation()
{
    uint32_t totalTime = propertyCallback_.GetTotalTime();
    if ((target_ == nullptr) || (totalTime == 0)) {
        return;
    }
    PropertyTransitionDemoCallback* callback =
        new PropertyTransitionDemoCallback(propertyCallback_, target_, totalTime);
    if (callback != nullptr) {
        animatorCallback_ = callback;
        callback->Start();
    }
}

void UITestPropertyTransition::StartOpacityTransition()
{
    propertyCallback_.SetDuration(TRANSITION_DURATION);
    propertyCallback_.SetOpacity(OPACITY_START, OPACITY_END);
    StartAnimation();
}

void UITestPropertyTransition::StartScaleTransition()
{
    propertyCallback_.SetDuration(TRANSITION_DURATION);
    propertyCallback_.SetScale(SCALE_START, SCALE_START, SCALE_END, SCALE_END);
    StartAnimation();
}

void UITestPropertyTransition::StartRotationTransition()
{
    propertyCallback_.SetDuration(TRANSITION_DURATION);
    propertyCallback_.SetRotation(0, ROTATION_END_ANGLE);
    StartAnimation();
}

void UITestPropertyTransition::StartColorTransition()
{
    propertyCallback_.SetDuration(TRANSITION_DURATION);
    propertyCallback_.SetColor(Color::Red(), Color::Blue());
    StartAnimation();
}

void UITestPropertyTransition::StartPositionTransition()
{
    propertyCallback_.SetDuration(TRANSITION_DURATION);
    propertyCallback_.SetPosition(TARGET_X, TARGET_Y, TARGET_END_X, TARGET_Y);
    StartAnimation();
}

void UITestPropertyTransition::StartBounceTransition()
{
    propertyCallback_.SetDuration(TRANSITION_DURATION);
    propertyCallback_.SetEasingFunc(EasingEquation::BounceEaseOut);
    propertyCallback_.SetPosition(TARGET_X, TARGET_Y, TARGET_END_X, TARGET_Y);
    StartAnimation();
}

void UITestPropertyTransition::StartSequentialTransition()
{
    propertyCallback_.SetDuration(SEQUENTIAL_TRANSITION_DURATION);
    propertyCallback_.SetRotation(0, ROTATION_END_ANGLE);
    nextPropertyCallback_.SetDuration(SEQUENTIAL_TRANSITION_DURATION);
    nextPropertyCallback_.SetPosition(TARGET_X, TARGET_Y, TARGET_END_X, TARGET_Y);
    propertyCallback_.AddTransitionAnimatorCallback(&nextPropertyCallback_, true);
    StartAnimation();
}

void UITestPropertyTransition::StartParallelTransition()
{
    propertyCallback_.SetDuration(TRANSITION_DURATION);
    propertyCallback_.SetRotation(0, ROTATION_END_ANGLE);
    nextPropertyCallback_.SetDuration(TRANSITION_DURATION);
    nextPropertyCallback_.SetPosition(TARGET_X, TARGET_Y, TARGET_END_X, TARGET_Y);
    propertyCallback_.AddTransitionAnimatorCallback(&nextPropertyCallback_, false);
    StartAnimation();
}

bool UITestPropertyTransition::OnClick(UIView& view, const ClickEvent& event)
{
    (void)event;
    StopAnimation();
    propertyCallback_.Reset();
    nextPropertyCallback_.Reset();
    ResetTarget();
    if (&view == opacityButton_) {
        StartOpacityTransition();
    } else if (&view == scaleButton_) {
        StartScaleTransition();
    } else if (&view == rotationButton_) {
        StartRotationTransition();
    } else if (&view == colorButton_) {
        StartColorTransition();
    } else if (&view == positionButton_) {
        StartPositionTransition();
    } else if (&view == bounceButton_) {
        StartBounceTransition();
    } else if (&view == sequentialButton_) {
        StartSequentialTransition();
    } else if (&view == parallelButton_) {
        StartParallelTransition();
    }
    return true;
}
} // namespace OHOS
