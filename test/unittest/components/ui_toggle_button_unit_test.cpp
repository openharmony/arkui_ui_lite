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

#include "components/ui_toggle_button.h"
#include "engines/gfx/gfx_engine_manager.h"
#include "engines/gfx/soft_engine.h"

#include <climits>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing::ext;
namespace OHOS {
#if GRAPHIC_ENABLE_SWITCH_FLAG
constexpr uint16_t DRAW_TEST_WIDTH = 100;
constexpr uint16_t DRAW_TEST_HEIGHT = 40;
constexpr uint16_t EXPECTED_CORNER_RADIUS = 9;
constexpr uint16_t EXPECTED_DEFAULT_THUMB_RADIUS = 8;
constexpr uint16_t EXPECTED_RESIZED_DEFAULT_THUMB_RADIUS = 16;
constexpr uint16_t EXPECTED_ON_THUMB_RADIUS = 9;
constexpr uint16_t EXPECTED_OFF_THUMB_RADIUS = 8;
constexpr uint16_t INVALID_THUMB_RADIUS = UINT16_MAX;

class MockSwitchSoftEngine : public SoftEngine {
public:
    MOCK_METHOD(void, DrawArc,
        (BufferInfo&, ArcInfo&, const Rect&, const Style&, OpacityType, uint8_t), (override));
    MOCK_METHOD(void, DrawRect,
        (BufferInfo&, const Rect&, const Rect&, const Style&, OpacityType), (override));
};

#if DEFAULT_ANIMATION
class TestUIToggleButton : public UIToggleButton {
public:
    void RunAnimationCallback()
    {
        Callback(this);
    }

    void RunAnimationOnStop()
    {
        OnStop(*this);
    }
};
#endif
#endif

class UIToggleButtonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
#if GRAPHIC_ENABLE_SWITCH_FLAG
    void SetUp() override;
    void TearDown() override;
#endif
    static UIToggleButton* toggleBtn_;

#if GRAPHIC_ENABLE_SWITCH_FLAG
protected:
    MockSwitchSoftEngine mockSwitchSoftEngine_;
    BaseGfxEngine* originGfxEngine_ = nullptr;
#endif
};

UIToggleButton* UIToggleButtonTest::toggleBtn_ = nullptr;

void UIToggleButtonTest::SetUpTestCase(void)
{
    if (toggleBtn_ == nullptr) {
        toggleBtn_ = new UIToggleButton();
    }
}

void UIToggleButtonTest::TearDownTestCase(void)
{
    if (toggleBtn_ != nullptr) {
        delete toggleBtn_;
        toggleBtn_ = nullptr;
    }
}

#if GRAPHIC_ENABLE_SWITCH_FLAG
void UIToggleButtonTest::SetUp()
{
    originGfxEngine_ = BaseGfxEngine::GetInstance();
    BaseGfxEngine::InitGfxEngine(&mockSwitchSoftEngine_);
}

void UIToggleButtonTest::TearDown()
{
    BaseGfxEngine::InitGfxEngine(originGfxEngine_);
}
#endif

/**
 * @tc.name: UIToggleButtonGetViewType_001
 * @tc.desc: Verify GetViewType function.
 * @tc.type: FUNC
 * @tc.require: AR000DSMQA
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonGetViewType_001, TestSize.Level1)
{
    if (toggleBtn_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    EXPECT_EQ(toggleBtn_->GetViewType(), UI_TOGGLE_BUTTON);
}

/**
 * @tc.name: UIToggleButtonSetState_001
 * @tc.desc: Verify SetState function.
 * @tc.type: FUNC
 * @tc.require: AR000F4E5H
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonSetState_001, TestSize.Level0)
{
    if (toggleBtn_ == nullptr) {
        EXPECT_NE(0, 0);
        return;
    }
    toggleBtn_->SetState(true);
    EXPECT_EQ(toggleBtn_->GetState(), true);

    toggleBtn_->SetState(false);
    EXPECT_EQ(toggleBtn_->GetState(), false);
}

#if GRAPHIC_ENABLE_SWITCH_FLAG
/**
 * @tc.name: UIToggleButtonDrawDefaultOnThumbStyle_001
 * @tc.desc: Verify default thumb drawing in the ON state.
 * @tc.type: FUNC
 * @tc.require: RM.011 RM.012
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonDrawDefaultOnThumbStyle_001, TestSize.Level0)
{
    BufferInfo bufferInfo = {};
    UIToggleButton button;
    button.SetPosition(0, 0, DRAW_TEST_WIDTH, DRAW_TEST_HEIGHT);
    Rect invalidatedArea = button.GetRect();
    int rectCallCount = 0;

    EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly([&](BufferInfo& dst, const Rect&, const Rect&, const Style& style, OpacityType) {
            EXPECT_EQ(dst.virAddr, bufferInfo.virAddr);
            if (rectCallCount++ == 1) {
                EXPECT_EQ(EXPECTED_CORNER_RADIUS, style.borderRadius_);
                EXPECT_EQ(OPA_OPAQUE, style.bgOpa_);
            }
        });
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .Times(1)
        .WillOnce([&](BufferInfo& dst, ArcInfo& arcInfo, const Rect&, const Style& style, OpacityType, uint8_t) {
            EXPECT_EQ(dst.virAddr, bufferInfo.virAddr);
            EXPECT_EQ(EXPECTED_DEFAULT_THUMB_RADIUS, arcInfo.radius);
            EXPECT_EQ(EXPECTED_DEFAULT_THUMB_RADIUS, style.lineWidth_);
            EXPECT_EQ(Color::White().full, style.lineColor_.full);
            EXPECT_EQ(54, arcInfo.center.x);
            EXPECT_EQ(20, arcInfo.center.y);
        });
    button.SetState(true);
    button.OnDraw(bufferInfo, invalidatedArea);
}

/**
 * @tc.name: UIToggleButtonDrawDefaultOffThumbStyle_001
 * @tc.desc: Verify default thumb drawing in the OFF state.
 * @tc.type: FUNC
 * @tc.require: RM.011 RM.012
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonDrawDefaultOffThumbStyle_001, TestSize.Level0)
{
    BufferInfo bufferInfo = {};
    UIToggleButton button;
    button.SetPosition(0, 0, DRAW_TEST_WIDTH, DRAW_TEST_HEIGHT);
    Rect invalidatedArea = button.GetRect();
    int rectCallCount = 0;

    EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly([&](BufferInfo& dst, const Rect&, const Rect&, const Style& style, OpacityType) {
            EXPECT_EQ(dst.virAddr, bufferInfo.virAddr);
            if (rectCallCount++ == 1) {
                EXPECT_EQ(EXPECTED_CORNER_RADIUS, style.borderRadius_);
                EXPECT_NE(OPA_OPAQUE, style.bgOpa_);
            }
        });
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .Times(1)
        .WillOnce([&](BufferInfo& dst, ArcInfo& arcInfo, const Rect&, const Style& style, OpacityType, uint8_t) {
            EXPECT_EQ(dst.virAddr, bufferInfo.virAddr);
            EXPECT_EQ(EXPECTED_DEFAULT_THUMB_RADIUS, arcInfo.radius);
            EXPECT_EQ(EXPECTED_DEFAULT_THUMB_RADIUS, style.lineWidth_);
            EXPECT_EQ(Color::White().full, style.lineColor_.full);
            EXPECT_EQ(45, arcInfo.center.x);
            EXPECT_EQ(20, arcInfo.center.y);
        });
    button.SetState(false);
    button.OnDraw(bufferInfo, invalidatedArea);
}

/**
 * @tc.name: UIToggleButtonDrawCustomOnThumbStyle_001
 * @tc.desc: Verify custom thumb drawing in the ON state.
 * @tc.type: FUNC
 * @tc.require: RM.011 RM.012
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonDrawCustomOnThumbStyle_001, TestSize.Level0)
{
    BufferInfo bufferInfo = {};
    UIToggleButton button;
    button.SetPosition(0, 0, DRAW_TEST_WIDTH, DRAW_TEST_HEIGHT);
    button.SetOnThumbSize(EXPECTED_ON_THUMB_RADIUS);
    button.SetOnThumbColor(Color::Red());
    button.SetOnBorderColor(Color::Blue());
    Rect invalidatedArea = button.GetRect();
    testing::InSequence sequence;

    EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(2);
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .WillOnce([](BufferInfo&, ArcInfo& arcInfo, const Rect&, const Style& style, OpacityType, uint8_t) {
            EXPECT_EQ(EXPECTED_ON_THUMB_RADIUS, arcInfo.radius);
            EXPECT_EQ(EXPECTED_ON_THUMB_RADIUS, style.lineWidth_);
            EXPECT_EQ(Color::Blue().full, style.lineColor_.full);
        });
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .WillOnce([](BufferInfo&, ArcInfo& arcInfo, const Rect&, const Style& style, OpacityType, uint8_t) {
            EXPECT_EQ(EXPECTED_ON_THUMB_RADIUS - 1, arcInfo.radius);
            EXPECT_EQ(EXPECTED_ON_THUMB_RADIUS - 1, style.lineWidth_);
            EXPECT_EQ(Color::Red().full, style.lineColor_.full);
        });
    button.SetState(true);
    button.OnDraw(bufferInfo, invalidatedArea);
}

/**
 * @tc.name: UIToggleButtonDrawCustomOffThumbStyle_001
 * @tc.desc: Verify custom thumb drawing in the OFF state.
 * @tc.type: FUNC
 * @tc.require: RM.011 RM.012
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonDrawCustomOffThumbStyle_001, TestSize.Level0)
{
    BufferInfo bufferInfo = {};
    UIToggleButton button;
    button.SetPosition(0, 0, DRAW_TEST_WIDTH, DRAW_TEST_HEIGHT);
    button.SetOffThumbSize(EXPECTED_OFF_THUMB_RADIUS);
    button.SetOffThumbColor(Color::Green());
    button.SetOffBorderColor(Color::Black());
    Rect invalidatedArea = button.GetRect();
    testing::InSequence sequence;

    EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(2);
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .WillOnce([](BufferInfo&, ArcInfo& arcInfo, const Rect&, const Style& style, OpacityType, uint8_t) {
            EXPECT_EQ(EXPECTED_OFF_THUMB_RADIUS, arcInfo.radius);
            EXPECT_EQ(EXPECTED_OFF_THUMB_RADIUS, style.lineWidth_);
            EXPECT_EQ(Color::Black().full, style.lineColor_.full);
        });
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .WillOnce([](BufferInfo&, ArcInfo& arcInfo, const Rect&, const Style& style, OpacityType, uint8_t) {
            EXPECT_EQ(EXPECTED_OFF_THUMB_RADIUS - 1, arcInfo.radius);
            EXPECT_EQ(EXPECTED_OFF_THUMB_RADIUS - 1, style.lineWidth_);
            EXPECT_EQ(Color::Green().full, style.lineColor_.full);
        });
    button.SetState(false);
    button.OnDraw(bufferInfo, invalidatedArea);
}

/**
 * @tc.name: UIToggleButtonDrawBoundaryOnThumbSize_001
 * @tc.desc: Verify invalid ON-state thumb sizes and resized fallback drawing.
 * @tc.type: FUNC
 * @tc.require: RM.011
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonDrawBoundaryOnThumbSize_001, TestSize.Level0)
{
    BufferInfo bufferInfo = {};
    UIToggleButton button;
    button.SetPosition(0, 0, DRAW_TEST_WIDTH, DRAW_TEST_HEIGHT);
    Rect invalidatedArea = button.GetRect();
    const uint16_t thumbSizes[] = {0, INVALID_THUMB_RADIUS, 200};

    for (uint16_t thumbSize : thumbSizes) {
        EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_))
            .Times(2);
        EXPECT_CALL(mockSwitchSoftEngine_,
            DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
            .Times(1)
            .WillOnce([](BufferInfo&, ArcInfo& arcInfo, const Rect&, const Style& style, OpacityType, uint8_t) {
                EXPECT_EQ(EXPECTED_DEFAULT_THUMB_RADIUS, arcInfo.radius);
                EXPECT_EQ(EXPECTED_DEFAULT_THUMB_RADIUS, style.lineWidth_);
            });
        button.SetState(true);
        button.SetOnThumbSize(thumbSize);
        button.OnDraw(bufferInfo, invalidatedArea);
        ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(&mockSwitchSoftEngine_));
        EXPECT_EQ(thumbSize, button.GetOnThumbSize());
    }

    EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(2);
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .Times(1)
        .WillOnce([](BufferInfo&, ArcInfo& arcInfo, const Rect&, const Style& style, OpacityType, uint8_t) {
            EXPECT_EQ(EXPECTED_RESIZED_DEFAULT_THUMB_RADIUS, arcInfo.radius);
            EXPECT_EQ(EXPECTED_RESIZED_DEFAULT_THUMB_RADIUS, style.lineWidth_);
        });
    button.SetPosition(0, 0, DRAW_TEST_WIDTH * 2, DRAW_TEST_HEIGHT * 2);
    invalidatedArea = button.GetRect();
    button.SetState(true);
    button.OnDraw(bufferInfo, invalidatedArea);
    ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(&mockSwitchSoftEngine_));
}

/**
 * @tc.name: UIToggleButtonDrawBoundaryOffThumbSize_001
 * @tc.desc: Verify invalid OFF-state thumb sizes in OnDraw.
 * @tc.type: FUNC
 * @tc.require: RM.011
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonDrawBoundaryOffThumbSize_001, TestSize.Level0)
{
    BufferInfo bufferInfo = {};
    UIToggleButton button;
    button.SetPosition(0, 0, DRAW_TEST_WIDTH, DRAW_TEST_HEIGHT);
    Rect invalidatedArea = button.GetRect();
    const uint16_t thumbSizes[] = {0, INVALID_THUMB_RADIUS, 200};

    for (uint16_t thumbSize : thumbSizes) {
        EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_))
            .Times(2);
        EXPECT_CALL(mockSwitchSoftEngine_,
            DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
            .Times(1)
            .WillOnce([](BufferInfo&, ArcInfo& arcInfo, const Rect&, const Style& style, OpacityType, uint8_t) {
                EXPECT_EQ(EXPECTED_DEFAULT_THUMB_RADIUS, arcInfo.radius);
                EXPECT_EQ(EXPECTED_DEFAULT_THUMB_RADIUS, style.lineWidth_);
            });
        button.SetState(false);
        button.SetOffThumbSize(thumbSize);
        button.OnDraw(bufferInfo, invalidatedArea);
        ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(&mockSwitchSoftEngine_));
        EXPECT_EQ(thumbSize, button.GetOffThumbSize());
    }
}

/**
 * @tc.name: UIToggleButtonStyleGetter_001
 * @tc.desc: Verify all thumb color and border color getters.
 * @tc.type: FUNC
 * @tc.require: RM.012
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonStyleGetter_001, TestSize.Level0)
{
    UIToggleButton button;
    button.SetOnThumbColor(Color::Red());
    button.SetOffThumbColor(Color::Green());
    button.SetOnBorderColor(Color::Blue());
    button.SetOffBorderColor(Color::Black());

    EXPECT_EQ(button.GetOnThumbColor().full, Color::Red().full);
    EXPECT_EQ(button.GetOffThumbColor().full, Color::Green().full);
    EXPECT_EQ(button.GetOnBorderColor().full, Color::Blue().full);
    EXPECT_EQ(button.GetOffBorderColor().full, Color::Black().full);
}

/**
 * @tc.name: UIToggleButtonDrawRtlAndSmallRadius_001
 * @tc.desc: Verify RTL positions and the no-border branch for a radius equal to the border width.
 * @tc.type: FUNC
 * @tc.require: RM.011 RM.012
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonDrawRtlAndSmallRadius_001, TestSize.Level0)
{
    BufferInfo bufferInfo = {};
    UIToggleButton button;
    button.SetPosition(0, 0, DRAW_TEST_WIDTH, DRAW_TEST_HEIGHT);
    button.EnableRtl(true);
    EXPECT_TRUE(button.IsRtl());
    Rect invalidatedArea = button.GetRect();

    EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(2);
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .Times(1)
        .WillOnce([](BufferInfo&, ArcInfo& arcInfo, const Rect&, const Style&, OpacityType, uint8_t) {
            EXPECT_EQ(54, arcInfo.center.x);
        });
    button.SetState(false);
    button.OnDraw(bufferInfo, invalidatedArea);
    ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(&mockSwitchSoftEngine_));

    button.SetOnThumbSize(1);
    button.SetOnBorderColor(Color::Blue());
    EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(2);
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .Times(1)
        .WillOnce([](BufferInfo&, ArcInfo& arcInfo, const Rect&, const Style& style, OpacityType, uint8_t) {
            EXPECT_EQ(1, arcInfo.radius);
            EXPECT_EQ(Color::White().full, style.lineColor_.full);
            EXPECT_EQ(45, arcInfo.center.x);
        });
    button.SetState(true);
    button.OnDraw(bufferInfo, invalidatedArea);
}

/**
 * @tc.name: UIToggleButtonRepeatedInvalidSize_001
 * @tc.desc: Verify repeated invalid ON/OFF sizes continue to use the default radius.
 * @tc.type: FUNC
 * @tc.require: RM.011
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonRepeatedInvalidSize_001, TestSize.Level0)
{
    BufferInfo bufferInfo = {};
    UIToggleButton button;
    button.SetPosition(0, 0, DRAW_TEST_WIDTH, DRAW_TEST_HEIGHT);
    Rect invalidatedArea = button.GetRect();
    const bool states[] = {true, false};
    for (bool state : states) {
        button.SetState(state);
        if (state) {
            button.SetOnThumbSize(INVALID_THUMB_RADIUS);
        } else {
            button.SetOffThumbSize(INVALID_THUMB_RADIUS);
        }
        for (uint8_t drawCount = 0; drawCount < 2; drawCount++) {
            EXPECT_CALL(mockSwitchSoftEngine_,
                DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(2);
            EXPECT_CALL(mockSwitchSoftEngine_,
                DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
                .Times(1)
                .WillOnce([](BufferInfo&, ArcInfo& arcInfo, const Rect&, const Style&, OpacityType, uint8_t) {
                    EXPECT_EQ(EXPECTED_DEFAULT_THUMB_RADIUS, arcInfo.radius);
                });
            button.OnDraw(bufferInfo, invalidatedArea);
            ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(&mockSwitchSoftEngine_));
        }
    }
}

#if DEFAULT_ANIMATION
/**
 * @tc.name: UIToggleButtonAnimationColor_001
 * @tc.desc: Verify ON/OFF animation callbacks with default and custom thumb colors.
 * @tc.type: FUNC
 * @tc.require: RM.012
 */
HWTEST_F(UIToggleButtonTest, UIToggleButtonAnimationColor_001, TestSize.Level0)
{
    BufferInfo bufferInfo = {};
    TestUIToggleButton button;
    button.SetPosition(0, 0, DRAW_TEST_WIDTH, DRAW_TEST_HEIGHT);
    button.SetOnThumbColor(Color::Red());
    button.SetOffThumbColor(Color::Green());
    button.SetState(true);
    button.RunAnimationCallback();
    button.RunAnimationOnStop();
    EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(2);
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .Times(1)
        .WillOnce([](BufferInfo&, ArcInfo&, const Rect&, const Style& style, OpacityType, uint8_t) {
            EXPECT_EQ(style.lineColor_.full, Color::Red().full);
        });
    button.OnDraw(bufferInfo, button.GetRect());
    ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(&mockSwitchSoftEngine_));
    button.SetState(false);
    button.RunAnimationCallback();
    button.RunAnimationOnStop();
    EXPECT_CALL(mockSwitchSoftEngine_, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(2);
    EXPECT_CALL(mockSwitchSoftEngine_,
        DrawArc(testing::_, testing::_, testing::_, testing::_, OPA_OPAQUE, CapType::CAP_NONE))
        .Times(1)
        .WillOnce([](BufferInfo&, ArcInfo&, const Rect&, const Style& style, OpacityType, uint8_t) {
            EXPECT_EQ(style.lineColor_.full, Color::Green().full);
        });
    button.OnDraw(bufferInfo, button.GetRect());
}
#endif
#endif
} // namespace OHOS
