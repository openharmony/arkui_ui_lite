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

#include "layout/flex_layout.h"

#include <climits>
#include <gtest/gtest.h>

using namespace testing::ext;
namespace OHOS {
namespace {
UIView* CreatView()
{
    uint16_t width = 100; // 100  view width
    uint16_t height = 100; // 100 view height
    auto view = new UIView();
    view->Resize(width, height);
    EXPECT_EQ(view->GetX(), 0);
    EXPECT_EQ(view->GetY(), 0);
    return view;
}

#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
void InitView(UIView& view)
{
    uint16_t width = 100;  // 100: view width
    uint16_t height = 100; // 100: view height
    view.Resize(width, height);
}
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT

}
class FlexLayoutTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static FlexLayout* flexLayout_;
};

FlexLayout* FlexLayoutTest::flexLayout_ = nullptr;

void FlexLayoutTest::SetUpTestCase(void)
{
    if (flexLayout_ == nullptr) {
        flexLayout_ = new FlexLayout();
        flexLayout_->SetPosition(0, 0, 600, 300); // 600: layout width; 300: layout height
    }
}

void FlexLayoutTest::TearDownTestCase(void)
{
    if (flexLayout_ != nullptr) {
        delete flexLayout_;
        flexLayout_ = nullptr;
    }
}

/**
 * @tc.name: FlexLayout_001
 * @tc.desc: Normal Process.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_001, TestSize.Level1)
{
    if (flexLayout_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    auto view1 = CreatView();
    flexLayout_->Add(view1);
    auto view2 = CreatView();
    flexLayout_->Add(view2);
    auto view3 = CreatView();
    flexLayout_->Add(view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1->GetX(), 0);
    EXPECT_EQ(view1->GetY(), 0);
    EXPECT_EQ(view2->GetX(), 100); // 100: view x after layout
    EXPECT_EQ(view2->GetY(), 0);
    EXPECT_EQ(view3->GetX(), 200); // 200: view x after layout
    EXPECT_EQ(view3->GetY(), 0);
    flexLayout_->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
}

/**
 * @tc.name: FlexLayout_002
 * @tc.desc: Normal Process.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_002, TestSize.Level1)
{
    if (flexLayout_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_CENTER);
    auto view1 = CreatView();
    flexLayout_->Add(view1);
    auto view2 = CreatView();
    flexLayout_->Add(view2);
    auto view3 = CreatView();
    flexLayout_->Add(view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1->GetX(), 0);
    EXPECT_EQ(view1->GetY(), 100); // 100: view x after layout
    EXPECT_EQ(view2->GetX(), 100); // 100: view x after layout
    EXPECT_EQ(view2->GetY(), 100); // 100: view y after layout
    EXPECT_EQ(view3->GetX(), 200); // 200: view x after layout
    EXPECT_EQ(view3->GetY(), 100); // 100: view y after layout
    flexLayout_->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
}

/**
 * @tc.name: FlexLayout_003
 * @tc.desc: Normal Process.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_003, TestSize.Level1)
{
    if (flexLayout_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR_R);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_CENTER);
    auto view1 = CreatView();
    flexLayout_->Add(view1);
    auto view2 = CreatView();
    flexLayout_->Add(view2);
    auto view3 = CreatView();
    flexLayout_->Add(view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1->GetX(), 500); // 500: view x after layout
    EXPECT_EQ(view1->GetY(), 100); // 100: view x after layout
    EXPECT_EQ(view2->GetX(), 400); // 400: view x after layout
    EXPECT_EQ(view2->GetY(), 100); // 100: view y after layout
    EXPECT_EQ(view3->GetX(), 300); // 300: view x after layout
    EXPECT_EQ(view3->GetY(), 100); // 100: view y after layout
    flexLayout_->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
}

/**
 * @tc.name: FlexLayout_004
 * @tc.desc: Normal Process.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_004, TestSize.Level1)
{
    if (flexLayout_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR_R);
    flexLayout_->SetMajorAxisAlign(ALIGN_CENTER);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_CENTER);
    auto view1 = CreatView();
    flexLayout_->Add(view1);
    auto view2 = CreatView();
    flexLayout_->Add(view2);
    auto view3 = CreatView();
    flexLayout_->Add(view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1->GetX(), 350); // 350: view x after layout
    EXPECT_EQ(view1->GetY(), 100); // 100: view x after layout
    EXPECT_EQ(view2->GetX(), 250); // 250: view x after layout
    EXPECT_EQ(view2->GetY(), 100); // 100: view y after layout
    EXPECT_EQ(view3->GetX(), 150); // 150: view x after layout
    EXPECT_EQ(view3->GetY(), 100); // 100: view y after layout
    flexLayout_->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
}

/**
 * @tc.name: FlexLayout_005
 * @tc.desc: Normal Process.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_005, TestSize.Level1)
{
    if (flexLayout_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_EVENLY);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    auto view1 = CreatView();
    flexLayout_->Add(view1);
    auto view2 = CreatView();
    flexLayout_->Add(view2);
    auto view3 = CreatView();
    flexLayout_->Add(view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1->GetX(), 0);
    EXPECT_EQ(view1->GetY(), 0);
    EXPECT_EQ(view2->GetX(), 0);
    EXPECT_EQ(view2->GetY(), 100); // 100: view y after layout
    EXPECT_EQ(view3->GetX(), 0);
    EXPECT_EQ(view3->GetY(), 200); // 200: view y after layout
    flexLayout_->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
}

/**
 * @tc.name: FlexLayout_006
 * @tc.desc: Normal Process.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_006, TestSize.Level1)
{
    if (flexLayout_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_AROUND);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    auto view1 = CreatView();
    flexLayout_->Add(view1);
    auto view2 = CreatView();
    flexLayout_->Add(view2);
    auto view3 = CreatView();
    flexLayout_->Add(view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1->GetX(), 0);
    EXPECT_EQ(view1->GetY(), 0);
    EXPECT_EQ(view2->GetX(), 0);
    EXPECT_EQ(view2->GetY(), 100); // 100: view y after layout
    EXPECT_EQ(view3->GetX(), 0);
    EXPECT_EQ(view3->GetY(), 200); // 200: view y after layout
    flexLayout_->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
}

/**
 * @tc.name: FlexLayout_007
 * @tc.desc: Normal Process.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_007, TestSize.Level1)
{
    if (flexLayout_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER_R);
    flexLayout_->SetMajorAxisAlign(ALIGN_BETWEEN);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    auto view1 = CreatView();
    flexLayout_->Add(view1);
    auto view2 = CreatView();
    flexLayout_->Add(view2);
    auto view3 = CreatView();
    flexLayout_->Add(view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1->GetX(), 0);
    EXPECT_EQ(view1->GetY(), 200);
    EXPECT_EQ(view2->GetX(), 0);
    EXPECT_EQ(view2->GetY(), 100); // 100: view y after layout
    EXPECT_EQ(view3->GetX(), 0);
    EXPECT_EQ(view3->GetY(), 0);
    flexLayout_->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
}

/**
 * @tc.name: FlexLayout_008
 * @tc.desc: Normal Process.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_008, TestSize.Level1)
{
    if (flexLayout_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER_R);
    flexLayout_->SetMajorAxisAlign(ALIGN_CENTER);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    auto view1 = CreatView();
    flexLayout_->Add(view1);
    auto view2 = CreatView();
    flexLayout_->Add(view2);
    auto view3 = CreatView();
    flexLayout_->Add(view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1->GetX(), 0);
    EXPECT_EQ(view1->GetY(), 200); // 200: view y after layout
    EXPECT_EQ(view2->GetX(), 0);
    EXPECT_EQ(view2->GetY(), 100); // 100: view y after layout
    EXPECT_EQ(view3->GetX(), 0);
    EXPECT_EQ(view3->GetY(), 0);
    flexLayout_->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
}

/**
 * @tc.name: FlexLayout_009
 * @tc.desc: Normal Process.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_009, TestSize.Level1)
{
    if (flexLayout_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: layout width; 300: layout height
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    auto view1 = CreatView();
    flexLayout_->Add(view1);
    auto view2 = CreatView();
    flexLayout_->Add(view2);
    auto view3 = CreatView();
    flexLayout_->Add(view3);
    auto view4 = CreatView();
    flexLayout_->Add(view4);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1->GetX(), 0);
    EXPECT_EQ(view1->GetY(), 0);
    EXPECT_EQ(view2->GetX(), 100);  // 100: view x after layout
    EXPECT_EQ(view2->GetY(), 0);
    EXPECT_EQ(view3->GetX(), 200);  // 200: view x after layout
    EXPECT_EQ(view3->GetY(), 0);
    EXPECT_EQ(view4->GetX(), 300);  // 300: view x after layout
    EXPECT_EQ(view4->GetY(), 0);
    flexLayout_->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
    delete view4;
}

/**
 * @tc.name: FlexLayout_010
 * @tc.desc: Normal Process.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_010, TestSize.Level1)
{
    if (flexLayout_ == nullptr) {
        EXPECT_EQ(1, 0);
        return;
    }
    flexLayout_->SetFlexWrap(true);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: layout width; 300: layout height
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    auto view1 = CreatView();
    flexLayout_->Add(view1);
    auto view2 = CreatView();
    flexLayout_->Add(view2);
    auto view3 = CreatView();
    flexLayout_->Add(view3);
    auto view4 = CreatView();
    flexLayout_->Add(view4);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1->GetX(), 0);
    EXPECT_EQ(view1->GetY(), 0);
    EXPECT_EQ(view2->GetX(), 100);  // 100: view x after layout
    EXPECT_EQ(view2->GetY(), 0);
    EXPECT_EQ(view3->GetX(), 200);  // 200: view x after layout
    EXPECT_EQ(view3->GetY(), 0);
    EXPECT_EQ(view4->GetX(), 0);
#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
    EXPECT_EQ(view4->GetY(), 150);
#else
    EXPECT_EQ(view4->GetY(), 100); // 100: view y after layout
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT
    flexLayout_->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
    delete view4;
}

#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
/**
 * @tc.name: FlexLayout_011
 * @tc.desc: Margin-left auto pushes child to the right side.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_011, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 100); // 300: layout width; 100: layout height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView viewA;
    viewA.Resize(50, 50); // 50x50 child
    flexLayout_->Add(&viewA);

    UIView viewB;
    viewB.Resize(50, 50); // 50x50 child
    viewB.SetMarginLeftAuto(true);
    flexLayout_->Add(&viewB);

    flexLayout_->LayoutChildren();

    // A starts flush with the left edge.
    EXPECT_EQ(viewA.GetX(), 0);
    EXPECT_EQ(viewA.GetY(), 0);
    // B is pushed to the right by margin-left:auto: x = 300 - 50 = 250
    EXPECT_EQ(viewB.GetX(), 250); // 250: view B x after auto margin
    EXPECT_EQ(viewB.GetY(), 0);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_012
 * @tc.desc: Margin-left auto takes priority over justify-content center.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_012, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 100); // 300: layout width; 100: layout height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_CENTER); // justify-content: center
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view;
    view.Resize(50, 50); // 50x50 child
    view.SetMarginLeftAuto(true);
    flexLayout_->Add(&view);

    flexLayout_->LayoutChildren();

    // margin-left:auto takes priority over justify-content:center
    // remaining space 300 - 50 = 250 is allocated entirely to margin-left
    // child is flush with the right edge: x = 250
    EXPECT_EQ(view.GetX(), 250); // 250: view x after auto margin overrides center align
    EXPECT_EQ(view.GetY(), 0);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_013
 * @tc.desc: Multiple children with margin-left auto split remaining space evenly.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_013, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 100); // 300: layout width; 100: layout height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView viewA;
    viewA.Resize(50, 50); // 50x50 child
    viewA.SetMarginLeftAuto(true);
    flexLayout_->Add(&viewA);

    UIView viewB;
    viewB.Resize(50, 50); // 50x50 child
    flexLayout_->Add(&viewB);

    UIView viewC;
    viewC.Resize(50, 50); // 50x50 child
    viewC.SetMarginLeftAuto(true);
    flexLayout_->Add(&viewC);

    flexLayout_->LayoutChildren();

    // remaining space 300 - (50+50+50) = 150, two auto margins each get 75
    // A: x = 75 (auto margin)
    EXPECT_EQ(viewA.GetX(), 75); // 75: auto margin value
    // B: x = 75 + 50 = 125 (right after A)
    EXPECT_EQ(viewB.GetX(), 125); // 125: view B x after A
    // C: x = 125 + 50 + 75 = 250 (auto margin)
    EXPECT_EQ(viewC.GetX(), 250); // 250: view C x after auto margin

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_014
 * @tc.desc: Display none elements are skipped during flex layout.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_014, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 100); // 300: layout width; 100: layout height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView viewA;
    viewA.Resize(50, 50); // 50x50 child
    flexLayout_->Add(&viewA);

    UIView viewB;
    viewB.Resize(50, 50); // 50x50 child
    viewB.SetVisible(false); // display: none
    flexLayout_->Add(&viewB);

    UIView viewC;
    viewC.Resize(50, 50); // 50x50 child
    flexLayout_->Add(&viewC);

    flexLayout_->LayoutChildren();

    // A and C are laid out normally, B is skipped
    EXPECT_EQ(viewA.GetX(), 0);
    EXPECT_EQ(viewA.GetY(), 0);
    // B is invisible, does not participate in layout, position remains default (0,0)
    EXPECT_EQ(viewB.GetX(), 0);
    EXPECT_EQ(viewB.GetY(), 0);
    // C is right after A, x = 50
    EXPECT_EQ(viewC.GetX(), 50); // 50: view C x after A (B skipped)
    EXPECT_EQ(viewC.GetY(), 0);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_015
 * @tc.desc: Overflow hidden clipping verification at UIView property level.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_015, TestSize.Level1)
{
    UIView view;
    // engine default overflow is OVERFLOW_VISIBLE (opt out of child clipping by default)
    EXPECT_EQ(view.GetOverflow(), OVERFLOW_VISIBLE);

    view.SetOverflow(OVERFLOW_HIDDEN);
    EXPECT_EQ(view.GetOverflow(), OVERFLOW_HIDDEN);

    view.SetOverflow(OVERFLOW_VISIBLE);
    EXPECT_EQ(view.GetOverflow(), OVERFLOW_VISIBLE);
}

/**
 * @tc.name: FlexLayout_016
 * @tc.desc: Horizontal layout with major axis END and secondary axis START.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_016, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300); // 600: layout width; 300: layout height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_END);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    InitView(view1);
    flexLayout_->Add(&view1);
    UIView view2;
    InitView(view2);
    flexLayout_->Add(&view2);
    UIView view3;
    InitView(view3);
    flexLayout_->Add(&view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 300); // 300: view x after END align
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 400); // 400: view x after END align
    EXPECT_EQ(view2.GetY(), 0);
    EXPECT_EQ(view3.GetX(), 500); // 500: view x after END align
    EXPECT_EQ(view3.GetY(), 0);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_017
 * @tc.desc: Horizontal layout with major axis END and secondary axis END.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_017, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_END);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);
    UIView view1;
    InitView(view1);
    flexLayout_->Add(&view1);
    UIView view2;
    InitView(view2);
    flexLayout_->Add(&view2);
    UIView view3;
    InitView(view3);
    flexLayout_->Add(&view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 300);
    EXPECT_EQ(view1.GetY(), 200); // 200: secondary END aligns to bottom
    EXPECT_EQ(view2.GetX(), 400);
    EXPECT_EQ(view2.GetY(), 200);
    EXPECT_EQ(view3.GetX(), 500);
    EXPECT_EQ(view3.GetY(), 200);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_018
 * @tc.desc: Horizontal layout with major axis BETWEEN and secondary axis START.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_018, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_BETWEEN);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    InitView(view1);
    flexLayout_->Add(&view1);
    UIView view2;
    InitView(view2);
    flexLayout_->Add(&view2);
    UIView view3;
    InitView(view3);
    flexLayout_->Add(&view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 250); // 250: (600-300)/2 + 100
    EXPECT_EQ(view2.GetY(), 0);
    EXPECT_EQ(view3.GetX(), 500); // 500: 250 + 100 + 150
    EXPECT_EQ(view3.GetY(), 0);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_019
 * @tc.desc: Horizontal layout with major axis AROUND and secondary axis START.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_019, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_AROUND);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    InitView(view1);
    flexLayout_->Add(&view1);
    UIView view2;
    InitView(view2);
    flexLayout_->Add(&view2);
    UIView view3;
    InitView(view3);
    flexLayout_->Add(&view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 50); // 50: interval/2 = 100/2
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 250); // 250: 50 + 100 + 100
    EXPECT_EQ(view2.GetY(), 0);
    EXPECT_EQ(view3.GetX(), 450); // 450: 250 + 100 + 100
    EXPECT_EQ(view3.GetY(), 0);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_020
 * @tc.desc: Horizontal layout with major axis EVENLY and secondary axis START.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_020, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_EVENLY);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    InitView(view1);
    flexLayout_->Add(&view1);
    UIView view2;
    InitView(view2);
    flexLayout_->Add(&view2);
    UIView view3;
    InitView(view3);
    flexLayout_->Add(&view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 75); // 75: interval = (600-300)/(3+1)
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 250); // 250: 75 + 100 + 75
    EXPECT_EQ(view2.GetY(), 0);
    EXPECT_EQ(view3.GetX(), 425); // 425: 250 + 100 + 75
    EXPECT_EQ(view3.GetY(), 0);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_021
 * @tc.desc: Vertical layout with major axis START and secondary axis START.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_021, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    InitView(view1);
    flexLayout_->Add(&view1);
    UIView view2;
    InitView(view2);
    flexLayout_->Add(&view2);
    UIView view3;
    InitView(view3);
    flexLayout_->Add(&view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 0);
    EXPECT_EQ(view2.GetY(), 100); // 100: view y after layout
    EXPECT_EQ(view3.GetX(), 0);
    EXPECT_EQ(view3.GetY(), 200); // 200: view y after layout
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_022
 * @tc.desc: Vertical layout with major axis END and secondary axis START (two views to show offset).
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_022, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_END);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    InitView(view1);
    flexLayout_->Add(&view1);
    UIView view2;
    InitView(view2);
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view1.GetY(), 100); // 100: 300 - 200 = 100
    EXPECT_EQ(view2.GetX(), 0);
    EXPECT_EQ(view2.GetY(), 200); // 200: 100 + 100
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_023
 * @tc.desc: Vertical layout with major axis START and secondary axis END (two views).
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_023, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 500); // 500: 600 - 100
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 500);
    EXPECT_EQ(view2.GetY(), 100);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_024
 * @tc.desc: Vertical layout with major axis CENTER and secondary axis END (two views).
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_024, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_CENTER);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 500);
    EXPECT_EQ(view1.GetY(), 50); // 50: (300 - 200) / 2
    EXPECT_EQ(view2.GetX(), 500);
    EXPECT_EQ(view2.GetY(), 150); // 150: 50 + 100
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_025
 * @tc.desc: Vertical layout with major axis BETWEEN and secondary axis START (two views).
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_025, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_BETWEEN);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 0);
    EXPECT_EQ(view2.GetY(), 200); // 200: 0 + 100 + 100(interval)
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_026
 * @tc.desc: Vertical layout with wrap enabled and four views (column wrap).
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_026, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300); // 600: layout width; 300: layout height
    flexLayout_->SetFlexWrap(true);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    UIView view3;
    view3.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view3);
    UIView view4;
    view4.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view4);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 0);
    EXPECT_EQ(view2.GetY(), 100); // 100: view y after layout
    EXPECT_EQ(view3.GetX(), 0);
    EXPECT_EQ(view3.GetY(), 200); // 200: view y after layout
    EXPECT_EQ(view4.GetX(), 300); // 300: stretched second column x
    EXPECT_EQ(view4.GetY(), 0);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_027
 * @tc.desc: Horizontal reverse layout with major axis CENTER and secondary axis START.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_027, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR_R);
    flexLayout_->SetMajorAxisAlign(ALIGN_CENTER);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    UIView view3;
    view3.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 350); // 350: 600 - 150 - 100
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 250); // 250: 600 - 250 - 100
    EXPECT_EQ(view2.GetY(), 0);
    EXPECT_EQ(view3.GetX(), 150); // 150: 600 - 350 - 100
    EXPECT_EQ(view3.GetY(), 0);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_028
 * @tc.desc: Empty children list should not crash during layout.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_028, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    flexLayout_->RemoveAll(); // ensure empty
    flexLayout_->LayoutChildren(); // should not crash
    EXPECT_EQ(flexLayout_->GetChildrenHead(), nullptr);
}

/**
 * @tc.name: FlexLayout_029
 * @tc.desc: Zero-sized child view should not affect layout of siblings.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_029, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);

    UIView view2;
    view2.Resize(0, 0); // 0x0 child
    flexLayout_->Add(&view2);

    UIView view3;
    view3.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view3);

    flexLayout_->LayoutChildren();

    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view1.GetY(), 0);
    // zero-sized view occupies no width, so view3 starts at same x as view2
    EXPECT_EQ(view2.GetX(), 100); // 100: right after view1
    EXPECT_EQ(view2.GetY(), 0);
    EXPECT_EQ(view3.GetX(), 100); // 100: same as view2 because view2 has zero width
    EXPECT_EQ(view3.GetY(), 0);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_030
 * @tc.desc: Horizontal wrap with align-content CENTER alignment.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_030, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300); // 300: layout width; 300: layout height
    flexLayout_->SetFlexWrap(true);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
    flexLayout_->SetAlignContent(ALIGN_CONTENT_CENTER);
#else
    flexLayout_->SetSecondaryAxisAlign(ALIGN_CENTER);
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    UIView view3;
    view3.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view3);
    UIView view4;
    view4.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view4);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view1.GetY(), 50); // 50: secondary CENTER offset for first row
    EXPECT_EQ(view2.GetX(), 100);
    EXPECT_EQ(view2.GetY(), 50);
    EXPECT_EQ(view3.GetX(), 200);
    EXPECT_EQ(view3.GetY(), 50);
    EXPECT_EQ(view4.GetX(), 0);
    EXPECT_EQ(view4.GetY(), 150); // 150: second row CENTER offset
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_031
 * @tc.desc: Horizontal wrap with align-content END alignment.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_031, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 300, 300);
    flexLayout_->SetFlexWrap(true);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
    flexLayout_->SetAlignContent(ALIGN_CONTENT_END);
#else
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    UIView view3;
    view3.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view3);
    UIView view4;
    view4.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view4);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view1.GetY(), 100); // 100: secondary END offset for first row
    EXPECT_EQ(view2.GetX(), 100);
    EXPECT_EQ(view2.GetY(), 100);
    EXPECT_EQ(view3.GetX(), 200);
    EXPECT_EQ(view3.GetY(), 100);
    EXPECT_EQ(view4.GetX(), 0);
    EXPECT_EQ(view4.GetY(), 200); // 200: second row END offset
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_032
 * @tc.desc: Vertical reverse layout with major axis START and secondary axis START.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_032, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER_R);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    UIView view3;
    view3.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 0);
    EXPECT_EQ(view1.GetY(), 200); // 200: 300 - 100
    EXPECT_EQ(view2.GetX(), 0);
    EXPECT_EQ(view2.GetY(), 100); // 100: 300 - 200
    EXPECT_EQ(view3.GetX(), 0);
    EXPECT_EQ(view3.GetY(), 0);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_033
 * @tc.desc: Vertical reverse layout with major axis END and secondary axis END (two views).
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_033, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER_R);
    flexLayout_->SetMajorAxisAlign(ALIGN_END);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 500); // 500: 600 - 100
    EXPECT_EQ(view1.GetY(), 100); // 100: 300 - 200 (END offset)
    EXPECT_EQ(view2.GetX(), 500);
    EXPECT_EQ(view2.GetY(), 0);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_034
 * @tc.desc: Horizontal layout with major axis CENTER and secondary axis END.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_034, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_CENTER);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    UIView view3;
    view3.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view3);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 150); // 150: (600-300)/2
    EXPECT_EQ(view1.GetY(), 200); // 200: secondary END
    EXPECT_EQ(view2.GetX(), 250);
    EXPECT_EQ(view2.GetY(), 200);
    EXPECT_EQ(view3.GetX(), 350);
    EXPECT_EQ(view3.GetY(), 200);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_035
 * @tc.desc: Vertical wrap with align-content END alignment.
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_035, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(true);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
    flexLayout_->SetAlignContent(ALIGN_CONTENT_END);
#else
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    UIView view3;
    view3.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view3);
    UIView view4;
    view4.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view4);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 400); // 400: secondary END for first column
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 400);
    EXPECT_EQ(view2.GetY(), 100);
    EXPECT_EQ(view3.GetX(), 400);
    EXPECT_EQ(view3.GetY(), 200);
    EXPECT_EQ(view4.GetX(), 500); // 500: secondary END for second column
    EXPECT_EQ(view4.GetY(), 0);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_036
 * @tc.desc: Vertical layout with major axis EVENLY and secondary axis END (two views).
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_036, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_EVENLY);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 500);
    EXPECT_EQ(view1.GetY(), 33); // 33: interval = (300-200)/(2+1)
    EXPECT_EQ(view2.GetX(), 500);
    EXPECT_EQ(view2.GetY(), 166); // 166: 33 + 100 + 33
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_037
 * @tc.desc: Vertical layout with major axis AROUND and secondary axis END (two views).
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_037, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_AROUND);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 500);
    EXPECT_EQ(view1.GetY(), 25); // 25: interval/2 = 50/2
    EXPECT_EQ(view2.GetX(), 500);
    EXPECT_EQ(view2.GetY(), 175); // 175: 25 + 100 + 50
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_038
 * @tc.desc: Vertical layout with major axis BETWEEN and secondary axis END (two views).
 * @tc.type: FUNC
 * @tc.require: AR000DSMR7
 */
HWTEST_F(FlexLayoutTest, FlexLayout_038, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_VER);
    flexLayout_->SetMajorAxisAlign(ALIGN_BETWEEN);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_END);
    UIView view1;
    view1.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view1);
    UIView view2;
    view2.Resize(100, 100); // 100: view width and height
    flexLayout_->Add(&view2);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view1.GetX(), 500);
    EXPECT_EQ(view1.GetY(), 0);
    EXPECT_EQ(view2.GetX(), 500);
    EXPECT_EQ(view2.GetY(), 200); // 200: 0 + 100 + 100(interval)
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_039
 * @tc.desc: Row-reverse layout with margin-left auto. The auto margin absorbs the
 *           remaining space on the main-start side (physical right in row-reverse).
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_039, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300); // 600: layout width; 300: layout height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR_R);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView viewA;
    viewA.Resize(100, 100); // 100x100 child
    viewA.SetMarginLeftAuto(true);
    flexLayout_->Add(&viewA);

    UIView viewB;
    viewB.Resize(100, 100); // 100x100 child
    flexLayout_->Add(&viewB);

    flexLayout_->LayoutChildren();

    // remaining space 600 - (100+100) = 400 is allocated to margin-left of A,
    // in row-reverse the space sits on the physical right side of A:
    // A: x = 600 - 400 - 100 = 100
    EXPECT_EQ(viewA.GetX(), 100); // 100: view A x after auto margin in row-reverse
    EXPECT_EQ(viewA.GetY(), 0);
    // B: x = 600 - 500 - 100 = 0 (physical left edge)
    EXPECT_EQ(viewB.GetX(), 0);
    EXPECT_EQ(viewB.GetY(), 0);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_040
 * @tc.desc: Auto margin is clamped to zero when children overflow the container
 *           (negative remaining space), children are placed from the main-start side.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_040, TestSize.Level1)
{
    ASSERT_NE(flexLayout_, nullptr);
    flexLayout_->SetPosition(0, 0, 600, 300); // 600: layout width; 300: layout height
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);

    UIView viewA;
    viewA.Resize(300, 100); // 300x100 child
    viewA.SetMarginLeftAuto(true);
    flexLayout_->Add(&viewA);

    UIView viewB;
    viewB.Resize(300, 100); // 300x100 child
    viewB.SetStyle(STYLE_MARGIN_LEFT, 50); // 50: fixed margin left
    flexLayout_->Add(&viewB);

    flexLayout_->LayoutChildren();

    // fixed total width 300 + (50 + 300) = 650 > 600, remaining space is negative
    // and clamped to 0, so the auto margin resolves to 0
    EXPECT_EQ(viewA.GetX(), 0);
    EXPECT_EQ(viewA.GetY(), 0);
    // B: x = 0 + 300 + 50 - 50 = 300
    EXPECT_EQ(viewB.GetX(), 300); // 300: view B x right after view A
    EXPECT_EQ(viewB.GetY(), 0);

    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_AlignSelfCenter
 * @tc.desc: A flex item overrides the parent's cross-axis start alignment with center.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_AlignSelfCenter, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 300, 200);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view;
    view.Resize(50, 50);
    view.SetAlignSelf(ALIGN_CENTER);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetY(), 75);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_AlignSelfFlexEnd
 * @tc.desc: A flex item overrides the parent's cross-axis center alignment with flex-end.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_AlignSelfFlexEnd, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 300, 200);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_CENTER);
    UIView view;
    view.Resize(50, 50);
    view.SetAlignSelf(ALIGN_END);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetY(), 150);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_AlignSelfKeepsDefaultMainSize
 * @tc.desc: Align-self alone does not shrink items on the main axis.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_AlignSelfKeepsDefaultMainSize, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 200, 200);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView first;
    UIView second;
    UIView third;
    first.Resize(100, 100);
    second.Resize(100, 100);
    third.Resize(100, 100);
    second.SetAlignSelf(ALIGN_CENTER);
    flexLayout_->Add(&first);
    flexLayout_->Add(&second);
    flexLayout_->Add(&third);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(first.GetWidth(), 100);
    EXPECT_EQ(second.GetWidth(), 100);
    EXPECT_EQ(third.GetWidth(), 100);
    EXPECT_EQ(second.GetY(), 50);
    EXPECT_EQ(third.GetX(), 200);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_AlignSelfStretch
 * @tc.desc: A flex item without an explicit cross-axis size stretches to the parent height.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_AlignSelfStretch, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 300, 200);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_CENTER);
    UIView view;
    view.Resize(50, 50);
    view.SetAlignSelf(UIView::ALIGN_SELF_STRETCH);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetY(), 0);
    EXPECT_EQ(view.GetHeight(), 200);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_AlignSelfStretchWithExplicitHeight
 * @tc.desc: Stretch does not overwrite an explicit cross-axis size.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_AlignSelfStretchWithExplicitHeight, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 300, 200);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_CENTER);
    UIView view;
    view.Resize(50, 50);
    view.SetHasExplicitHeight(true);
    view.SetAlignSelf(UIView::ALIGN_SELF_STRETCH);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetY(), 0);
    EXPECT_EQ(view.GetHeight(), 50);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_FlexGrow
 * @tc.desc: Verify remaining main-axis space is assigned with flex-grow ratio 1:2:1.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_FlexGrow, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView first;
    UIView second;
    UIView third;
    first.Resize(100, 100);
    second.Resize(100, 100);
    third.Resize(100, 100);
    first.SetFlexGrow(1);
    second.SetFlexGrow(2);
    third.SetFlexGrow(1);
    flexLayout_->Add(&first);
    flexLayout_->Add(&second);
    flexLayout_->Add(&third);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(first.GetWidth(), 175);
    EXPECT_EQ(second.GetWidth(), 250);
    EXPECT_EQ(third.GetWidth(), 175);
    EXPECT_EQ(second.GetX(), 175);
    EXPECT_EQ(third.GetX(), 425);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_FlexShrink
 * @tc.desc: Verify flex-shrink:0 keeps an item size while the other items shrink.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_FlexShrink, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 200, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView first;
    UIView second;
    UIView third;
    first.Resize(100, 100);
    second.Resize(100, 100);
    third.Resize(100, 100);
    first.SetFlexShrink(1);
    second.SetFlexShrink(2);
    third.SetFlexShrink(0);
    flexLayout_->Add(&first);
    flexLayout_->Add(&second);
    flexLayout_->Add(&third);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(first.GetWidth(), 67);
    EXPECT_EQ(second.GetWidth(), 33);
    EXPECT_EQ(third.GetWidth(), 100);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_FlexBasisPx
 * @tc.desc: Verify a pixel flex-basis replaces the child main-axis size.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_FlexBasisPx, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 600, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView first;
    UIView second;
    first.Resize(100, 100);
    second.Resize(100, 100);
    first.SetFlexBasis(200);
    flexLayout_->Add(&first);
    flexLayout_->Add(&second);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(first.GetWidth(), 200);
    EXPECT_EQ(second.GetX(), 200);
    EXPECT_EQ(second.GetWidth(), 100);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_MinWidthStopsShrink
 * @tc.desc: A shrinking flex item does not become narrower than min-width.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_MinWidthStopsShrink, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 60, 200);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view;
    view.Resize(100, 100);
    view.SetFlexShrink(1);
    view.SetMinWidth(80);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetWidth(), 80);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_MaxWidthStopsGrow
 * @tc.desc: A growing flex item does not become wider than max-width.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_MaxWidthStopsGrow, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 300, 200);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view;
    view.Resize(100, 100);
    view.SetFlexGrow(1);
    view.SetMaxWidth(180);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetWidth(), 180);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_MaxWidthFreezeRedistributesGrow
 * @tc.desc: A flex item capped by max-width freezes, and the remaining grow space is redistributed.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_MaxWidthFreezeRedistributesGrow, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 350, 200); // 350: container width
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view1;
    UIView view2;
    UIView view3;
    UIView view4;
    UIView view5;
    UIView view6;
    view1.Resize(50, 100); // 50: initial width
    view2.Resize(50, 100); // 50: initial width
    view3.Resize(50, 100); // 50: initial width
    view4.Resize(50, 100); // 50: initial width
    view5.Resize(50, 100); // 50: initial width
    view6.Resize(50, 100); // 50: initial width
    view1.SetFlexGrow(1);
    view2.SetFlexGrow(1);
    view3.SetFlexGrow(1);
    view4.SetFlexGrow(1);
    view5.SetFlexGrow(1);
    view1.SetMaxWidth(100); // 100: max width
    view2.SetMaxWidth(100); // 100: max width
    view3.SetMaxWidth(100); // 100: max width
    view4.SetMaxWidth(-20); // -20: negative max width means unlimited in layout
    view5.SetMaxWidth(0);
    flexLayout_->Add(&view1);
    flexLayout_->Add(&view2);
    flexLayout_->Add(&view3);
    flexLayout_->Add(&view4);
    flexLayout_->Add(&view5);
    flexLayout_->Add(&view6);

    flexLayout_->LayoutChildren();

    EXPECT_EQ(view1.GetWidth(), 75); // 75: redistributed grow width
    EXPECT_EQ(view2.GetWidth(), 75); // 75: redistributed grow width
    EXPECT_EQ(view3.GetWidth(), 75); // 75: redistributed grow width
    EXPECT_EQ(view4.GetWidth(), 75); // 75: redistributed grow width
    EXPECT_EQ(view5.GetWidth(), 0);
    EXPECT_EQ(view6.GetWidth(), 50); // 50: no flex-grow
    EXPECT_EQ(view6.GetX(), 300); // 300: 75 * 4
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_MinHeightConstraint
 * @tc.desc: A flex item does not become shorter than min-height.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_MinHeightConstraint, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 300, 200);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view;
    view.Resize(100, 40);
    view.SetMinHeight(70);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetHeight(), 70);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_MaxHeightConstraint
 * @tc.desc: A flex item does not become taller than max-height.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_MaxHeightConstraint, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 300, 200);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view;
    view.Resize(100, 120);
    view.SetMaxHeight(80);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetHeight(), 80);
    flexLayout_->RemoveAll();
}

/**
 * @tc.name: FlexLayout_AspectRatioFromExplicitWidth
 * @tc.desc: An explicit width and aspect-ratio compute the item height.
 * @tc.type: FUNC
 */
HWTEST_F(FlexLayoutTest, FlexLayout_AspectRatioFromExplicitWidth, TestSize.Level1)
{
    flexLayout_->SetFlexWrap(false);
    flexLayout_->SetPosition(0, 0, 500, 300);
    flexLayout_->SetLayoutDirection(LAYOUT_HOR);
    flexLayout_->SetMajorAxisAlign(ALIGN_START);
    flexLayout_->SetSecondaryAxisAlign(ALIGN_START);
    UIView view;
    view.Resize(300, 50);
    view.SetHasExplicitWidth(true);
    view.SetAspectRatio(150);
    flexLayout_->Add(&view);
    flexLayout_->LayoutChildren();
    EXPECT_EQ(view.GetWidth(), 300);
    EXPECT_EQ(view.GetHeight(), 200);
    flexLayout_->RemoveAll();
}

#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT

} // namespace OHOS
