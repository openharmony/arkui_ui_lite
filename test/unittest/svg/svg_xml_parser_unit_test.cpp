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

#include <gtest/gtest.h>
#include "securec.h"
#include "svg/svg_types.h"
#include "svg/svg_xml_parser.h"
#include <cstring>

using namespace testing::ext;

namespace OHOS {

namespace {

class TestListener : public SvgXmlParserListener {
public:
    void OnStartElement(const char* tag) override
    {
        CopyString(tag, lastStart_, sizeof(lastStart_));
        startCount_++;
    }

    void OnEndElement(const char* tag) override
    {
        CopyString(tag, lastEnd_, sizeof(lastEnd_));
        endCount_++;
    }

    void OnAttribute(const char* name, const char* value) override
    {
        if (name != nullptr && strcmp(name, "id") == 0) {
            CopyString(value, lastId_, sizeof(lastId_));
        }
        attrCount_++;
    }

    void OnText(const char* text) override
    {
        CopyString(text, lastText_, sizeof(lastText_));
        textCount_++;
    }

    const char* GetLastStart() const { return lastStart_; }
    const char* GetLastEnd() const { return lastEnd_; }
    const char* GetLastId() const { return lastId_; }
    const char* GetLastText() const { return lastText_; }
    uint32_t GetStartCount() const { return startCount_; }
    uint32_t GetEndCount() const { return endCount_; }
    uint32_t GetAttrCount() const { return attrCount_; }
    uint32_t GetTextCount() const { return textCount_; }

private:
    static void CopyString(const char* src, char* dst, uint32_t maxLen)
    {
        if (src == nullptr || dst == nullptr || maxLen == 0) {
            return;
        }
        uint32_t len = strlen(src) + 1;
        if (len > maxLen) {
            len = maxLen;
        }
        if (memcpy_s(dst, maxLen, src, len) != EOK) {
            dst[0] = '\0';
        }
    }

    char lastStart_[64] = {0};
    char lastEnd_[64] = {0};
    char lastId_[64] = {0};
    char lastText_[256] = {0};
    uint32_t startCount_ = 0;
    uint32_t endCount_ = 0;
    uint32_t attrCount_ = 0;
    uint32_t textCount_ = 0;
};

} // namespace

class SvgXmlParserTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: SvgXmlParserComment_001
 * @tc.desc: Verify a comment containing '>' does not leak its content as text nodes.
 * @tc.type: FUNC
 */
HWTEST_F(SvgXmlParserTest, SvgXmlParserComment_001, TestSize.Level1)
{
    TestListener listener;
    SvgXmlParser parser;
    EXPECT_EQ(parser.Parse("<svg><!-- a > b --><rect id='r'/></svg>", &listener), SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(listener.GetTextCount(), 0U);
    EXPECT_STREQ(listener.GetLastId(), "r");
    EXPECT_EQ(listener.GetStartCount(), 2U); // svg, rect
    EXPECT_EQ(listener.GetEndCount(), 2U);   // rect (self-closing), svg
}

/**
 * @tc.name: SvgXmlParserCData_002
 * @tc.desc: Verify a CDATA section containing '>' is recognized and skipped, not leaking text.
 * @tc.type: FUNC
 */
HWTEST_F(SvgXmlParserTest, SvgXmlParserCData_002, TestSize.Level1)
{
    TestListener listener;
    SvgXmlParser parser;
    EXPECT_EQ(parser.Parse("<svg><![CDATA[a > b]]></svg>", &listener), SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(listener.GetTextCount(), 0U);
    EXPECT_EQ(listener.GetStartCount(), 1U);
    EXPECT_EQ(listener.GetEndCount(), 1U);
}

/**
 * @tc.name: SvgXmlParserLongAttrValue_003
 * @tc.desc: Verify an attribute value longer than the internal buffer does not misalign parsing.
 * @tc.type: FUNC
 */
HWTEST_F(SvgXmlParserTest, SvgXmlParserLongAttrValue_003, TestSize.Level1)
{
    TestListener listener;
    SvgXmlParser parser;

    // Build an attribute value that exceeds MAX_ATTR_VALUE_LEN (256) but is properly closed.
    char longValue[512] = {0};
    longValue[0] = '\'';
    for (uint32_t i = 1; i < 260; ++i) {
        longValue[i] = 'x';
    }
    longValue[260] = '\'';
    longValue[261] = '\0';

    char xml[1024] = {0};
    int written = snprintf_s(xml, sizeof(xml), sizeof(xml) - 1,
                             "<svg><rect id=%s fill='red'/><circle id='c'/></svg>", longValue);
    ASSERT_GT(written, 0);

    EXPECT_EQ(parser.Parse(xml, &listener), SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(listener.GetStartCount(), 3U); // svg, rect, circle
    EXPECT_EQ(listener.GetEndCount(), 3U);     // rect (self-closing), circle (self-closing), svg
    EXPECT_STREQ(listener.GetLastId(), "c");
}

/**
 * @tc.name: SvgXmlParserProcessingInstruction_004
 * @tc.desc: Verify an XML processing instruction is skipped without breaking the following tag.
 * @tc.type: FUNC
 */
HWTEST_F(SvgXmlParserTest, SvgXmlParserProcessingInstruction_004, TestSize.Level1)
{
    TestListener listener;
    SvgXmlParser parser;
    EXPECT_EQ(parser.Parse("<?xml version='1.0'?><svg id='s'></svg>", &listener), SvgResult::SVG_RESULT_OK);
    EXPECT_EQ(listener.GetStartCount(), 1U); // svg only
    EXPECT_EQ(listener.GetEndCount(), 1U);     // svg
    EXPECT_STREQ(listener.GetLastId(), "s");
}

} // namespace OHOS
