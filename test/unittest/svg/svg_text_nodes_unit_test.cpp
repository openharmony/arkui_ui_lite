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
#include "components/ui_canvas.h"
#include "svg/svg_document.h"
#include "svg/svg_text_nodes.h"

using namespace testing::ext;

namespace OHOS {

namespace {

class TestUICanvas : public UICanvas {
public:
    TestUICanvas() {}
    virtual ~TestUICanvas() {}
};

// Exposes protected recording / geometry hooks for direct invocation in tests.
class TestTextNode : public SvgTextNode {
public:
    using SvgTextNode::RecordGeometry;
    using SvgTextNode::SetGeometryAttribute;
};

class TestTextAreaNode : public SvgTextAreaNode {
public:
    using SvgTextAreaNode::SetGeometryAttribute;
};

} // namespace

class SvgTextNodesTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(SvgTextNodesTest, TextSetContentAndBounds_001, TestSize.Level1)
{
    SvgTextNode text;
    EXPECT_TRUE(text.SetAttribute("x", "10"));
    EXPECT_TRUE(text.SetAttribute("y", "20"));
    EXPECT_TRUE(text.SetAttribute("font-size", "16"));
    EXPECT_TRUE(text.SetTextContent("hello"));
    Rect b = text.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 10);
    // y=20 is the baseline. The box must start above it and reach at least it; how far
    // it extends below depends on whether real font metrics (FontHeader descender) are
    // available, which is environment dependent, so assert the invariant not a constant.
    EXPECT_LT(b.GetTop(), 20);
    EXPECT_GE(b.GetBottom(), 20);
}

HWTEST_F(SvgTextNodesTest, TSpanSetAttributes_003, TestSize.Level1)
{
    SvgTSpanNode span;
    EXPECT_TRUE(span.SetAttribute("x", "10"));
    EXPECT_TRUE(span.SetAttribute("y", "20"));
    EXPECT_TRUE(span.SetAttribute("dx", "5"));
    EXPECT_TRUE(span.SetAttribute("dy", "10"));
    EXPECT_TRUE(span.SetAttribute("font-size", "12"));
    SUCCEED();
}

HWTEST_F(SvgTextNodesTest, TextAreaBoundsWithSize_005, TestSize.Level1)
{
    SvgTextAreaNode area;
    area.SetAttribute("x", "5");
    area.SetAttribute("y", "10");
    area.SetAttribute("width", "100");
    area.SetAttribute("height", "50");
    Rect b = area.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 5);
    EXPECT_EQ(b.GetTop(), -6);
    EXPECT_EQ(b.GetWidth(), 100);
    EXPECT_EQ(b.GetHeight(), 50);
}

// Rejects null name / null value without touching state.
HWTEST_F(SvgTextNodesTest, TextSetAttributeNullRejected_006, TestSize.Level1)
{
    SvgTextNode text;
    EXPECT_FALSE(text.SetAttribute(nullptr, "10"));
    EXPECT_FALSE(text.SetAttribute("x", nullptr));
}

// The "value" attribute is routed straight into SetTextContent.
HWTEST_F(SvgTextNodesTest, TextValueAttributeRoutesToContent_007, TestSize.Level1)
{
    SvgTextNode text;
    EXPECT_TRUE(text.SetAttribute("value", "hello"));
    EXPECT_STREQ(text.GetTextContent(), "hello");
}

// With no text content, bounds are pure geometry arithmetic.
HWTEST_F(SvgTextNodesTest, TextEmptyBoundsUsesArithmetic_008, TestSize.Level1)
{
    SvgTextNode text;
    text.SetAttribute("x", "10");
    text.SetAttribute("y", "20");
    text.SetAttribute("dx", "5");
    text.SetAttribute("dy", "2");
    text.SetAttribute("font-size", "16");
    Rect b = text.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 15);
    EXPECT_EQ(b.GetTop(), 6);
    EXPECT_EQ(b.GetRight(), 15);
    EXPECT_EQ(b.GetBottom(), 22);
}

// Clearing content through a null pointer must succeed and drop the buffer.
HWTEST_F(SvgTextNodesTest, TextSetTextContentNullClears_009, TestSize.Level1)
{
    SvgTextNode text;
    EXPECT_TRUE(text.SetTextContent("abc"));
    EXPECT_NE(text.GetTextContent(), nullptr);
    EXPECT_TRUE(text.SetTextContent(nullptr));
    EXPECT_EQ(text.GetTextContent(), nullptr);
}

// Unknown geometry attribute is rejected by the text node.
HWTEST_F(SvgTextNodesTest, TextGeometryAttributeUnknownRejected_010, TestSize.Level1)
{
    TestTextNode text;
    EXPECT_FALSE(text.SetGeometryAttribute("width", "10"));
    EXPECT_TRUE(text.SetGeometryAttribute("x", "10"));
    EXPECT_TRUE(text.SetGeometryAttribute("font-size", "20"));
}

// RecordGeometry must visit tspan children without crashing.
HWTEST_F(SvgTextNodesTest, TextRecordGeometryVisitsChildren_011, TestSize.Level1)
{
    TestTextNode text;
    text.ApplyInheritedState(Paint(), TransAffine());
    text.AppendChild(new SvgTSpanNode());
    TestUICanvas canvas;
    text.RecordGeometry(canvas);
    SUCCEED();
}

// TextArea with no size falls back to the plain text bounds computation.
HWTEST_F(SvgTextNodesTest, TextAreaFallbackBoundsWhenNoSize_012, TestSize.Level1)
{
    SvgTextAreaNode area;
    area.SetAttribute("x", "10");
    area.SetAttribute("y", "20");
    area.SetAttribute("dx", "3");
    Rect b = area.GetLocalBounds();
    EXPECT_EQ(b.GetLeft(), 13);
    EXPECT_EQ(b.GetTop(), 4);
    EXPECT_EQ(b.GetRight(), 13);
    EXPECT_EQ(b.GetBottom(), 20);
}

// Unknown geometry attribute is rejected after width/height handling.
HWTEST_F(SvgTextNodesTest, TextAreaGeometryAttributeUnknownRejected_013, TestSize.Level1)
{
    TestTextAreaNode area;
    EXPECT_TRUE(area.SetGeometryAttribute("width", "100"));
    EXPECT_TRUE(area.SetGeometryAttribute("height", "50"));
    EXPECT_FALSE(area.SetGeometryAttribute("rx", "10"));
}

// Cloning a textArea preserves size, content and children.
HWTEST_F(SvgTextNodesTest, TextAreaCloneCopiesSizeAndContent_014, TestSize.Level1)
{
    SvgTextAreaNode area;
    area.SetAttribute("x", "5");
    area.SetAttribute("y", "10");
    area.SetAttribute("width", "100");
    area.SetAttribute("height", "50");
    area.SetTextContent("hi");
    area.AppendChild(new SvgTSpanNode());
    SvgElementBase* cloneBase = area.Clone();
    ASSERT_NE(cloneBase, nullptr);
    SvgTextAreaNode* clone = static_cast<SvgTextAreaNode*>(cloneBase);
    Rect b = clone->GetLocalBounds();
    EXPECT_EQ(b.GetWidth(), 100);
    EXPECT_EQ(b.GetHeight(), 50);
    EXPECT_STREQ(clone->GetTextContent(), "hi");
    delete cloneBase;
}

// Unknown attribute on a tspan is forwarded to the paint state (rejected there).
HWTEST_F(SvgTextNodesTest, TSpanSetAttributeUnknownRejected_015, TestSize.Level1)
{
    SvgTSpanNode span;
    EXPECT_FALSE(span.SetAttribute(nullptr, "1"));
    EXPECT_TRUE(span.SetAttribute("id", "s1"));
    EXPECT_TRUE(span.SetAttribute("fill", "#ff0000"));
    EXPECT_FALSE(span.SetAttribute("bogus", "1"));
}

// ClampFontSize boundaries: 0 and >=256 fall back to the default (16).
HWTEST_F(SvgTextNodesTest, TSpanFontSizeClamped_016, TestSize.Level1)
{
    SvgTSpanNode span;
    span.SetAttribute("font-size", "0");
    EXPECT_EQ(span.GetFontSize(), 16);
    span.SetAttribute("font-size", "300");
    EXPECT_EQ(span.GetFontSize(), 16);
    span.SetAttribute("font-size", "20");
    EXPECT_EQ(span.GetFontSize(), 20);
}

// SetTextContent allocates on copy and clears on null.
HWTEST_F(SvgTextNodesTest, TSpanSetTextContentNullAndCopy_017, TestSize.Level1)
{
    SvgTSpanNode span;
    EXPECT_TRUE(span.SetTextContent("abc"));
    EXPECT_STREQ(span.GetTextContent(), "abc");
    EXPECT_TRUE(span.SetTextContent(nullptr));
    EXPECT_EQ(span.GetTextContent(), nullptr);
}

// Cloned tspan carries its geometry and text content.
HWTEST_F(SvgTextNodesTest, TSpanCloneCopiesState_018, TestSize.Level1)
{
    SvgTSpanNode span;
    span.SetAttribute("x", "10");
    span.SetAttribute("y", "20");
    span.SetAttribute("dx", "5");
    span.SetTextContent("spantext");
    SvgElementBase* cloneBase = span.Clone();
    ASSERT_NE(cloneBase, nullptr);
    SvgTSpanNode* clone = static_cast<SvgTSpanNode*>(cloneBase);
    EXPECT_EQ(clone->GetX(), 10);
    EXPECT_EQ(clone->GetY(), 20);
    EXPECT_EQ(clone->GetDx(), 5);
    EXPECT_STREQ(clone->GetTextContent(), "spantext");
    delete cloneBase;
}

// Cloning a text node copies content and recursively clones children.
HWTEST_F(SvgTextNodesTest, TextCloneCopiesContentAndChildren_022, TestSize.Level1)
{
    SvgTextNode text;
    text.SetTextContent("root");
    text.AppendChild(new SvgTSpanNode());
    SvgElementBase* cloneBase = text.Clone();
    ASSERT_NE(cloneBase, nullptr);
    SvgTextNode* clone = static_cast<SvgTextNode*>(cloneBase);
    EXPECT_STREQ(clone->GetTextContent(), "root");
    delete cloneBase;
}

// Text node with a <tspan> child computes bounds via MeasureTSpanBounds.
HWTEST_F(SvgTextNodesTest, TextWithTSpanBounds_023, TestSize.Level1)
{
    SvgTextNode text;
    text.SetAttribute("x", "10");
    text.SetAttribute("y", "20");
    text.SetAttribute("font-size", "10");

    SvgTSpanNode* span = new SvgTSpanNode();
    span->SetAttribute("x", "5");
    span->SetAttribute("y", "25");
    span->SetAttribute("dx", "3");
    span->SetAttribute("dy", "2");
    span->SetTextContent("ace");
    text.AppendChild(span);

    Rect b = text.GetLocalBounds();
    // tspan position uses x + dx = 5 + 3 = 8; the empty root text leaves top at 10.
    EXPECT_EQ(b.GetLeft(), 8);
    EXPECT_EQ(b.GetTop(), 10);
    EXPECT_GE(b.GetBottom(), 27);
    EXPECT_GE(b.GetRight(), 8);
}

// When a text leaf inherits a rotated CTM, its view bounds must become the bbox of
// the rotated local bounds. This ensures the leaf canvas is large enough to hold
// the rotated glyph bitmap and prevents the non-start clipping seen in group
// rotation animations.
HWTEST_F(SvgTextNodesTest, TextBoundsRotateWithGroup_025, TestSize.Level1)
{
    SvgTextNode text;
    text.SetAttribute("x", "125");
    text.SetAttribute("y", "80");
    text.SetAttribute("font-size", "12");
    text.SetTextContent("group");

    // 90-degree clockwise rotation around the 021 group centre (130,40).
    TransAffine rot;
    rot.Translate(-130.0f, -40.0f);
    rot.Rotate(3.14159265358979323846f / 2.0f);
    rot.Translate(130.0f, 40.0f);

    text.ApplyInheritedState(Paint(), rot);
    Rect screen = text.GetOrigRect();
    Rect local = text.GetLocalBounds();

    // A 90-degree rotation swaps width and height. Allow a couple of pixels for
    // integer rounding in ComputeTransformedBounds.
    EXPECT_GE(screen.GetWidth(), local.GetHeight() - 2);
    EXPECT_GE(screen.GetHeight(), local.GetWidth() - 2);
}

} // namespace OHOS
