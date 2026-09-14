/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <climits>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "components/ui_arc_scroll_bar.h"
#include "components/ui_box_scroll_bar.h"
#include "components/ui_list.h"
#include "components/ui_scroll_view.h"
#include "engines/gfx/gfx_engine_manager.h"
#include "engines/gfx/soft_engine.h"

using testing::ext::TestSize;

namespace OHOS {
#if GRAPHIC_ENABLE_SCROLL_FLAG
class MockScrollSoftEngine : public SoftEngine {
public:
    MOCK_METHOD(void, DrawArc,
        (BufferInfo&, ArcInfo&, const Rect&, const Style&, OpacityType, uint8_t), (override));
    MOCK_METHOD(void, DrawRect,
        (BufferInfo&, const Rect&, const Rect&, const Style&, OpacityType), (override));
};

class GfxEngineRestoreGuard final {
public:
    explicit GfxEngineRestoreGuard(BaseGfxEngine& replacement) : original_(BaseGfxEngine::GetInstance())
    {
        BaseGfxEngine::InitGfxEngine(&replacement);
    }

    ~GfxEngineRestoreGuard()
    {
        BaseGfxEngine::InitGfxEngine(original_);
    }

    GfxEngineRestoreGuard(const GfxEngineRestoreGuard&) = delete;
    GfxEngineRestoreGuard& operator=(const GfxEngineRestoreGuard&) = delete;

private:
    BaseGfxEngine* original_;
};
#endif

class ScrollBarTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void ScrollBarTest::SetUpTestCase() {}

void ScrollBarTest::TearDownTestCase() {}

class TestArcScrollBar : public UIArcScrollBar {
public:
    int16_t GetWidth() const
    {
        return width_;
    }

    int16_t GetRadius() const
    {
        return radius_;
    }

    const Style* GetBackStyle() const
    {
        return backgroundStyle_;
    }

    const Style* GetForeStyle() const
    {
        return foregroundStyle_;
    }

    float GetProportion() const
    {
        return foregroundProportion_;
    }

    uint8_t GetOpacity() const
    {
        return opacity_;
    }

#if GRAPHIC_ENABLE_SCROLL_FLAG
    Point GetCenter() const
    {
        return center_;
    }
#endif
};

class TestBoxScrollBar : public UIBoxScrollBar {
public:
    int16_t GetWidth() const
    {
        return backgroundRect_.GetWidth();
    }

    int16_t GetHeight() const
    {
        return backgroundRect_.GetHeight();
    }

    const Style* GetBackStyle() const
    {
        return backgroundStyle_;
    }

    const Style* GetForeStyle() const
    {
        return foregroundStyle_;
    }

    float GetProportion() const
    {
        return foregroundProportion_;
    }

    uint8_t GetOpacity() const
    {
        return opacity_;
    }
};

#if GRAPHIC_ENABLE_SCROLL_FLAG
class TestScrollBar : public UIAbstractScrollBar {
public:
    float GetForegroundProportion() const
    {
        return foregroundProportion_;
    }

    float GetScrollProgress() const
    {
        return scrollProgress_;
    }

    uint8_t GetBarBackgroundOpacity(uint8_t backgroundOpa) const
    {
        return GetBackgroundOpacity(backgroundOpa);
    }

    uint8_t GetBarSliderOpacity(uint8_t backgroundOpa) const
    {
        return GetSliderOpacity(backgroundOpa);
    }
};
#endif

class TestUIScrollView : public UIScrollView {
public:
    bool GetXScrollBarVisible() const
    {
        return xScrollBarVisible_;
    }

    bool GetYScrollBarVisible() const
    {
        return yScrollBarVisible_;
    }
#if GRAPHIC_ENABLE_SCROLL_FLAG

    UIAbstractScrollBar* GetXScrollBar() const
    {
        return xScrollBar_;
    }

    UIAbstractScrollBar* GetYScrollBar() const
    {
        return yScrollBar_;
    }

    uint16_t GetScrollBarWidthForTest() const
    {
        return GetScrollBarWidth();
    }
#endif
};

#if GRAPHIC_ENABLE_SCROLL_FLAG
class TestUIAbstractScroll : public UIAbstractScroll {
public:
    bool GetXScrollBarVisible() const
    {
        return xScrollBarVisible_;
    }

    bool GetYScrollBarVisible() const
    {
        return yScrollBarVisible_;
    }

    UIAbstractScrollBar* GetXScrollBar() const
    {
        return xScrollBar_;
    }

    UIAbstractScrollBar* GetYScrollBar() const
    {
        return yScrollBar_;
    }

    void CallRefreshScrollBarParams()
    {
        RefreshScrollBarParams();
    }

    void SetTestYScrollBar(UIAbstractScrollBar* bar)
    {
        if (bar == yScrollBar_) {
            return;
        }
        if (yScrollBar_ != nullptr) {
            delete yScrollBar_;
        }
        // Ownership of the heap-allocated test bar is transferred to the scroll view.
        yScrollBar_ = bar;
    }

    void SetTestXScrollBar(UIAbstractScrollBar* bar)
    {
        if (bar == xScrollBar_) {
            return;
        }
        if (xScrollBar_ != nullptr) {
            delete xScrollBar_;
        }
        // Ownership of the heap-allocated test bar is transferred to the scroll view.
        xScrollBar_ = bar;
    }

    void CallSyncIndicatorStyle(UIAbstractScrollBar* bar)
    {
        SyncIndicatorStyle(bar);
    }

    void CallDrawScrollBarOnRect(BufferInfo& bufferInfo, const Rect& invalidatedArea, const Rect& scrollRect)
    {
        DrawScrollBarOnRect(bufferInfo, invalidatedArea, scrollRect, OPA_OPAQUE);
    }

    void CallDrawScrollBarOnCircle(BufferInfo& bufferInfo, const Rect& invalidatedArea, const Rect& scrollRect)
    {
        DrawScrollBarOnCircle(bufferInfo, invalidatedArea, scrollRect, OPA_OPAQUE);
    }

    void CallDrawScrollBars(BufferInfo& bufferInfo, const Rect& invalidatedArea, const Rect& scrollRect)
    {
        DrawScrollBars(bufferInfo, invalidatedArea, scrollRect, OPA_OPAQUE);
    }

protected:
    bool DragXInner(int16_t distance) override
    {
        // This test stub only needs to accept the drag path.
        (void)distance;
        return true;
    }

    bool DragYInner(int16_t distance) override
    {
        // This test stub only needs to accept the drag path.
        (void)distance;
        return true;
    }
};
#endif

class TestUIList : public UIList {
public:
    bool GetXScrollBarVisible() const
    {
        return xScrollBarVisible_;
    }

    bool GetYScrollBarVisible() const
    {
        return yScrollBarVisible_;
    }
};

HWTEST_F(ScrollBarTest, UIScrollBarSetXScrollBarVisible, TestSize.Level0)
{
    TestUIScrollView scrollView;
    TestUIList uiList;

    EXPECT_FALSE(scrollView.GetXScrollBarVisible());
    EXPECT_FALSE(uiList.GetXScrollBarVisible());
    BaseGfxEngine::GetInstance()->SetScreenShape(ScreenShape::CIRCLE);
    scrollView.SetXScrollBarVisible(true);
    uiList.SetXScrollBarVisible(true);
    EXPECT_FALSE(scrollView.GetXScrollBarVisible());
    EXPECT_FALSE(uiList.GetXScrollBarVisible());

    BaseGfxEngine::GetInstance()->SetScreenShape(ScreenShape::RECTANGLE);
    scrollView.SetXScrollBarVisible(true);
    uiList.SetXScrollBarVisible(true);
    EXPECT_TRUE(scrollView.GetXScrollBarVisible());
    EXPECT_TRUE(uiList.GetXScrollBarVisible());
}

HWTEST_F(ScrollBarTest, UIScrollBarSetYScrollBarVisible, TestSize.Level0)
{
    TestUIScrollView scrollView;
    TestUIList uiList;

    EXPECT_FALSE(scrollView.GetYScrollBarVisible());
    EXPECT_FALSE(uiList.GetYScrollBarVisible());
    scrollView.SetYScrollBarVisible(true);
    uiList.SetYScrollBarVisible(true);
    EXPECT_TRUE(scrollView.GetYScrollBarVisible());
    EXPECT_TRUE(uiList.GetYScrollBarVisible());
}

HWTEST_F(ScrollBarTest, UIScrollBarSetPosition, TestSize.Level0)
{
    constexpr int16_t VALID_POSITION = 5;
    constexpr int16_t VALID_LEN = 5;
    constexpr int16_t NEGATIVE_LEN = -5;
    constexpr int16_t ZERO_LEN = 0;

    TestArcScrollBar arcBar;
    arcBar.SetPosition(VALID_POSITION, VALID_POSITION, VALID_LEN, VALID_LEN);
    EXPECT_EQ(arcBar.GetWidth(), VALID_LEN);
    EXPECT_EQ(arcBar.GetRadius(), VALID_LEN);

    TestBoxScrollBar boxBar;
    boxBar.SetPosition(VALID_POSITION, VALID_POSITION, VALID_LEN, VALID_LEN);
    EXPECT_EQ(boxBar.GetWidth(), VALID_LEN);
    EXPECT_EQ(boxBar.GetHeight(), VALID_LEN);

    arcBar.SetPosition(VALID_POSITION, VALID_POSITION, VALID_LEN, NEGATIVE_LEN);
    EXPECT_NE(arcBar.GetRadius(), NEGATIVE_LEN);

    boxBar.SetPosition(VALID_POSITION, VALID_POSITION, VALID_LEN, NEGATIVE_LEN);
    EXPECT_NE(boxBar.GetHeight(), NEGATIVE_LEN);

    arcBar.SetPosition(VALID_POSITION, VALID_POSITION, ZERO_LEN, VALID_LEN);
    EXPECT_NE(arcBar.GetWidth(), ZERO_LEN);

    boxBar.SetPosition(VALID_POSITION, VALID_POSITION, ZERO_LEN, VALID_LEN);
    EXPECT_NE(boxBar.GetWidth(), ZERO_LEN);
}

HWTEST_F(ScrollBarTest, UIScrollBarGetBarStyle, TestSize.Level0)
{
    Style& defaultBackStyle = StyleDefault::GetScrollBarBackgroundStyle();
    Style& defaultForeStyle = StyleDefault::GetScrollBarForegroundStyle();

    TestArcScrollBar arcBar;
    TestBoxScrollBar boxBar;

    EXPECT_EQ(arcBar.GetBackStyle()->GetStyle(STYLE_LINE_COLOR), defaultBackStyle.GetStyle(STYLE_LINE_COLOR));
    EXPECT_EQ(arcBar.GetForeStyle()->GetStyle(STYLE_LINE_COLOR), defaultForeStyle.GetStyle(STYLE_LINE_COLOR));

    EXPECT_EQ(arcBar.GetBackStyle()->GetStyle(STYLE_LINE_OPA), defaultBackStyle.GetStyle(STYLE_LINE_OPA));
    EXPECT_EQ(arcBar.GetForeStyle()->GetStyle(STYLE_LINE_OPA), defaultForeStyle.GetStyle(STYLE_LINE_OPA));

    EXPECT_EQ(boxBar.GetBackStyle()->GetStyle(STYLE_BACKGROUND_COLOR),
              defaultBackStyle.GetStyle(STYLE_BACKGROUND_COLOR));
    EXPECT_EQ(boxBar.GetForeStyle()->GetStyle(STYLE_BACKGROUND_COLOR),
              defaultForeStyle.GetStyle(STYLE_BACKGROUND_COLOR));

    EXPECT_EQ(boxBar.GetBackStyle()->GetStyle(STYLE_BACKGROUND_OPA), defaultBackStyle.GetStyle(STYLE_BACKGROUND_OPA));
    EXPECT_EQ(boxBar.GetForeStyle()->GetStyle(STYLE_BACKGROUND_OPA), defaultForeStyle.GetStyle(STYLE_BACKGROUND_OPA));
}

HWTEST_F(ScrollBarTest, UIScrollBarSetForegroundProportion, TestSize.Level0)
{
    TestArcScrollBar arcBar;
    TestBoxScrollBar boxBar;

    float proportion[] = {0, 0.5, 1};
    for (size_t i = 0; i < sizeof(proportion) / sizeof(proportion[0]); i++) {
        arcBar.SetForegroundProportion(proportion[i]);
        EXPECT_EQ(arcBar.GetProportion(), proportion[i]);

        boxBar.SetForegroundProportion(proportion[i]);
        EXPECT_EQ(boxBar.GetProportion(), proportion[i]);
    }

    float invalidProportion[] = {1.5, -0.5};
    for (size_t i = 0; i < sizeof(invalidProportion) / sizeof(invalidProportion[0]); i++) {
        arcBar.SetForegroundProportion(invalidProportion[i]);
        EXPECT_NE(arcBar.GetProportion(), invalidProportion[i]);

        boxBar.SetForegroundProportion(invalidProportion[i]);
        EXPECT_NE(boxBar.GetProportion(), invalidProportion[i]);
    }
}

HWTEST_F(ScrollBarTest, UIScrollBarSetOpacity, TestSize.Level0)
{
    TestArcScrollBar arcBar;
    TestBoxScrollBar boxBar;

    uint8_t opa[] = {0, 255};
    for (size_t i = 0; i < sizeof(opa) / sizeof(opa[0]); i++) {
        arcBar.SetOpacity(opa[i]);
        EXPECT_EQ(arcBar.GetOpacity(), opa[i]);

        boxBar.SetOpacity(opa[i]);
        EXPECT_EQ(boxBar.GetOpacity(), opa[i]);
    }
}

#if GRAPHIC_ENABLE_SCROLL_FLAG
/**
 * @tc.name: UIScrollBarSetIndicatorWidth
 * @tc.desc: Verify setting indicator width and invalid value fallback.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarSetIndicatorWidth, TestSize.Level0)
{
    TestArcScrollBar arcBar;
    TestBoxScrollBar boxBar;

    uint16_t width = 8;
    arcBar.SetIndicatorWidth(width);
    EXPECT_EQ(arcBar.GetIndicatorWidth(), width);
    boxBar.SetIndicatorWidth(width);
    EXPECT_EQ(boxBar.GetIndicatorWidth(), width);

    arcBar.SetIndicatorWidth(0);
    EXPECT_EQ(arcBar.GetIndicatorWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
    boxBar.SetIndicatorWidth(0);
    EXPECT_EQ(boxBar.GetIndicatorWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);

    arcBar.SetIndicatorWidth(INT16_MAX + 1);
    EXPECT_EQ(arcBar.GetIndicatorWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
    boxBar.SetIndicatorWidth(INT16_MAX + 1);
    EXPECT_EQ(boxBar.GetIndicatorWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
}

/**
 * @tc.name: UIScrollBarSetIndicatorColor
 * @tc.desc: Verify setting the indicator color.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarSetIndicatorColor, TestSize.Level0)
{
    TestArcScrollBar arcBar;
    TestBoxScrollBar boxBar;

    ColorType color = Color::Red();
    arcBar.SetIndicatorColor(color);
    EXPECT_EQ(arcBar.GetIndicatorColor().full, color.full);
    boxBar.SetIndicatorColor(color);
    EXPECT_EQ(boxBar.GetIndicatorColor().full, color.full);
}

/**
 * @tc.name: UIScrollBarSetIndicatorBorderRadius
 * @tc.desc: Verify setting and clearing the indicator border radius.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarSetIndicatorBorderRadius, TestSize.Level0)
{
    TestArcScrollBar arcBar;
    TestBoxScrollBar boxBar;

    uint16_t borderRadius = 4;
    arcBar.SetIndicatorBorderRadius(borderRadius);
    EXPECT_EQ(arcBar.GetIndicatorBorderRadius(), borderRadius);
    boxBar.SetIndicatorBorderRadius(borderRadius);
    EXPECT_EQ(boxBar.GetIndicatorBorderRadius(), borderRadius);

    arcBar.SetIndicatorBorderRadius(0);
    EXPECT_EQ(arcBar.GetIndicatorBorderRadius(), 0);
    boxBar.SetIndicatorBorderRadius(0);
    EXPECT_EQ(boxBar.GetIndicatorBorderRadius(), 0);
}

/**
 * @tc.name: UIScrollBarSetIndicatorMinLength
 * @tc.desc: Verify setting indicator minimum length and invalid value fallback.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarSetIndicatorMinLength, TestSize.Level0)
{
    TestArcScrollBar arcBar;
    TestBoxScrollBar boxBar;

    uint16_t minLength = 20;
    arcBar.SetIndicatorMinLength(minLength);
    EXPECT_EQ(arcBar.GetIndicatorMinLength(), minLength);
    boxBar.SetIndicatorMinLength(minLength);
    EXPECT_EQ(boxBar.GetIndicatorMinLength(), minLength);

    arcBar.SetIndicatorMinLength(0);
    EXPECT_EQ(arcBar.GetIndicatorMinLength(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
    boxBar.SetIndicatorMinLength(0);
    EXPECT_EQ(boxBar.GetIndicatorMinLength(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);

    arcBar.SetIndicatorMinLength(INT16_MAX + 1);
    EXPECT_EQ(arcBar.GetIndicatorMinLength(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
    boxBar.SetIndicatorMinLength(INT16_MAX + 1);
    EXPECT_EQ(boxBar.GetIndicatorMinLength(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
}

/**
 * @tc.name: UIScrollBarSetIndicatorOpacity
 * @tc.desc: Verify setting the indicator opacity.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarSetIndicatorOpacity, TestSize.Level0)
{
    TestArcScrollBar arcBar;
    TestBoxScrollBar boxBar;

    uint8_t opa[] = {0, 128, 255};
    for (size_t i = 0; i < sizeof(opa) / sizeof(opa[0]); i++) {
        arcBar.SetIndicatorOpacity(opa[i]);
        EXPECT_EQ(arcBar.GetIndicatorOpacity(), opa[i]);

        boxBar.SetIndicatorOpacity(opa[i]);
        EXPECT_EQ(boxBar.GetIndicatorOpacity(), opa[i]);
    }
}

/**
 * @tc.name: UIScrollBarResetIndicatorStyle
 * @tc.desc: Verify resetting the indicator style.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarResetIndicatorStyle, TestSize.Level0)
{
    TestArcScrollBar arcBar;
    TestBoxScrollBar boxBar;

    uint16_t width = 8;
    uint16_t minLength = 20;
    uint8_t opacity = 100;
    arcBar.SetIndicatorWidth(width);
    arcBar.SetIndicatorMinLength(minLength);
    arcBar.SetIndicatorOpacity(opacity);
    boxBar.SetIndicatorWidth(width);
    boxBar.SetIndicatorMinLength(minLength);
    boxBar.SetIndicatorOpacity(opacity);

    arcBar.ResetIndicatorStyle();
    EXPECT_EQ(arcBar.GetIndicatorWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
    EXPECT_EQ(arcBar.GetIndicatorMinLength(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
    EXPECT_EQ(arcBar.GetIndicatorOpacity(), OPA_OPAQUE);

    boxBar.ResetIndicatorStyle();
    EXPECT_EQ(boxBar.GetIndicatorWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
    EXPECT_EQ(boxBar.GetIndicatorMinLength(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
    EXPECT_EQ(boxBar.GetIndicatorOpacity(), OPA_OPAQUE);
}

/**
 * @tc.name: UIScrollViewSetIndicatorStyle
 * @tc.desc: Verify setting the complete scroll indicator style.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollViewSetIndicatorStyle, TestSize.Level0)
{
    UIScrollView scrollView;

    ScrollIndicatorStyle style = {8, 20, 4, Color::Red(), 200};
    scrollView.SetIndicatorStyle(style);
    ScrollIndicatorStyle ret = scrollView.GetIndicatorStyle();
    EXPECT_EQ(ret.width, style.width);
    EXPECT_EQ(ret.minLength, style.minLength);
    EXPECT_EQ(ret.borderRadius, style.borderRadius);
    EXPECT_EQ(ret.color.full, style.color.full);
    EXPECT_EQ(ret.opacity, style.opacity);

    ScrollIndicatorStyle invalidStyle = {0, 0, 0, Color::Red(), OPA_OPAQUE};
    scrollView.SetIndicatorStyle(invalidStyle);
    EXPECT_EQ(scrollView.GetIndicatorStyle().width, UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
    EXPECT_EQ(scrollView.GetIndicatorStyle().minLength, UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
}

/**
 * @tc.name: UIScrollViewSetIndicatorAttribute
 * @tc.desc: Verify setting individual scroll indicator attributes.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollViewSetIndicatorAttribute, TestSize.Level0)
{
    UIScrollView scrollView;

    uint16_t width = 8;
    scrollView.SetIndicatorWidth(width);
    EXPECT_EQ(scrollView.GetIndicatorWidth(), width);
    scrollView.SetIndicatorWidth(0);
    EXPECT_EQ(scrollView.GetIndicatorWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);

    uint16_t minLength = 20;
    scrollView.SetIndicatorMinLength(minLength);
    EXPECT_EQ(scrollView.GetIndicatorMinLength(), minLength);
    scrollView.SetIndicatorMinLength(0);
    EXPECT_EQ(scrollView.GetIndicatorMinLength(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);

    ColorType color = Color::Blue();
    scrollView.SetIndicatorColor(color);
    EXPECT_EQ(scrollView.GetIndicatorColor().full, color.full);

    uint16_t borderRadius = 5;
    scrollView.SetIndicatorBorderRadius(borderRadius);
    EXPECT_EQ(scrollView.GetIndicatorBorderRadius(), borderRadius);

    uint8_t opacity = 100;
    scrollView.SetIndicatorOpacity(opacity);
    EXPECT_EQ(scrollView.GetIndicatorOpacity(), opacity);
}

/**
 * @tc.name: UIScrollViewResetIndicatorStyle
 * @tc.desc: Verify resetting scroll view indicator attributes.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollViewResetIndicatorStyle, TestSize.Level0)
{
    UIScrollView scrollView;

    scrollView.SetIndicatorWidth(8);
    scrollView.SetIndicatorMinLength(20);
    scrollView.SetIndicatorBorderRadius(5);
    scrollView.SetIndicatorOpacity(100);
    scrollView.ResetIndicatorStyle();
    EXPECT_EQ(scrollView.GetIndicatorWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
    EXPECT_EQ(scrollView.GetIndicatorMinLength(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
    EXPECT_EQ(scrollView.GetIndicatorBorderRadius(), 0);
    EXPECT_EQ(scrollView.GetIndicatorOpacity(), OPA_OPAQUE);
}

/**
 * @tc.name: UIScrollViewSyncIndicatorStyle
 * @tc.desc: Verify cached indicator style is applied to a new vertical scroll bar.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollViewSyncIndicatorStyle, TestSize.Level0)
{
    TestUIScrollView scrollView;

    uint16_t width = 8;
    scrollView.SetIndicatorWidth(width);
    BaseGfxEngine::GetInstance()->SetScreenShape(ScreenShape::RECTANGLE);
    scrollView.SetYScrollBarVisible(true);
    ASSERT_TRUE(scrollView.GetYScrollBar() != nullptr);
    EXPECT_EQ(scrollView.GetYScrollBar()->GetIndicatorWidth(), width);
}

/**
 * @tc.name: UIScrollViewSyncIndicatorStyleToXScrollBar
 * @tc.desc: Verify cached indicator style is applied to a new horizontal scroll bar.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollViewSyncIndicatorStyleToXScrollBar, TestSize.Level0)
{
    TestUIScrollView scrollView;

    uint16_t width = 8;
    ColorType color = Color::Red();
    uint16_t minLength = 20;
    uint8_t opacity = 100;
    scrollView.SetIndicatorWidth(width);
    scrollView.SetIndicatorColor(color);
    scrollView.SetIndicatorMinLength(minLength);
    scrollView.SetIndicatorOpacity(opacity);
    BaseGfxEngine::GetInstance()->SetScreenShape(ScreenShape::RECTANGLE);
    scrollView.SetXScrollBarVisible(true);
    ASSERT_TRUE(scrollView.GetXScrollBar() != nullptr);
    EXPECT_EQ(scrollView.GetXScrollBar()->GetIndicatorWidth(), width);
    EXPECT_EQ(scrollView.GetXScrollBar()->GetIndicatorColor().full, color.full);
    EXPECT_EQ(scrollView.GetXScrollBar()->GetIndicatorMinLength(), minLength);
    EXPECT_EQ(scrollView.GetXScrollBar()->GetIndicatorOpacity(), opacity);
}

/**
 * @tc.name: UIScrollBarIndicatorInvalidMaxValue
 * @tc.desc: Verify oversized indicator attributes fall back to defaults.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarIndicatorInvalidMaxValue, TestSize.Level0)
{
    UIScrollView scrollView;

    scrollView.SetIndicatorWidth(INT16_MAX + 1);
    EXPECT_EQ(scrollView.GetIndicatorWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);

    scrollView.SetIndicatorMinLength(INT16_MAX + 1);
    EXPECT_EQ(scrollView.GetIndicatorMinLength(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);

    ScrollIndicatorStyle style = {static_cast<uint16_t>(INT16_MAX + 1), static_cast<uint16_t>(INT16_MAX + 1),
                                  0, Color::Red(), OPA_OPAQUE};
    scrollView.SetIndicatorStyle(style);
    EXPECT_EQ(scrollView.GetIndicatorStyle().width, UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
    EXPECT_EQ(scrollView.GetIndicatorStyle().minLength, UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
}

/**
 * @tc.name: UIScrollBarRefreshScrollBarParams
 * @tc.desc: Verify vertical scroll bar parameters are refreshed from content geometry.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarRefreshScrollBarParams, TestSize.Level0)
{
    TestUIAbstractScroll scrollView;
    scrollView.SetPosition(0, 0, 100, 100);
    scrollView.SetScrollBlankSize(10);
    UIView child;
    child.SetPosition(-20, -50, 50, 200);
    scrollView.Add(&child);

    TestScrollBar* bar = new TestScrollBar();
    // SetTestYScrollBar takes ownership and deletes the bar with scrollView.
    scrollView.SetTestYScrollBar(bar);
    scrollView.SetYScrollBarVisible(true);
    scrollView.CallRefreshScrollBarParams();

    EXPECT_FLOAT_EQ(bar->GetForegroundProportion(), 100.0f / 220.0f); // viewport / content plus blanks
    EXPECT_FLOAT_EQ(bar->GetScrollProgress(), 0.5f); // (blank - top) / (content - viewport)
}

/**
 * @tc.name: UIScrollBarRefreshScrollBarParamsLargeContent
 * @tc.desc: Verify scroll bar parameter calculation does not overflow for large content.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarRefreshScrollBarParamsLargeContent, TestSize.Level0)
{
    TestUIAbstractScroll scrollView;
    scrollView.SetPosition(0, 0, 100, 100);
    scrollView.SetScrollBlankSize(10);
    UIView child;
    child.SetPosition(0, 0, 50, INT16_MAX);
    scrollView.Add(&child);

    TestScrollBar* bar = new TestScrollBar();
    // SetTestYScrollBar takes ownership and deletes the bar with scrollView.
    scrollView.SetTestYScrollBar(bar);
    scrollView.SetYScrollBarVisible(true);
    scrollView.CallRefreshScrollBarParams();

    EXPECT_FLOAT_EQ(bar->GetForegroundProportion(), 100.0f / 32787.0f);
    EXPECT_FLOAT_EQ(bar->GetScrollProgress(), 10.0f / 32687.0f);
}

/**
 * @tc.name: UIScrollBarGetBackgroundAndSliderOpacity
 * @tc.desc: Verify background and slider opacity composition.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarGetBackgroundAndSliderOpacity, TestSize.Level0)
{
    TestScrollBar bar;

    EXPECT_EQ(bar.GetBarBackgroundOpacity(OPA_OPAQUE), OPA_OPAQUE);
    EXPECT_EQ(bar.GetBarSliderOpacity(OPA_OPAQUE), OPA_OPAQUE);

    bar.SetOpacity(100);
    EXPECT_EQ(bar.GetBarBackgroundOpacity(OPA_OPAQUE), 100);
    EXPECT_EQ(bar.GetBarSliderOpacity(OPA_OPAQUE), 100);

    bar.SetOpacity(OPA_OPAQUE);
    bar.SetIndicatorOpacity(128);
    EXPECT_EQ(bar.GetBarBackgroundOpacity(OPA_OPAQUE), OPA_OPAQUE);
    EXPECT_EQ(bar.GetBarSliderOpacity(OPA_OPAQUE), 128);

    bar.SetOpacity(200);
    EXPECT_EQ(bar.GetBarBackgroundOpacity(128), 100);
    EXPECT_EQ(bar.GetBarSliderOpacity(128), 50);
}

/**
 * @tc.name: UIScrollViewGetScrollBarWidth
 * @tc.desc: Verify default, custom, and reset scroll bar width.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollViewGetScrollBarWidth, TestSize.Level0)
{
    TestUIScrollView scrollView;

    EXPECT_EQ(scrollView.GetScrollBarWidthForTest(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);

    uint16_t width = 8;
    scrollView.SetIndicatorWidth(width);
    EXPECT_EQ(scrollView.GetScrollBarWidthForTest(), width);

    scrollView.ResetIndicatorStyle();
    EXPECT_EQ(scrollView.GetScrollBarWidthForTest(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
}

/**
 * @tc.name: UIScrollViewUpdateExistingIndicatorStyle
 * @tc.desc: Verify indicator style updates existing horizontal and vertical scroll bars.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollViewUpdateExistingIndicatorStyle, TestSize.Level0)
{
    TestUIScrollView scrollView;
    BaseGfxEngine::GetInstance()->SetScreenShape(ScreenShape::RECTANGLE);
    scrollView.SetXScrollBarVisible(true);
    scrollView.SetYScrollBarVisible(true);
    ASSERT_NE(scrollView.GetXScrollBar(), nullptr);
    ASSERT_NE(scrollView.GetYScrollBar(), nullptr);

    ScrollIndicatorStyle style = {8, 20, 5, Color::Red(), 100};
    scrollView.SetIndicatorStyle(style);
    for (UIAbstractScrollBar* bar : {scrollView.GetXScrollBar(), scrollView.GetYScrollBar()}) {
        EXPECT_EQ(bar->GetIndicatorWidth(), style.width);
        EXPECT_EQ(bar->GetIndicatorMinLength(), style.minLength);
        EXPECT_EQ(bar->GetIndicatorBorderRadius(), style.borderRadius);
        EXPECT_EQ(bar->GetIndicatorColor().full, style.color.full);
        EXPECT_EQ(bar->GetIndicatorOpacity(), style.opacity);
    }

    scrollView.SetIndicatorWidth(9);
    scrollView.SetIndicatorColor(Color::Blue());
    scrollView.SetIndicatorBorderRadius(6);
    scrollView.SetIndicatorMinLength(21);
    scrollView.SetIndicatorOpacity(101);
    for (UIAbstractScrollBar* bar : {scrollView.GetXScrollBar(), scrollView.GetYScrollBar()}) {
        EXPECT_EQ(bar->GetIndicatorWidth(), 9);
        EXPECT_EQ(bar->GetIndicatorColor().full, Color::Blue().full);
        EXPECT_EQ(bar->GetIndicatorBorderRadius(), 6);
        EXPECT_EQ(bar->GetIndicatorMinLength(), 21);
        EXPECT_EQ(bar->GetIndicatorOpacity(), 101);
    }

    scrollView.ResetIndicatorStyle();
    for (UIAbstractScrollBar* bar : {scrollView.GetXScrollBar(), scrollView.GetYScrollBar()}) {
        EXPECT_EQ(bar->GetIndicatorWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
        EXPECT_EQ(bar->GetIndicatorMinLength(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_MIN_LEN);
        EXPECT_EQ(bar->GetIndicatorBorderRadius(), 0);
        EXPECT_EQ(bar->GetIndicatorOpacity(), OPA_OPAQUE);
    }
}

/**
 * @tc.name: UIScrollViewSyncIndicatorStyleBranches
 * @tc.desc: Verify null and configured indicator style synchronization paths.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollViewSyncIndicatorStyleBranches, TestSize.Level0)
{
    TestUIAbstractScroll scrollView;
    scrollView.CallSyncIndicatorStyle(nullptr);

    TestScrollBar bar;
    scrollView.SetIndicatorBorderRadius(7);
    scrollView.CallSyncIndicatorStyle(&bar);
    EXPECT_EQ(bar.GetIndicatorBorderRadius(), 7);

    scrollView.ResetIndicatorStyle();
    scrollView.CallSyncIndicatorStyle(&bar);
    EXPECT_EQ(bar.GetIndicatorBorderRadius(), 0);
}

/**
 * @tc.name: UIScrollBarRefreshHorizontalAndBoundaryParams
 * @tc.desc: Verify horizontal scroll bar parameter and boundary paths.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarRefreshHorizontalAndBoundaryParams, TestSize.Level0)
{
    TestUIAbstractScroll scrollView;
    scrollView.SetPosition(0, 0, 100, 100);
    scrollView.CallRefreshScrollBarParams();

    UIView child;
    child.SetPosition(-20, 0, 200, 100);
    scrollView.Add(&child);
    TestScrollBar* xBar = new TestScrollBar();
    scrollView.SetTestXScrollBar(xBar);
    scrollView.SetXScrollBarVisible(true);
    scrollView.CallRefreshScrollBarParams();
    EXPECT_FLOAT_EQ(xBar->GetForegroundProportion(), 0.5f);
    EXPECT_FLOAT_EQ(xBar->GetScrollProgress(), 0.2f);

    child.SetPosition(0, 0, 100, 100);
    scrollView.CallRefreshScrollBarParams();
    EXPECT_FLOAT_EQ(xBar->GetForegroundProportion(), 1.0f);

    child.SetPosition(0, 0, 0, 100);
    scrollView.CallRefreshScrollBarParams();
}

/**
 * @tc.name: UIScrollBarDrawRectBranches
 * @tc.desc: Verify rectangular scroll bars are positioned on both configured sides.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarDrawRectBranches, TestSize.Level0)
{
    MockScrollSoftEngine mockEngine;
    GfxEngineRestoreGuard engineGuard(mockEngine);
    BufferInfo bufferInfo = {};
    Rect drawRect(0, 0, 99, 99);

    TestUIAbstractScroll scrollView;
    TestBoxScrollBar* xBar = new TestBoxScrollBar();
    TestBoxScrollBar* yBar = new TestBoxScrollBar();
    scrollView.SetTestXScrollBar(xBar);
    scrollView.SetTestYScrollBar(yBar);
    scrollView.SetXScrollBarVisible(true);
    scrollView.SetYScrollBarVisible(true);
    EXPECT_CALL(mockEngine, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(1));
    scrollView.SetScrollBarSide(SCROLL_BAR_RIGHT_SIDE);
    scrollView.CallDrawScrollBarOnRect(bufferInfo, drawRect, drawRect);
    EXPECT_EQ(yBar->GetWidth(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);
    EXPECT_EQ(xBar->GetHeight(), UIAbstractScrollBar::DEFAULT_SCROLL_BAR_WIDTH);

    scrollView.SetScrollBarSide(SCROLL_BAR_LEFT_SIDE);
    scrollView.CallDrawScrollBarOnRect(bufferInfo, drawRect, drawRect);
}

/**
 * @tc.name: UIScrollBarDrawCircleBranches
 * @tc.desc: Verify circular scroll bar visibility and center position branches.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarDrawCircleBranches, TestSize.Level0)
{
    MockScrollSoftEngine mockEngine;
    GfxEngineRestoreGuard engineGuard(mockEngine);
    BufferInfo bufferInfo = {};
    Rect drawRect(0, 0, 99, 99);

    TestUIAbstractScroll circleScroll;
    TestArcScrollBar* arcBar = new TestArcScrollBar();
    circleScroll.SetTestYScrollBar(arcBar);
    circleScroll.SetYScrollBarVisible(false);
    circleScroll.CallDrawScrollBarOnCircle(bufferInfo, drawRect, drawRect);
    circleScroll.SetYScrollBarVisible(true);
    circleScroll.SetPosition(0, 0, 100, 100);
    EXPECT_CALL(mockEngine, DrawArc(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(1));
    circleScroll.CallDrawScrollBarOnCircle(bufferInfo, drawRect, drawRect);
    EXPECT_EQ(arcBar->GetCenter().x, 50);
    Point center = {30, 40};
    circleScroll.SetScrollBarCenter(center);
    circleScroll.CallDrawScrollBarOnCircle(bufferInfo, drawRect, drawRect);
    EXPECT_EQ(arcBar->GetCenter().x, center.x);
    EXPECT_EQ(arcBar->GetCenter().y, center.y);
}

/**
 * @tc.name: UIScrollBarDrawScrollBarsDispatch
 * @tc.desc: Verify DrawScrollBars dispatches by screen shape.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarDrawScrollBarsDispatch, TestSize.Level0)
{
    MockScrollSoftEngine mockEngine;
    GfxEngineRestoreGuard engineGuard(mockEngine);
    BufferInfo bufferInfo = {};
    Rect drawRect(0, 0, 99, 99);

    TestUIAbstractScroll rectScroll;
    rectScroll.SetTestYScrollBar(new TestBoxScrollBar());
    rectScroll.SetYScrollBarVisible(true);
    BaseGfxEngine::GetInstance()->SetScreenShape(ScreenShape::RECTANGLE);
    EXPECT_CALL(mockEngine, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(1));
    rectScroll.CallDrawScrollBars(bufferInfo, drawRect, drawRect);
    testing::Mock::VerifyAndClearExpectations(&mockEngine);

    TestUIAbstractScroll circleScroll;
    circleScroll.SetPosition(0, 0, 100, 100);
    circleScroll.SetTestYScrollBar(new TestArcScrollBar());
    circleScroll.SetYScrollBarVisible(true);
    EXPECT_CALL(mockEngine, DrawArc(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(1));
    BaseGfxEngine::GetInstance()->SetScreenShape(ScreenShape::CIRCLE);
    circleScroll.CallDrawScrollBars(bufferInfo, drawRect, drawRect);
}

/**
 * @tc.name: UIScrollBarForegroundLengthBoundary
 * @tc.desc: Verify box and arc foreground length clamping.
 * @tc.type: FUNC
 */
HWTEST_F(ScrollBarTest, UIScrollBarForegroundLengthBoundary, TestSize.Level0)
{
    MockScrollSoftEngine mockEngine;
    BaseGfxEngine* originEngine = BaseGfxEngine::GetInstance();
    BaseGfxEngine::InitGfxEngine(&mockEngine);
    BufferInfo bufferInfo = {};
    Rect invalidatedArea(0, 0, 99, 99);

    TestBoxScrollBar boxBar;
    boxBar.SetPosition(0, 0, 4, 20);
    boxBar.SetForegroundProportion(0.5f);
    boxBar.SetIndicatorMinLength(2);
    EXPECT_CALL(mockEngine, DrawRect(testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(1));
    boxBar.OnDraw(bufferInfo, invalidatedArea, OPA_OPAQUE);
    boxBar.SetIndicatorMinLength(30);
    boxBar.SetForegroundProportion(0.0f);
    boxBar.OnDraw(bufferInfo, invalidatedArea, OPA_OPAQUE);

    TestArcScrollBar arcBar;
    arcBar.SetPosition(50, 50, 4, 40);
    arcBar.SetForegroundProportion(0.5f);
    arcBar.SetIndicatorMinLength(2);
    EXPECT_CALL(mockEngine, DrawArc(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(testing::AtLeast(1));
    arcBar.OnDraw(bufferInfo, invalidatedArea, OPA_OPAQUE);
    arcBar.SetIndicatorMinLength(INT16_MAX);
    arcBar.SetForegroundProportion(0.0f);
    arcBar.OnDraw(bufferInfo, invalidatedArea, OPA_OPAQUE);
    BaseGfxEngine::InitGfxEngine(originEngine);
}
#endif
} // namespace OHOS
