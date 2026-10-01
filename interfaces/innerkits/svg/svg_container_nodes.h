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

#ifndef GRAPHIC_LITE_SVG_CONTAINER_NODES_H
#define GRAPHIC_LITE_SVG_CONTAINER_NODES_H

#include "components/ui_view_group.h"
#include "gfx_utils/diagram/common/paint.h"
#include "gfx_utils/list.h"
#include "gfx_utils/trans_affine.h"
#include "svg_attribute_parser.h"
#include "svg_document.h"
#include "svg_element_base.h"
#include "svg_leaf_node.h"
#include "svg_paint_state.h"

namespace OHOS {

class SvgContainerNode : public UIViewGroup, public SvgElementBase {
public:
    SvgContainerNode();
    ~SvgContainerNode() override;
    SvgContainerNode(const SvgContainerNode&) = delete;
    SvgContainerNode& operator=(const SvgContainerNode&) = delete;

    SvgElementCategory GetCategory() const override
    {
        return SVG_CATEGORY_VIEW;
    }

    UIViewType GetViewType() const override
    {
        return UI_SVG_CONTAINER;
    }

    bool SetAttribute(const char* name, const char* value) override;

    const TransAffine& GetTransform() const override { return paintState_.GetTransform(); }

    // Returns this node's accumulated (parent * own) CTM. Animation callbacks use it
    // to compose a child's transform onto the correct parent matrix.
    const TransAffine& GetAccumulatedTransform() const { return accumulatedTransform_; }

    void SetTransform(const TransAffine& transform) override;

    void AppendChild(SvgElementBase* child) override;

    // Virtual so SvgRootNode can re-apply its own viewBox mapping after the
    // inherited state has been pushed into it (see SvgRootNode::ApplyInheritedState).
    virtual void ApplyInheritedState(const Paint& parentPaint, const TransAffine& parentTransform);

    void PushStateToChildren();

    void RefreshChildState(SvgElementBase* child);

    void OnDocumentAttached(SvgDocument* doc) override;

    SvgElementBase* Clone() const override;

protected:
    void CopyStateFrom(const SvgContainerNode& other);

    virtual void CloneChildrenInto(SvgContainerNode* dest) const;

    virtual void OnChildAdded(UIView* child);

    void PropagateDocument(SvgElementBase* element);

    // By default containers are sized to the union of their children's bounds.
    // Groups return true here so they are sized to their parent's bounds; this
    // prevents transformed children from being clipped by an axis-aligned bbox
    // that does not account for the container's own rotation/scale animation.
    virtual bool ExpandToParentBounds() const { return false; }

    void ResizeToChildren();
    bool ComputeChildrenBounds(int16_t& minX, int16_t& minY, int16_t& maxX, int16_t& maxY) const;
    void ApplyChildrenBounds(int16_t minX, int16_t minY, int16_t maxX, int16_t maxY);

    bool resizeToChildren_ = false;

    SvgPaintState paintState_;
    Paint inheritedPaint_;
    TransAffine accumulatedTransform_;
    List<SvgElementBase*> ownedChildren_;
};

class SvgRootNode : public SvgContainerNode {
public:
    bool SetAttribute(const char* name, const char* value) override;

    void SyncViewport(int16_t fallbackW, int16_t fallbackH);

    void UpdateViewportTransform();

    // A root maps its own user space into its viewport via its viewBox. That mapping
    // must compose with the transform inherited from its ancestors. Without this
    // override the viewBox mapping installed by SyncViewport is lost as soon as
    // inherited state is propagated again (for example when the parent appends this
    // root as a child).
    void ApplyInheritedState(const Paint& parentPaint, const TransAffine& parentTransform) override;

    SvgElementBase* Clone() const override;

    void AttachDocument(SvgDocument* doc);

private:
    void ComputeViewBoxTransform(TransAffine& matrix, int16_t viewportW, int16_t viewportH) const;

    int16_t width_ = 0;
    int16_t height_ = 0;
    float viewBox_[4] = {0.0f};
    bool hasViewBox_ = false;
    uint8_t preserveAlign_ = SvgAttributeParser::PRESERVE_ALIGN_XMIDYMID; // xMidYMid meet (SVG spec default)
    bool preserveSlice_ = false;
};

class SvgGroupNode : public SvgContainerNode {
public:
    SvgGroupNode() { resizeToChildren_ = true; }
    SvgGroupNode(const SvgGroupNode&) = delete;
    SvgGroupNode& operator=(const SvgGroupNode&) = delete;
    SvgElementBase* Clone() const override;

    // Use the parent's content rect as the group's content rect. This prevents
    // child invalidations from being truncated to the group's own (possibly
    // smaller) bounds when the group's transform pushes children outside.
    Rect GetContentRect() override;

protected:
    bool ExpandToParentBounds() const override { return true; }
};

class SvgUseNode : public SvgContainerNode {
public:
    SvgUseNode();

    ~SvgUseNode() override;

    bool SetAttribute(const char* name, const char* value) override;

    void OnDocumentReady() override;

    void OnDocumentAttached(SvgDocument* doc) override;

    SvgElementBase* Clone() const override;

private:
    void ResolveAndExpand();

    char* ref_ = nullptr;
    int16_t x_ = 0;
    int16_t y_ = 0;
    int16_t width_ = 0;
    int16_t height_ = 0;
    bool expanded_ = false;
};

class SvgSwitchNode : public SvgContainerNode {
public:
    bool SetAttribute(const char* name, const char* value) override;

    void AppendChild(SvgElementBase* child) override;

    SvgElementBase* Clone() const override;

protected:
    virtual bool EvaluateChildConditions(SvgElementBase* child) const;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_CONTAINER_NODES_H
