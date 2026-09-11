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

#ifndef GRAPHIC_LITE_SVG_LEAF_NODE_H
#define GRAPHIC_LITE_SVG_LEAF_NODE_H

#include "components/ui_canvas.h"
#include "components/ui_view.h"
#include "gfx_utils/diagram/common/paint.h"
#include "gfx_utils/list.h"
#include "gfx_utils/trans_affine.h"
#include "svg_element_base.h"
#include "svg_paint_state.h"

namespace OHOS {

// Axis-aligned bounding box of a rectangle after applying an affine transform,
// expressed in the space the transform maps into. Shared by leaf stroke-ink
// margins and container sizing so a transformed child is not clipped to its
// untransformed box.
Rect ComputeTransformedBounds(const Rect& localBounds, const TransAffine& transform);

class SvgContainerNode;

class SvgLeafNode : public UIView, public SvgElementBase {
public:
    SvgLeafNode();

    ~SvgLeafNode() override;

    SvgElementCategory GetCategory() const override
    {
        return SVG_CATEGORY_VIEW;
    }

    bool SetAttribute(const char* name, const char* value) override;

    const TransAffine& GetTransform() const override { return paintState_.GetTransform(); }

    void SetTransform(const TransAffine& transform) override;

    void AppendChild(SvgElementBase* child) override;

    void OnDocumentAttached(SvgDocument* doc) override;

    UIViewType GetViewType() const override
    {
        return UI_SVG_LEAF;
    }

    void OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea) override;

    virtual void ApplyInheritedState(const Paint& parentPaint, const TransAffine& parentTransform);

    virtual Rect GetLocalBounds() const = 0;

protected:
    virtual bool SetGeometryAttribute(const char* name, const char* value) = 0;

    /**
     * @brief Records geometry into the provided canvas.
     *
     * Subclasses must bake GetRecordTransform() into the coordinates they pass
     * to the canvas, so that recorded points live in LEAF-LOCAL space (origin
     * at the transformed bounds' top-left). The canvas rect is synced to the
     * leaf's absolute rect, so replay applies exactly that offset and no double
     * translation occurs.
     */
    virtual void RecordGeometry(UICanvas& canvas) = 0;

    void RebuildGeometry();

    void CopyStateFrom(const SvgLeafNode& other);

    /**
     * @brief Returns the transform to map gradient coordinates from local/user space
     *        to the space in which this leaf's geometry is recorded.
     *
     * The default implementation returns GetRecordTransform(), used by leaves that
     * bake the accumulated transform into their recorded canvas path. Leaves that
     * keep their geometry in local space (e.g. circle/ellipse for sub-pixel AA)
     * override this to return identity.
     */
    virtual TransAffine GetGradientCoordinateTransform() const
    {
        return GetRecordTransform();
    }

    /**
     * @brief Returns the transform to bake into recorded points.
     *
     * This is Translate(-B.left, -B.top) x accumulatedTransform, where B is the
     * transformed bounding box of the local geometry. When a point is
     * transformed by this matrix and the canvas replays with its rect offset at
     * B.left/B.top, the result lands at the correct absolute position.
     */
    TransAffine GetRecordTransform() const;

    SvgPaintState paintState_;
    UICanvas canvas_;
    Paint inheritedPaint_;
    TransAffine accumulatedTransform_;
    Rect transformedBounds_;
    List<SvgElementBase*> children_;

private:
    void RegisterAnimationChild(SvgElementBase* child);

    Rect ComputeInkBounds(const Rect& localBounds) const;
    void ResolveGradientPaint(const Rect& localBounds);
    void UpdateViewAndCanvas();
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_LEAF_NODE_H
