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

#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
#include "components/ui_scroll_view.h"
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT
#include "layout/flex_layout.h"

#include <climits>
#include <gtest/gtest.h>

using ::testing::ext::TestSize;
namespace OHOS {

#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
class FlexLayoutEnhancementTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static FlexLayout* flexLayout_;
};

FlexLayout* FlexLayoutEnhancementTest::flexLayout_ = nullptr;

void FlexLayoutEnhancementTest::SetUpTestCase(void)
{
    if (flexLayout_ == nullptr) {
        flexLayout_ = new FlexLayout();
        flexLayout_->SetPosition(0, 0, 600, 300); // 600: layout width; 300: layout height
    }
}

void FlexLayoutEnhancementTest::TearDownTestCase(void)
{
    if (flexLayout_ != nullptr) {
        delete flexLayout_;
        flexLayout_ = nullptr;
    }
}

/**
 * @tc.name: FlexLayout_AlignContentSpaceBetween
 * @tc.desc: Verify wrapped lines occupy both cross-axis edges with equal space between them.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AlignContentSpaceBetween, TestSize.Level1)
{
    flexLayout_->SetPosition(0, 0, 300, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetFlexWrap(FlexLayout::WRAP);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetAlignContent(ALIGN_CONTENT_BETWEEN);
    UIView views[9];
    for (uint8_t i = 0; i < 9; i++) {
        views[i].Resize(100, 50);
        flexLayout_->Add(&views[i]);
    }
    flexLayout_->LayoutChildren();
    EXPECT_EQ(views[0].GetY(), 0);
    EXPECT_EQ(views[3].GetY(), 125);
    EXPECT_EQ(views[6].GetY(), 250);
    flexLayout_->RemoveAll();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
}

/**
 * @tc.name: FlexLayout_AlignContentStretch
 * @tc.desc: Verify three wrapped lines stretch evenly while items keep their own cross size.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AlignContentStretch, TestSize.Level1)
{
    flexLayout_->SetPosition(0, 0, 300, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetFlexWrap(FlexLayout::WRAP);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetAlignContent(ALIGN_CONTENT_STRETCH);
    UIView views[9];
    for (uint8_t i = 0; i < 9; i++) {
        views[i].Resize(100, 50);
        flexLayout_->Add(&views[i]);
    }
    flexLayout_->LayoutChildren();
    EXPECT_EQ(views[0].GetY(), 0);
    EXPECT_EQ(views[3].GetY(), 100);
    EXPECT_EQ(views[6].GetY(), 200);
    EXPECT_EQ(views[0].GetHeight(), 50);
    EXPECT_EQ(views[3].GetHeight(), 50);
    EXPECT_EQ(views[6].GetHeight(), 50);
    flexLayout_->RemoveAll();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
}

/**
 * @tc.name: FlexLayout_AlignContentStretchThenCenter
 * @tc.desc: Verify align-content stretch is not retained after switching to center.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AlignContentStretchThenCenter, TestSize.Level1)
{
    flexLayout_->SetPosition(0, 0, 300, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetFlexWrap(FlexLayout::WRAP);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView views[9];
    for (uint8_t i = 0; i < 9; i++) {
        views[i].Resize(100, 50);
        flexLayout_->Add(&views[i]);
    }
    flexLayout_->SetAlignContent(ALIGN_CONTENT_STRETCH);
    flexLayout_->LayoutChildren();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_CENTER);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(views[0].GetY(), 75);
    EXPECT_EQ(views[0].GetHeight(), 50);
    flexLayout_->RemoveAll();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
}

/**
 * @tc.name: FlexLayout_AlignContentStretchThenStart
 * @tc.desc: Verify align-content stretch is not retained after switching to flex-start.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AlignContentStretchThenStart, TestSize.Level1)
{
    flexLayout_->SetPosition(0, 0, 300, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetFlexWrap(FlexLayout::WRAP);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView views[9];
    for (uint8_t i = 0; i < 9; i++) {
        views[i].Resize(100, 50);
        flexLayout_->Add(&views[i]);
    }
    flexLayout_->SetAlignContent(ALIGN_CONTENT_STRETCH);
    flexLayout_->LayoutChildren();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(views[0].GetY(), 0);
    EXPECT_EQ(views[0].GetHeight(), 50);
    flexLayout_->RemoveAll();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
}

/**
 * @tc.name: FlexLayout_AlignContentStretchRestoresExplicitCrossSize
 * @tc.desc: Verify align-content stretch is temporary for an explicit cross-axis size.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AlignContentStretchRestoresExplicitCrossSize, TestSize.Level1)
{
    flexLayout_->SetPosition(0, 0, 300, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetFlexWrap(FlexLayout::WRAP);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetAlignContent(ALIGN_CONTENT_STRETCH);
    UIView views[9];
    for (uint8_t i = 0; i < 9; i++) {
        views[i].Resize(100, 50);
        views[i].SetHasExplicitHeight(true);
        flexLayout_->Add(&views[i]);
    }
    flexLayout_->LayoutChildren();
    EXPECT_EQ(views[0].GetHeight(), 50);
    flexLayout_->SetAlignContent(ALIGN_CONTENT_CENTER);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(views[0].GetY(), 75);
    EXPECT_EQ(views[0].GetHeight(), 50);
    EXPECT_EQ(views[3].GetHeight(), 50);
    flexLayout_->RemoveAll();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
}

/**
 * @tc.name: FlexLayout_AlignContentStretchSingleLine
 * @tc.desc: Verify stretch fills the single line of a wrapped container (W3C: a multi-line
 *           container applies align-content even when it has only one flex line).
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AlignContentStretchSingleLine, TestSize.Level1)
{
    flexLayout_->SetPosition(0, 0, 300, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetFlexWrap(FlexLayout::WRAP);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetAlignContent(ALIGN_CONTENT_STRETCH);
    UIView views[2];
    for (uint8_t i = 0; i < 2; i++) {
        views[i].Resize(100, 50); // 100x50 children fit in a single line
        flexLayout_->Add(&views[i]);
    }
    // align-self stretch observes the stretched line cross size
    views[1].SetAlignSelf(UIView::ALIGN_SELF_STRETCH);
    flexLayout_->LayoutChildren();
    // the single line is stretched to the container cross size 300, the item fills the line
    EXPECT_EQ(views[1].GetHeight(), 300); // 300: container height
    EXPECT_EQ(views[0].GetHeight(), 50); // 50: item without stretch keeps its cross size
    flexLayout_->RemoveAll();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
}

/**
 * @tc.name: FlexLayout_AlignContentStretchOverflowFallback
 * @tc.desc: Verify stretch falls back to flex-start when free cross space is negative
 *           (lines are not shrunk).
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AlignContentStretchOverflowFallback, TestSize.Level1)
{
    flexLayout_->SetPosition(0, 0, 300, 100); // 100: container height smaller than the lines
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetFlexWrap(FlexLayout::WRAP);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetAlignContent(ALIGN_CONTENT_STRETCH);
    UIView views[6];
    for (uint8_t i = 0; i < 6; i++) {
        views[i].Resize(100, 80); // 100x80 children form two lines of cross size 80
        flexLayout_->Add(&views[i]);
    }
    flexLayout_->LayoutChildren();
    // two lines of 80 overflow the 100 container height: no shrink, packed from cross-start
    EXPECT_EQ(views[0].GetY(), 0);
    EXPECT_EQ(views[0].GetHeight(), 80); // 80: line cross size not shrunk
    EXPECT_EQ(views[3].GetY(), 80); // 80: second line right after the first
    EXPECT_EQ(views[3].GetHeight(), 80); // 80: line cross size not shrunk
    flexLayout_->RemoveAll();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
}

/**
 * @tc.name: FlexLayout_AlignContentStretchRemainderToLastLine
 * @tc.desc: Verify the stretch division remainder goes to the last line so the line cross
 *           sizes sum exactly to the container cross size.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AlignContentStretchRemainderToLastLine, TestSize.Level1)
{
    flexLayout_->SetPosition(0, 0, 300, 301); // 301: leaves a division remainder
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetFlexWrap(FlexLayout::WRAP);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetAlignContent(ALIGN_CONTENT_STRETCH);
    UIView views[6];
    for (uint8_t i = 0; i < 6; i++) {
        views[i].Resize(100, 100); // 100x100 children form two lines of cross size 100
        flexLayout_->Add(&views[i]);
    }
    // align-self stretch observes the last line cross size
    views[3].SetAlignSelf(UIView::ALIGN_SELF_STRETCH);
    flexLayout_->LayoutChildren();
    // free space 101 split over two lines: 50 + 51, the remainder goes to the last line
    EXPECT_EQ(views[3].GetY(), 150); // 150: first line cross size 100 + 50
    EXPECT_EQ(views[3].GetHeight(), 151); // 151: last line cross size 100 + 51
    flexLayout_->RemoveAll();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
}

/**
 * @tc.name: FlexLayout_AlignContentBetweenSingleLine
 * @tc.desc: Verify space-between falls back to flex-start on a single flex line.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AlignContentBetweenSingleLine, TestSize.Level1)
{
    flexLayout_->SetPosition(0, 0, 300, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetFlexWrap(FlexLayout::WRAP);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetAlignContent(ALIGN_CONTENT_BETWEEN);
    UIView views[2];
    for (uint8_t i = 0; i < 2; i++) {
        views[i].Resize(100, 50); // 100x50 children fit in a single line
        flexLayout_->Add(&views[i]);
    }
    flexLayout_->LayoutChildren();
    // single line: space-between falls back to flex-start
    EXPECT_EQ(views[0].GetY(), 0);
    EXPECT_EQ(views[1].GetY(), 0);
    flexLayout_->RemoveAll();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
}

/**
 * @tc.name: FlexLayout_AlignContentBetweenOverflowFallback
 * @tc.desc: Verify space-between falls back to flex-start when free cross space is negative
 *           (lines do not overlap).
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AlignContentBetweenOverflowFallback, TestSize.Level1)
{
    flexLayout_->SetPosition(0, 0, 300, 100); // 100: container height smaller than the lines
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetFlexWrap(FlexLayout::WRAP);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetAlignContent(ALIGN_CONTENT_BETWEEN);
    UIView views[6];
    for (uint8_t i = 0; i < 6; i++) {
        views[i].Resize(100, 80); // 100x80 children form two lines of cross size 80
        flexLayout_->Add(&views[i]);
    }
    flexLayout_->LayoutChildren();
    // negative free space: packed from cross-start instead of a negative line gap
    EXPECT_EQ(views[0].GetY(), 0);
    EXPECT_EQ(views[3].GetY(), 80); // 80: second line right after the first
    flexLayout_->RemoveAll();
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
}
/* FlexLayout_041~058: align-items:stretch suite from the flex CSS enhancement,
 * renumbered from 011~028 to avoid collision with the overflow/auto-margin suite. */
/**
 * @tc.name: FlexLayout_041
 * @tc.desc: align-items:stretch only stretches children without an explicit cross-axis size (row direction).
 *           Container 330x140, child A has explicit height 90 (SetHasExplicitHeight(true)),
 *           child B has no explicit height (default auto); after stretch A keeps 90, B fills 140.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_041, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 330, 140); // 330: layout width; 140: layout height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView viewA;
    viewA.Resize(60, 90); // 60: child width; 90: explicit child height
    viewA.SetHasExplicitHeight(true);
    flexLayout_->Add(&viewA);

    UIView viewB;
    viewB.Resize(60, 0); // 60: child width; 0: auto height
    flexLayout_->Add(&viewB);

    flexLayout_->LayoutChildren();

    // A has explicit height, not stretched, aligned to flex-start top
    EXPECT_EQ(viewA.GetHeight(), 90); // 90: explicit height kept
    EXPECT_EQ(viewA.GetY(), 0);
    // B has no explicit height, stretched to container line height
    EXPECT_EQ(viewB.GetHeight(), 140); // 140: stretched to line cross size
    EXPECT_EQ(viewB.GetY(), 0);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_042
 * @tc.desc: align-items:stretch only stretches children without an explicit cross-axis size (column direction).
 *           Container 140x330, child A has explicit width 90 (SetHasExplicitWidth(true)),
 *           child B has no explicit width (default auto); after stretch A keeps 90, B fills 140.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_042, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 140, 330); // 140: layout width; 330: layout height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView viewA;
    viewA.Resize(90, 60); // 90: explicit child width; 60: child height
    viewA.SetHasExplicitWidth(true);
    flexLayout_->Add(&viewA);

    UIView viewB;
    viewB.Resize(0, 60); // 0: auto width; 60: child height
    flexLayout_->Add(&viewB);

    flexLayout_->LayoutChildren();

    // A has explicit width, not stretched, aligned to flex-start left
    EXPECT_EQ(viewA.GetWidth(), 90); // 90: explicit width kept
    EXPECT_EQ(viewA.GetX(), 0);
    // B has no explicit width, stretched to container column width
    EXPECT_EQ(viewB.GetWidth(), 140); // 140: stretched to line cross size
    EXPECT_EQ(viewB.GetX(), 0);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_048
 * @tc.desc: align-items:flex-end (row direction).
 *           Container 500x300, child 100x100, cross-axis position = 300 - 100 = 200.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_048, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 500, 300); // 500: layout width; 300: layout height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);

    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    flexLayout_->LayoutChildren();

    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view1.GetY(), 200); // 200: cross axis end pos

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_050
 * @tc.desc: Direction switch restores the original size of stretch children.
 *           After row + stretch fills children, switching to column should restore the original child height.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_050, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 200, 200); // 200: layout width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView view;
    view.Resize(50, 50); // 50: original height
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();

    // After stretch, height is filled to 200
    EXPECT_EQ(view.GetHeight(), 200); // 200: stretched to container height

    // Switch to column direction, no longer stretch, should restore original height 50
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetHeight(), 50); // 50: restored original height

    // Switch back to row, stretch again
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetHeight(), 200); // 200: stretched again

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_051
 * @tc.desc: Switching align-items from stretch to flex-start restores the original size.
 *           After row + stretch fills children, switching to flex-start should restore the original child height.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_051, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 200, 200);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView view;
    view.Resize(50, 50);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();

    EXPECT_EQ(view.GetHeight(), 200); // 200: stretched

    // Switch to flex-start, restore original height
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetHeight(), 50); // 50: restored

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_0511
 * @tc.desc: Switching align-items from stretch to flex-start restores wrapped auto-height children.
 *           After wrap + stretch fills auto-height children, switching to flex-start should restore
 *           the original child height on each line.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_0511, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 250, 200);
    flexLayout_->SetFlexWrap(true);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView view1;
    view1.Resize(100, 60);
    view1.SetHasExplicitHeight(true);
    flexLayout_->Add(&view1);

    UIView view2;
    view2.Resize(100, 20);
    flexLayout_->Add(&view2);

    UIView view3;
    view3.Resize(100, 60);
    view3.SetHasExplicitHeight(true);
    flexLayout_->Add(&view3);

    UIView view4;
    view4.Resize(100, 20);
    flexLayout_->Add(&view4);

    flexLayout_->LayoutChildren();
    EXPECT_EQ(view2.GetHeight(), 60);
    EXPECT_EQ(view4.GetHeight(), 60);

    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view2.GetHeight(), 20);
    EXPECT_EQ(view4.GetHeight(), 20);
    EXPECT_EQ(view4.GetY(), 60);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_052
 * @tc.desc: Dynamically setting an explicit size clears the saved flag;
 *           subsequent restore should not overwrite the user-set value.
 *           After stretch saves the original size, calling SetHeightExplicit + SetHeight
 *           clears the saved flag; direction-switch restore logic skips, keeping the user-set value 60.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_052, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 200, 200);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView view;
    view.Resize(50, 50); // original height 50
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();

    // After stretch, original size is saved
    EXPECT_TRUE(view.IsOriginalHeightSaved());
    EXPECT_EQ(view.GetOriginalHeight(), 50);
    EXPECT_EQ(view.GetHeight(), 200);

    // Dynamically set explicit height 60, saved flag should be cleared
    view.SetHasExplicitHeight(true);
    view.SetHeight(60);
    EXPECT_FALSE(view.IsOriginalHeightSaved());
    EXPECT_EQ(view.GetHeight(), 60);

    // Direction switch triggers RestoreChildrenOriginalSize, saved already cleared, should keep 60
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetHeight(), 60); // 60: user-set explicit height kept

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_053
 * @tc.desc: In wrap mode stretch should stretch to line height (not total container height).
 *           Container 300x300, three children 150x50 each, after wrap the first two fill the first row,
 *           the 3rd child wraps to the second row, line height is 50, should be stretched to 50 instead of 300.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_053, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: layout width and height
    flexLayout_->SetFlexWrap(true); // wrap
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView view1;
    view1.Resize(150, 50); // 150: width; 50: height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(150, 50);
    flexLayout_->Add(&view2);
    UIView view3; // Wrap to second row
    view3.Resize(150, 50);
    flexLayout_->Add(&view3);

    flexLayout_->LayoutChildren();

    // First row height = max(50, 50) = 50, view1/2 stretched to 50 (equals row height)
    EXPECT_EQ(view1.GetHeight(), 50);
    EXPECT_EQ(view2.GetHeight(), 50);
    // Second row height is also 50, view3 stretched to 50, not total container height 300
    EXPECT_EQ(view3.GetHeight(), 50);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_054
 * @tc.desc: Stretch calculation correctly deducts margin / padding / border.
 *           Container 200x200 row stretch, child marginTop=10 marginBottom=10,
 *           paddingTop=5 paddingBottom=5, border=2.
 *           newCross = 200 - 20 - 14 = 166.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_054, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 200, 200); // 200: container width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView view;
    view.SetStyle(STYLE_MARGIN_TOP, 10);
    view.SetStyle(STYLE_MARGIN_BOTTOM, 10);
    view.SetStyle(STYLE_PADDING_TOP, 5);
    view.SetStyle(STYLE_PADDING_BOTTOM, 5);
    view.SetStyle(STYLE_BORDER_WIDTH, 2);
    view.Resize(50, 50); // content size 50x50, rect_ includes padding/border
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();

    // content height = lineCross(200) - margin(20) - padBorder(14) = 166
    EXPECT_EQ(view.GetHeight(), 166); // 166: stretched content height
    // margin-box top stays at line start; marginTop must not be counted twice
    EXPECT_EQ(view.GetY(), 0); // 0: margin-box top at line start

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_055
 * @tc.desc: When stretch margin is too large causing negative available space, child height is clamped to 0.
 *           Container 30x100 row stretch, child marginTop=20 marginBottom=20.
 *           newCross = 30 - 40 = -10 -> clamp 0.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_055, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 100, 30); // 100: container width; 30: container height (cross axis)
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView view;
    view.SetStyle(STYLE_MARGIN_TOP, 20);
    view.SetStyle(STYLE_MARGIN_BOTTOM, 20);
    view.Resize(50, 50);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();

    // available cross = 30 - 40 = -10, clamped to 0
    EXPECT_EQ(view.GetHeight(), 0); // 0: clamped when cross size <= 0

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_056
 * @tc.desc: Column wrap stretch should stretch to column width (not total container width).
 *           Container 300x300, column wrap, 4 children:
 *           view1(50x150) view2(0x150) fill the first column (height 300),
 *           view3(60x150) view4(0x150) fill the second column (height 300),
 *           Second column width = max(60, 0) = 60, view4 is stretched to 60.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_056, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(true);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView view1;
    view1.Resize(50, 150); // 50: explicit width
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(0, 150); // 0: auto width
    flexLayout_->Add(&view2);
    UIView view3;
    view3.Resize(60, 150); // 60: explicit width, second column
    flexLayout_->Add(&view3);
    UIView view4;
    view4.Resize(0, 150); // 0: auto width, second column
    flexLayout_->Add(&view4);

    flexLayout_->LayoutChildren();

    // column 0 width = max(50, 0) = 50
    EXPECT_EQ(view1.GetWidth(), 50);
    // view2 auto, stretched to column 0 width
    EXPECT_EQ(view2.GetWidth(), 50);
    // column 1 width = max(60, 0) = 60
    EXPECT_EQ(view3.GetWidth(), 60);
    // view4 auto, stretched to column 1 width
    EXPECT_EQ(view4.GetWidth(), 60); // 60: stretched to column width

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_058
 * @tc.desc: Re-layout with unchanged direction and align-items:stretch does not restore
 *           or corrupt the saved original size; a later direction switch still restores
 *           the true original height.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_058, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 200, 200); // 200: layout width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_STRETCH);

    UIView view;
    view.Resize(50, 50); // 50: original height
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetHeight(), 200); // 200: stretched to container height

    // Re-layout with unchanged settings: no restore, saved original stays the pre-stretch value
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetHeight(), 200); // 200: still stretched
    EXPECT_EQ(view.GetOriginalHeight(), 50); // 50: saved original not overwritten by stretched value

    // Direction switch afterwards still restores the true original height
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetHeight(), 50); // 50: restored true original height

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_059
 * @tc.desc: Absolute positioned child accepts negative left/top inset values.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_059, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 160); // 300: width; 160: height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView normalView;
    normalView.Resize(100, 50); // 100: width; 50: height
    flexLayout_->Add(&normalView);
    UIView absoluteView;
    absoluteView.Resize(50, 50); // 50: absolute child size
    absoluteView.SetPositionType(POSITION_ABSOLUTE);
    absoluteView.SetFlexLeft(-25); // -25: negative left inset
    absoluteView.SetFlexTop(-25); // -25: negative top inset
    flexLayout_->Add(&absoluteView);

    flexLayout_->LayoutChildren();

    EXPECT_EQ(normalView.GetX(), 0);
    EXPECT_EQ(normalView.GetY(), 0);
    EXPECT_EQ(absoluteView.GetX(), -25); // -25: absolute child can overflow left
    EXPECT_EQ(absoluteView.GetY(), -25); // -25: absolute child can overflow top

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_060
 * @tc.desc: Absolute positioned child falls back to the initial containing block.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_060, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    UIScrollView root;
    FlexLayout page;
    root.SetPosition(0, 0, 300, 160); // 300: initial containing block width; 160: height
    page.SetPosition(0, 0, 300, 500); // 300: page width; 500: long page height
    flexLayout_->SetPosition(40, 20, 120, 80); // 40: x offset; 20: y offset; 120x80 child container
    root.Add(&page);
    page.Add(flexLayout_);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView absoluteView;
    absoluteView.Resize(50, 50); // 50: absolute child size
    absoluteView.SetPositionType(POSITION_ABSOLUTE);
    absoluteView.SetFlexRightPercent(50.0f); // 50: right is 150px of 300px
    absoluteView.SetFlexBottomPercent(25.0f); // 25: bottom is 40px of 160px
    flexLayout_->Add(&absoluteView);

    flexLayout_->LayoutChildren();

    EXPECT_EQ(absoluteView.GetX(), 60); // 60 = 300 - 50 - 150 - 40(parent x)
    EXPECT_EQ(absoluteView.GetY(), 50); // 50 = 160 - 50 - 40 - 20(parent y)
    EXPECT_EQ(absoluteView.GetRect().GetX(), 100); // 100 = 300 - 50 - 150
    EXPECT_EQ(absoluteView.GetRect().GetY(), 70); // 70 = 160 - 50 - 40

    root.MoveChildByOffset(0, -80); // -80: scroll down the long page by 80 px
    flexLayout_->LayoutChildren();
    EXPECT_EQ(absoluteView.GetX(), 60); // 60: parent-local position is stable after scrolling
    EXPECT_EQ(absoluteView.GetY(), 50); // 50: parent-local position is stable after scrolling
    EXPECT_EQ(absoluteView.GetRect().GetX(), 100); // 100: x is unchanged by vertical scroll
    EXPECT_EQ(absoluteView.GetRect().GetY(), -10); // -10 = 70 - 80, not fixed to the viewport

    flexLayout_->RemoveAll();
    page.RemoveAll();
    root.RemoveAll();
}

/**
 * @tc.name: FlexLayout_061
 * @tc.desc: Clearing absolute insets removes stale values and falls back to the static position.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_061, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 160); // 300: width; 160: height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView absoluteView;
    absoluteView.Resize(50, 50); // 50: absolute child size
    absoluteView.SetPositionType(POSITION_ABSOLUTE);
    absoluteView.SetFlexLeft(-1); // -1: valid negative inset, not an unset sentinel
    absoluteView.SetFlexTop(-1); // -1: valid negative inset, not an unset sentinel
    flexLayout_->Add(&absoluteView);

    flexLayout_->LayoutChildren();
    EXPECT_EQ(absoluteView.GetX(), -1);
    EXPECT_EQ(absoluteView.GetY(), -1);

    absoluteView.ClearFlexLeft();
    absoluteView.ClearFlexTop();
    flexLayout_->LayoutChildren();
    EXPECT_EQ(absoluteView.GetX(), 0);
    EXPECT_EQ(absoluteView.GetY(), 0);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_062
 * @tc.desc: Align-self stretch restores the original cross size after switching to non-stretch alignment.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_062, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 200, 200); // 200: layout width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view;
    view.Resize(50, 50); // 50: original child size
    view.SetAlignSelf(UIView::ALIGN_SELF_STRETCH);
    flexLayout_->Add(&view);

    flexLayout_->LayoutChildren();
    EXPECT_TRUE(view.IsOriginalHeightSaved());
    EXPECT_EQ(view.GetOriginalHeight(), 50); // 50: original height
    EXPECT_EQ(view.GetHeight(), 200); // 200: stretched height

    view.SetAlignSelf(ALIGN_CENTER);
    flexLayout_->LayoutChildren();
    EXPECT_FALSE(view.IsOriginalHeightSaved());
    EXPECT_EQ(view.GetHeight(), 50); // 50: restored height

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_063
 * @tc.desc: Absolute positioned ancestor is used as the containing block.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_063, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    FlexLayout root;
    root.SetPosition(0, 0, 300, 160); // 300: initial containing block width; 160: height
    flexLayout_->SetPosition(40, 20, 200, 120); // 40,20: parent offset; 200x120 parent size
    root.Add(flexLayout_);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    FlexLayout absoluteAncestor;
    absoluteAncestor.SetPosition(0, 0, 120, 90); // 120x90: containing block size
    absoluteAncestor.SetPositionType(POSITION_ABSOLUTE);
    absoluteAncestor.SetFlexLeft(30); // 30: ancestor x in initial containing block
    absoluteAncestor.SetFlexTop(40); // 40: ancestor y in initial containing block
    flexLayout_->Add(&absoluteAncestor);

    UIView absoluteChild;
    absoluteChild.Resize(20, 10); // 20x10: child size
    absoluteChild.SetPositionType(POSITION_ABSOLUTE);
    absoluteChild.SetFlexRight(10); // 10: right inset in absolute ancestor
    absoluteChild.SetFlexBottom(20); // 20: bottom inset in absolute ancestor
    absoluteAncestor.Add(&absoluteChild);

    flexLayout_->LayoutChildren();

    EXPECT_EQ(absoluteAncestor.GetRect().GetX(), 30); // 30: relative to initial containing block
    EXPECT_EQ(absoluteAncestor.GetRect().GetY(), 40); // 40: relative to initial containing block
    EXPECT_EQ(absoluteChild.GetX(), 90); // 90 = 120 - 20 - 10
    EXPECT_EQ(absoluteChild.GetY(), 60); // 60 = 90 - 10 - 20
    EXPECT_EQ(absoluteChild.GetRect().GetX(), 120); // 120 = 30 + 90
    EXPECT_EQ(absoluteChild.GetRect().GetY(), 100); // 100 = 40 + 60

    absoluteAncestor.RemoveAll();
    flexLayout_->RemoveAll();
    root.RemoveAll();
}

/**
 * @tc.name: FlexLayout_GapPixelShorthand
 * @tc.desc: SetGap sets both row and column gaps; the main-axis gap applies between items.
 *           Container 300x300, two children 100x50, gap 20 -> second child at x=120.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_GapPixelShorthand, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetGap(20); // 20: gap for both directions

    EXPECT_EQ(flexLayout_->GetRowGap(), 20); // 20: row gap
    EXPECT_EQ(flexLayout_->GetColumnGap(), 20); // 20: column gap

    UIView view1;
    view1.Resize(100, 50); // 100: width; 50: height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 50);
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();

    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view2.GetX(), 120); // 120 = 100 + 20 gap
    flexLayout_->RemoveAll();
    flexLayout_->SetGap(0);
}

/**
 * @tc.name: FlexLayout_GapNegativeClampsToZero
 * @tc.desc: Negative row/column gap values are clamped to zero.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_GapNegativeClampsToZero, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetRowGap(-5); // -5: negative row gap
    flexLayout_->SetColumnGap(-5); // -5: negative column gap

    EXPECT_EQ(flexLayout_->GetRowGap(), 0);
    EXPECT_EQ(flexLayout_->GetColumnGap(), 0);

    UIView view1;
    view1.Resize(100, 50); // 100: width; 50: height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 50);
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view2.GetX(), 100); // 100: no gap applied
    flexLayout_->RemoveAll();
    flexLayout_->SetGap(0);
}

/**
 * @tc.name: FlexLayout_GapPercentResolvesAgainstContainer
 * @tc.desc: Percent row/column gaps resolve against the container height/width at layout time.
 *           Container 300x300, rowGap 50% -> 150, columnGap 10% -> 30; three children 100x50 wrap
 *           into 2 lines: child2 x=130 (gap in line), child3 y=200 (line1 cross 50 + rowGap 150).
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_GapPercentResolvesAgainstContainer, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(true);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetAlignContent(ALIGN_CONTENT_START);
    flexLayout_->SetRowGapPercent(50.0f); // 50: row gap percent of container height
    flexLayout_->SetColumnGapPercent(10.0f); // 10: column gap percent of container width

    UIView views[3];
    for (uint8_t i = 0; i < 3; i++) {
        views[i].Resize(100, 50); // 100: width; 50: height
        flexLayout_->Add(&views[i]);
    }
    flexLayout_->LayoutChildren();

    EXPECT_EQ(flexLayout_->GetRowGap(), 150); // 150 = 50% of 300
    EXPECT_EQ(flexLayout_->GetColumnGap(), 30); // 30 = 10% of 300
    EXPECT_EQ(views[1].GetX(), 130); // 130 = 100 + 30 column gap
    EXPECT_EQ(views[2].GetY(), 200); // 200 = 50 (line1 cross) + 150 (row gap)
    flexLayout_->RemoveAll();
    flexLayout_->SetGap(0);
}

/**
 * @tc.name: FlexLayout_GapPercentAutoBaseAndNonPositive
 * @tc.desc: Percent gap with an auto base resolves to zero; non-positive percent resolves to zero.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_GapPercentAutoBaseAndNonPositive, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetColumnGapPercent(10.0f); // 10: percent column gap
    flexLayout_->SetGapBaseAuto(true, false); // width base is auto -> percent resolves to zero
    flexLayout_->SetRowGapPercent(-5.0f); // -5: non-positive percent

    UIView view1;
    view1.Resize(100, 50); // 100: width; 50: height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 50);
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();

    EXPECT_EQ(flexLayout_->GetColumnGap(), 0); // 0: auto base resolves percent to zero
    EXPECT_EQ(flexLayout_->GetRowGap(), 0); // 0: non-positive percent resolves to zero
    EXPECT_EQ(view2.GetX(), 100); // 100: no gap applied
    flexLayout_->RemoveAll();
    flexLayout_->SetGapBaseAuto(false, false);
    flexLayout_->SetGap(0);
}

/**
 * @tc.name: FlexLayout_PhasedJustifyContentCenterEnd
 * @tc.desc: justify-content center/flex-end apply in the phased layout path (forced by align-self).
 *           Container 600x100, two children 100x100, free space 400.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_PhasedJustifyContentCenterEnd, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 100); // 600: width; 100: height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_CENTER);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view1;
    view1.Resize(100, 100); // 100: child width and height
    view1.SetAlignSelf(ALIGN_CENTER); // forces the phased path
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100);
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 200); // 200 = 400 free / 2
    EXPECT_EQ(view2.GetX(), 300); // 300 = 200 + 100

    flexLayout_->SetMajorAxisAlign(ALIGN_END);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 400); // 400 = 600 - 200
    flexLayout_->RemoveAll();
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
}

/**
 * @tc.name: FlexLayout_PhasedJustifyContentBetweenAroundEvenly
 * @tc.desc: justify-content space-between/around/evenly apply in the phased layout path.
 *           Container 600x100, two children 100x100, free space 400.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_PhasedJustifyContentBetweenAroundEvenly, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 100); // 600: width; 100: height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view1;
    view1.Resize(100, 100); // 100: child width and height
    view1.SetAlignSelf(ALIGN_CENTER); // forces the phased path
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100);
    flexLayout_->Add(&view2);

    flexLayout_->SetMajorAxisAlign(ALIGN_BETWEEN);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view2.GetX(), 500); // 500 = 100 + 400 interval

    flexLayout_->SetMajorAxisAlign(ALIGN_AROUND);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 100); // 100: half of the 200 interval
    EXPECT_EQ(view2.GetX(), 400); // 400 = 100 + 100 + 200

    flexLayout_->SetMajorAxisAlign(ALIGN_EVENLY);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 133); // 133 = 400 / 3
    EXPECT_EQ(view2.GetX(), 366); // 366 = 133 + 100 + 133
    flexLayout_->RemoveAll();
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
}

/**
 * @tc.name: FlexLayout_PhasedJustifyContentOverflowAndSingle
 * @tc.desc: justify-content with negative free space falls back to center; single-item line
 *           also packs at center in the phased path.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_PhasedJustifyContentOverflowAndSingle, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 100); // 600: width; 100: height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_CENTER);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view1;
    view1.Resize(400, 100); // 400: child width (overflows together with view2)
    view1.SetAlignSelf(ALIGN_CENTER); // forces the phased path
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(400, 100); // 400: child width
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    // free space is -200: negative free space packs from center
    EXPECT_EQ(view1.GetX(), -100); // -100: half of the overflow on the left
    EXPECT_EQ(view2.GetX(), 300); // 300: after the first child
    flexLayout_->RemoveAll();

    // single item in the line: packs at center
    UIView single;
    single.Resize(100, 100); // 100: child width and height
    single.SetAlignSelf(ALIGN_CENTER);
    flexLayout_->Add(&single);
    flexLayout_->SetMajorAxisAlign(ALIGN_BETWEEN);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(single.GetX(), 250); // 250: center of the remaining 500 free space
    flexLayout_->RemoveAll();
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
}

/**
 * @tc.name: FlexLayout_AspectRatioExplicitHeight
 * @tc.desc: aspect-ratio derives the main size from an explicit cross size.
 *           Explicit height 50, ratio 200 (2:1) -> width 100.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AspectRatioExplicitHeight, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view;
    view.Resize(0, 50); // 50: explicit height; width unset
    view.SetHasExplicitHeight(true);
    view.SetAspectRatio(200); // 200: ratio 2.0 (width = height * 2)
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetWidth(), 100); // 100 = 50 * 200 / 100
    EXPECT_EQ(view.GetHeight(), 50); // 50: explicit height unchanged
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_AspectRatioBothExplicit
 * @tc.desc: aspect-ratio is skipped when both dimensions are explicit.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AspectRatioBothExplicit, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view;
    view.Resize(60, 50); // 60: explicit width; 50: explicit height
    view.SetHasExplicitWidth(true);
    view.SetHasExplicitHeight(true);
    view.SetAspectRatio(200); // 200: ratio 2.0
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetWidth(), 60); // 60: unchanged
    EXPECT_EQ(view.GetHeight(), 50); // 50: unchanged
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_AspectRatioAutoBoth
 * @tc.desc: aspect-ratio derives width from an auto cross size in row direction,
 *           and height from an auto cross size in column direction.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AspectRatioAutoBoth, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView rowView;
    rowView.Resize(0, 40); // 40: measured height; width auto
    rowView.SetAspectRatio(200); // 200: ratio 2.0
    flexLayout_->Add(&rowView);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(rowView.GetWidth(), 80); // 80 = 40 * 200 / 100
    flexLayout_->RemoveAll();

    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    UIView columnView;
    columnView.Resize(60, 0); // 60: measured width; height auto
    columnView.SetAspectRatio(100); // 100: ratio 1.0
    flexLayout_->Add(&columnView);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(columnView.GetHeight(), 60); // 60 = 60 * 100 / 100
    flexLayout_->RemoveAll();
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
}

/**
 * @tc.name: FlexLayout_AbsoluteBothInsetsStretch
 * @tc.desc: absolute child with both left+right (top+bottom) insets stretches to the
 *           containing block minus the insets. Root container 300x160 acts as the
 *           initial containing block.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AbsoluteBothInsetsStretch, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 160); // 300: width; 160: height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView absoluteView;
    absoluteView.Resize(50, 50); // 50: original size (overridden by double insets)
    absoluteView.SetPositionType(POSITION_ABSOLUTE);
    absoluteView.SetFlexLeft(20); // 20: left inset
    absoluteView.SetFlexRight(30); // 30: right inset
    absoluteView.SetFlexTop(10); // 10: top inset
    absoluteView.SetFlexBottom(20); // 20: bottom inset
    flexLayout_->Add(&absoluteView);
    flexLayout_->LayoutChildren();

    EXPECT_EQ(absoluteView.GetX(), 20); // 20: left inset
    EXPECT_EQ(absoluteView.GetY(), 10); // 10: top inset
    EXPECT_EQ(absoluteView.GetWidth(), 250); // 250 = 300 - 20 - 30
    EXPECT_EQ(absoluteView.GetHeight(), 130); // 130 = 160 - 10 - 20
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_AbsoluteBothInsetsNegativeClamp
 * @tc.desc: absolute child with double insets exceeding the containing block clamps
 *           the stretched size to zero.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_AbsoluteBothInsetsNegativeClamp, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 160); // 300: width; 160: height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView absoluteView;
    absoluteView.Resize(50, 50); // 50: original size
    absoluteView.SetPositionType(POSITION_ABSOLUTE);
    absoluteView.SetFlexLeft(200); // 200: left inset
    absoluteView.SetFlexRight(200); // 200: right inset; 300 - 200 - 200 < 0 -> clamp to 0
    absoluteView.SetFlexTop(100); // 100: top inset
    absoluteView.SetFlexBottom(100); // 100: bottom inset; 160 - 100 - 100 < 0 -> clamp to 0
    flexLayout_->Add(&absoluteView);
    flexLayout_->LayoutChildren();

    EXPECT_EQ(absoluteView.GetWidth(), 0); // 0: clamped
    EXPECT_EQ(absoluteView.GetHeight(), 0); // 0: clamped
    EXPECT_EQ(absoluteView.GetX(), 200); // 200: left inset still applies
    EXPECT_EQ(absoluteView.GetY(), 100); // 100: top inset still applies
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_VerFlexBasis
 * @tc.desc: flex-basis sets the main size in column direction.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_VerFlexBasis, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view;
    view.Resize(100, 50); // 100: width; 50: height
    view.SetFlexBasis(120); // 120: basis becomes the height in column direction
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetHeight(), 120); // 120: basis applied on the vertical main axis
    flexLayout_->RemoveAll();
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
}

/**
 * @tc.name: FlexLayout_VerGrow
 * @tc.desc: flex-grow distributes free space in column direction.
 *           Container 300x300, two children 100x100 with grow 1/1 -> heights 150/150.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_VerGrow, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view1;
    view1.Resize(100, 100); // 100: child width and height
    view1.SetFlexGrow(1);
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100);
    view2.SetFlexGrow(1);
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetHeight(), 150); // 150 = 100 + 50 (half of 100 free space)
    EXPECT_EQ(view2.GetHeight(), 150); // 150: same
    EXPECT_EQ(view2.GetY(), 150); // 150: after the first child
    flexLayout_->RemoveAll();
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
}

/**
 * @tc.name: FlexLayout_VerShrink
 * @tc.desc: flex-shrink shrinks overflow in column direction.
 *           Container 300x300, two children 100x200 with shrink 1 -> heights 150/150.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_VerShrink, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: container width and height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view1;
    view1.Resize(100, 200); // 100: width; 200: height
    view1.SetFlexShrink(1);
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 200); // 100: width; 200: height
    view2.SetFlexShrink(1);
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetHeight(), 150); // 150 = 200 - 50 (half of 100 overflow)
    EXPECT_EQ(view2.GetHeight(), 150); // 150: same
    EXPECT_EQ(view2.GetY(), 150); // 150: after the first child
    flexLayout_->RemoveAll();
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
}

/**
 * @tc.name: FlexLayout_GapPercentInt16MaxClamp
 * @tc.desc: Percent gap resolves against a container size of INT16_MAX and clamps to INT16_MAX
 *           in ResolveGapPercent.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutEnhancementTest, FlexLayout_GapPercentInt16MaxClamp, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, INT16_MAX, INT16_MAX);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->SetRowGapPercent(100.0f); // 100: percent of INT16_MAX
    flexLayout_->SetColumnGapPercent(100.0f);

    UIView view;
    view.Resize(1, 10);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();

    EXPECT_EQ(flexLayout_->GetRowGap(), INT16_MAX);
    EXPECT_EQ(flexLayout_->GetColumnGap(), INT16_MAX);

    flexLayout_->RemoveAll();
    flexLayout_->SetGap(0);
}
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT
} // namespace OHOS
