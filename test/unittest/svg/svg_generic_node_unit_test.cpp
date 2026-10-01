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
#include "svg/svg_document.h"
#include "svg/svg_generic_node.h"

using namespace testing::ext;

namespace OHOS {

class SvgGenericNodeTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(SvgGenericNodeTest, AttributesRetained_001, TestSize.Level1)
{
    SvgGenericNode font(SVG_FONT);
    EXPECT_TRUE(font.SetAttribute("id", "font1"));
    EXPECT_TRUE(font.SetAttribute("horiz-adv-x", "100"));
    EXPECT_STREQ(font.GetAttribute("horiz-adv-x"), "100");
    EXPECT_EQ(font.GetCategory(), SVG_CATEGORY_GENERIC);
}

HWTEST_F(SvgGenericNodeTest, ChildrenAndText_002, TestSize.Level1)
{
    SvgGenericNode style(SVG_STYLE);
    style.SetTextContent(".cls{fill:red}");
    SvgGenericNode* child = new SvgGenericNode(SVG_METADATA);
    style.AppendChild(child);
    SUCCEED();
}

// Null name / value are rejected without touching the attribute list.
HWTEST_F(SvgGenericNodeTest, GenericSetAttributeNullRejected_003, TestSize.Level1)
{
    SvgGenericNode node(SVG_FONT);
    EXPECT_FALSE(node.SetAttribute(nullptr, "1"));
    EXPECT_FALSE(node.SetAttribute("id", nullptr));
}

// Re-setting an existing attribute updates the stored value.
HWTEST_F(SvgGenericNodeTest, GenericSetAttributeUpdatesExisting_004, TestSize.Level1)
{
    SvgGenericNode node(SVG_FONT);
    EXPECT_TRUE(node.SetAttribute("horiz-adv-x", "100"));
    EXPECT_STREQ(node.GetAttribute("horiz-adv-x"), "100");
    EXPECT_TRUE(node.SetAttribute("horiz-adv-x", "200"));
    EXPECT_STREQ(node.GetAttribute("horiz-adv-x"), "200");
}

// A null child is ignored; the real child appended afterwards is reachable.
HWTEST_F(SvgGenericNodeTest, GenericAppendChildNullIgnored_005, TestSize.Level1)
{
    SvgGenericNode parent(SVG_GROUP);
    SvgDocument doc;
    parent.OnDocumentAttached(&doc);
    parent.AppendChild(nullptr);
    SvgGenericNode* child = new SvgGenericNode(SVG_METADATA);
    parent.AppendChild(child);
    EXPECT_EQ(child->GetDocument(), &doc);
}

// Attaching the document to the parent propagates it to existing children.
HWTEST_F(SvgGenericNodeTest, GenericDocumentAttachesToChildren_006, TestSize.Level1)
{
    SvgGenericNode parent(SVG_GROUP);
    SvgGenericNode* child = new SvgGenericNode(SVG_METADATA);
    parent.AppendChild(child);
    SvgDocument doc;
    parent.OnDocumentAttached(&doc);
    EXPECT_EQ(child->GetDocument(), &doc);
}

// SetTextContent handles a null pointer (clears) as well as a copy.
HWTEST_F(SvgGenericNodeTest, GenericSetTextContentNullAndCopy_007, TestSize.Level1)
{
    SvgGenericNode node(SVG_STYLE);
    node.SetTextContent(".cls{fill:red}");
    node.SetTextContent(nullptr);
    node.SetTextContent("x");
    SUCCEED();
}

// A null attribute name yields no match.
HWTEST_F(SvgGenericNodeTest, GenericGetAttributeNullName_008, TestSize.Level1)
{
    SvgGenericNode node(SVG_FONT);
    EXPECT_EQ(node.GetAttribute(nullptr), nullptr);
}

// A missing attribute name yields a null pointer.
HWTEST_F(SvgGenericNodeTest, GenericGetAttributeMissing_009, TestSize.Level1)
{
    SvgGenericNode node(SVG_FONT);
    node.SetAttribute("horiz-adv-x", "100");
    EXPECT_EQ(node.GetAttribute("missing"), nullptr);
}

} // namespace OHOS
