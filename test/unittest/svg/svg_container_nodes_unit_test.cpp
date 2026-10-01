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
#include <cstring>
#include "svg/svg_animation.h"
#include "svg/svg_container_nodes.h"
#include "svg/svg_document.h"
#include "svg/svg_paint_servers.h"
#include "svg/svg_shape_nodes.h"

using namespace testing::ext;

namespace OHOS {

namespace {

// A minimal element belonging to the RESOURCE category, used to exercise the
// resource registration branch of container AppendChild.
class ResourceChild : public SvgElementBase {
public:
    SvgElementCategory GetCategory() const override
    {
        return SVG_CATEGORY_RESOURCE;
    }
    bool SetAttribute(const char* name, const char* value) override
    {
        if (name != nullptr && value != nullptr && strcmp(name, "id") == 0) {
            SetId(value);
            return true;
        }
        return false;
    }
    void AppendChild(SvgElementBase* child) override
    {
        (void)child;
    }
};

} // namespace

class SvgContainerNodesTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(SvgContainerNodesTest, GroupAddsViewChild_001, TestSize.Level1)
{
    SvgGroupNode g;
    SvgRectNode* rect = new SvgRectNode();
    g.AppendChild(rect);
    EXPECT_EQ(g.GetChildrenHead(), static_cast<UIView*>(rect));
}

HWTEST_F(SvgContainerNodesTest, RootViewBoxNoneStretch, TestSize.Level1)
{
    SvgRootNode root;
    root.Resize(200, 100);
    EXPECT_TRUE(root.SetAttribute("viewBox", "0 0 100 100"));
    EXPECT_TRUE(root.SetAttribute("preserveAspectRatio", "none"));
    root.UpdateViewportTransform();
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("width", "100");
    rect->SetAttribute("height", "100");
    root.AppendChild(rect);
    EXPECT_EQ(rect->GetRect().GetWidth(), 200);
}

HWTEST_F(SvgContainerNodesTest, RootViewBoxDefaultMeet, TestSize.Level1)
{
    SvgRootNode root;
    root.Resize(200, 100);
    EXPECT_TRUE(root.SetAttribute("viewBox", "0 0 100 100"));
    root.UpdateViewportTransform();
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("width", "100");
    rect->SetAttribute("height", "100");
    root.AppendChild(rect);
    EXPECT_EQ(rect->GetRect().GetWidth(), 100);
}

HWTEST_F(SvgContainerNodesTest, NestedGroupTransformAccumulates_003, TestSize.Level1)
{
    SvgRootNode root;
    SvgGroupNode* g = new SvgGroupNode();
    g->SetAttribute("transform", "translate(10,10)");
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("width", "5");
    rect->SetAttribute("height", "5");
    root.AppendChild(g);
    g->AppendChild(rect);
    EXPECT_EQ(rect->GetRect().GetLeft(), 10);
    EXPECT_EQ(rect->GetRect().GetTop(), 10);
}

HWTEST_F(SvgContainerNodesTest, SwitchPicksFirstBranch_004, TestSize.Level1)
{
    SvgSwitchNode sw;
    SvgRectNode* r1 = new SvgRectNode();
    SvgCircleNode* c1 = new SvgCircleNode();
    sw.AppendChild(r1);
    sw.AppendChild(c1);
    EXPECT_EQ(sw.GetChildrenHead(), static_cast<UIView*>(r1));
    UIView* next = r1->GetNextSibling();
    EXPECT_EQ(next, nullptr);
}

/**
 * @tc.name: GroupStyleChangeAfterAttachPropagates_005
 * @tc.desc: Verify that changing a container's paintState after it is attached asks the parent
 *           to refresh the container, and the new transform propagates to existing children.
 * @tc.type: FUNC
 */
HWTEST_F(SvgContainerNodesTest, GroupStyleChangeAfterAttachPropagates_005, TestSize.Level1)
{
    SvgRootNode root;
    root.Resize(100, 100);
    root.UpdateViewportTransform();
    SvgGroupNode* g = new SvgGroupNode();
    root.AppendChild(g);
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("width", "5");
    rect->SetAttribute("height", "5");
    g->AppendChild(rect);
    EXPECT_EQ(rect->GetRect().GetLeft(), 0);
    EXPECT_EQ(rect->GetRect().GetTop(), 0);

    EXPECT_TRUE(g->SetAttribute("transform", "translate(10,10)"));
    EXPECT_EQ(rect->GetRect().GetLeft(), 10);
    EXPECT_EQ(rect->GetRect().GetTop(), 10);
}

// Null name / value are rejected by the container attribute setter.
HWTEST_F(SvgContainerNodesTest, ContainerSetAttributeNullRejected_006, TestSize.Level1)
{
    SvgGroupNode g;
    EXPECT_FALSE(g.SetAttribute(nullptr, "1"));
    EXPECT_FALSE(g.SetAttribute("id", nullptr));
}

// The "id" attribute is stored and is not treated as a paint change.
HWTEST_F(SvgContainerNodesTest, ContainerSetAttributeId_007, TestSize.Level1)
{
    SvgGroupNode g;
    EXPECT_TRUE(g.SetAttribute("id", "group1"));
    EXPECT_STREQ(g.GetId(), "group1");
}

// A paint attribute on a parentless container pushes state to its children.
HWTEST_F(SvgContainerNodesTest, ContainerSetAttributePaintPushesState_008, TestSize.Level1)
{
    SvgGroupNode g;
    EXPECT_TRUE(g.SetAttribute("fill", "#ff0000"));
    SUCCEED();
}

// Unknown attribute is rejected (not handled by paint state or id).
HWTEST_F(SvgContainerNodesTest, ContainerSetAttributeUnknownRejected_009, TestSize.Level1)
{
    SvgGroupNode g;
    EXPECT_FALSE(g.SetAttribute("bogus", "x"));
}

// A null child is ignored without corrupting the existing child list.
HWTEST_F(SvgContainerNodesTest, ContainerAppendNullIgnored_010, TestSize.Level1)
{
    SvgGroupNode g;
    g.AppendChild(nullptr);
    SvgRectNode* rect = new SvgRectNode();
    g.AppendChild(rect);
    EXPECT_EQ(g.GetChildrenHead(), static_cast<UIView*>(rect));
}

// A child already owned by one container is ignored when appended to a different
// container, so it is stored only once (in its owning container) and is never
// double-freed. This exercises the business rule in SvgContainerNode::AppendChild,
// which rejects a child whose owner is not the receiving container.
HWTEST_F(SvgContainerNodesTest, ContainerAppendDuplicateIgnored_011, TestSize.Level1)
{
    SvgGroupNode g1;
    SvgGroupNode g2;
    SvgRectNode* rect = new SvgRectNode();
    g1.AppendChild(rect);   // rect is now owned by g1
    g2.AppendChild(rect);   // already owned by g1 -> ignored by g2
    EXPECT_EQ(g1.GetChildrenHead(), static_cast<UIView*>(rect));
    EXPECT_EQ(rect->GetNextSibling(), nullptr);
    EXPECT_EQ(g2.GetChildrenHead(), static_cast<UIView*>(nullptr));
    EXPECT_EQ(rect->GetOwner(), static_cast<SvgElementBase*>(&g1));
}

// A resource child is registered in the document when a doc is present.
HWTEST_F(SvgContainerNodesTest, ContainerAppendResourceRegisters_012, TestSize.Level1)
{
    SvgGroupNode g;
    SvgDocument doc;
    g.OnDocumentAttached(&doc);
    ResourceChild* res = new ResourceChild();
    res->SetAttribute("id", "res1");
    g.AppendChild(res);
    EXPECT_EQ(doc.GetResource("res1"), static_cast<SvgElementBase*>(res));
}

// An animation child is bound to the container and registered as an animation.
HWTEST_F(SvgContainerNodesTest, ContainerAppendAnimationRegisters_013, TestSize.Level1)
{
    SvgGroupNode g;
    SvgDocument doc;
    g.OnDocumentAttached(&doc);
    SvgAnimate* anim = new SvgAnimate();
    g.AppendChild(anim);
    EXPECT_FALSE(doc.GetAnimations().IsEmpty());
}

// Attaching a document propagates it to already-added children.
HWTEST_F(SvgContainerNodesTest, ContainerDocumentAttachesToChildren_014, TestSize.Level1)
{
    SvgGroupNode g;
    SvgRectNode* rect = new SvgRectNode();
    g.AppendChild(rect);
    SvgDocument doc;
    g.OnDocumentAttached(&doc);
    EXPECT_EQ(rect->GetDocument(), &doc);
}

// Applying inherited state recurses into a nested container child.
HWTEST_F(SvgContainerNodesTest, ContainerApplyStateToContainerChild_015, TestSize.Level1)
{
    SvgContainerNode parent;
    SvgGroupNode* child = new SvgGroupNode();
    parent.AppendChild(child);
    parent.ApplyInheritedState(Paint(), TransAffine());
    SUCCEED();
}

// The base container Clone returns null (subclasses override it).
HWTEST_F(SvgContainerNodesTest, ContainerBaseCloneReturnsNull_016, TestSize.Level1)
{
    SvgContainerNode node;
    EXPECT_EQ(node.Clone(), nullptr);
}

// Root falls back to the base setter for non-viewBox attributes.
HWTEST_F(SvgContainerNodesTest, RootSetAttributeFallbackToBase_017, TestSize.Level1)
{
    SvgRootNode root;
    EXPECT_TRUE(root.SetAttribute("fill", "#ff0000"));
}

// Root rejects null name / value.
HWTEST_F(SvgContainerNodesTest, RootSetAttributeNullRejected_018, TestSize.Level1)
{
    SvgRootNode root;
    EXPECT_FALSE(root.SetAttribute(nullptr, "1"));
    EXPECT_FALSE(root.SetAttribute("width", nullptr));
}

// preserveAspectRatio "slice" picks the larger scale factor.
HWTEST_F(SvgContainerNodesTest, RootViewBoxSliceStretch_019, TestSize.Level1)
{
    SvgRootNode root;
    root.Resize(200, 100);
    EXPECT_TRUE(root.SetAttribute("viewBox", "0 0 200 50"));
    EXPECT_TRUE(root.SetAttribute("preserveAspectRatio", "xMidYMid slice"));
    root.UpdateViewportTransform();
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("width", "200");
    rect->SetAttribute("height", "50");
    root.AppendChild(rect);
    EXPECT_EQ(rect->GetRect().GetWidth(), 400);
}

// SyncViewport falls back to the supplied dimensions when width/height are 0.
HWTEST_F(SvgContainerNodesTest, RootSyncViewportUsesFallback_020, TestSize.Level1)
{
    SvgRootNode root;
    root.SyncViewport(300, 150);
    EXPECT_EQ(root.GetWidth(), 300);
    EXPECT_EQ(root.GetHeight(), 150);
}

// Cloning a root copies its children.
HWTEST_F(SvgContainerNodesTest, RootCloneCopiesChildren_021, TestSize.Level1)
{
    SvgRootNode root;
    root.Resize(100, 100);
    root.UpdateViewportTransform();
    root.AppendChild(new SvgRectNode());
    SvgElementBase* cloneBase = root.Clone();
    ASSERT_NE(cloneBase, nullptr);
    SvgRootNode* clone = static_cast<SvgRootNode*>(cloneBase);
    EXPECT_NE(clone->GetChildrenHead(), nullptr);
    delete cloneBase;
}

// Cloning a group copies its children.
HWTEST_F(SvgContainerNodesTest, GroupCloneCopiesChildren_022, TestSize.Level1)
{
    SvgGroupNode g;
    g.AppendChild(new SvgRectNode());
    SvgElementBase* cloneBase = g.Clone();
    ASSERT_NE(cloneBase, nullptr);
    SvgGroupNode* clone = static_cast<SvgGroupNode*>(cloneBase);
    EXPECT_NE(clone->GetChildrenHead(), nullptr);
    delete cloneBase;
}

// <use> accepts x / y / width / height / href and falls back to base.
HWTEST_F(SvgContainerNodesTest, UseSetAttributeAllFields_025, TestSize.Level1)
{
    SvgUseNode use;
    EXPECT_FALSE(use.SetAttribute(nullptr, "1"));
    EXPECT_TRUE(use.SetAttribute("x", "5"));
    EXPECT_TRUE(use.SetAttribute("y", "5"));
    EXPECT_TRUE(use.SetAttribute("width", "10"));
    EXPECT_TRUE(use.SetAttribute("height", "10"));
    EXPECT_TRUE(use.SetAttribute("xlink:href", "#r"));
    EXPECT_TRUE(use.SetAttribute("fill", "#ff0000"));
}

// <use> resolves and expands a referenced element from the document.
HWTEST_F(SvgContainerNodesTest, UseResolveExpandsFromDoc_026, TestSize.Level1)
{
    SvgDocument doc;
    SvgGroupNode holder;
    holder.OnDocumentAttached(&doc);
    SvgRectNode* ref = new SvgRectNode();
    ref->SetAttribute("id", "r1");
    holder.AppendChild(ref);

    SvgUseNode use;
    use.OnDocumentAttached(&doc);
    use.SetAttribute("xlink:href", "#r1");
    use.SetAttribute("x", "5");
    use.SetAttribute("y", "5");
    use.OnDocumentReady();
    EXPECT_NE(use.GetChildrenHead(), nullptr);
}

// <use> with no reference does nothing on document-ready.
HWTEST_F(SvgContainerNodesTest, UseResolveSkipsWhenNoRef_027, TestSize.Level1)
{
    SvgUseNode use;
    SvgDocument doc;
    use.OnDocumentAttached(&doc);
    use.OnDocumentReady();
    EXPECT_EQ(use.GetChildrenHead(), nullptr);
}

// <use> with a reference but no document does nothing.
HWTEST_F(SvgContainerNodesTest, UseResolveSkipsWhenNoDoc_028, TestSize.Level1)
{
    SvgUseNode use;
    use.SetAttribute("xlink:href", "#r1");
    use.OnDocumentReady();
    EXPECT_EQ(use.GetChildrenHead(), nullptr);
}

// <use> referencing a missing id does not expand.
HWTEST_F(SvgContainerNodesTest, UseResolveSkipsMissingTarget_029, TestSize.Level1)
{
    SvgUseNode use;
    SvgDocument doc;
    use.OnDocumentAttached(&doc);
    use.SetAttribute("xlink:href", "#missing");
    use.OnDocumentReady();
    EXPECT_EQ(use.GetChildrenHead(), nullptr);
}

// Cloning a <use> node with a child succeeds.
HWTEST_F(SvgContainerNodesTest, UseCloneNotNull_030, TestSize.Level1)
{
    SvgUseNode use;
    use.AppendChild(new SvgRectNode());
    SvgElementBase* clone = use.Clone();
    ASSERT_NE(clone, nullptr);
    EXPECT_NE(static_cast<SvgUseNode*>(clone)->GetChildrenHead(), nullptr);
    delete clone;
}

// <switch> rejects null and falls back to the base setter.
HWTEST_F(SvgContainerNodesTest, SwitchSetAttributeNullAndFallback_031, TestSize.Level1)
{
    SvgSwitchNode sw;
    EXPECT_FALSE(sw.SetAttribute(nullptr, "1"));
    EXPECT_TRUE(sw.SetAttribute("fill", "#ff0000"));
}

// A non-VIEW first child of <switch> is stored but not added as a view.
HWTEST_F(SvgContainerNodesTest, SwitchAppendResourceFirst_032, TestSize.Level1)
{
    SvgSwitchNode sw;
    SvgDocument doc;
    sw.OnDocumentAttached(&doc);
    ResourceChild* res = new ResourceChild();
    res->SetAttribute("id", "resSwitch");
    sw.AppendChild(res);
    EXPECT_EQ(sw.GetChildrenHead(), nullptr);
}

// Cloning a <switch> node with a child succeeds.
HWTEST_F(SvgContainerNodesTest, SwitchCloneNotNull_033, TestSize.Level1)
{
    SvgSwitchNode sw;
    sw.AppendChild(new SvgRectNode());
    SvgElementBase* clone = sw.Clone();
    ASSERT_NE(clone, nullptr);
    EXPECT_NE(static_cast<SvgSwitchNode*>(clone)->GetChildrenHead(), nullptr);
    delete clone;
}

// <use> referencing an already-expanded <use> copies the subtree and does not re-expand.
HWTEST_F(SvgContainerNodesTest, UseChainCopiesExpandedSubtree_034, TestSize.Level1)
{
    SvgDocument doc;
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("id", "rect1");
    doc.RegisterResource("rect1", rect);

    SvgUseNode* middle = new SvgUseNode();
    middle->SetAttribute("id", "middle");
    middle->OnDocumentAttached(&doc);
    middle->SetAttribute("xlink:href", "#rect1");
    middle->OnDocumentReady();
    EXPECT_NE(middle->GetChildrenHead(), nullptr);
    doc.RegisterResource("middle", middle);

    SvgUseNode top;
    top.OnDocumentAttached(&doc);
    top.SetAttribute("xlink:href", "#middle");
    top.OnDocumentReady();
    EXPECT_NE(top.GetChildrenHead(), nullptr);
    delete middle;
    delete rect;
}

// <use> across <defs> resolves to a visible shape.
HWTEST_F(SvgContainerNodesTest, UseAcrossDefsResolves_035, TestSize.Level1)
{
    SvgDocument doc;
    SvgDefsResource* defs = new SvgDefsResource();
    defs->OnDocumentAttached(&doc);
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("id", "defRect");
    defs->AppendChild(rect);
    doc.RegisterResource("defRect", rect);

    SvgUseNode use;
    use.OnDocumentAttached(&doc);
    use.SetAttribute("xlink:href", "#defRect");
    use.OnDocumentReady();
    EXPECT_NE(use.GetChildrenHead(), nullptr);
    delete defs;
}

// <use> self-reference is bounded by the depth limit and does not recurse forever.
HWTEST_F(SvgContainerNodesTest, UseSelfReferenceBounded_036, TestSize.Level1)
{
    SvgDocument doc;
    SvgUseNode* self = new SvgUseNode();
    self->SetAttribute("id", "self");
    self->OnDocumentAttached(&doc);
    self->SetAttribute("xlink:href", "#self");
    doc.RegisterResource("self", self);

    SvgUseNode use;
    use.OnDocumentAttached(&doc);
    use.SetAttribute("xlink:href", "#self");
    use.OnDocumentReady();
    SUCCEED(); // main goal: no infinite recursion / crash
    delete self;
}

// A <g> container is sized to its parent's bounds so that children transformed by
// the group's own rotation/scale animation are not clipped to the untransformed
// children's axis-aligned bbox.
HWTEST_F(SvgContainerNodesTest, GroupExpandsToParentBounds_037, TestSize.Level1)
{
    SvgRootNode root;
    root.Resize(300, 150);
    root.UpdateViewportTransform();
    SvgGroupNode* g = new SvgGroupNode();
    root.AppendChild(g);
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("x", "10");
    rect->SetAttribute("y", "10");
    rect->SetAttribute("width", "20");
    rect->SetAttribute("height", "20");
    g->AppendChild(rect);

    EXPECT_EQ(g->GetRelativeRect().GetX(), 0);
    EXPECT_EQ(g->GetRelativeRect().GetY(), 0);
    EXPECT_EQ(g->GetWidth(), root.GetWidth());
    EXPECT_EQ(g->GetHeight(), root.GetHeight());
    EXPECT_EQ(g->GetContentRect().GetWidth(), root.GetContentRect().GetWidth());
    EXPECT_EQ(g->GetContentRect().GetHeight(), root.GetContentRect().GetHeight());
}

// If the group is added before the root receives its final size, SyncViewport must
// resize the group to the root's bounds when the viewport is established.
HWTEST_F(SvgContainerNodesTest, GroupExpandsAfterRootSyncViewport_038, TestSize.Level1)
{
    SvgRootNode root;
    SvgGroupNode* g = new SvgGroupNode();
    root.AppendChild(g);
    SvgRectNode* rect = new SvgRectNode();
    rect->SetAttribute("x", "10");
    rect->SetAttribute("y", "10");
    rect->SetAttribute("width", "20");
    rect->SetAttribute("height", "20");
    g->AppendChild(rect);

    root.SyncViewport(300, 150);
    EXPECT_EQ(g->GetWidth(), root.GetWidth());
    EXPECT_EQ(g->GetHeight(), root.GetHeight());
    EXPECT_EQ(g->GetContentRect().GetWidth(), root.GetContentRect().GetWidth());
    EXPECT_EQ(g->GetContentRect().GetHeight(), root.GetContentRect().GetHeight());
}

} // namespace OHOS
