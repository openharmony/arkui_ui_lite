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
#include "svg/svg_container_nodes.h"
#include "svg/svg_document.h"
#include "svg/svg_dom_builder.h"
#include "svg/svg_generic_node.h"
#include "svg/svg_paint_servers.h"
#include "svg/svg_shape_nodes.h"
#include "svg/svg_text_nodes.h"
#include "svg/svg_types.h"
#include "svg/svg_xml_parser.h"

using namespace testing::ext;

namespace OHOS {

class SvgDocumentTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(SvgDocumentTest, RegisterAndFindResource_001, TestSize.Level1)
{
    SvgDocument doc;
    SvgLinearGradientResource* grad = new SvgLinearGradientResource();
    grad->SetAttribute("id", "grad1");
    doc.RegisterResource("grad1", grad);
    EXPECT_EQ(doc.GetResource("grad1"), static_cast<SvgElementBase*>(grad));
    EXPECT_EQ(doc.GetResource("nope"), nullptr);
    delete grad;
}

HWTEST_F(SvgDocumentTest, SetRootAndGetRootView_002, TestSize.Level1)
{
    SvgDocument doc;
    SvgRootNode* root = new SvgRootNode();
    doc.SetRoot(root);
    EXPECT_EQ(doc.GetRoot(), static_cast<SvgElementBase*>(root));
    EXPECT_EQ(doc.GetRootView(), root);
}

HWTEST_F(SvgDocumentTest, HostView_003, TestSize.Level1)
{
    SvgDocument doc;
    EXPECT_EQ(doc.GetHostView(), nullptr);
    UIView view;
    doc.SetHostView(&view);
    EXPECT_EQ(doc.GetHostView(), &view);
}

HWTEST_F(SvgDocumentTest, DefsRegistration_004, TestSize.Level1)
{
    SvgDocument doc;
    SvgDefsResource* defs = new SvgDefsResource();
    defs->OnDocumentAttached(&doc);
    SvgLinearGradientResource* grad = new SvgLinearGradientResource();
    grad->SetAttribute("id", "grad1");
    defs->AppendChild(grad);
    EXPECT_EQ(doc.GetResource("grad1"), static_cast<SvgElementBase*>(grad));
    delete defs;
}

HWTEST_F(SvgDocumentTest, ResolvePaintServerNotFound_005, TestSize.Level1)
{
    SvgDocument doc;
    Paint paint;
    Rect bounds = {0, 0, 100, 100};
    EXPECT_FALSE(doc.ResolvePaintServer("missing", paint, bounds, nullptr, true));
}

HWTEST_F(SvgDocumentTest, LeafCloneViaUse_006, TestSize.Level1)
{
    SvgDocument doc;
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("id", "rect1");
    rect->SetAttribute("width", "100");
    rect->SetAttribute("height", "50");
    doc.RegisterResource("rect1", rect);

    SvgUseNode use;
    use.OnDocumentAttached(&doc);
    use.SetAttribute("href", "#rect1");
    use.OnDocumentReady();

    EXPECT_NE(use.GetChildrenHead(), nullptr);
    delete rect;
}

// Cover SvgDocument::GetResource null/empty guards (svg_document.cpp).
HWTEST_F(SvgDocumentTest, GetResourceGuards_007, TestSize.Level1)
{
    SvgDocument doc;
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("id", "rect1");
    doc.RegisterResource("rect1", rect);
    EXPECT_EQ(doc.GetResource(nullptr), nullptr);
    EXPECT_EQ(doc.GetResource(""), nullptr);
    delete rect;
}

// Cover SvgDocument::RegisterResource empty-id/null-res reject and duplicate-id update (svg_document.cpp).
HWTEST_F(SvgDocumentTest, RegisterResourceDuplicateAndInvalid_008, TestSize.Level1)
{
    SvgDocument doc;
    SvgRectNode* rect1 = new SvgRectNode();
    SvgRectNode* rect2 = new SvgRectNode();
    // invalid inputs are ignored, nothing registered
    doc.RegisterResource("", rect1);
    doc.RegisterResource(nullptr, rect1);
    doc.RegisterResource("dup", nullptr);
    EXPECT_EQ(doc.GetResource("dup"), nullptr);
    // first registration
    doc.RegisterResource("dup", rect1);
    EXPECT_EQ(doc.GetResource("dup"), static_cast<SvgElementBase*>(rect1));
    // duplicate id updates the node in place
    doc.RegisterResource("dup", rect2);
    EXPECT_EQ(doc.GetResource("dup"), static_cast<SvgElementBase*>(rect2));
    delete rect1;
    delete rect2;
}

// Cover the success path of SvgDocument::ResolvePaintServer for gradient and solid-color servers.
HWTEST_F(SvgDocumentTest, ResolvePaintServerOk_009, TestSize.Level1)
{
    SvgDocument doc;
    Paint paint;
    Rect bounds = {0, 0, 100, 100};

    SvgLinearGradientResource* grad = new SvgLinearGradientResource();
    grad->SetAttribute("id", "gradOk");
    grad->SetAttribute("x1", "0");
    grad->SetAttribute("y1", "0");
    grad->SetAttribute("x2", "1");
    grad->SetAttribute("y2", "0");
    doc.RegisterResource("gradOk", grad);
    EXPECT_TRUE(doc.ResolvePaintServer("gradOk", paint, bounds, nullptr, true));
    delete grad;

    SvgSolidColorResource* solid = new SvgSolidColorResource();
    solid->SetAttribute("id", "solidOk");
    solid->SetAttribute("solid-color", "#ff0000");
    doc.RegisterResource("solidOk", solid);
    EXPECT_TRUE(doc.ResolvePaintServer("solidOk", paint, bounds, nullptr, true));
    delete solid;
}

// Cover SvgDocument::GetRootView returning nullptr when the root is not an SvgRootNode.
HWTEST_F(SvgDocumentTest, GetRootViewNonRoot_010, TestSize.Level1)
{
    SvgDocument doc;
    SvgRectNode* rect = new SvgRectNode();
    doc.SetRoot(rect);
    EXPECT_EQ(doc.GetRoot(), static_cast<SvgElementBase*>(rect));
    EXPECT_EQ(doc.GetRootView(), nullptr);
}

// Cover SvgDocument::ClearIds (svg_document.cpp).
HWTEST_F(SvgDocumentTest, ClearIds_011, TestSize.Level1)
{
    SvgDocument doc;
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("id", "rectClear");
    doc.RegisterResource("rectClear", rect);
    ASSERT_NE(doc.GetResource("rectClear"), nullptr);
    doc.ClearIds();
    EXPECT_EQ(doc.GetResource("rectClear"), nullptr);
    delete rect;
}

// Cover SvgDomBuilder::Build(null) and SvgXmlParser::Parse null guards
// (svg_dom_builder.cpp / svg_xml_parser.cpp).
HWTEST_F(SvgDocumentTest, DomBuilderNullInput_012, TestSize.Level1)
{
    SvgDocument doc;
    SvgDomBuilder builder;
    EXPECT_EQ(builder.Build(nullptr, doc), SvgResult::SVG_RESULT_INVALID_PARAM);

    SvgXmlParser parser;
    EXPECT_EQ(parser.Parse(nullptr, &builder), SvgResult::SVG_RESULT_INVALID_PARAM);
    EXPECT_EQ(parser.Parse("<svg/>", nullptr), SvgResult::SVG_RESULT_INVALID_PARAM);
}

// Cover SvgDocument/SvgXmlParser/SvgDomBuilder/CreateSvgElementByType for a recognized
// tree plus the factory default branch (unknown tag -> SvgGenericNode) (svg_element_factory.cpp).
HWTEST_F(SvgDocumentTest, DomBuilderUnknownTag_013, TestSize.Level1)
{
    SvgDocument doc;
    SvgDomBuilder builder;
    EXPECT_EQ(builder.Build("<svg><unknownTag id=\"u1\" foo=\"bar\"/></svg>", doc),
              SvgResult::SVG_RESULT_OK);

    SvgElementBase* root = doc.GetRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(doc.GetRootView(), static_cast<SvgRootNode*>(root));

    SvgElementBase* unknown = doc.GetResource("u1");
    ASSERT_NE(unknown, nullptr);
    SvgGenericNode* generic = dynamic_cast<SvgGenericNode*>(unknown);
    ASSERT_NE(generic, nullptr);
    EXPECT_EQ(generic->GetCategory(), SVG_CATEGORY_GENERIC);
    EXPECT_STREQ(generic->GetAttribute("foo"), "bar");
}

// Cover style-attribute parsing (SvgDomBuilder::ParseStyleAttribute -> OnAttribute).
HWTEST_F(SvgDocumentTest, DomBuilderStyleAttr_014, TestSize.Level1)
{
    SvgDocument doc;
    SvgDomBuilder builder;
    EXPECT_EQ(builder.Build("<svg><foo id=\"x\" style=\"a:1;b:2\"/></svg>", doc), SvgResult::SVG_RESULT_OK);

    SvgElementBase* node = doc.GetResource("x");
    ASSERT_NE(node, nullptr);
    SvgGenericNode* generic = dynamic_cast<SvgGenericNode*>(node);
    ASSERT_NE(generic, nullptr);
    EXPECT_STREQ(generic->GetAttribute("a"), "1");
    EXPECT_STREQ(generic->GetAttribute("b"), "2");
}

// Cover comment and CDATA handling in SvgXmlParser (ParseComment branches).
HWTEST_F(SvgDocumentTest, DomBuilderCommentCData_015, TestSize.Level1)
{
    SvgDocument doc;
    SvgDomBuilder builder;
    EXPECT_EQ(builder.Build("<svg><!-- a comment --><rect id=\"r2\"/></svg>", doc),
              SvgResult::SVG_RESULT_OK);
    EXPECT_NE(doc.GetResource("r2"), nullptr);

    SvgDocument doc2;
    SvgDomBuilder builder2;
    EXPECT_EQ(builder2.Build("<svg><![CDATA[ignored]]><rect id=\"r3\"/></svg>", doc2),
              SvgResult::SVG_RESULT_OK);
    EXPECT_NE(doc2.GetResource("r3"), nullptr);
}

// Cover the multi-root guard: a second top-level element after the root is set is skipped.
HWTEST_F(SvgDocumentTest, DomBuilderSecondRootSkipped_016, TestSize.Level1)
{
    SvgDocument doc;
    SvgDomBuilder builder;
    EXPECT_EQ(builder.Build("<svg id=\"a\"/><svg id=\"b\"/>", doc), SvgResult::SVG_RESULT_OK);
    EXPECT_NE(doc.GetResource("a"), nullptr);
    EXPECT_EQ(doc.GetResource("b"), nullptr);
}

// Cover SvgDomBuilder::OnText with a SvgTextNode.
HWTEST_F(SvgDocumentTest, DomBuilderTextNode_017, TestSize.Level1)
{
    SvgDocument doc;
    SvgDomBuilder builder;
    EXPECT_EQ(builder.Build("<svg><text id=\"t1\">hello</text></svg>", doc),
              SvgResult::SVG_RESULT_OK);
    SvgElementBase* text = doc.GetResource("t1");
    ASSERT_NE(text, nullptr);
    EXPECT_NE(dynamic_cast<SvgTextNode*>(text), nullptr);
}

} // namespace OHOS
