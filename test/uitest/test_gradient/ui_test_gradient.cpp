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
#include "ui_test_gradient.h"

#include "common/screen.h"
#include "components/ui_label_button.h"
#include "components/ui_label.h"
#include "gfx_utils/gradient_info.h"
#include "test_resource_config.h"

namespace OHOS {
namespace {
constexpr int16_t VIEW_X = 16;
constexpr int16_t VIEW_W = 300;
constexpr int16_t VIEW_H = 80;
constexpr int16_t LABEL_H = 28;
constexpr int16_t LABEL_W = 300;
constexpr int16_t GROUP_GAP = 16;
constexpr int16_t START_Y = 16;
constexpr int16_t LABEL_OFFSET_Y = 4;

constexpr uint8_t ALPHA_OPAQUE = 0xFF;

constexpr uint8_t RED_R = 0xFF;
constexpr uint8_t RED_G = 0x00;
constexpr uint8_t RED_B = 0x00;

constexpr uint8_t ORANGE_R = 0xFF;
constexpr uint8_t ORANGE_G = 0x98;
constexpr uint8_t ORANGE_B = 0x00;

constexpr uint8_t YELLOW_R = 0xFF;
constexpr uint8_t YELLOW_G = 0xEB;
constexpr uint8_t YELLOW_B = 0x3B;

constexpr uint8_t GREEN_R = 0x44;
constexpr uint8_t GREEN_G = 0xAA;
constexpr uint8_t GREEN_B = 0x44;

constexpr uint8_t BLUE_R = 0x00;
constexpr uint8_t BLUE_G = 0x00;
constexpr uint8_t BLUE_B = 0xFF;

constexpr float STOP_START = 0.0f;
constexpr float STOP_QUARTER = 0.25f;
constexpr float STOP_THIRD = 1.0f / 3;
constexpr float STOP_MID = 0.5f;
constexpr float STOP_TWO_THIRDS = 2.0f / 3;
constexpr float STOP_THREE_QUARTERS = 0.75f;
constexpr float STOP_END = 1.0f;

template<typename T, size_t N>
constexpr size_t ArraySize(T (&)[N]) { return N; }

GradientColorStop MakeRedStop(float offset) { return GradientColorStop(RED_R, RED_G, RED_B, ALPHA_OPAQUE, offset); }
GradientColorStop MakeOrangeStop(float offset)
{
    return GradientColorStop(ORANGE_R, ORANGE_G, ORANGE_B, ALPHA_OPAQUE, offset);
}
GradientColorStop MakeYellowStop(float offset)
{
    return GradientColorStop(YELLOW_R, YELLOW_G, YELLOW_B, ALPHA_OPAQUE, offset);
}
GradientColorStop MakeGreenStop(float offset)
{
    return GradientColorStop(GREEN_R, GREEN_G, GREEN_B, ALPHA_OPAQUE, offset);
}
GradientColorStop MakeBlueStop(float offset) { return GradientColorStop(BLUE_R, BLUE_G, BLUE_B, ALPHA_OPAQUE, offset); }

GradientInfo* CreateGradientInfo(CssGradientDirection dir, const GradientColorStop* stops, uint8_t count)
{
    GradientInfo* info = new GradientInfo();
    info->direction = dir;
    info->SetColorStops(stops, count);
    return info;
}

int16_t g_currentY = START_Y;
} // namespace

void UITestGradient::SetUp()
{
    if (container_ == nullptr) {
        container_ = new UIScrollView();
        container_->Resize(Screen::GetInstance().GetWidth(),
                           Screen::GetInstance().GetHeight() - BACK_BUTTON_HEIGHT);
        container_->SetHorizontalScrollState(false);
    }
    g_currentY = START_Y;
}

void UITestGradient::TearDown()
{
    DeleteChildren(container_);
    container_ = nullptr;
}

const UIView* UITestGradient::GetTestView()
{
    UIKitGradientTestTwoColor001();
    UIKitGradientTestThreeColor002();
    UIKitGradientTestFiveColor003();
    UIKitGradientTestDirectionRight004();
    UIKitGradientTestDirectionLeft005();
    UIKitGradientTestDirectionBottom006();
    UIKitGradientTestDirectionBottomRight007();
    UIKitGradientTestButtonMultiColor008();
    return container_;
}

UIView* UITestGradient::CreateGradientView(int16_t x, int16_t y, int16_t w, int16_t h, GradientInfo* info)
{
    UIView* view = new UIView();
    view->SetPosition(x, y, w, h);
    view->SetGradientInfo(info);
    container_->Add(view);
    return view;
}

void UITestGradient::AddLabel(const char* text, int16_t y)
{
    UILabel* label = new UILabel();
    label->SetPosition(VIEW_X, y, LABEL_W, LABEL_H);
    label->SetText(text);
    label->SetFont(DEFAULT_VECTOR_FONT_FILENAME, FONT_DEFAULT_SIZE);
    container_->Add(label);
}

void UITestGradient::UIKitGradientTestTwoColor001()
{
    GradientColorStop stops[] = {
        MakeRedStop(STOP_START),
        MakeBlueStop(STOP_END),
    };
    CreateGradientView(VIEW_X, g_currentY, VIEW_W, VIEW_H,
                       CreateGradientInfo(CssGradientDirection::TO_RIGHT, stops, ArraySize(stops)));
    AddLabel("双色渐变 to right", g_currentY + VIEW_H + LABEL_OFFSET_Y);
    g_currentY += VIEW_H + LABEL_H + GROUP_GAP;
}

void UITestGradient::UIKitGradientTestThreeColor002()
{
    GradientColorStop stops[] = {
        MakeRedStop(STOP_START),
        MakeYellowStop(STOP_MID),
        MakeBlueStop(STOP_END),
    };
    CreateGradientView(VIEW_X, g_currentY, VIEW_W, VIEW_H,
                       CreateGradientInfo(CssGradientDirection::TO_RIGHT, stops, ArraySize(stops)));
    AddLabel("三色渐变 to right", g_currentY + VIEW_H + LABEL_OFFSET_Y);
    g_currentY += VIEW_H + LABEL_H + GROUP_GAP;
}

void UITestGradient::UIKitGradientTestFiveColor003()
{
    GradientColorStop stops[] = {
        MakeRedStop(STOP_START),
        MakeOrangeStop(STOP_QUARTER),
        MakeYellowStop(STOP_MID),
        MakeGreenStop(STOP_THREE_QUARTERS),
        MakeBlueStop(STOP_END),
    };
    CreateGradientView(VIEW_X, g_currentY, VIEW_W, VIEW_H,
                       CreateGradientInfo(CssGradientDirection::TO_RIGHT, stops, ArraySize(stops)));
    AddLabel("五色渐变 to right", g_currentY + VIEW_H + LABEL_OFFSET_Y);
    g_currentY += VIEW_H + LABEL_H + GROUP_GAP;
}

void UITestGradient::UIKitGradientTestDirectionRight004()
{
    GradientColorStop stops[] = {
        MakeRedStop(STOP_START),
        MakeBlueStop(STOP_END),
    };
    CreateGradientView(VIEW_X, g_currentY, VIEW_W, VIEW_H,
                       CreateGradientInfo(CssGradientDirection::TO_RIGHT, stops, ArraySize(stops)));
    AddLabel("方向 to right", g_currentY + VIEW_H + LABEL_OFFSET_Y);
    g_currentY += VIEW_H + LABEL_H + GROUP_GAP;
}

void UITestGradient::UIKitGradientTestDirectionLeft005()
{
    GradientColorStop stops[] = {
        MakeRedStop(STOP_START),
        MakeBlueStop(STOP_END),
    };
    CreateGradientView(VIEW_X, g_currentY, VIEW_W, VIEW_H,
                       CreateGradientInfo(CssGradientDirection::TO_LEFT, stops, ArraySize(stops)));
    AddLabel("方向 to left", g_currentY + VIEW_H + LABEL_OFFSET_Y);
    g_currentY += VIEW_H + LABEL_H + GROUP_GAP;
}

void UITestGradient::UIKitGradientTestDirectionBottom006()
{
    GradientColorStop stops[] = {
        MakeRedStop(STOP_START),
        MakeBlueStop(STOP_END),
    };
    CreateGradientView(VIEW_X, g_currentY, VIEW_W, VIEW_H,
                       CreateGradientInfo(CssGradientDirection::TO_BOTTOM, stops, ArraySize(stops)));
    AddLabel("方向 to bottom", g_currentY + VIEW_H + LABEL_OFFSET_Y);
    g_currentY += VIEW_H + LABEL_H + GROUP_GAP;
}

void UITestGradient::UIKitGradientTestDirectionBottomRight007()
{
    GradientColorStop stops[] = {
        MakeRedStop(STOP_START),
        MakeBlueStop(STOP_END),
    };
    CreateGradientView(VIEW_X, g_currentY, VIEW_W, VIEW_H,
                       CreateGradientInfo(CssGradientDirection::TO_BOTTOM_RIGHT, stops, ArraySize(stops)));
    AddLabel("方向 to bottom right", g_currentY + VIEW_H + LABEL_OFFSET_Y);
    g_currentY += VIEW_H + LABEL_H + GROUP_GAP;
}

void UITestGradient::UIKitGradientTestButtonMultiColor008()
{
    GradientColorStop stops[] = {
        MakeRedStop(STOP_START),
        MakeOrangeStop(STOP_THIRD),
        MakeYellowStop(STOP_TWO_THIRDS),
        MakeBlueStop(STOP_END),
    };

    GradientInfo* info = CreateGradientInfo(CssGradientDirection::TO_RIGHT, stops, ArraySize(stops));

    UILabelButton* button = new UILabelButton();
    button->SetPosition(VIEW_X, g_currentY, VIEW_W, VIEW_H);
    button->SetText("多色渐变按钮");
    button->SetFont(DEFAULT_VECTOR_FONT_FILENAME, FONT_DEFAULT_SIZE);
    button->SetStyle(STYLE_TEXT_COLOR, Color::White().full);
    button->SetStyle(STYLE_TEXT_OPA, OPA_OPAQUE);
    button->SetGradientInfo(info);
    container_->Add(button);

    AddLabel("Button 多色渐变", g_currentY + VIEW_H + LABEL_OFFSET_Y);
    g_currentY += VIEW_H + LABEL_H + GROUP_GAP;
}
} // namespace OHOS
