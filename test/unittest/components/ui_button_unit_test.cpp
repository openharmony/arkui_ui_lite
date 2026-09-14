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

#include "components/ui_button.h"

#include <climits>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "test_resource_config.h"
#include "draw/draw_utils.h"
#include "engines/gfx/gfx_engine_manager.h"
#include "engines/gfx/soft_engine.h"

using namespace testing::ext;

namespace OHOS {
namespace {
    const Point INIT_POS = { 10, 12 };
}
#if DEFAULT_ANIMATION && GRAPHIC_ENABLE_BUTTON_FLAG
class MockButtonSoftEngine : public SoftEngine {
public:
    MOCK_METHOD(void, DrawRect,
        (BufferInfo&, const Rect&, const Rect&, const Style&, OpacityType), (override));
};

class TestUIButton : public UIButton {
public:
    void SetTestState(ButtonState state)
    {
        SetState(state);
    }

    void StartAnimator()
    {
        animator_.Start();
    }

    void RunAnimatorCallback()
    {
        animator_.Callback(this);
    }

    void RunAnimatorOnStop()
    {
        animator_.OnStop(*this);
    }
};
#endif

class UIButtonTest : public testing::Test {
public:
    UIButtonTest() : button_(nullptr) {}
    virtual ~UIButtonTest() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void TearDown();
    void SetUp();
    UIButton* button_;
};

void UIButtonTest::SetUp()
{
    if (button_ == nullptr) {
        button_ = new UIButton();
    }
}

void UIButtonTest::TearDown()
{
    if (button_ != nullptr) {
        delete button_;
        button_ = nullptr;
    }
}

/**
 * @tc.name: UIButtonGetViewType_001
 * @tc.desc: Verify GetViewType function.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonGetViewType_001, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    EXPECT_EQ(button_->GetViewType(), UI_BUTTON);
}

/**
 * @tc.name: UIButtonSetImageSrc_001
 * @tc.desc: Verify SetImageSrc function.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonSetImageSrc_001, TestSize.Level0)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetImageSrc(BLUE_RGB888_IMAGE_PATH, BLUE_RGB565_IMAGE_PATH);
    ASSERT_TRUE(button_->GetCurImageSrc());

    if (button_->GetCurImageSrc()->GetPath() == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    EXPECT_EQ(strcmp(button_->GetCurImageSrc()->GetPath(), BLUE_RGB888_IMAGE_PATH), 0);
    PressEvent event(INIT_POS);
    button_->OnPressEvent(event);
    if (button_->GetCurImageSrc()->GetPath() == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    EXPECT_EQ(strcmp(button_->GetCurImageSrc()->GetPath(), BLUE_RGB565_IMAGE_PATH), 0);
}

/**
 * @tc.name:UIButtonSetImagePosition_001
 * @tc.desc: Verify SetImagePosition function.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonSetImagePosition_001, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetImagePosition(INIT_POS.x, INIT_POS.y);
    EXPECT_EQ(button_->GetImageX(), INIT_POS.x);
    EXPECT_EQ(button_->GetImageY(), INIT_POS.y);
}

/**
 * @tc.name: UIButtonSetStyle_001
 * @tc.desc: Verify SetStyle function.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonSetStyle_001, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetStateForStyle(UIButton::ButtonState::PRESSED);
    button_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Blue().full);
    EXPECT_EQ(button_->GetStyle(STYLE_BACKGROUND_COLOR), Color::Blue().full);
    EXPECT_EQ(button_->GetStyleForState(STYLE_BACKGROUND_COLOR, UIButton::ButtonState::PRESSED), Color::Blue().full);
}

/**
 * @tc.name: UIButtonSetStyle_002
 * @tc.desc: Verify SetStyle function.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonSetStyle_002, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetStateForStyle(UIButton::ButtonState::INACTIVE);
    button_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Red().full);
    EXPECT_EQ(button_->GetStyle(STYLE_BACKGROUND_COLOR), Color::Red().full);
    EXPECT_EQ(button_->GetStyleForState(STYLE_BACKGROUND_COLOR, UIButton::ButtonState::INACTIVE), Color::Red().full);
}

/**
 * @tc.name: UIButtonSetStyle_003
 * @tc.desc: Verify SetStyle function.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonSetStyle_003, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetStateForStyle(UIButton::ButtonState::RELEASED);
    button_->SetStyle(STYLE_BACKGROUND_COLOR, Color::Green().full);
    EXPECT_EQ(button_->GetStyle(STYLE_BACKGROUND_COLOR), Color::Green().full);
    EXPECT_EQ(button_->GetStyleForState(STYLE_BACKGROUND_COLOR, UIButton::ButtonState::RELEASED), Color::Green().full);
}

/**
 * @tc.name: UIButtonIsTouchable_001
 * @tc.desc: Verify IsTouchable function, equal.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonIsTouchable_001, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->Enable();
    EXPECT_EQ(button_->IsTouchable(), true);
    button_->Disable();
    EXPECT_EQ(button_->IsTouchable(), false);
}

/**
 * @tc.name: UIButtonSetSize_001
 * @tc.desc: Verify SetSize function, equal.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonSetSize_001, TestSize.Level0)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    const int16_t releasedWidth = 5;
    const int16_t releasedHeight = 25;
    const int16_t paddingdLeft = 10;
    const int16_t paddingdTop = 20;
    const int16_t borderWidth = 2;
    const int16_t posX = 50;
    const int16_t posY = 100;
    button_->SetStyleForState(STYLE_PADDING_LEFT, paddingdLeft, UIButton::ButtonState::RELEASED);
    button_->SetStyleForState(STYLE_BORDER_WIDTH, borderWidth, UIButton::ButtonState::RELEASED);
    button_->SetStyleForState(STYLE_PADDING_TOP, paddingdTop, UIButton::ButtonState::RELEASED);

    ReleaseEvent releaseEvent(INIT_POS);
    button_->OnReleaseEvent(releaseEvent);
button_->SetPosition(posX, posY);
    button_->SetWidth(releasedWidth);
    button_->SetHeight(releasedHeight);
    EXPECT_EQ(button_->GetWidth(), releasedWidth);
    EXPECT_EQ(button_->GetHeight(), releasedHeight);
    EXPECT_EQ(button_->GetContentRect().GetWidth(), releasedWidth);
    EXPECT_EQ(button_->GetContentRect().GetHeight(), releasedHeight);
    EXPECT_EQ(button_->GetContentRect().GetX(), posX + paddingdLeft + borderWidth);
    EXPECT_EQ(button_->GetContentRect().GetY(), posY + paddingdTop + borderWidth);
}

/**
 * @tc.name: UIButtonSetSize_002
 * @tc.desc: Verify SetSize function, equal.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonSetSize_002, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    const int16_t pressWidth = 5;
    const int16_t pressHeight = 25;
    const int16_t paddingdLeft = 10;
    const int16_t paddingdTop = 20;
    const int16_t borderWidth = 2;
    const int16_t posX = 50;
    const int16_t posY = 100;
    button_->SetStyleForState(STYLE_PADDING_LEFT, paddingdLeft, UIButton::ButtonState::PRESSED);
    button_->SetStyleForState(STYLE_BORDER_WIDTH, borderWidth, UIButton::ButtonState::PRESSED);
    button_->SetStyleForState(STYLE_PADDING_TOP, paddingdTop, UIButton::ButtonState::PRESSED);

    PressEvent pressEvent(INIT_POS);
    button_->OnPressEvent(pressEvent);
    button_->SetPosition(posX, posY);
    button_->SetWidth(pressWidth);
    button_->SetHeight(pressHeight);
    EXPECT_EQ(button_->GetWidth(), pressWidth);
    EXPECT_EQ(button_->GetHeight(), pressHeight);
    EXPECT_EQ(button_->GetContentRect().GetWidth(), pressWidth);
    EXPECT_EQ(button_->GetContentRect().GetHeight(), pressHeight);
    EXPECT_EQ(button_->GetContentRect().GetX(), posX + paddingdLeft + borderWidth);
    EXPECT_EQ(button_->GetContentRect().GetY(), posY + paddingdTop + borderWidth);
}

/**
 * @tc.name: UIButtonSetSize_003
 * @tc.desc: Verify SetSize function, equal.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonSetSize_003, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    const int16_t inactiveWidth = 5;
    const int16_t inactiveHeight = 25;
    const int16_t paddingdLeft = 10;
    const int16_t paddingdTop = 20;
    const int16_t borderWidth = 2;
    const int16_t posX = 50;
    const int16_t posY = 100;
    button_->SetStyleForState(STYLE_PADDING_LEFT, paddingdLeft, UIButton::ButtonState::INACTIVE);
    button_->SetStyleForState(STYLE_BORDER_WIDTH, borderWidth, UIButton::ButtonState::INACTIVE);
    button_->SetStyleForState(STYLE_PADDING_TOP, paddingdTop, UIButton::ButtonState::INACTIVE);

    button_->Disable();
    button_->SetPosition(posX, posY);
    button_->SetWidth(inactiveWidth);
    button_->SetHeight(inactiveHeight);
    EXPECT_EQ(button_->GetWidth(), inactiveWidth);
    EXPECT_EQ(button_->GetHeight(), inactiveHeight);
    EXPECT_EQ(button_->GetContentRect().GetWidth(), inactiveWidth);
    EXPECT_EQ(button_->GetContentRect().GetHeight(), inactiveHeight);
    EXPECT_EQ(button_->GetContentRect().GetX(), posX + paddingdLeft + borderWidth);
    EXPECT_EQ(button_->GetContentRect().GetY(), posY + paddingdTop + borderWidth);
}

/**
 * @tc.name: UIButtonSetStyleForState_001
 * @tc.desc: Verify SetStyleForState function, equal.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonSetStyleForState_001, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetStyleForState(STYLE_BACKGROUND_COLOR, Color::Red().full, UIButton::ButtonState::RELEASED);
    EXPECT_EQ(button_->GetStyleForState(STYLE_BACKGROUND_COLOR, UIButton::ButtonState::RELEASED),
        Color::Red().full);
    button_->SetStyleForState(STYLE_BACKGROUND_COLOR, Color::Green().full, UIButton::ButtonState::PRESSED);
    EXPECT_EQ(button_->GetStyleForState(STYLE_BACKGROUND_COLOR, UIButton::ButtonState::PRESSED),
        Color::Green().full);
    button_->SetStyleForState(STYLE_BACKGROUND_COLOR, Color::Yellow().full, UIButton::ButtonState::INACTIVE);
    EXPECT_EQ(button_->GetStyleForState(STYLE_BACKGROUND_COLOR, UIButton::ButtonState::INACTIVE),
        Color::Yellow().full);
}

/**
 * @tc.name: UIButtonSetStateForStyle_001
 * @tc.desc: Verify SetStyle function.
 * @tc.type: FUNC
 * @tc.require: SR000DRSH1
 */
HWTEST_F(UIButtonTest, UIButtonSetStateForStyle_001, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetStyleForState(STYLE_BACKGROUND_COLOR, Color::Red().full, UIButton::ButtonState::RELEASED);
    EXPECT_EQ(button_->GetStyleForState(STYLE_BACKGROUND_COLOR, UIButton::ButtonState::RELEASED),
        Color::Red().full);
    button_->SetStyleForState(STYLE_BACKGROUND_COLOR, Color::Green().full, UIButton::ButtonState::PRESSED);
    EXPECT_EQ(button_->GetStyleForState(STYLE_BACKGROUND_COLOR, UIButton::ButtonState::PRESSED),
        Color::Green().full);
    button_->SetStateForStyle(UIButton::ButtonState::RELEASED);
    EXPECT_EQ(button_->GetStyle(STYLE_BACKGROUND_COLOR), Color::Red().full);
    button_->SetStateForStyle(UIButton::ButtonState::PRESSED);
    EXPECT_EQ(button_->GetStyle(STYLE_BACKGROUND_COLOR), Color::Green().full);
}

/**
 * @tc.name: UIButtonSetImageSrc_002
 * @tc.desc: Verify SetImageSrc function.
 */
HWTEST_F(UIButtonTest, UIButtonSetImageSrc_002, TestSize.Level0)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    ImageInfo* defaultImgSrc  = static_cast<ImageInfo*>(UIMalloc(sizeof(ImageInfo)));
    ImageInfo* triggeredImgSrc = static_cast<ImageInfo*>(UIMalloc(sizeof(ImageInfo)));
    button_->SetImageSrc(defaultImgSrc, triggeredImgSrc);
    ASSERT_TRUE(button_->GetCurImageSrc());
    EXPECT_EQ(button_->GetCurImageSrc()->GetSrcType(), IMG_SRC_VARIABLE);
    UIFree(triggeredImgSrc);
    triggeredImgSrc = nullptr;
    UIFree(defaultImgSrc);
    defaultImgSrc = nullptr;
}

/**
 * @tc.name: UIButtonEnableButtonAnimation_001
 * @tc.desc: Verify EnableButtonAnimation function.
 */
HWTEST_F(UIButtonTest, UIButtonEnableButtonAnimation_001, TestSize.Level0)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    bool enable = true;
    button_->EnableButtonAnimation(enable);
    EXPECT_EQ(button_->GetEnableButtonAnimation(), enable);
}

#if GRAPHIC_ENABLE_BUTTON_FLAG
/**
 * @tc.name: UIButtonEnableButtonAnimationCompatibility_001
 * @tc.desc: Verify EnableButtonAnimation maps to the corresponding animation effect.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonEnableButtonAnimationCompatibility_001, TestSize.Level0)
{
    button_->EnableButtonAnimation(false);
    EXPECT_FALSE(button_->GetEnableButtonAnimation());
    EXPECT_EQ(button_->GetAnimationEffect(), UIButton::BUTTON_ANIMATION_NONE);
    button_->EnableButtonAnimation(true);
    EXPECT_TRUE(button_->GetEnableButtonAnimation());
    EXPECT_EQ(button_->GetAnimationEffect(), UIButton::BUTTON_ANIMATION_SCALE);
}

/**
 * @tc.name: UIButtonSetTouchExpand_001
 * @tc.desc: Verify SetTouchExpand and GetTouchExpand function.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonSetTouchExpand_001, TestSize.Level0)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    const int16_t left = 10;
    const int16_t top = 20;
    const int16_t right = 30;
    const int16_t bottom = 40;
    button_->SetTouchExpand(left, top, right, bottom);
    int16_t retLeft = 0;
    int16_t retTop = 0;
    int16_t retRight = 0;
    int16_t retBottom = 0;
    button_->GetTouchExpand(retLeft, retTop, retRight, retBottom);
    EXPECT_EQ(retLeft, left);
    EXPECT_EQ(retTop, top);
    EXPECT_EQ(retRight, right);
    EXPECT_EQ(retBottom, bottom);
}

/**
 * @tc.name: UIButtonSetTouchExpand_002
 * @tc.desc: Verify SetTouchExpand function with invalid values restored to 0.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonSetTouchExpand_002, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetTouchExpand(-1, -1, -1, -1);
    int16_t retLeft = 1;
    int16_t retTop = 1;
    int16_t retRight = 1;
    int16_t retBottom = 1;
    button_->GetTouchExpand(retLeft, retTop, retRight, retBottom);
    EXPECT_EQ(retLeft, 0);
    EXPECT_EQ(retTop, 0);
    EXPECT_EQ(retRight, 0);
    EXPECT_EQ(retBottom, 0);
}

/**
 * @tc.name: UIButtonGetTouchableRect_001
 * @tc.desc: Verify GetTouchableRect function expands the button rect by touch expand values.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonGetTouchableRect_001, TestSize.Level0)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    const int16_t posX = 50;
    const int16_t posY = 100;
    const int16_t width = 40;
    const int16_t height = 30;
    const int16_t left = 10;
    const int16_t top = 20;
    const int16_t right = 30;
    const int16_t bottom = 40;
    button_->SetPosition(posX, posY);
    button_->SetWidth(width);
    button_->SetHeight(height);
    button_->SetTouchExpand(left, top, right, bottom);
    Rect rect = button_->GetTouchableRect();
    EXPECT_EQ(rect.GetLeft(), posX - left);
    EXPECT_EQ(rect.GetTop(), posY - top);
    EXPECT_EQ(rect.GetRight(), posX + width - 1 + right);
    EXPECT_EQ(rect.GetBottom(), posY + height - 1 + bottom);
}
#endif

#if DEFAULT_ANIMATION && GRAPHIC_ENABLE_BUTTON_FLAG
/**
 * @tc.name: UIButtonSetAnimationRepeatCount_001
 * @tc.desc: Verify SetAnimationRepeatCount and GetAnimationRepeatCount function.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonSetAnimationRepeatCount_001, TestSize.Level0)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    const uint16_t repeatCount = 3;
    button_->SetAnimationRepeatCount(repeatCount);
    EXPECT_EQ(button_->GetAnimationRepeatCount(), repeatCount);
    button_->SetAnimationRepeatCount(0);
    EXPECT_EQ(button_->GetAnimationRepeatCount(), 0);
}

/**
 * @tc.name: UIButtonSetAnimationEffect_001
 * @tc.desc: Verify SetAnimationEffect and GetAnimationEffect function.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonSetAnimationEffect_001, TestSize.Level0)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetAnimationEffect(UIButton::BUTTON_ANIMATION_NONE);
    EXPECT_EQ(button_->GetAnimationEffect(), UIButton::BUTTON_ANIMATION_NONE);
    EXPECT_EQ(button_->GetEnableButtonAnimation(), false);
    button_->SetAnimationEffect(UIButton::BUTTON_ANIMATION_SCALE);
    EXPECT_EQ(button_->GetAnimationEffect(), UIButton::BUTTON_ANIMATION_SCALE);
    EXPECT_EQ(button_->GetEnableButtonAnimation(), true);
    button_->SetAnimationEffect(static_cast<UIButton::ButtonAnimationEffect>(2));
    EXPECT_EQ(button_->GetAnimationEffect(), UIButton::BUTTON_ANIMATION_SCALE);
    EXPECT_EQ(button_->GetEnableButtonAnimation(), true);
}

/**
 * @tc.name: UIButtonSetAnimationRepeatCount_002
 * @tc.desc: Verify SetAnimationRepeatCount(0) disables the click animation.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonSetAnimationRepeatCount_002, TestSize.Level0)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetAnimationRepeatCount(3);
    EXPECT_TRUE(button_->GetEnableButtonAnimation());
    button_->SetAnimationRepeatCount(0);
    EXPECT_EQ(button_->GetAnimationRepeatCount(), 0);
    EXPECT_FALSE(button_->GetEnableButtonAnimation());

    PressEvent pressEvent(INIT_POS);
    button_->OnPressEvent(pressEvent);
    ReleaseEvent releaseEvent(INIT_POS);
    button_->OnReleaseEvent(releaseEvent);
}

/**
 * @tc.name: UIButtonSetAnimationEffect_002
 * @tc.desc: Verify SetAnimationEffect(BUTTON_ANIMATION_NONE) disables the click animation.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonSetAnimationEffect_002, TestSize.Level0)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetAnimationEffect(UIButton::BUTTON_ANIMATION_SCALE);
    EXPECT_TRUE(button_->GetEnableButtonAnimation());
    button_->SetAnimationEffect(UIButton::BUTTON_ANIMATION_NONE);
    EXPECT_EQ(button_->GetAnimationEffect(), UIButton::BUTTON_ANIMATION_NONE);
    EXPECT_FALSE(button_->GetEnableButtonAnimation());

    PressEvent pressEvent(INIT_POS);
    button_->OnPressEvent(pressEvent);
    CancelEvent cancelEvent(INIT_POS);
    button_->OnCancelEvent(cancelEvent);
}

/**
 * @tc.name: UIButtonAnimationEventAndDraw_001
 * @tc.desc: Verify enabled animation event paths and pressed-state mask drawing.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonAnimationEventAndDraw_001, TestSize.Level0)
{
    TestUIButton button;
    button.SetPosition(0, 0, 40, 30);
    button.SetAnimationRepeatCount(2);
    button.SetAnimationEffect(UIButton::BUTTON_ANIMATION_SCALE);

    PressEvent pressEvent(INIT_POS);
    button.OnPressEvent(pressEvent);
    ReleaseEvent releaseEvent(INIT_POS);
    button.OnReleaseEvent(releaseEvent);
    button.OnPressEvent(pressEvent);
    CancelEvent cancelEvent(INIT_POS);
    button.OnCancelEvent(cancelEvent);

    MockButtonSoftEngine mockEngine;
    BaseGfxEngine* originEngine = BaseGfxEngine::GetInstance();
    BaseGfxEngine::InitGfxEngine(&mockEngine);
    BufferInfo bufferInfo = {};
    Rect invalidatedArea = button.GetRect();

    button.SetTestState(UIButton::ButtonState::PRESSED);
    EXPECT_CALL(mockEngine, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1);
    button.OnPostDraw(bufferInfo, invalidatedArea);
    testing::Mock::VerifyAndClearExpectations(&mockEngine);

    button.SetTestState(UIButton::ButtonState::RELEASED);
    EXPECT_CALL(mockEngine, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(0);
    button.OnPostDraw(bufferInfo, invalidatedArea);
    BaseGfxEngine::InitGfxEngine(originEngine);
}

/**
 * @tc.name: UIButtonAnimatorState_001
 * @tc.desc: Verify animator callback, reset, repeat, and disabled branches.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonAnimatorState_001, TestSize.Level0)
{
    TestUIButton button;
    button.SetPosition(0, 0, 40, 30);
    button.SetAnimationRepeatCount(2);
    button.SetAnimationEffect(UIButton::BUTTON_ANIMATION_SCALE);
    button.SetTestState(UIButton::ButtonState::PRESSED);
    button.StartAnimator();
    button.RunAnimatorCallback();
    button.RunAnimatorOnStop();

    button.SetTestState(UIButton::ButtonState::RELEASED);
    button.RunAnimatorOnStop();

    button.StartAnimator();
    button.RunAnimatorCallback();
    button.SetAnimationEffect(UIButton::BUTTON_ANIMATION_NONE);
    EXPECT_FALSE(button.GetEnableButtonAnimation());
    button.RunAnimatorCallback();
    button.StartAnimator();

    button.SetAnimationEffect(UIButton::BUTTON_ANIMATION_SCALE);
    button.SetAnimationRepeatCount(0);
    EXPECT_FALSE(button.GetEnableButtonAnimation());
}
#endif

#if GRAPHIC_ENABLE_BUTTON_FLAG
/**
 * @tc.name: UIButtonSetTouchExpand_003
 * @tc.desc: Verify SetTouchExpand function with values greater than screen size restored to 0.
 * @tc.type: FUNC
 */
HWTEST_F(UIButtonTest, UIButtonSetTouchExpand_003, TestSize.Level1)
{
    if (button_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    button_->SetTouchExpand(INT16_MAX, INT16_MAX, INT16_MAX, INT16_MAX);
    int16_t retLeft = 1;
    int16_t retTop = 1;
    int16_t retRight = 1;
    int16_t retBottom = 1;
    button_->GetTouchExpand(retLeft, retTop, retRight, retBottom);
    EXPECT_EQ(retLeft, 0);
    EXPECT_EQ(retTop, 0);
    EXPECT_EQ(retRight, 0);
    EXPECT_EQ(retBottom, 0);
}
#endif
} // namespace OHOS
