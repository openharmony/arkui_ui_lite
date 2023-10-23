/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common/spannable_string.h"
#include "font/ui_font_header.h"
#include "gfx_utils/color.h"
#include "gfx_utils/list.h"
#include "securec.h"

#include <climits>
#include <gtest/gtest.h>
#if defined(ENABLE_SPANNABLE_STRING) && ENABLE_SPANNABLE_STRING

using namespace testing::ext;
namespace OHOS {
class SP_test : public SpannableString {
public:
    uint16_t GetSpanListSize()
    {
        return spanList_.Size();
    }
    uint16_t GetSizeListSize()
    {
        return sizeList_.Size();
    }
    uint16_t GetFontIdListSize()
    {
        return fontIdList_.Size();
    }
    uint16_t GetHeightListSize()
    {
        return heightList_.Size();
    }
    uint16_t GetIsSpannableLen()
    {
        return isSpannableLen_;
    }
    bool SetSpannable(bool value, uint16_t startIndex, uint16_t endIndex)
    {
        return SpannableString::SetSpannable(value, startIndex, endIndex);
    }
    void SetFontHeight(int16_t inputHeight, uint16_t startIndex, uint16_t endIndex)
    {
        return SpannableString::SetFontHeight(inputHeight, startIndex, endIndex);
    }
};

class SpannableStringTest : public testing::Test {
public:
    SpannableStringTest() : spannableString_(nullptr) {}
    virtual ~SpannableStringTest() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp();
    void TearDown();
    SP_test *spannableString_;
};

void SpannableStringTest::SetUp()
{
    if (spannableString_ == nullptr) {
        spannableString_ = new SP_test();
    }
}

void SpannableStringTest::TearDown()
{
    if (spannableString_ != nullptr) {
        delete spannableString_;
        spannableString_ = nullptr;
    }
}

/**
 * @tc.name: SpannableString_SpannableString_001
 * @tc.desc: Verify SpannableString function, each attribute shall be  0 or nullptr, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SpannableString_001, TestSize.Level1)
{
    if (spannableString_ == nullptr) {
        EXPECT_NE(spannableString_, nullptr);
        return;
    }
    EXPECT_EQ(spannableString_->GetSpanListSize(), 0);
    EXPECT_EQ(spannableString_->GetIsSpannableLen(), 0);
    EXPECT_EQ(spannableString_->GetSizeListSize(), 0);
    EXPECT_EQ(spannableString_->GetFontIdListSize(), 0);
    EXPECT_EQ(spannableString_->GetHeightListSize(), 0);
}

/**
 * @tc.name: SpannableString_SetSpannable_001
 * @tc.desc: Verify SetSpannable function, capability, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetSpannable_001, TestSize.Level1)
{
    if (spannableString_ == nullptr) {
        EXPECT_NE(spannableString_, nullptr);
        return;
    }
    uint16_t default_is_spannable_len = 10; // default_is_spannable_len is 10
    uint16_t start_1 = 2;
    uint16_t end_1 = 5;
    spannableString_->SetSpannable(true, start_1, end_1);
    EXPECT_EQ(spannableString_->GetSpannableLen(), default_is_spannable_len);
    for (uint16_t i = 0; i < start_1; i++) {
        EXPECT_EQ(spannableString_->GetSpannable(i), false);
    }
    for (uint16_t i = start_1; i < end_1; i++) {
        EXPECT_EQ(spannableString_->GetSpannable(i), true);
    }
    for (uint16_t i = end_1; i < default_is_spannable_len; i++) {
        EXPECT_EQ(spannableString_->GetSpannable(i), false);
    }
}

/**
 * @tc.name: SpannableString_SetSpannable_002
 * @tc.desc: Verify SetSpannable function, out-of-bound, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetSpannable_002, TestSize.Level1)
{
    uint16_t default_is_spannable_len = 10;
    uint16_t start = 9;
    uint16_t end = 12;
    spannableString_->SetSpannable(true, start, end);
    EXPECT_EQ(default_is_spannable_len * 2 + 1, spannableString_->GetIsSpannableLen());
    if ((default_is_spannable_len * 2 + 1) == spannableString_->GetIsSpannableLen()) {
        for (uint16_t i = 0; i < start; i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), false);
        }
        for (uint16_t i = start; i < end; i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), true);
        }
        for (uint16_t i = end; i < (default_is_spannable_len * 2 + 1); i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), true);
        }
    }
}

/**
 * @tc.name: SpannableString_SetSpannable_002
 * @tc.desc: Verify SetSpannable function, out-of-bound, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetSpannable_003, TestSize.Level1)
{
    uint16_t default_is_spannable_len = 10;
    uint16_t start = 0;
    uint16_t end = 10;
    spannableString_->SetSpannable(true, start, end);
    EXPECT_EQ(default_is_spannable_len, spannableString_->GetIsSpannableLen());
    if (default_is_spannable_len == spannableString_->GetIsSpannableLen()) {
        for (uint16_t i = 0; i < start; i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), false);
        }
        for (uint16_t i = start; i < default_is_spannable_len; i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), true);
        }
    }
}

/**
 * @tc.name: SpannableString_SetSpannable_004
 * @tc.desc: Verify SetSpannable function, out-of-bound, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetSpannable_004, TestSize.Level1)
{
    uint16_t default_is_spannable_len = 10;
    uint16_t start = 0;
    uint16_t end = 0;
    spannableString_->SetSpannable(true, start, end);

    // since (0<0)!=true, isSpannableLen is still 0.
    EXPECT_EQ(0, spannableString_->GetIsSpannableLen());
}

/**
 * @tc.name: SpannableString_SetSpannable_005
 * @tc.desc: Verify SetSpannable function, out-of-bound, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetSpannable_005, TestSize.Level1)
{
    uint16_t default_is_spannable_len = 10;
    uint16_t start = 10;
    uint16_t end = 10;
    spannableString_->SetSpannable(true, start, end);
    EXPECT_EQ(default_is_spannable_len, spannableString_->GetIsSpannableLen());
    if (default_is_spannable_len == spannableString_->GetIsSpannableLen()) {
        for (uint16_t i = 0; i < start; i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), false);
        }
    }
}

/**
 * @tc.name: SpannableString_SetSpannable_006
 * @tc.desc: Verify SetSpannable function, out-of-bound, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetSpannable_006, TestSize.Level1)
{
    uint16_t default_is_spannable_len = 10;
    uint16_t start = 9;
    uint16_t end = 10;
    spannableString_->SetSpannable(true, start, end);
    EXPECT_EQ(default_is_spannable_len, spannableString_->GetIsSpannableLen());
    if (default_is_spannable_len == spannableString_->GetIsSpannableLen()) {
        for (uint16_t i = 0; i < start; i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), false);
        }
        for (uint16_t i = start; i < end; i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), true);
        }
    }
}

/**
 * @tc.name: SpannableString_SetSpannable_007
 * @tc.desc: Verify SetSpannable function, de-duplicate, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetSpannable_007, TestSize.Level1)
{
    uint16_t default_is_spannable_len = 10;
    uint16_t start_1 = 2;
    uint16_t end_1 = 5;
    spannableString_->SetSpannable(true, start_1, end_1);
    uint16_t start_2 = 4;
    uint16_t end_2 = 6;
    spannableString_->SetSpannable(true, start_2, end_2);
    EXPECT_EQ(default_is_spannable_len, spannableString_->GetIsSpannableLen());
    if (default_is_spannable_len == spannableString_->GetIsSpannableLen()) {
        for (uint16_t i = 0; i < start_1; i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), false);
        }
        for (uint16_t i = start_1; i < end_2; i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), true);
        }
        for (uint16_t i = end_2; i < default_is_spannable_len; i++) {
            EXPECT_EQ(spannableString_->GetSpannable(i), false);
        }
    }
}

/**
 * @tc.name: SpannableString_GetSpannable_001
 * @tc.desc: Verify SetSpannable function, must trust isSpannableLen_ is right, then verify capability, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_GetSpannable_001, TestSize.Level1)
{
    uint16_t default_is_spannable_len = 10;
    EXPECT_EQ(default_is_spannable_len, spannableString_->GetIsSpannableLen());
    uint16_t start = 2;
    uint16_t end = 5;
    spannableString_->SetSpannable(true, start, end);
    for (uint16_t i = start; i < end; i++) {
        bool value = spannableString_->GetSpannable(i);
        EXPECT_EQ(true, value);
    }
}

/**
 * @tc.name: SpannableString_GetSpannable_002
 * @tc.desc: Verify GetSpannable function, must trust isSpannableLen_ is right, out-of-bound, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_GetSpannable_002, TestSize.Level1)
{
    uint16_t default_is_spannable_len = 10;
    uint16_t start = 2;
    uint16_t end = 5;
    spannableString_->SetSpannable(true, start, end);
    EXPECT_EQ(default_is_spannable_len, spannableString_->GetIsSpannableLen());
    bool value_0 = spannableString_->GetSpannable(0);
    bool value_2 = spannableString_->GetSpannable(2);
    bool value_4 = spannableString_->GetSpannable(4);
    bool value_5 = spannableString_->GetSpannable(5);
    bool value_9 = spannableString_->GetSpannable(9);
    bool value_10 = spannableString_->GetSpannable(10);
    EXPECT_EQ(false, value_0);
    EXPECT_EQ(true, value_2);
    EXPECT_EQ(true, value_4);
    EXPECT_EQ(false, value_5);
    EXPECT_EQ(false, value_9);
    EXPECT_EQ(false, value_10);
}

/**
 * @tc.name: SpannableString_SetFontSize_001
 * @tc.desc: Verify SetFontSize function, must trust GetFontSize is OK, then verify SetFontSize, capability, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_001, TestSize.Level1)
{
    uint8_t fontSize = 16;
    uint16_t start = 2;
    uint16_t end = 5;
    spannableString_->SetFontSize(fontSize, start, end);
    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = start; i < end; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_002
 * @tc.desc: Verify SetFontSize function, de-duplicate, case same font 0, not draw on requirements analysis, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_002, TestSize.Level1)
{
    uint8_t fontSize = 16;
    uint16_t start_1 = 2;
    uint16_t end_1 = 4;
    spannableString_->SetFontSize(fontSize, start_1, end_1);
    uint16_t start_2 = 6;
    uint16_t end_2 = 8;
    spannableString_->SetFontSize(fontSize, start_2, end_2);
    EXPECT_EQ(2, spannableString_->GetSizeListSize());
    for (uint16_t i = start_1; i < end_1; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize, tempFontSize);
    }
    for (uint16_t i = start_2; i < end_2; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_003
 * @tc.desc: Verify SetFontSize function, de-duplicate, same font case 1, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_003, TestSize.Level1)
{
    uint8_t fontSize = 16;
    uint16_t start_1 = 4;
    uint16_t end_1 = 6;
    spannableString_->SetFontSize(fontSize, start_1, end_1);
    uint16_t start_2 = 2;
    uint16_t end_2 = 5;
    spannableString_->SetFontSize(fontSize, start_2, end_2);
    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = start_2; i < end_1; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_004
 * @tc.desc: Verify SetFontSize function, de-duplicate, same font case 2, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_004, TestSize.Level1)
{
    uint8_t fontSize = 16;
    uint16_t start_1 = 2;
    uint16_t end_1 = 8;
    spannableString_->SetFontSize(fontSize, start_1, end_1);
    uint16_t start_2 = 4;
    uint16_t end_2 = 6;
    spannableString_->SetFontSize(fontSize, start_2, end_2);
    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = start_1; i < end_1; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_005
 * @tc.desc: Verify SetFontSize function, de-duplicate, same font case 3, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_005, TestSize.Level1)
{
    uint8_t fontSize = 16;
    uint16_t start_1 = 2;
    uint16_t end_1 = 6;
    spannableString_->SetFontSize(fontSize, start_1, end_1);
    uint16_t start_2 = 4;
    uint16_t end_2 = 8;
    spannableString_->SetFontSize(fontSize, start_2, end_2);
    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = start_1; i < end_2; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_006
 * @tc.desc: Verify SetFontSize function, de-duplicate, same font case 4, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_006, TestSize.Level1)
{
    uint8_t fontSize = 16;
    uint16_t start_1 = 4;
    uint16_t end_1 = 6;
    spannableString_->SetFontSize(fontSize, start_1, end_1);
    uint16_t start_2 = 2;
    uint16_t end_2 = 8;
    spannableString_->SetFontSize(fontSize, start_2, end_2);
    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = start_2; i < end_2; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_007
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 1, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_007, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 2;
    uint16_t end_red = 4;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 6;
    uint16_t end_green = 8;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(2, spannableString_->GetSizeListSize());
    for (uint16_t i = start_red; i < end_red; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_red, tempFontSize);
    }
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_008
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 2, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_008, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 2;
    uint16_t end_red = 4;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 4;
    uint16_t end_green = 6;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(2, spannableString_->GetSizeListSize());
    for (uint16_t i = start_red; i < start_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_red, tempFontSize);
    }
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_009
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 3, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_009, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 2;
    uint16_t end_red = 6;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 4;
    uint16_t end_green = 8;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(2, spannableString_->GetSizeListSize());
    for (uint16_t i = start_red; i < start_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_red, tempFontSize);
    }
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_010
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 4, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_010, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 2;
    uint16_t end_red = 6;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 4;
    uint16_t end_green = 6;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(2, spannableString_->GetSizeListSize());
    for (uint16_t i = start_red; i < start_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_red, tempFontSize);
    }
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_011
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 5, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_011, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 2;
    uint16_t end_red = 8;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 4;
    uint16_t end_green = 6;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(3, spannableString_->GetSizeListSize());
    for (uint16_t i = start_red; i < start_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_red, tempFontSize);
    }
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
    for (uint16_t i = end_green; i < end_red; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_red, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_012
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 6, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_012, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 2;
    uint16_t end_red = 4;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 2;
    uint16_t end_green = 6;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_013
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 7, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_013, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 2;
    uint16_t end_red = 6;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 2;
    uint16_t end_green = 6;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_014
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 8, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_014, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 2;
    uint16_t end_red = 8;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 2;
    uint16_t end_green = 6;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(2, spannableString_->GetSizeListSize());
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
    for (uint16_t i = end_green; i < end_red; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_red, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_015
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 9, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_015, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 4;
    uint16_t end_red = 6;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 2;
    uint16_t end_green = 8;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_016
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 10, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_016, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 4;
    uint16_t end_red = 6;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 2;
    uint16_t end_green = 6;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_017
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 11, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_017, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 4;
    uint16_t end_red = 8;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 2;
    uint16_t end_green = 6;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(2, spannableString_->GetSizeListSize());
    for (uint16_t i = end_green; i < end_red; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_red, tempFontSize);
    }
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_018
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 12, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_018, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 6;
    uint16_t end_red = 8;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 2;
    uint16_t end_green = 6;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(2, spannableString_->GetSizeListSize());
    for (uint16_t i = start_red; i < end_red; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_red, tempFontSize);
    }
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}

/**
 * @tc.name: SpannableString_SetFontSize_019
 * @tc.desc: Verify SetFontSize function, de-duplicate, different font case 13, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontSize_019, TestSize.Level1)
{
    uint8_t fontSize_red = 16;
    uint16_t start_red = 6;
    uint16_t end_red = 8;
    spannableString_->SetFontSize(fontSize_red, start_red, end_red);
    uint8_t fontSize_green = 23;
    uint16_t start_green = 2;
    uint16_t end_green = 6;
    spannableString_->SetFontSize(fontSize_green, start_green, end_green);
    EXPECT_EQ(2, spannableString_->GetSizeListSize());
    for (uint16_t i = start_red; i < end_red; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_red, tempFontSize);
    }
    for (uint16_t i = start_green; i < end_green; i++) {
        uint8_t tempFontSize = 0;
        spannableString_->GetFontSize(i, tempFontSize);
        EXPECT_EQ(fontSize_green, tempFontSize);
    }
}
/* * here de-duplicate has already been verified, no need verifiy anymore */

/**
 * @tc.name: SpannableString_GetFontSize_001
 * @tc.desc: Verify SetFontSize function, capability & bound, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_GetFontSize_001, TestSize.Level1)
{
    uint8_t fontSize = 16;
    uint16_t start = 6;
    uint16_t end = 8;
    spannableString_->SetFontSize(fontSize, start, end);
    uint8_t output_0 = 0;
    bool hasGet_0 = spannableString_->GetFontSize(0, output_0);
    EXPECT_EQ(false, hasGet_0);
    EXPECT_EQ(0, output_0);
    uint8_t output_6 = 0;
    bool hasGet_6 = spannableString_->GetFontSize(6, output_6);
    EXPECT_EQ(true, hasGet_6);
    EXPECT_EQ(fontSize, output_6);
    uint8_t output_8 = 0;
    bool hasGet_8 = spannableString_->GetFontSize(8, output_8);
    EXPECT_EQ(false, hasGet_8);
    EXPECT_EQ(0, output_8);
}

/**
 * @tc.name: SpannableString_SetFontId_001
 * @tc.desc: Verify SetFontId function, capability, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontId_001, TestSize.Level1)
{
    uint16_t fontId = 16;
    uint16_t start = 6;
    uint16_t end = 8;
    spannableString_->SetFontId(fontId, start, end);
    EXPECT_EQ(1, spannableString_->GetFontIdListSize());
    for (uint16_t i = start; i < end; i++) {
        uint16_t tempFontId = 0;
        spannableString_->GetFontId(i, tempFontId);
        EXPECT_EQ(fontId, tempFontId);
    }
}

/**
 * @tc.name: SpannableString_GetFontId_001
 * @tc.desc: Verify GetFontId function, capability & bound, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_GetFontId_001, TestSize.Level1)
{
    uint16_t fontId = 16;
    uint16_t start = 6;
    uint16_t end = 8;
    spannableString_->SetFontId(fontId, start, end);
    uint16_t output_0 = 0;
    bool hasFind_0 = false;
    hasFind_0 = spannableString_->GetFontId(0, output_0);
    EXPECT_EQ(false, hasFind_0);
    EXPECT_EQ(0, output_0);
    uint16_t output_6 = 0;
    bool hasFind_6 = false;
    hasFind_6 = spannableString_->GetFontId(6, output_6);
    EXPECT_EQ(true, hasFind_6);
    EXPECT_EQ(fontId, output_6);
    uint16_t output_8 = 0;
    bool hasFind_8 = false;
    hasFind_8 = spannableString_->GetFontId(8, output_8);
    EXPECT_EQ(false, hasFind_8);
    EXPECT_EQ(0, output_8);
}

/**
 * @tc.name: SpannableString_SetFontHeight_001
 * @tc.desc: Verify SetFontHeight function, capability, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetFontHeight_001, TestSize.Level1)
{
    int16_t height = 16;
    uint16_t start = 6;
    uint16_t end = 8;
    spannableString_->SetFontHeight(height, start, end);
    EXPECT_EQ(1, spannableString_->GetHeightListSize());
    for (uint16_t i = start; i < end; i++) {
        int16_t tempHeight = 0;
        spannableString_->GetFontHeight(i, tempHeight, 0, 20);
        EXPECT_EQ(height, tempHeight);
    }
}

/**
 * @tc.name: SpannableString_GetHeight_001
 * @tc.desc: Verify GetHeight function, capability & out-of-bound, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_GetHeight_001, TestSize.Level1)
{
    int16_t height = 16;
    uint16_t start = 6;
    uint16_t end = 8;
    spannableString_->SetFontHeight(height, start, end);
    int16_t output_0 = 0;
    bool hasFind_0 = spannableString_->GetFontHeight(0, output_0, 0, 20);
    EXPECT_EQ(false, hasFind_0);
    EXPECT_EQ(0, output_0);
    int16_t output_6 = 0;
    bool hasFind_6 = spannableString_->GetFontHeight(6, output_6, 0, 20);
    EXPECT_EQ(true, hasFind_6);
    EXPECT_EQ(height, output_6);
    int16_t output_8 = 0;
    bool hasFind_8 = spannableString_->GetFontHeight(8, output_8, 0, 20);
    EXPECT_EQ(false, hasFind_8);
    EXPECT_EQ(0, output_8);
}

/**
 * @tc.name: SpannableString_SetSpannableString_001
 * @tc.desc: Verify SetSpannableString function, capability, cover blank,equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetSpannableString_001, TestSize.Level1)
{
    SpannableString *input = new SpannableString();
    const char *text_ = "hello world hello openharmony";
    input->InitIsSpannable(strlen(text_));
    TextStyle textStyle = TextStyle::TEXT_STYLE_NORMAL;
    uint16_t textStyleStart = 2;
    uint16_t textStyleEnd = 4;
    input->SetTextStyle(textStyle, textStyleStart, textStyleEnd);
    uint8_t inputFontSize = 17;
    uint16_t fontSizeStart = 6;
    uint16_t fontSizeEnd = 8;
    input->SetFontSize(inputFontSize, fontSizeStart, fontSizeEnd);
    uint16_t inputFontId = 21;
    uint16_t fontIdStart = 10;
    uint16_t fontIdEnd = 12;
    input->SetFontId(inputFontId, fontIdStart, fontIdEnd);
    uint16_t inputHeight = 25;
    uint16_t heightStart = 14;
    uint16_t heightEnd = 16;
    input->SetFontHeight(inputHeight, heightStart, heightEnd);

    const SpannableString *inputPtr = input;

    spannableString_->SetSpannableString(inputPtr);

    EXPECT_EQ(1, spannableString_->GetSpanListSize());
    // textStyle value has not been check

    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = fontSizeStart; i < fontSizeEnd; i++) {
        EXPECT_EQ(true, spannableString_->GetSpannable(i));
        uint8_t tempSize = 0;
        spannableString_->GetFontSize(i, tempSize);
        EXPECT_EQ(inputFontSize, tempSize);
    }

    EXPECT_EQ(1, spannableString_->GetFontIdListSize());
    for (uint16_t i = fontIdStart; i < fontIdEnd; i++) {
        EXPECT_EQ(true, spannableString_->GetSpannable(i));
        uint16_t tempFontId = 0;
        spannableString_->GetFontId(i, tempFontId);
        EXPECT_EQ(inputFontId, tempFontId);
    }

    EXPECT_EQ(1, spannableString_->GetHeightListSize());
    for (uint16_t i = heightStart; i < heightEnd; i++) {
        EXPECT_EQ(true, spannableString_->GetSpannable(i));
        int16_t tempHeight = 0;
        spannableString_->GetFontHeight(i, tempHeight, 0, 20);
        EXPECT_EQ(inputHeight, tempHeight);
    }
}

/**
 * @tc.name: SpannableString_SetSpannableString_002
 * @tc.desc: Verify SetSpannableString function, capability, componments already exist and now cover them, equal.
 * @tc.type: FUNC
 */
HWTEST_F(SpannableStringTest, SpannableString_SetSpannableString_002, TestSize.Level1)
{
    SpannableString *input = new SpannableString();
    const char *text_1 = "hello world hello openharmony";
    input->InitIsSpannable(strlen(text_1));
    TextStyle textStyle = TextStyle::TEXT_STYLE_NORMAL;
    uint16_t textStyleStart = 2;
    uint16_t textStyleEnd = 4;
    input->SetTextStyle(textStyle, textStyleStart, textStyleEnd);
    uint8_t inputFontSize = 17;
    uint16_t fontSizeStart = 6;
    uint16_t fontSizeEnd = 8;
    input->SetFontSize(inputFontSize, fontSizeStart, fontSizeEnd);
    uint16_t inputFontId = 21;
    uint16_t fontIdStart = 10;
    uint16_t fontIdEnd = 12;
    input->SetFontId(inputFontId, fontIdStart, fontIdEnd);
    uint16_t inputHeight = 25;
    uint16_t heightStart = 14;
    uint16_t heightEnd = 16;
    input->SetFontHeight(inputHeight, heightStart, heightEnd);

    const char *text_2 = "hello world hello openharmony";
    spannableString_->InitIsSpannable(strlen(text_2));
    TextStyle textStyle_exist = TextStyle::TEXT_STYLE_NORMAL;
    uint16_t textStyleStart_exist = 3;
    uint16_t textStyleEnd_exist = 14;
    spannableString_->SetTextStyle(textStyle_exist, textStyleStart_exist, textStyleEnd_exist);
    uint8_t inputFontSize_exist = 18;
    uint16_t fontSizeStart_exist = 7;
    uint16_t fontSizeEnd_exist = 18;
    spannableString_->SetFontSize(inputFontSize_exist, fontSizeStart_exist, fontSizeEnd_exist);
    uint16_t inputFontId_exist = 22;
    uint16_t fontIdStart_exist = 11;
    uint16_t fontIdEnd_exist = 22;
    spannableString_->SetFontId(inputFontId_exist, fontIdStart_exist, fontIdEnd_exist);
    uint16_t inputHeight_exist = 26;
    uint16_t heightStart_exist = 15;
    uint16_t heightEnd_exist = 26;
    spannableString_->SetFontHeight(inputHeight_exist, heightStart_exist, heightEnd_exist);

    const SpannableString *inputPtr = input;
    spannableString_->SetSpannableString(inputPtr);

    EXPECT_EQ(1, spannableString_->GetSpanListSize());
    // textStyle value has not been check

    EXPECT_EQ(1, spannableString_->GetSizeListSize());
    for (uint16_t i = fontSizeStart; i < fontSizeEnd; i++) {
        EXPECT_EQ(true, spannableString_->GetSpannable(i));
        uint8_t tempSize = 0;
        spannableString_->GetFontSize(i, tempSize);
        EXPECT_EQ(inputFontSize, tempSize);
    }

    EXPECT_EQ(1, spannableString_->GetFontIdListSize());
    for (uint16_t i = fontIdStart; i < fontIdEnd; i++) {
        EXPECT_EQ(true, spannableString_->GetSpannable(i));
        uint16_t tempFontId = 0;
        spannableString_->GetFontId(i, tempFontId);
        EXPECT_EQ(inputFontId, tempFontId);
    }

    EXPECT_EQ(1, spannableString_->GetHeightListSize());
    for (uint16_t i = heightStart; i < heightEnd; i++) {
        EXPECT_EQ(true, spannableString_->GetSpannable(i));
        int16_t tempHeight = 0;
        spannableString_->GetFontHeight(i, tempHeight, 0, 20);
        EXPECT_EQ(inputHeight, tempHeight);
    }
}
} // namespace OHOS
#endif // ENABLE_SPANNABLE_STRING
