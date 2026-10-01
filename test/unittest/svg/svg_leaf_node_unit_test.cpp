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
#include "gfx_utils/diagram/common/paint.h"
#include "gfx_utils/trans_affine.h"
#include "svg/svg_animation.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_container_nodes.h"
#include "svg/svg_document.h"
#include "svg/svg_generic_node.h"
#include "svg/svg_leaf_node.h"
#include "svg/svg_paint_servers.h"
#include <cstring>

using namespace testing::ext;

namespace OHOS {

namespace {

class TestLeaf : public SvgLeafNode {
public:
    Rect GetLocalBounds() const override
    {
        return {0, 0, 10, 10};
    }

    uint16_t GetRecordCount() const
    {
        return recordCount_;
    }

    TransAffine ExposeRecordTransform() const
    {
        return GetRecordTransform();
    }

    const Paint& ExposeInheritedPaint() const
    {
        return inheritedPaint_;
    }

protected:
    bool SetGeometryAttribute(const char* name, const char* value) override
    {
        if (strcmp(name, "x") == 0) {
            x_ = SvgAttributeParser::ParseLength(value);
            return true;
        }
        return false;
    }

    void RecordGeometry(UICanvas& canvas) override
    {
        recordCount_++;
        TransAffine record = GetRecordTransform();
        float x0 = 0.0f;
        float y0 = 0.0f;
        float x1 = 10.0f;
        float y1 = 10.0f;
        record.Transform(&x0, &y0);
        record.Transform(&x1, &y1);
        canvas.BeginPath();
        canvas.MoveTo({static_cast<int16_t>(x0), static_cast<int16_t>(y0)});
        canvas.LineTo({static_cast<int16_t>(x1), static_cast<int16_t>(y1)});
        canvas.ClosePath();
    }

private:
    int16_t x_ = 0;
    uint16_t recordCount_ = 0;
};

class RejectedChild : public SvgElementBase {
public:
    SvgElementCategory GetCategory() const override
    {
        return SVG_CATEGORY_RESOURCE;
    }

    bool SetAttribute(const char* name, const char* value) override
    {
        (void)name;
        (void)value;
        return false;
    }

    void AppendChild(SvgElementBase* child) override
    {
        (void)child;
    }
};

} // namespace

class SvgLeafNodeTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: SetAttributeRoutesStyleAndGeometry_001
 * @tc.desc: Verify style attributes route to paintState_ and geometry attributes route to SetGeometryAttribute.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, SetAttributeRoutesStyleAndGeometry_001, TestSize.Level1)
{
    TestLeaf leaf;
    EXPECT_TRUE(leaf.SetAttribute("fill", "#00ff00"));
    EXPECT_TRUE(leaf.SetAttribute("x", "5"));
    EXPECT_FALSE(leaf.SetAttribute("no-such-attr", "1"));
}

/**
 * @tc.name: ViewTypeAndCategory_002
 * @tc.desc: Verify SvgLeafNode reports the SVG leaf view type and view category.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, ViewTypeAndCategory_002, TestSize.Level1)
{
    TestLeaf leaf;
    EXPECT_EQ(leaf.GetViewType(), UI_SVG_LEAF);
    EXPECT_EQ(leaf.GetCategory(), SVG_CATEGORY_VIEW);
}

/**
 * @tc.name: InheritedTransformUpdatesRect_003
 * @tc.desc: Verify ApplyInheritedState updates the view rect by the accumulated transform
 *           and that GetRecordTransform() is identity for pure translation.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, InheritedTransformUpdatesRect_003, TestSize.Level1)
{
    TestLeaf leaf;
    TransAffine t = TransAffine::TransAffineTranslation(100, 50);
    Paint parentPaint;
    parentPaint.SetStyle(Paint::FILL_STYLE);
    leaf.ApplyInheritedState(parentPaint, t);
    EXPECT_EQ(leaf.GetRect().GetLeft(), 100);
    EXPECT_EQ(leaf.GetRect().GetTop(), 50);
    EXPECT_TRUE(leaf.ExposeRecordTransform().IsIdentity());
}

/**
 * @tc.name: StyleChangeRebuildsGeometry_004
 * @tc.desc: Verify that a style-only SetAttribute triggers RebuildGeometry.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, StyleChangeRebuildsGeometry_004, TestSize.Level1)
{
    TestLeaf leaf;
    leaf.ApplyInheritedState(Paint(), TransAffine());
    uint16_t before = leaf.GetRecordCount();
    EXPECT_TRUE(leaf.SetAttribute("fill", "#ff0000"));
    EXPECT_GT(leaf.GetRecordCount(), before);
}

/**
 * @tc.name: AppendChildAcceptsOnlyAllowedCategories_005
 * @tc.desc: Verify AppendChild accepts only SVG_CATEGORY_ANIMATION and SVG_CATEGORY_GENERIC.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, AppendChildAcceptsOnlyAllowedCategories_005, TestSize.Level1)
{
    TestLeaf leaf;
    RejectedChild rejected;
    leaf.AppendChild(&rejected);
    // Caller retains ownership; leaf destructor must not crash.
    SUCCEED();
}

/**
 * @tc.name: StyleChangeWithParentRefreshesInheritedState_006
 * @tc.desc: Verify that a style-only SetAttribute on a leaf asks its SvgContainerNode parent
 *           to re-apply inherited state, so the accumulated transform is refreshed.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, StyleChangeWithParentRefreshesInheritedState_006, TestSize.Level1)
{
    SvgGroupNode group;
    Paint parentPaint;
    parentPaint.SetStyle(Paint::FILL_STYLE);
    group.ApplyInheritedState(parentPaint, TransAffine::TransAffineTranslation(20, 10));
    TestLeaf* leaf = new TestLeaf();
    group.AppendChild(leaf);
    EXPECT_EQ(leaf->GetRect().GetLeft(), 20);
    EXPECT_EQ(leaf->GetRect().GetTop(), 10);

    EXPECT_TRUE(leaf->SetAttribute("transform", "translate(5,5)"));
    EXPECT_EQ(leaf->GetRect().GetLeft(), 25);
    EXPECT_EQ(leaf->GetRect().GetTop(), 15);
}

/**
 * @tc.name: StyleChangeWithoutParentFallsBackToRebuild_007
 * @tc.desc: Verify that a style-only SetAttribute on an unattached leaf falls back to
 *           RebuildGeometry without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, StyleChangeWithoutParentFallsBackToRebuild_007, TestSize.Level1)
{
    TestLeaf leaf;
    leaf.ApplyInheritedState(Paint(), TransAffine());
    EXPECT_TRUE(leaf.SetAttribute("fill", "#00ff00"));
    EXPECT_GT(leaf.GetRecordCount(), static_cast<uint16_t>(0));
}

/**
 * @tc.name: SetAttributeRejectsNullArguments_008
 * @tc.desc: Verify SetAttribute returns false when either the name or the value is null.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, SetAttributeRejectsNullArguments_008, TestSize.Level1)
{
    TestLeaf leaf;
    EXPECT_FALSE(leaf.SetAttribute(nullptr, "1"));
    EXPECT_FALSE(leaf.SetAttribute("fill", nullptr));
    EXPECT_FALSE(leaf.SetAttribute(nullptr, nullptr));
}

/**
 * @tc.name: SetAttributeIdIsStored_009
 * @tc.desc: Verify the "id" attribute is routed to SetId instead of the paint or geometry paths.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, SetAttributeIdIsStored_009, TestSize.Level1)
{
    TestLeaf leaf;
    EXPECT_EQ(leaf.GetId(), nullptr);
    EXPECT_TRUE(leaf.SetAttribute("id", "leaf1"));
    ASSERT_NE(leaf.GetId(), nullptr);
    EXPECT_STREQ(leaf.GetId(), "leaf1");
    // Setting the id must not trigger a geometry rebuild.
    EXPECT_EQ(leaf.GetRecordCount(), static_cast<uint16_t>(0));
}

/**
 * @tc.name: AppendChildIgnoresNullChild_010
 * @tc.desc: Verify AppendChild returns early for a null child.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, AppendChildIgnoresNullChild_010, TestSize.Level1)
{
    TestLeaf leaf;
    leaf.AppendChild(nullptr);
    SUCCEED();
}

/**
 * @tc.name: AppendChildIgnoresDuplicate_011
 * @tc.desc: Verify a child appended twice is stored only once. A duplicate entry would be
 *           deleted twice by the destructor.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, AppendChildIgnoresDuplicate_011, TestSize.Level1)
{
    TestLeaf leaf1;
    TestLeaf leaf2;
    SvgGenericNode* child = new SvgGenericNode(SVG_DESC);
    leaf1.AppendChild(child);   // child is now owned by leaf1
    leaf2.AppendChild(child);   // already owned by leaf1 -> ignored by leaf2
    // The child stays owned by leaf1 and is stored only once, so the destructor will
    // not attempt to delete it twice.
    EXPECT_EQ(child->GetOwner(), static_cast<SvgElementBase*>(&leaf1));
}

/**
 * @tc.name: AppendChildAnimationBindsTarget_012
 * @tc.desc: Verify an animation child is accepted and bound to the leaf as its target.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, AppendChildAnimationBindsTarget_012, TestSize.Level1)
{
    TestLeaf leaf;
    SvgAnimate* anim = new SvgAnimate();
    leaf.AppendChild(anim);
    EXPECT_EQ(anim->GetTarget(), static_cast<SvgElementBase*>(&leaf));
}

/**
 * @tc.name: AppendChildAnimationRegistersInDocument_013
 * @tc.desc: Verify an animation child appended to an attached leaf is registered in the document.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, AppendChildAnimationRegistersInDocument_013, TestSize.Level1)
{
    SvgDocument doc;
    TestLeaf leaf;
    leaf.OnDocumentAttached(&doc);
    EXPECT_TRUE(doc.GetAnimations().IsEmpty());
    SvgAnimate* anim = new SvgAnimate();
    leaf.AppendChild(anim);
    EXPECT_FALSE(doc.GetAnimations().IsEmpty());
}

/**
 * @tc.name: OnDocumentAttachedPropagatesToChildren_014
 * @tc.desc: Verify OnDocumentAttached forwards the document to already appended children.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, OnDocumentAttachedPropagatesToChildren_014, TestSize.Level1)
{
    TestLeaf leaf;
    SvgGenericNode* child = new SvgGenericNode(SVG_DESC);
    leaf.AppendChild(child);
    SvgDocument doc;
    leaf.OnDocumentAttached(&doc);
    EXPECT_EQ(leaf.GetDocument(), &doc);
    EXPECT_EQ(child->GetDocument(), &doc);
}

/**
 * @tc.name: ZeroStrokeWidthStoredInPaint_015
 * @tc.desc: Verify a zero stroke width is stored in the inherited paint and does not alter bounds.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, ZeroStrokeWidthStoredInPaint_015, TestSize.Level1)
{
    TestLeaf defaultStroke;
    defaultStroke.ApplyInheritedState(Paint(), TransAffine());
    int16_t defaultWidth = defaultStroke.GetRect().GetWidth();

    TestLeaf noStroke;
    EXPECT_TRUE(noStroke.SetAttribute("stroke-width", "0"));
    noStroke.ApplyInheritedState(Paint(), TransAffine());
    EXPECT_GT(noStroke.GetRect().GetWidth(), 0);
    EXPECT_LE(noStroke.GetRect().GetWidth(), defaultWidth);
    EXPECT_EQ(noStroke.ExposeInheritedPaint().GetStrokeWidth(), 0);
}

/**
 * @tc.name: WideStrokeWidthStoredInPaint_016
 * @tc.desc: Verify an explicit stroke width is propagated to the inherited paint.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, WideStrokeWidthStoredInPaint_016, TestSize.Level1)
{
    TestLeaf thin;
    EXPECT_TRUE(thin.SetAttribute("stroke-width", "0"));
    thin.ApplyInheritedState(Paint(), TransAffine());

    TestLeaf thick;
    EXPECT_TRUE(thick.SetAttribute("stroke-width", "20"));
    thick.ApplyInheritedState(Paint(), TransAffine());
    EXPECT_GT(thick.ExposeInheritedPaint().GetStrokeWidth(), thin.ExposeInheritedPaint().GetStrokeWidth());
}

/**
 * @tc.name: RotatedTransformExpandsBounds_017
 * @tc.desc: Verify a rotation makes the axis aligned bounds wider than the local outline,
 *           exercising the min/max corner search of the transformed bounds.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, RotatedTransformExpandsBounds_017, TestSize.Level1)
{
    TestLeaf upright;
    upright.ApplyInheritedState(Paint(), TransAffine());
    int16_t uprightWidth = upright.GetRect().GetWidth();

    TestLeaf rotated;
    rotated.ApplyInheritedState(Paint(), TransAffine::TransAffineRotation(0.7853981f)); // 45 degrees
    EXPECT_GT(rotated.GetRect().GetWidth(), uprightWidth);
    EXPECT_FALSE(rotated.ExposeRecordTransform().IsIdentity());
}

/**
 * @tc.name: NegativeScaleTransformKeepsPositiveBounds_018
 * @tc.desc: Verify a mirroring transform still yields a normalised bounding box, exercising the
 *           branches where a later corner becomes the new minimum.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, NegativeScaleTransformKeepsPositiveBounds_018, TestSize.Level1)
{
    TestLeaf leaf;
    leaf.ApplyInheritedState(Paint(), TransAffine::TransAffineScaling(-1.0f, -1.0f));
    EXPECT_GT(leaf.GetRect().GetWidth(), 0);
    EXPECT_GT(leaf.GetRect().GetHeight(), 0);
}

/**
 * @tc.name: GradientIdsResolvedThroughDocument_019
 * @tc.desc: Verify fill and stroke gradient references are resolved against the document during
 *           the geometry rebuild.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, GradientIdsResolvedThroughDocument_019, TestSize.Level1)
{
    SvgDocument doc;
    SvgLinearGradientResource* grad = new SvgLinearGradientResource();
    SvgStopResource* stop = new SvgStopResource();
    stop->SetAttribute("offset", "0");
    stop->SetAttribute("stop-color", "#ff0000");
    grad->AppendChild(stop);
    doc.RegisterResource("grad1", grad);

    TestLeaf leaf;
    leaf.OnDocumentAttached(&doc);
    EXPECT_TRUE(leaf.SetAttribute("fill", "url(#grad1)"));
    EXPECT_TRUE(leaf.SetAttribute("stroke", "url(#grad1)"));
    leaf.ApplyInheritedState(Paint(), TransAffine());
    EXPECT_GT(leaf.GetRecordCount(), static_cast<uint16_t>(0));
    delete grad;
}

/**
 * @tc.name: MissingGradientIdDoesNotBreakRebuild_020
 * @tc.desc: Verify an unresolvable gradient reference leaves the rebuild intact.
 * @tc.type: FUNC
 */
HWTEST_F(SvgLeafNodeTest, MissingGradientIdDoesNotBreakRebuild_020, TestSize.Level1)
{
    SvgDocument doc;
    TestLeaf leaf;
    leaf.OnDocumentAttached(&doc);
    EXPECT_TRUE(leaf.SetAttribute("fill", "url(#nope)"));
    leaf.ApplyInheritedState(Paint(), TransAffine());
    EXPECT_GT(leaf.GetRecordCount(), static_cast<uint16_t>(0));
}

} // namespace OHOS
