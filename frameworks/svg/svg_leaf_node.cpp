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

#include "svg/svg_leaf_node.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_animation.h"
#include "svg/svg_container_nodes.h"
#include "svg/svg_document.h"
#include <cmath>
namespace OHOS {

constexpr uint8_t CORNER_COUNT = 4;

namespace {

// Extra room, in user units, that a stroke can occupy outside the fill outline.
// A stroke is centred on the path, so it reaches strokeWidth/2 outward; round and
// square end caps extend that much again past the path endpoints, and a miter join
// can spike up to miterlimit * strokeWidth/2 away from a sharp corner. Returns 0
// when the shape is not stroked, so unstroked shapes keep the exact outline box.
// The result is consumed in user space (before the CTM is applied) because the
// rasterizer scales the stroke along with the path.
int32_t ComputeStrokeInkMargin(const Paint& paint)
{
    Paint::PaintStyle style = paint.GetStyle();
    if ((style != Paint::STROKE_STYLE && style != Paint::STROKE_FILL_STYLE) ||
        paint.GetStrokeWidth() <= 0) {
        return 0;
    }
    const float half = static_cast<float>(paint.GetStrokeWidth()) / 2.0f;
    float margin = half;
    LineJoin join = paint.GetLineJoin();
    if (join == MITER_JOIN || join == MITER_JOIN_REVERT) {
        float miterLimit = paint.GetMiterLimit();
        if (miterLimit > 1.0f) {
            margin = half * miterLimit;
        }
    }
    LineCap cap = paint.GetLineCap();
    if (cap == SQUARE_CAP || cap == ROUND_CAP) {
        margin += half;
    }
    // Round up so odd stroke widths and anti-aliasing spill are not clipped.
    return static_cast<int32_t>(std::ceil(margin)) + 1;
}

} // namespace

// Computes the axis-aligned bounding box of a rectangle after applying an affine
// transform, expressed in the same coordinate space the transform maps into. Used
// both for leaf stroke ink margins and for sizing a container from its children's
// transformed footprints, so an offset/rotated child is not clipped to the
// untransformed box.
Rect ComputeTransformedBounds(const Rect& localBounds, const TransAffine& transform)
{
    // Local bounds store inclusive right/bottom, but scaling must operate on the
    // pixel extent (exclusive right/bottom). Otherwise a width-W rect scaled by S
    // would become (W-1)*S+1 pixels instead of W*S pixels.
    float minX = static_cast<float>(localBounds.GetLeft());
    float minY = static_cast<float>(localBounds.GetTop());
    float maxX = static_cast<float>(localBounds.GetRight() + 1);
    float maxY = static_cast<float>(localBounds.GetBottom() + 1);

    float xs[CORNER_COUNT] = {minX, maxX, minX, maxX};
    float ys[CORNER_COUNT] = {minY, minY, maxY, maxY};
    for (uint8_t i = 0; i < CORNER_COUNT; i++) {
        transform.Transform(&xs[i], &ys[i]);
    }

    float newMinX = xs[0];
    float newMinY = ys[0];
    float newMaxX = xs[0];
    float newMaxY = ys[0];
    for (uint8_t i = 1; i < CORNER_COUNT; i++) {
        if (xs[i] < newMinX) {
            newMinX = xs[i];
        }
        if (ys[i] < newMinY) {
            newMinY = ys[i];
        }
        if (xs[i] > newMaxX) {
            newMaxX = xs[i];
        }
        if (ys[i] > newMaxY) {
            newMaxY = ys[i];
        }
    }

    int16_t left = SvgAttributeParser::ClampToInt16(newMinX);
    int16_t top = SvgAttributeParser::ClampToInt16(newMinY);
    int16_t right = SvgAttributeParser::ClampToInt16(std::ceil(newMaxX)) - 1;
    int16_t bottom = SvgAttributeParser::ClampToInt16(std::ceil(newMaxY)) - 1;
    return {left, top, right, bottom};
}

SvgLeafNode::SvgLeafNode() : inheritedPaint_(), transformedBounds_()
{
    accumulatedTransform_.Reset();
    // Leaf canvas is only a recording surface for the shape; its background must be
    // transparent so sibling/overlapping shapes show through outside the geometry.
    canvas_.SetStyle(STYLE_BACKGROUND_OPA, OPA_TRANSPARENT);
}

SvgLeafNode::~SvgLeafNode()
{
    while (!children_.IsEmpty()) {
        SvgElementBase* child = children_.Front();
        children_.PopFront();
        UiDelete(child);
    }
}

bool SvgLeafNode::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "id") == 0) {
        SetIdAndRegister(value);
        return true;
    }
    if (paintState_.SetAttribute(name, value)) {
        SvgContainerNode* parent = dynamic_cast<SvgContainerNode*>(GetParent());
        if (parent != nullptr) {
            parent->RefreshChildState(this);
        } else {
            RebuildGeometry();
        }
        return true;
    }
    if (SetGeometryAttribute(name, value)) {
        RebuildGeometry();
        return true;
    }
    return false;
}

void SvgLeafNode::AppendChild(SvgElementBase* child)
{
    if (child == nullptr) {
        return;
    }
    if (child->GetOwner() != nullptr && child->GetOwner() != this) {
        return;
    }
    child->SetOwner(this);
    SvgElementCategory category = child->GetCategory();
    if (category == SVG_CATEGORY_ANIMATION || category == SVG_CATEGORY_GENERIC) {
        children_.PushBack(child);
        if (category == SVG_CATEGORY_ANIMATION) {
            RegisterAnimationChild(child);
        }
        return;
    }
    // Caller retains ownership of rejected children.
}

void SvgLeafNode::RegisterAnimationChild(SvgElementBase* child)
{
    SvgAnimation* anim = dynamic_cast<SvgAnimation*>(child);
    if (anim == nullptr) {
        return;
    }
    anim->SetTarget(this);
    anim->ResolveTarget();
    if (doc_ != nullptr) {
        doc_->RegisterAnimation(child);
    }
}

void SvgLeafNode::OnDocumentAttached(SvgDocument* doc)
{
    SvgElementBase::OnDocumentAttached(doc);
    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End(); node = node->next_) {
        if (node->data_ != nullptr) {
            node->data_->OnDocumentAttached(doc);
        }
    }
}

void SvgLeafNode::OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea)
{
    Rect absolute = GetOrigRect();
    canvas_.SetPosition(absolute.GetLeft(), absolute.GetTop());
    canvas_.Resize(absolute.GetWidth(), absolute.GetHeight());
    canvas_.OnDraw(gfxDstBuffer, invalidatedArea);
}

void SvgLeafNode::ApplyInheritedState(const Paint& parentPaint, const TransAffine& parentTransform)
{
    inheritedPaint_ = paintState_.Apply(parentPaint);
    accumulatedTransform_ = parentTransform;
    SvgAttributeParser::PostMultiply(accumulatedTransform_, paintState_.GetTransform());

    // Geometry is baked into the recorded canvas path, so the paint transform
    // must not apply the same transform again during replay.
    inheritedPaint_.SetTransform(1.0f, 0.0f, 0.0f, 1.0f, 0, 0);

    RebuildGeometry();
}

void SvgLeafNode::SetTransform(const TransAffine& transform)
{
    paintState_.SetTransform(transform);
    // AnimateTransform updates the leaf's local transform each frame. Recompose
    // the accumulated CTM from the parent's current transform and rebuild the
    // geometry so the view rect and recorded path follow the animation.
    SvgContainerNode* parent = dynamic_cast<SvgContainerNode*>(GetParent());
    accumulatedTransform_ = (parent != nullptr) ? parent->GetAccumulatedTransform() : TransAffine();
    SvgAttributeParser::PostMultiply(accumulatedTransform_, transform);
    RebuildGeometry();
}

Rect SvgLeafNode::ComputeInkBounds(const Rect& localBounds) const
{
    // The leaf view is sized to the transformed ink box of the shape. The stroke
    // allowance is applied in user space BEFORE the CTM: the rasterizer scales the
    // stroke together with the path, so inflating the already-transformed box by the
    // raw stroke width under-covers by the viewBox scale factor and clips the stroke
    // at the outline extremes (the top/bottom/left/right of a circle, for instance).
    // Absolute positioning is preserved because GetRecordTransform() offsets the
    // recorded path by transformedBounds_, which moves together with this view.
    Rect inkBounds = localBounds;
    // Gradient objectBoundingBox units are defined against the fill outline, so the
    // stroke allowance must only widen the ink box used for the view/canvas rect.
    int32_t strokeMargin = ComputeStrokeInkMargin(inheritedPaint_);
    if (strokeMargin > 0) {
        int32_t left = static_cast<int32_t>(
            SvgAttributeParser::ClampToInt16(static_cast<float>(inkBounds.GetLeft()))) - strokeMargin;
        int32_t top = static_cast<int32_t>(
            SvgAttributeParser::ClampToInt16(static_cast<float>(inkBounds.GetTop()))) - strokeMargin;
        int32_t right = static_cast<int32_t>(
            SvgAttributeParser::ClampToInt16(static_cast<float>(inkBounds.GetRight()))) + strokeMargin;
        int32_t bottom = static_cast<int32_t>(
            SvgAttributeParser::ClampToInt16(static_cast<float>(inkBounds.GetBottom()))) + strokeMargin;
        inkBounds = Rect(SvgAttributeParser::ClampToInt16(left),
                         SvgAttributeParser::ClampToInt16(top),
                         SvgAttributeParser::ClampToInt16(right),
                         SvgAttributeParser::ClampToInt16(bottom));
    }
    return inkBounds;
}

void SvgLeafNode::ResolveGradientPaint(const Rect& localBounds)
{
    if (doc_ == nullptr) {
        return;
    }
    TransAffine recordSpace = GetGradientCoordinateTransform();
    const char* fillId = paintState_.GetFillGradientId();
    if (fillId != nullptr) {
        if (!doc_->ResolvePaintServer(fillId, inheritedPaint_, localBounds, &recordSpace, true)) {
            // SVG: an invalid paint-server reference with no fallback is treated as 'none'.
            inheritedPaint_.SetFillColor(Color::GetColorFromRGBA(0, 0, 0, 0));
        }
    }
    const char* strokeId = paintState_.GetStrokeGradientId();
    if (strokeId != nullptr) {
        if (!doc_->ResolvePaintServer(strokeId, inheritedPaint_, localBounds, &recordSpace, false)) {
            inheritedPaint_.SetStrokeColor(Color::GetColorFromRGBA(0, 0, 0, 0));
        }
    }
}

void SvgLeafNode::UpdateViewAndCanvas()
{
    int16_t width = transformedBounds_.GetWidth();
    int16_t height = transformedBounds_.GetHeight();
    SetPosition(transformedBounds_.GetLeft(), transformedBounds_.GetTop());
    Resize(width, height);

    Rect absolute = GetOrigRect();
    canvas_.SetPosition(absolute.GetLeft(), absolute.GetTop());
    canvas_.Resize(absolute.GetWidth(), absolute.GetHeight());
    Invalidate();
}

void SvgLeafNode::RebuildGeometry()
{
    canvas_.Clear();

    Rect localBounds = GetLocalBounds();
    Rect inkBounds = ComputeInkBounds(localBounds);
    transformedBounds_ = ComputeTransformedBounds(inkBounds, accumulatedTransform_);

    ResolveGradientPaint(localBounds);
    RecordGeometry(canvas_);
    UpdateViewAndCanvas();
}

void SvgLeafNode::CopyStateFrom(const SvgLeafNode& other)
{
    paintState_.CopyFrom(other.paintState_);
    inheritedPaint_ = other.inheritedPaint_;
    accumulatedTransform_ = other.accumulatedTransform_;
}

TransAffine SvgLeafNode::GetRecordTransform() const
{
    TransAffine record = accumulatedTransform_;
    record.Translate(-static_cast<float>(transformedBounds_.GetLeft()),
                     -static_cast<float>(transformedBounds_.GetTop()));
    return record;
}

} // namespace OHOS
