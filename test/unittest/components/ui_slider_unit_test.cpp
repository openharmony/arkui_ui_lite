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

#include "components/ui_slider.h"
#include <climits>
#include <gtest/gtest.h>

#if GRAPHIC_ENABLE_SLIDER_FLAG
#include <cstring>
#include <vector>
#include "common/typed_text.h"
#include "engines/gfx/gfx_engine_manager.h"
#include "engines/gfx/soft_engine.h"
#include "font/ui_font.h"
#include "graphic_config.h"
#include "securec.h"
#endif

using namespace testing::ext;
namespace OHOS {
#if GRAPHIC_ENABLE_SLIDER_FLAG
namespace {
constexpr uint16_t SLIDER_WIDTH = 100;
constexpr uint16_t SLIDER_HEIGHT = 40;
constexpr int16_t TRACK_HEIGHT = 10;
constexpr int16_t TRACK_TOP = (SLIDER_HEIGHT - TRACK_HEIGHT) / 2;
constexpr int32_t RANGE_MAX = 100;
constexpr int32_t RANGE_MIN = 0;
constexpr uint8_t TEST_TOAST_FONT_SIZE = 12;
constexpr int16_t DOT_MARKINGS_SIZE = 10;
constexpr int16_t HALF_DIVISOR = 2;
constexpr int16_t GRADIENT_STEP = 8;
constexpr uint8_t GRADIENT_SLICES = 13;
constexpr int16_t GRADIENT_TAIL_WIDTH = 4;
}

class MockSliderSoftEngine : public SoftEngine {
public:
    struct DrawRectParams {
        Rect rect;
        Style style;
    };

    struct DrawLineParams {
        Point start;
        Point end;
        int16_t width;
        ColorType color;
    };

    void DrawRect(BufferInfo&, const Rect& rect, const Rect&, const Style& style, OpacityType) override
    {
        drawRectParams_.push_back({rect, style});
    }

    void DrawLine(BufferInfo&, const Point& start, const Point& end, const Rect&, int16_t width,
                  ColorType color, OpacityType) override
    {
        drawLineParams_.push_back({start, end, width, color});
    }

    void DrawLetter(BufferInfo&, const uint8_t*, const Rect&, const Rect&, const uint8_t, const ColorType&,
                    const OpacityType) override
    {
        drawLetterCount_++;
    }

    void DrawLetter(BufferInfo&, const Rect&, LetterDataInfo&) override
    {
        drawLetterCount_++;
    }

    void DrawLetterPath(BufferInfo&, const Rect&, LetterPathDataInfo&, const TransformMap*,
                        const TransformDataInfo*) override
    {
        drawLetterCount_++;
    }

    void DrawLetterPathWithClip(BufferInfo&, const Rect&, const Rect&, LetterPathDataInfo&, const TransformMap*,
                                const TransformDataInfo*) override
    {
        drawLetterCount_++;
    }

    void DrawArc(BufferInfo&, ArcInfo&, const Rect&, const Style&, OpacityType, uint8_t) override {}

    void DrawCubicBezier(BufferInfo&, const Point&, const Point&, const Point&, const Point&, const Rect&,
                         int16_t, ColorType, OpacityType) override {}

    void DrawTransform(BufferInfo&, const Rect&, const Point&, ColorType, OpacityType, const TransformMap&,
                       const TransformDataInfo&) override {}

    void ClipCircle(const ImageInfo*, float, float, float) override {}

    void ClipScreenShape(const ImageInfo*, float, float, float, float) override {}

    void Blit(BufferInfo&, const Point&, const BufferInfo&, const Rect&, const BlendOption&) override {}

    void Fill(BufferInfo&, const Rect&, const ColorType, const OpacityType) override {}

    void DrawPath(BufferInfo&, void*, const Paint&, const Rect&, const Rect&, const Style&) override {}

    void FillPath(BufferInfo&, void*, const Paint&, const Rect&, const Rect&, const Style&) override {}

    uint8_t* AllocBuffer(uint32_t, uint32_t) override
    {
        return nullptr;
    }

    void FreeBuffer(uint8_t*, uint32_t) override {}

    void Reset()
    {
        drawRectParams_.clear();
        drawLineParams_.clear();
        drawLetterCount_ = 0;
    }

    size_t GetDrawRectCount() const
    {
        return drawRectParams_.size();
    }

    size_t GetDrawLineCount() const
    {
        return drawLineParams_.size();
    }

    uint32_t GetDrawLetterCount() const
    {
        return drawLetterCount_;
    }

    const DrawRectParams& GetDrawRectParams(size_t index) const
    {
        return drawRectParams_.at(index);
    }

    const DrawLineParams& GetDrawLineParams(size_t index) const
    {
        return drawLineParams_.at(index);
    }

private:
    std::vector<DrawRectParams> drawRectParams_;
    std::vector<DrawLineParams> drawLineParams_;
    uint32_t drawLetterCount_ = 0;
};
#endif

class UISliderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
#if GRAPHIC_ENABLE_SLIDER_FLAG
    void SetUp() override;
    void TearDown() override;
#endif
    static UISlider* slider_;

protected:
#if GRAPHIC_ENABLE_SLIDER_FLAG
    MockSliderSoftEngine mockSoftEngine_;
    BaseGfxEngine* originGfxEngine_ = nullptr;

    static void InitSlider(UISlider& slider)
    {
        slider.SetPosition(0, 0, SLIDER_WIDTH, SLIDER_HEIGHT);
        slider.SetValidWidth(SLIDER_WIDTH);
        slider.SetValidHeight(TRACK_HEIGHT);
        slider.SetRange(RANGE_MAX, RANGE_MIN);
    }

    static void InitVerticalSlider(UISlider& slider)
    {
        slider.SetPosition(0, 0, SLIDER_HEIGHT, SLIDER_WIDTH);
        slider.SetValidWidth(TRACK_HEIGHT);
        slider.SetValidHeight(SLIDER_WIDTH);
        slider.SetRange(RANGE_MAX, RANGE_MIN);
    }

    static void InitDrawEnv(BufferInfo& bufInfo, Rect& invalidatedArea)
    {
        bufInfo = {};
        invalidatedArea.SetRect(0, 0, SLIDER_WIDTH - 1, SLIDER_HEIGHT - 1);
    }

    static bool IsToastFontAvailable()
    {
        uint16_t fontId = UIFont::GetInstance()->GetFontId(DEFAULT_VECTOR_FONT_FILENAME);
        return TypedText::GetTextWidth("0", fontId, TEST_TOAST_FONT_SIZE, 1, 0) > 0;
    }

    void DrawAndExpectRectCount(UISlider& slider, BufferInfo& bufInfo, Rect& invalidatedArea, size_t expected)
    {
        mockSoftEngine_.Reset();
        slider.OnDraw(bufInfo, invalidatedArea);
        EXPECT_EQ(mockSoftEngine_.GetDrawRectCount(), expected);
    }

    void ExpectToastDrawn(UISlider& slider, BufferInfo& bufInfo, Rect& invalidatedArea,
                          size_t baseRects, uint32_t baseLetters)
    {
        if (!IsToastFontAvailable()) {
            GTEST_SKIP() << "toast font not available";
        }
        mockSoftEngine_.Reset();
        slider.OnDraw(bufInfo, invalidatedArea);
        EXPECT_EQ(mockSoftEngine_.GetDrawRectCount(), baseRects + 1);
        EXPECT_GT(mockSoftEngine_.GetDrawLetterCount(), baseLetters);
    }

    void ExpectDotMarkings(const int16_t* expectedX, size_t count)
    {
        uint8_t dotCount = 0;
        for (size_t i = 0; i < mockSoftEngine_.GetDrawRectCount(); i++) {
            const MockSliderSoftEngine::DrawRectParams& param = mockSoftEngine_.GetDrawRectParams(i);
            if ((param.rect.GetWidth() != DOT_MARKINGS_SIZE + 1) ||
                (param.rect.GetHeight() != DOT_MARKINGS_SIZE + 1) ||
                (param.style.borderRadius_ != DOT_MARKINGS_SIZE / HALF_DIVISOR) ||
                (param.style.bgColor_.full != Color::Gray().full)) {
                continue;
            }
            EXPECT_EQ(param.rect.GetY(), TRACK_TOP);
            bool xMatched = false;
            for (size_t j = 0; j < count; j++) {
                xMatched = xMatched || (param.rect.GetX() == expectedX[j]);
            }
            EXPECT_TRUE(xMatched);
            dotCount++;
        }
        EXPECT_EQ(dotCount, count);
    }
#endif
};

UISlider* UISliderTest::slider_ = nullptr;

void UISliderTest::SetUpTestCase(void)
{
    if (slider_ == nullptr) {
        slider_ = new UISlider();
    }
}

void UISliderTest::TearDownTestCase(void)
{
    if (slider_ != nullptr) {
        delete slider_;
        slider_ = nullptr;
    }
}

#if GRAPHIC_ENABLE_SLIDER_FLAG
void UISliderTest::SetUp()
{
    originGfxEngine_ = BaseGfxEngine::GetInstance();
    mockSoftEngine_.Reset();
    BaseGfxEngine::InitGfxEngine(&mockSoftEngine_);
}

void UISliderTest::TearDown()
{
    BaseGfxEngine::InitGfxEngine(originGfxEngine_);
}
#endif

/**
 * @tc.name:UISliderGetViewType_001
 * @tc.desc: Verify GetViewType function, equal.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderGetViewType_001, TestSize.Level1)
{
    if (slider_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    EXPECT_EQ(slider_->GetViewType(), UI_SLIDER);
}

/**
 * @tc.name:UISliderSetKnobWidth_001
 * @tc.desc: Verify SetKnobWidth function, equal.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetKnobWidth_001, TestSize.Level0)
{
    if (slider_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    const int16_t width = 10;

    slider_->SetKnobWidth(width);
    EXPECT_EQ(slider_->GetKnobWidth(), width);
}

/**
 * @tc.name:UISliderSetKnobStyle_001
 * @tc.desc: Verify SetKnobStyle function, equal.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetKnobStyle_001, TestSize.Level0)
{
    if (slider_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    slider_->SetKnobStyle(STYLE_BACKGROUND_COLOR, Color::Gray().full);
    EXPECT_EQ(slider_->GetKnobStyle().bgColor_.full, Color::Gray().full);
}

#if GRAPHIC_ENABLE_SLIDER_FLAG
/**
 * @tc.name:UISliderSetDisabled_001
 * @tc.desc: Verify that click/drag events are ignored while disabled and resume after re-enabling.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetDisabled_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);
    BufferInfo bufInfo;
    Rect invalidatedArea;
    InitDrawEnv(bufInfo, invalidatedArea);

    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    const size_t baseRects = mockSoftEngine_.GetDrawRectCount();
    const uint32_t baseLetters = mockSoftEngine_.GetDrawLetterCount();

    EXPECT_EQ(slider.IsDisabled(), false);
    slider.SetDisabled(true);
    EXPECT_EQ(slider.IsDisabled(), true);
    ClickEvent click({50, 20});
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 0);
    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    EXPECT_EQ(mockSoftEngine_.GetDrawRectCount(), baseRects);

    slider.SetDisabledToastMsg("slider disabled");
    PressEvent press({50, 20});
    slider.OnPressEvent(press);
    ExpectToastDrawn(slider, bufInfo, invalidatedArea, baseRects, baseLetters);
    ReleaseEvent release({50, 20});
    slider.OnReleaseEvent(release);
    DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects);

    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 0);
    DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects);

    slider.OnPressEvent(press);
    ExpectToastDrawn(slider, bufInfo, invalidatedArea, baseRects, baseLetters);
    slider.OnReleaseEvent(release);
    DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects);

    DragEvent drag({60, 20}, {50, 20}, {10, 0});
    slider.OnDragEvent(drag);
    EXPECT_EQ(slider.GetValue(), 0);
    ExpectToastDrawn(slider, bufInfo, invalidatedArea, baseRects, baseLetters);
    slider.OnDragEndEvent(drag);
    DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects);

    slider.SetDisabled(false);
    EXPECT_EQ(slider.IsDisabled(), false);
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 50);
}

/**
 * @tc.name:UISliderDisabledDragEndAndRotate_001
 * @tc.desc: Verify disabled drag-end and enabled or disabled rotate event handling.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderDisabledDragEndAndRotate_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);
    DragEvent drag({60, 20}, {50, 20}, {10, 0});
    slider.SetDisabled(true);
    slider.OnDragEndEvent(drag);
    EXPECT_EQ(slider.GetValue(), 0);
#if ENABLE_ROTATE_INPUT
    RotateEvent rotateEvent(-20);
    slider.OnRotateEvent(rotateEvent);
    EXPECT_EQ(slider.GetValue(), 0);
    slider.SetDisabled(false);
    slider.OnRotateEvent(rotateEvent);
    EXPECT_GT(slider.GetValue(), 0);
#endif
}

/**
 * @tc.name:UISliderSetDisabledToastMsg_001
 * @tc.desc: Verify setting, truncating, and clearing the disabled toast message.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetDisabledToastMsg_001, TestSize.Level0)
{
    UISlider slider;
    EXPECT_EQ(slider.GetDisabledToastMsg(), nullptr);

    slider.SetDisabledToastMsg("disabled");
    EXPECT_STREQ(slider.GetDisabledToastMsg(), "disabled");

    char longMsg[UISlider::MAX_DISABLED_TOAST_MSG_LEN + 10];
    errno_t ret = memset_s(longMsg, sizeof(longMsg), 'a', sizeof(longMsg) - 1);
    ASSERT_EQ(ret, EOK);
    longMsg[sizeof(longMsg) - 1] = '\0';
    slider.SetDisabledToastMsg(longMsg);
    EXPECT_EQ(std::strlen(slider.GetDisabledToastMsg()), UISlider::MAX_DISABLED_TOAST_MSG_LEN);

    slider.SetDisabledToastMsg("");
    EXPECT_EQ(slider.GetDisabledToastMsg(), nullptr);
    slider.SetDisabledToastMsg("disabled");
    slider.SetDisabledToastMsg(nullptr);
    EXPECT_EQ(slider.GetDisabledToastMsg(), nullptr);
}

/**
 * @tc.name:UISliderSetValues_001
 * @tc.desc: Verify mark value sorting, deduplication, truncation, and click snapping.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetValues_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);

    int32_t values[5] = {50, 10, 30, 10, 20};
    slider.SetValues(values, 5);
    EXPECT_EQ(slider.GetValuesCount(), 4);

    ClickEvent click({46, 20});
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 50);

    int32_t manyValues[UISlider::MAX_MARK_VALUE_COUNT + 1];
    for (uint16_t i = 0; i <= UISlider::MAX_MARK_VALUE_COUNT; i++) {
        manyValues[i] = i;
    }
    slider.SetValues(manyValues, UISlider::MAX_MARK_VALUE_COUNT + 1);
    EXPECT_EQ(slider.GetValuesCount(), UISlider::MAX_MARK_VALUE_COUNT);

    slider.SetValues(nullptr, 0);
    EXPECT_EQ(slider.GetValuesCount(), 0);
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 46);

    int32_t outOfRange[2] = {200, 300};
    slider.SetValues(outOfRange, 2);
    EXPECT_EQ(slider.GetValuesCount(), 2);
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 46);
}

/**
 * @tc.name:UISliderMixedValuesAndEndpoints_001
 * @tc.desc: Verify mixed out-of-range values and endpoint clamping.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderMixedValuesAndEndpoints_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);
    int32_t mixedValues[3] = {-10, 40, 200};
    slider.SetValues(mixedValues, 3);
    ClickEvent click({46, 20});
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 40);
    ClickEvent minClick({0, 20});
    ClickEvent maxClick({99, 20});
    slider.OnClickEvent(minClick);
    EXPECT_EQ(slider.GetValue(), RANGE_MIN);
    slider.OnClickEvent(maxClick);
    EXPECT_EQ(slider.GetValue(), RANGE_MAX);
}

/**
 * @tc.name:UISliderSetBgGradientColors_001
 * @tc.desc: Verify setting, truncating, and clearing the background gradient colors.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetBgGradientColors_001, TestSize.Level0)
{
    UISlider slider;
    ColorType colors[3];
    colors[0] = Color::Red();
    colors[1] = Color::Green();
    colors[2] = Color::Blue();

    slider.SetBgGradientColors(colors, 3);
    EXPECT_EQ(slider.GetBgGradientColorsCount(), 3);

    ColorType manyColors[UISlider::MAX_GRADIENT_COLOR_COUNT + 1];
    for (uint8_t i = 0; i <= UISlider::MAX_GRADIENT_COLOR_COUNT; i++) {
        manyColors[i] = Color::Red();
    }
    slider.SetBgGradientColors(manyColors, UISlider::MAX_GRADIENT_COLOR_COUNT + 1);
    EXPECT_EQ(slider.GetBgGradientColorsCount(), UISlider::MAX_GRADIENT_COLOR_COUNT);

    slider.SetBgGradientColors(colors, 1);
    EXPECT_EQ(slider.GetBgGradientColorsCount(), 1);

    slider.SetBgGradientColors(nullptr, 3);
    EXPECT_EQ(slider.GetBgGradientColorsCount(), 0);
    slider.SetBgGradientColors(colors, 3);
    slider.SetBgGradientColors(colors, 0);
    EXPECT_EQ(slider.GetBgGradientColorsCount(), 0);
}

/**
 * @tc.name:UISliderSetOnTintGradientColors_001
 * @tc.desc: Verify setting, truncating, and clearing the foreground gradient colors.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetOnTintGradientColors_001, TestSize.Level0)
{
    UISlider slider;
    ColorType colors[2];
    colors[0] = Color::Red();
    colors[1] = Color::Blue();

    slider.SetOnTintGradientColors(colors, 2);
    EXPECT_EQ(slider.GetOnTintGradientColorsCount(), 2);

    ColorType manyColors[UISlider::MAX_GRADIENT_COLOR_COUNT + 1];
    for (uint8_t i = 0; i <= UISlider::MAX_GRADIENT_COLOR_COUNT; i++) {
        manyColors[i] = Color::Blue();
    }
    slider.SetOnTintGradientColors(manyColors, UISlider::MAX_GRADIENT_COLOR_COUNT + 1);
    EXPECT_EQ(slider.GetOnTintGradientColorsCount(), UISlider::MAX_GRADIENT_COLOR_COUNT);

    slider.SetOnTintGradientColors(nullptr, 0);
    EXPECT_EQ(slider.GetOnTintGradientColorsCount(), 0);
    slider.SetOnTintGradientColors(colors, 1);
    EXPECT_EQ(slider.GetOnTintGradientColorsCount(), 1);
}

/**
 * @tc.name:UISliderEnableToast_001
 * @tc.desc: Verify the value toast shows on press/drag and hides on release/drag end.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderEnableToast_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);
    slider.SetValue(20);
    BufferInfo bufInfo;
    Rect invalidatedArea;
    InitDrawEnv(bufInfo, invalidatedArea);

    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    const size_t baseRects = mockSoftEngine_.GetDrawRectCount();
    const uint32_t baseLetters = mockSoftEngine_.GetDrawLetterCount();

    slider.EnableToast(true);
    EXPECT_EQ(slider.IsToastEnabled(), true);

    PressEvent press({50, 20});
    slider.OnPressEvent(press);
    EXPECT_EQ(slider.GetValue(), 50);
    ExpectToastDrawn(slider, bufInfo, invalidatedArea, baseRects, baseLetters);

    ReleaseEvent release({50, 20});
    slider.OnReleaseEvent(release);
    DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects);

    ClickEvent click({50, 20});
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 50);
    DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects);

    slider.OnPressEvent(press);
    ExpectToastDrawn(slider, bufInfo, invalidatedArea, baseRects, baseLetters);
    slider.OnReleaseEvent(release);
    DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects);

    DragEvent drag({60, 20}, {50, 20}, {10, 0});
    slider.OnDragEvent(drag);
    EXPECT_EQ(slider.GetValue(), 60);
    if (IsToastFontAvailable()) {
        DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects + 1);
        DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects + 1);
    }

    DragEvent dragEnd({60, 20}, {60, 20}, {0, 0});
    slider.OnDragEndEvent(dragEnd);
    DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects);

    slider.EnableToast(false);
    EXPECT_EQ(slider.IsToastEnabled(), false);
    slider.OnPressEvent(press);
    DrawAndExpectRectCount(slider, bufInfo, invalidatedArea, baseRects);
}

/**
 * @tc.name:UISliderReMeasureToastErase_001
 * @tc.desc: Verify slider toast erase requests are processed during remeasurement.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderReMeasureToastErase_001, TestSize.Level0)
{
    UISlider firstSlider;
    UISlider secondSlider;
    InitSlider(firstSlider);
    InitSlider(secondSlider);
    firstSlider.SetDisabled(true);
    secondSlider.SetDisabled(true);
    firstSlider.SetDisabledToastMsg("disabled");
    secondSlider.SetDisabledToastMsg("disabled");

    BufferInfo bufInfo;
    Rect invalidatedArea;
    InitDrawEnv(bufInfo, invalidatedArea);
    PressEvent firstPress({40, 20});
    PressEvent secondPress({60, 20});

    firstSlider.OnPressEvent(firstPress);
    secondSlider.OnPressEvent(secondPress);
    mockSoftEngine_.Reset();
    firstSlider.OnDraw(bufInfo, invalidatedArea);
    secondSlider.OnDraw(bufInfo, invalidatedArea);
    if (IsToastFontAvailable()) {
        EXPECT_GT(mockSoftEngine_.GetDrawLetterCount(), 0U);
    }

    firstSlider.ReMeasure();
    secondSlider.ReMeasure();
    mockSoftEngine_.Reset();
    firstSlider.OnDraw(bufInfo, invalidatedArea);
    const size_t firstSliderRects = mockSoftEngine_.GetDrawRectCount();
    const uint32_t firstSliderLetters = mockSoftEngine_.GetDrawLetterCount();
    EXPECT_EQ(firstSliderLetters, 0U);
    mockSoftEngine_.Reset();
    secondSlider.OnDraw(bufInfo, invalidatedArea);
    EXPECT_EQ(mockSoftEngine_.GetDrawRectCount(), firstSliderRects);
    EXPECT_EQ(mockSoftEngine_.GetDrawLetterCount(), 0U);
}

/**
 * @tc.name:UISliderEnableTicks_001
 * @tc.desc: Verify step snapping and its priority against configured mark values.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderEnableTicks_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);
    slider.SetStep(10);

    ClickEvent click({47, 20});
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 47);

    slider.EnableTicks(true);
    EXPECT_EQ(slider.IsTicksEnabled(), true);
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 50);
    ClickEvent click2({44, 20});
    slider.OnClickEvent(click2);
    EXPECT_EQ(slider.GetValue(), 40);

    int32_t values[1] = {45};
    slider.SetValues(values, 1);
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 45);

    slider.EnableTicks(false);
    EXPECT_EQ(slider.IsTicksEnabled(), false);
    slider.SetValues(nullptr, 0);
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 47);
}

/**
 * @tc.name:UISliderZeroStepTicks_001
 * @tc.desc: Verify step zero uses the fallback snapping step.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderZeroStepTicks_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);
    slider.SetStep(0);
    slider.EnableTicks(true);
    ClickEvent click({47, 20});
    slider.OnClickEvent(click);
    EXPECT_EQ(slider.GetValue(), 47);
}

/**
 * @tc.name:UISliderSetShowMarkings_001
 * @tc.desc: Verify LINE/DOT markings drawing by step or configured mark values.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetShowMarkings_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);
    slider.SetValue(50);
    slider.SetStep(10);
    BufferInfo bufInfo;
    Rect invalidatedArea;
    InitDrawEnv(bufInfo, invalidatedArea);

    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    const size_t baseRects = mockSoftEngine_.GetDrawRectCount();
    const size_t baseLines = mockSoftEngine_.GetDrawLineCount();

    EXPECT_EQ(slider.IsMarkingsEnabled(), false);
    slider.SetShowMarkings(true);
    EXPECT_EQ(slider.IsMarkingsEnabled(), true);
    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    EXPECT_EQ(mockSoftEngine_.GetDrawLineCount(), baseLines + 11);
    EXPECT_EQ(mockSoftEngine_.GetDrawRectCount(), baseRects);

    slider.SetStep(25);
    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    EXPECT_EQ(mockSoftEngine_.GetDrawLineCount(), baseLines + 5);

    int32_t values[3] = {10, 50, 90};
    slider.SetValues(values, 3);
    slider.SetMarkingsSize(DOT_MARKINGS_SIZE);
    slider.SetMarkingsType(UISlider::MARKINGS_DOT);
    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    EXPECT_EQ(mockSoftEngine_.GetDrawLineCount(), baseLines);
    const int16_t expectedX[3] = {5, 45, 85};
    ExpectDotMarkings(expectedX, 3);
}

/**
 * @tc.name:UISliderSetMarkingsSize_001
 * @tc.desc: Verify markings size setting and fallback to default on invalid values.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetMarkingsSize_001, TestSize.Level0)
{
    UISlider slider;
    EXPECT_EQ(slider.GetMarkingsSize(), 4);

    slider.SetMarkingsSize(8);
    EXPECT_EQ(slider.GetMarkingsSize(), 8);

    slider.SetMarkingsSize(0);
    EXPECT_EQ(slider.GetMarkingsSize(), 4);
    slider.SetMarkingsSize(-1);
    EXPECT_EQ(slider.GetMarkingsSize(), 4);
    slider.SetMarkingsSize(UISlider::MAX_MARKINGS_SIZE + 1);
    EXPECT_EQ(slider.GetMarkingsSize(), 4);
}

/**
 * @tc.name:UISliderSetShowMarkText_001
 * @tc.desc: Verify mark text drawing and the equivalent alias APIs.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetShowMarkText_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);
    int32_t values[3] = {10, 50, 90};
    slider.SetValues(values, 3);
    BufferInfo bufInfo;
    Rect invalidatedArea;
    InitDrawEnv(bufInfo, invalidatedArea);

    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    const uint32_t baseLetters = mockSoftEngine_.GetDrawLetterCount();

    EXPECT_EQ(slider.IsMarkTextEnabled(), false);
    slider.SetShowMarkText(true);
    EXPECT_EQ(slider.IsMarkTextEnabled(), true);
    if (IsToastFontAvailable()) {
        mockSoftEngine_.Reset();
        slider.OnDraw(bufInfo, invalidatedArea);
        EXPECT_GT(mockSoftEngine_.GetDrawLetterCount(), baseLetters);
    }
    slider.SetShowMarkText(false);
    EXPECT_EQ(slider.IsMarkTextEnabled(), false);

    slider.EnableMarkText(true);
    EXPECT_EQ(slider.IsMarkTextEnabled(), true);
    slider.EnableMarkText(false);
    EXPECT_EQ(slider.IsMarkTextEnabled(), false);

    slider.EnableMarkings(true);
    EXPECT_EQ(slider.IsMarkingsEnabled(), true);
    slider.EnableMarkings(false);
    EXPECT_EQ(slider.IsMarkingsEnabled(), false);
}

/**
 * @tc.name:UISliderSetExpandClickArea_001
 * @tc.desc: Verify that clicks outside the track take effect only after click area expansion.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetExpandClickArea_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);
    ClickEvent outOfTrack({50, 2});

    EXPECT_EQ(slider.IsExpandClickAreaEnabled(), false);
    slider.OnClickEvent(outOfTrack);
    EXPECT_EQ(slider.GetValue(), 0);

    slider.SetExpandClickArea(true);
    EXPECT_EQ(slider.IsExpandClickAreaEnabled(), true);
    slider.OnClickEvent(outOfTrack);
    EXPECT_EQ(slider.GetValue(), 50);

    slider.SetExpandClickArea(false);
    slider.SetValue(0);
    slider.OnClickEvent(outOfTrack);
    EXPECT_EQ(slider.GetValue(), 0);
}

/**
 * @tc.name:UISliderSetMarkingsType_001
 * @tc.desc: Verify markings type switching and fallback to default on invalid values.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderSetMarkingsType_001, TestSize.Level0)
{
    UISlider slider;
    EXPECT_EQ(slider.GetMarkingsType(), UISlider::MARKINGS_LINE);

    slider.SetMarkingsType(UISlider::MARKINGS_DOT);
    EXPECT_EQ(slider.GetMarkingsType(), UISlider::MARKINGS_DOT);

    slider.SetMarkingsType(static_cast<UISlider::MarkingsType>(100));
    EXPECT_EQ(slider.GetMarkingsType(), UISlider::MARKINGS_LINE);
}

/**
 * @tc.name:UISliderDrawGradient_001
 * @tc.desc: Verify the background track is drawn as gradient slices.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderDrawGradient_001, TestSize.Level0)
{
    UISlider slider;
    InitSlider(slider);
    slider.SetValue(50);
    BufferInfo bufInfo;
    Rect invalidatedArea;
    InitDrawEnv(bufInfo, invalidatedArea);

    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    const size_t baseRects = mockSoftEngine_.GetDrawRectCount();

    ColorType colors[2];
    colors[0] = Color::Red();
    colors[1] = Color::Blue();
    slider.SetBgGradientColors(colors, 2);
    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    EXPECT_EQ(mockSoftEngine_.GetDrawRectCount(), baseRects + 12);

    uint8_t sliceCount = 0;
    bool firstSliceFound = false;
    ColorType firstSliceColor = Color::GetColorFromRGBA(245, 0, 10, 0xFF);
    for (size_t i = 0; i < mockSoftEngine_.GetDrawRectCount(); i++) {
        const MockSliderSoftEngine::DrawRectParams& param = mockSoftEngine_.GetDrawRectParams(i);
        if ((param.rect.GetHeight() == TRACK_HEIGHT) && (param.rect.GetY() == TRACK_TOP) &&
            ((param.rect.GetWidth() == GRADIENT_STEP) || (param.rect.GetWidth() == GRADIENT_TAIL_WIDTH))) {
            if (param.rect.GetX() == 0) {
                EXPECT_EQ(param.style.bgColor_.full, firstSliceColor.full);
                firstSliceFound = true;
            }
            sliceCount++;
        }
    }
    EXPECT_EQ(sliceCount, GRADIENT_SLICES);
    EXPECT_TRUE(firstSliceFound);

    slider.SetBgGradientColors(nullptr, 0);
    mockSoftEngine_.Reset();
    slider.OnDraw(bufInfo, invalidatedArea);
    EXPECT_EQ(mockSoftEngine_.GetDrawRectCount(), baseRects);
}

/**
 * @tc.name:UISliderDirectionAndForegroundGradient_001
 * @tc.desc: Verify reverse/vertical value calculation and selected-track gradient drawing.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderDirectionAndForegroundGradient_001, TestSize.Level0)
{
    UISlider horizontal;
    InitSlider(horizontal);
    horizontal.SetDirection(UISlider::Direction::DIR_RIGHT_TO_LEFT);
    horizontal.OnClickEvent(ClickEvent({0, 20}));
    EXPECT_EQ(horizontal.GetValue(), RANGE_MAX);
    horizontal.OnClickEvent(ClickEvent({99, 20}));
    EXPECT_EQ(horizontal.GetValue(), RANGE_MIN);

    UISlider vertical;
    InitVerticalSlider(vertical);
    vertical.SetDirection(UISlider::Direction::DIR_TOP_TO_BOTTOM);
    vertical.OnClickEvent(ClickEvent({20, 0}));
    EXPECT_EQ(vertical.GetValue(), RANGE_MIN);
    vertical.OnClickEvent(ClickEvent({20, 99}));
    EXPECT_EQ(vertical.GetValue(), RANGE_MAX);
    vertical.SetDirection(UISlider::Direction::DIR_BOTTOM_TO_TOP);
    vertical.OnClickEvent(ClickEvent({20, 0}));
    EXPECT_EQ(vertical.GetValue(), RANGE_MAX);
    vertical.OnClickEvent(ClickEvent({20, 99}));
    EXPECT_EQ(vertical.GetValue(), RANGE_MIN);

    horizontal.SetDirection(UISlider::Direction::DIR_LEFT_TO_RIGHT);
    horizontal.SetValue(50);
    BufferInfo bufferInfo;
    Rect invalidatedArea;
    InitDrawEnv(bufferInfo, invalidatedArea);
    mockSoftEngine_.Reset();
    horizontal.OnDraw(bufferInfo, invalidatedArea);
    const size_t baseRects = mockSoftEngine_.GetDrawRectCount();
    ColorType colors[2] = {Color::Green(), Color::Blue()};
    horizontal.SetOnTintGradientColors(colors, 2);
    mockSoftEngine_.Reset();
    horizontal.OnDraw(bufferInfo, invalidatedArea);
    EXPECT_GT(mockSoftEngine_.GetDrawRectCount(), baseRects);
    mockSoftEngine_.Reset();
    horizontal.OnDraw(bufferInfo, Rect(200, 200, 220, 220));

    vertical.SetValue(50);
    Rect verticalArea(0, 0, SLIDER_HEIGHT - 1, SLIDER_WIDTH - 1);
    vertical.SetOnTintGradientColors(nullptr, 0);
    mockSoftEngine_.Reset();
    vertical.OnDraw(bufferInfo, verticalArea);
    const size_t verticalBaseRects = mockSoftEngine_.GetDrawRectCount();
    vertical.SetOnTintGradientColors(colors, 2);
    mockSoftEngine_.Reset();
    vertical.OnDraw(bufferInfo, verticalArea);
    EXPECT_GT(mockSoftEngine_.GetDrawRectCount(), verticalBaseRects);
}

/**
 * @tc.name:UISliderVerticalMarkingsAndText_001
 * @tc.desc: Verify vertical/reverse markings and step-generated mark text paths.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UISliderTest, UISliderVerticalMarkingsAndText_001, TestSize.Level0)
{
    UISlider slider;
    InitVerticalSlider(slider);
    slider.SetDirection(UISlider::Direction::DIR_BOTTOM_TO_TOP);
    slider.SetStep(25);
    slider.SetShowMarkings(true);
    slider.SetShowMarkText(true);
    BufferInfo bufferInfo = {};
    Rect invalidatedArea(0, 0, SLIDER_HEIGHT - 1, SLIDER_WIDTH - 1);

    mockSoftEngine_.Reset();
    slider.OnDraw(bufferInfo, invalidatedArea);
    EXPECT_GT(mockSoftEngine_.GetDrawLineCount(), 0U);
    if (IsToastFontAvailable()) {
        EXPECT_GT(mockSoftEngine_.GetDrawLetterCount(), 0U);
    }

    int32_t values[3] = {10, 50, 90};
    slider.SetValues(values, 3);
    slider.SetMarkingsType(UISlider::MARKINGS_DOT);
    mockSoftEngine_.Reset();
    slider.OnDraw(bufferInfo, invalidatedArea);
    EXPECT_GT(mockSoftEngine_.GetDrawRectCount(), 0U);
}
#endif
} // namespace OHOS
