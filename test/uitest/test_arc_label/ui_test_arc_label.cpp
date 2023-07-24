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

#include "ui_test_arc_label.h"
#include "common/screen.h"
#include "font/ui_font.h"

#define COMPATIBILITY_MODE
namespace OHOS {
namespace {
const int16_t GAP = 5;
const int16_t TITLE_HEIGHT = 29;
const uint16_t RADIUS = 150;
#ifdef COMPATIBILITY_MODE
const uint16_t CENTER_X = 185;
const uint16_t CENTER_Y = 155;
#else
const uint16_t CENTER_X = 200;
const uint16_t CENTER_Y = 200;
#endif
const uint16_t LABEL_WIDTH = 400;
const uint16_t LABEL_HEIGHT = 400;
const uint16_t LABEL_WIDTH_500 = 500;
const uint16_t LABEL_HEIGHT_500 = 500;
const uint16_t ANIMATOR_LOOPCOUNT = 2;
const uint16_t ANIMATOR_SPEED = 15;
const std::string DEFAULT_ARCLABEL_TEXT = "考虑线段在框内, 框外和部分在框内";
} // namespace

void UITestArcLabel::SetUp()
{
    if (container_ == nullptr) {
        container_ = new UIScrollView();
        container_->SetThrowDrag(true);
        container_->SetHorizontalScrollState(false);
        container_->Resize(Screen::GetInstance().GetWidth(), Screen::GetInstance().GetHeight() - BACK_BUTTON_HEIGHT);
        positionX_ = 0;
        positionY_ = 0;
    }
}

void UITestArcLabel::TearDown()
{
    DeleteChildren(container_);
    container_ = nullptr;
}

void UITestArcLabel::InnerTestTitle(const char* title, int16_t x, int16_t y) const
{
    UILabel* titleLabel = new UILabel();
    titleLabel->SetPosition(x, y, Screen::GetInstance().GetWidth(), TITLE_HEIGHT);
    titleLabel->SetFont(DEFAULT_VECTOR_FONT_FILENAME, FONT_DEFAULT_SIZE);
    titleLabel->SetText(title);
    container_->Add(titleLabel);
}

const UIView* UITestArcLabel::GetTestView()
{
#ifdef COMPATIBILITY_MODE
    UIKitUIArcLabelTestDisplay001();
    UIKitUIArcLabelTestDisplay002();
    UIKitUIArcLabelTestDisplay003();
    UIKitUIArcLabelTestDisplay004();

    UIKitUIArcLabelTestAlign001();
    UIKitUIArcLabelTestAlign002();
    UIKitUIArcLabelTestAlign003();
#else
    UIKitUIArcLabelTestIncompatible001();
    UIKitUIArcLabelTestIncompatible002();
    UIKitUIArcLabelTestIncompatible003();
    UIKitUIArcLabelTestIncompatible004();
#endif
    return container_;
}

void UITestArcLabel::UIKitUIArcLabelTestDisplay001()
{
    const char* title = "显示内部角度从0到270的弧形文本 ";
    // 0: start angle  270: end angle
    TestArcLabelDisplay(title, 0, 270, TextOrientation::INSIDE, VIEW_DISTANCE_TO_LEFT_SIDE,
                        VIEW_DISTANCE_TO_TOP_SIDE);
}

void UITestArcLabel::UIKitUIArcLabelTestDisplay002()
{
    const char* title = "显示内部角度从0到-270的弧形文本 ";
    // 0: start angle  -270: end angle  100: y-coordinate
    TestArcLabelDisplay(title, 0, -270, TextOrientation::INSIDE, VIEW_DISTANCE_TO_LEFT_SIDE, 100);
}

void UITestArcLabel::UIKitUIArcLabelTestDisplay003()
{
    const char* title = "显示外部角度从0到270的弧形文本 ";
    // 0: start angle  270: end angle  200: y-coordinate
    TestArcLabelDisplay(title, 0, 270, TextOrientation::OUTSIDE, VIEW_DISTANCE_TO_LEFT_SIDE, 200);
}

void UITestArcLabel::UIKitUIArcLabelTestDisplay004()
{
    const char* title = "显示外部角度从0到-270的弧形文本 ";
    // 0: start angle  -270: end angle  300: y-coordinate
    TestArcLabelDisplay(title, 0, -270, TextOrientation::OUTSIDE, VIEW_DISTANCE_TO_LEFT_SIDE, 300);
}

void UITestArcLabel::TestArcLabelDisplay(const char* title,
                                         const int16_t startAngle,
                                         const int16_t endAngle,
                                         const TextOrientation orientation,
                                         int16_t x,
                                         int16_t y)
{
    if (container_ != nullptr) {
        InnerTestTitle(title, x, positionY_);
        UIArcLabel* label = new UIArcLabel();
        label->SetArcTextCenter(CENTER_X, CENTER_Y + positionY_ + GAP * 4); // 4: 4 times GAP
        label->SetArcTextRadius(RADIUS);
        label->SetArcTextAngle(startAngle, endAngle);
        label->SetArcTextOrientation(orientation);
        label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, 30); // 30: font size
        label->SetText("012345678ABCDEF0123456789ABCDE");
        label->SetStyle(STYLE_LETTER_SPACE, 10); // 10: space
        container_->Add(label);
        positionY_ += (RADIUS * 2) + GAP + TITLE_HEIGHT; // 2: diameter
    }
}

class LabelButtonClickListener : public OHOS::UIView::OnClickListener {
public:
    explicit LabelButtonClickListener(UIView* view) : view_(view)
    {
    }

    bool OnClick(UIView& view, const ClickEvent& event) override
    {
        UIArcLabel* label = static_cast<UIArcLabel*>(view_);
        if (label == nullptr) {
            return false;
        }

        label->SetRollCount(ANIMATOR_LOOPCOUNT);
        label->SetRollSpeed(ANIMATOR_SPEED);
        label->Start();
    }
private:
    UIView* view_;
};

class TestArcLabelScrollListener : public ArcLabelScrollListener {
public:
    explicit TestArcLabelScrollListener(UIView* view) : view_(view)
    {
    }

    void Finish() override
    {
        UILabelButton* button = static_cast<UILabelButton*>(view_);
        if (button == nullptr) {
            return;
        }

        button->SetText("旋转动画结束！！！");
    }

private:
    UIView* view_;
};

UILabelButton* UITestArcLabel::GetTestUIButton(const char* buttonText, int16_t x, int16_t y, const char* id)
{
    if (buttonText == nullptr) {
        return nullptr;
    }

    UILabelButton* labelButton = new UILabelButton();
    labelButton->SetPosition(x, y);
    labelButton->Resize(LABEL_WIDTH, BUTTON_HEIGHT1);
    labelButton->SetText(buttonText);
    labelButton->SetViewId(id);
    labelButton->SetFont(DEFAULT_VECTOR_FONT_FILENAME, FONT_DEFAULT_SIZE);
    labelButton->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_STYLE_BORDER_RADIUS_VALUE, UIButton::PRESSED);
    labelButton->SetStyleForState(STYLE_BORDER_RADIUS, BUTTON_STYLE_BORDER_RADIUS_VALUE, UIButton::INACTIVE);
    labelButton->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::RELEASED);
    labelButton->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::PRESSED);
    labelButton->SetStyleForState(STYLE_BACKGROUND_COLOR, BUTTON_STYLE_BACKGROUND_COLOR_VALUE, UIButton::INACTIVE);
    return labelButton;
}

void UITestArcLabel::UIKitUIArcLabelTestIncompatible001()
{
    if (container_ == nullptr) {
        return;
    }

    positionX_ = 0;
    positionY_ = 0;
    UILabelButton* button = GetTestUIButton("0-90点击旋转", positionX_, positionY_, nullptr);
    if (button == nullptr) {
        return;
    }
    UIArcLabel* label = new UIArcLabel();
    if (label == nullptr) {
        return;
    }
    label->SetText(DEFAULT_ARCLABEL_TEXT.c_str());
    label->SetArcTextCenter(CENTER_X, CENTER_Y + TITLE_HEIGHT);
    label->SetArcTextRadius(RADIUS);
    label->SetCompatibilityMode(false);
    label->SetArcTextAngle(0, 90);                      // 0: start angle, 90: end angle.
    label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, 30);   // 30: font size
    label->SetPosition(0, BUTTON_HEIGHT1, LABEL_WIDTH, LABEL_HEIGHT);
    label->RegisterScrollListener(new TestArcLabelScrollListener(button));
    button->SetOnClickListener(new LabelButtonClickListener(label));

    container_->Add(button);
    container_->Add(label);
}

void UITestArcLabel::UIKitUIArcLabelTestIncompatible002()
{
    if (container_ == nullptr) {
        return;
    }

    positionX_ = LABEL_WIDTH_500;
    positionY_ = 0;
    UILabelButton* button = GetTestUIButton("90-0点击旋转", positionX_, positionY_, nullptr);
    if (button == nullptr) {
        return;
    }
    UIArcLabel* label = new UIArcLabel();
    if (label == nullptr) {
        return;
    }
    label->SetText(DEFAULT_ARCLABEL_TEXT.c_str());
    label->SetArcTextCenter(CENTER_X + LABEL_WIDTH_500, CENTER_Y + TITLE_HEIGHT);
    label->SetArcTextRadius(RADIUS);
    label->SetCompatibilityMode(false);
    label->SetArcTextAngle(90, 0);                      // 90: start angle, 0: end angle.
    label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, 30);   // 30: font size
    label->SetPosition(LABEL_WIDTH_500, BUTTON_HEIGHT1, LABEL_WIDTH, LABEL_HEIGHT);
    label->RegisterScrollListener(new TestArcLabelScrollListener(button));
    button->SetOnClickListener(new LabelButtonClickListener(label));

    container_->Add(button);
    container_->Add(label);
}

void UITestArcLabel::UIKitUIArcLabelTestIncompatible003()
{
    if (container_ == nullptr) {
        return;
    }

    positionX_ = 0;
    positionY_ = LABEL_HEIGHT_500 - BUTTON_HEIGHT1;
    UILabelButton* button = GetTestUIButton("30-260点击旋转", positionX_, positionY_, nullptr);
    if (button == nullptr) {
        return;
    }
    UIArcLabel* label = new UIArcLabel();
    if (label == nullptr) {
        return;
    }
    label->SetText(DEFAULT_ARCLABEL_TEXT.c_str());
    label->SetArcTextCenter(CENTER_X, CENTER_Y + LABEL_HEIGHT_500);
    label->SetArcTextRadius(RADIUS);
    label->SetCompatibilityMode(false);
    label->SetArcTextAngle(30, 260);                      // 30: start angle, 260: end angle.
    label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, 30);   // 30: font size
    label->SetPosition(0, LABEL_HEIGHT_500, LABEL_WIDTH, LABEL_HEIGHT);
    label->RegisterScrollListener(new TestArcLabelScrollListener(button));
    button->SetOnClickListener(new LabelButtonClickListener(label));

    container_->Add(button);
    container_->Add(label);
}

void UITestArcLabel::UIKitUIArcLabelTestIncompatible004()
{
    if (container_ == nullptr) {
        return;
    }

    positionX_ = LABEL_WIDTH_500;
    positionY_ = LABEL_HEIGHT_500 - BUTTON_HEIGHT1;
    UILabelButton* button = GetTestUIButton("260-30点击旋转", positionX_, positionY_, nullptr);
    if (button == nullptr) {
        return;
    }
    UIArcLabel* label = new UIArcLabel();
    if (label == nullptr) {
        return;
    }
    label->SetText(DEFAULT_ARCLABEL_TEXT.c_str());
    label->SetArcTextCenter(CENTER_X + LABEL_WIDTH_500, CENTER_Y + LABEL_HEIGHT_500);
    label->SetArcTextRadius(RADIUS);
    label->SetCompatibilityMode(false);
    label->SetArcTextAngle(260, 30);                      // 260: start angle, 30: end angle.
    label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, 30);   // 30: font size
    label->SetPosition(LABEL_WIDTH_500, LABEL_HEIGHT_500, LABEL_WIDTH, LABEL_HEIGHT);
    label->RegisterScrollListener(new TestArcLabelScrollListener(button));
    button->SetOnClickListener(new LabelButtonClickListener(label));

    container_->Add(button);
    container_->Add(label);
}

void UITestArcLabel::UIKitUIArcLabelTestAlign001()
{
    if (container_ != nullptr) {
        // 2: half of screen width
        positionX_ = Screen::GetInstance().GetWidth() / 2 + VIEW_DISTANCE_TO_LEFT_SIDE;
        positionY_ = 0;
        InnerTestTitle("弧形文本在0到90度之间左对齐 ", positionX_, positionY_);
        positionY_ += TITLE_HEIGHT + GAP;
        UIView* back = new UIView();
        back->SetPosition(positionX_, positionY_ + GAP);
        back->Resize(RADIUS, RADIUS);
        back->SetStyle(STYLE_BACKGROUND_COLOR, Color::Gray().full);
        back->SetStyle(STYLE_BACKGROUND_OPA, OPA_OPAQUE);

        UIArcLabel* label = new UIArcLabel();
        label->SetArcTextCenter(positionX_, CENTER_Y + positionY_);
        label->SetArcTextRadius(RADIUS);
        label->SetArcTextAngle(0, 90);                    // 0: start angle, 90: end angle
        label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, 30); // 30: font size
        label->SetText("LEFT");
        label->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_LEFT);
        label->SetStyle(STYLE_LETTER_SPACE, 6); // 6: space

        container_->Add(back);
        container_->Add(label);
        positionY_ += RADIUS + GAP + TITLE_HEIGHT;
    }
}

void UITestArcLabel::UIKitUIArcLabelTestAlign002()
{
    if (container_ != nullptr) {
        InnerTestTitle("弧形文本在0到90度之间居中对齐 ", positionX_, positionY_);
        positionY_ += TITLE_HEIGHT + GAP;
        UIView* back = new UIView();
        back->SetPosition(positionX_, positionY_ + GAP);
        back->Resize(RADIUS, RADIUS);
        back->SetStyle(STYLE_BACKGROUND_COLOR, Color::Gray().full);
        back->SetStyle(STYLE_BACKGROUND_OPA, OPA_OPAQUE);

        UIArcLabel* label = new UIArcLabel();
        label->SetArcTextCenter(positionX_, CENTER_Y + positionY_);
        label->SetArcTextRadius(RADIUS);
        label->SetArcTextAngle(0, 90);                    // 0: start angle, 90: end angle
        label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, 30); // 30: font size
        label->SetText("CENTER");
        label->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_CENTER);
        label->SetStyle(STYLE_LETTER_SPACE, 6); // 6: space

        container_->Add(back);
        container_->Add(label);
        positionY_ += RADIUS + GAP + TITLE_HEIGHT;
    }
}

void UITestArcLabel::UIKitUIArcLabelTestAlign003()
{
    if (container_ != nullptr) {
        InnerTestTitle("弧形文本在0到90度之间右对齐 ", positionX_, positionY_);
        positionY_ += TITLE_HEIGHT + GAP;
        UIView* back = new UIView();
        back->SetPosition(positionX_, positionY_ + GAP);
        back->Resize(RADIUS, RADIUS);
        back->SetStyle(STYLE_BACKGROUND_COLOR, Color::Gray().full);
        back->SetStyle(STYLE_BACKGROUND_OPA, OPA_OPAQUE);

        UIArcLabel* label = new UIArcLabel();
        label->SetArcTextCenter(positionX_, CENTER_Y + positionY_);
        label->SetArcTextRadius(RADIUS);
        label->SetArcTextAngle(0, 90);                    // 0: start angle, 90: end angle
        label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, 30); // 30: font size
        label->SetText("RIGHT");
        label->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_RIGHT);
        label->SetStyle(STYLE_LETTER_SPACE, 6); // 6: space

        container_->Add(back);
        container_->Add(label);
        positionY_ += RADIUS + GAP + TITLE_HEIGHT;
    }
}
} // namespace OHOS
