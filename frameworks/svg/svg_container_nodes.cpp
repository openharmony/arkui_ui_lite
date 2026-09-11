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

#include "svg/svg_container_nodes.h"
#include "svg/svg_attribute_parser.h"
#include "svg/svg_animation.h"
#include "svg/svg_string_util.h"
#include "securec.h"
#include <algorithm>
#include <cstring>

namespace OHOS {

namespace {

constexpr uint8_t MAX_USE_DEPTH = 16;
thread_local uint8_t g_useDepth = 0;

constexpr uint8_t PRESERVE_ALIGN_NONE = 9;
constexpr uint8_t ALIGN_X_DIVISOR = 3;
constexpr float ALIGN_FACTOR_MID = 0.5f;

constexpr uint8_t TRANSFORM_BUF_LEN = 64;

struct UseDepthGuard {
    explicit UseDepthGuard(uint8_t& depth) : depth_(depth) { depth_++; }
    ~UseDepthGuard() { depth_--; }
    UseDepthGuard(const UseDepthGuard&) = delete;
    UseDepthGuard& operator=(const UseDepthGuard&) = delete;
private:
    uint8_t& depth_;
};

} // namespace

SvgContainerNode::SvgContainerNode() : inheritedPaint_(), accumulatedTransform_()
{
    accumulatedTransform_.Reset();
    // SVG initial paint: fill is black, stroke is none. The default Paint has
    // STROKE_FILL_STYLE with a white stroke, so reset to fill-only to avoid
    // every shape inheriting an unwanted white border.
    inheritedPaint_.SetStyle(Paint::FILL_STYLE);
    // Containers are logical groupings; they must not draw an opaque background
    // that would obscure sibling or child shapes.
    SetStyle(STYLE_BACKGROUND_OPA, OPA_TRANSPARENT);
}

SvgContainerNode::~SvgContainerNode()
{
    while (!ownedChildren_.IsEmpty()) {
        SvgElementBase* child = ownedChildren_.Front();
        ownedChildren_.PopFront();
        UiDelete(child);
    }
}

void SvgContainerNode::SetTransform(const TransAffine& transform)
{
    paintState_.SetTransform(transform);
    // Recompute the accumulated CTM from the parent's so children inherit the
    // composed (base * animated) transform. PushStateToChildren alone would reuse
    // the stale accumulatedTransform_ from the last tree layout and the rect would
    // never reflect the animation.
    UIView* parentView = GetParent();
    if (parentView != nullptr && parentView->GetViewType() == UI_SVG_CONTAINER) {
        SvgContainerNode* parent = static_cast<SvgContainerNode*>(parentView);
        accumulatedTransform_ = parent->GetAccumulatedTransform();
    } else {
        accumulatedTransform_.Reset();
    }
    SvgAttributeParser::PostMultiply(accumulatedTransform_, transform);
    PushStateToChildren();
    // The container's own canvas/clip size is handled by ResizeToChildren()
    // during layout. Groups are expanded to their parent's bounds so animated
    // transforms do not clip rotated children; no per-frame resize is needed.
}

bool SvgContainerNode::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "id") == 0) {
        SetIdAndRegister(value);
        return true;
    }
    if (paintState_.SetAttribute(name, value)) {
        UIView* parentView = GetParent();
        if (parentView != nullptr && parentView->GetViewType() == UI_SVG_CONTAINER) {
            SvgContainerNode* parent = static_cast<SvgContainerNode*>(parentView);
            parent->RefreshChildState(this);
        } else {
            PushStateToChildren();
        }
        return true;
    }
    return false;
}

void SvgContainerNode::AppendChild(SvgElementBase* child)
{
    if (child == nullptr) {
        return;
    }
    if (child->GetOwner() != nullptr && child->GetOwner() != this) {
        return;
    }
    child->SetOwner(this);
    ownedChildren_.PushBack(child);
    if (doc_ != nullptr) {
        child->OnDocumentAttached(doc_);
    }
    SvgElementCategory category = child->GetCategory();
    if (category == SVG_CATEGORY_VIEW) {
        UIView* view = dynamic_cast<UIView*>(child);
        if (view != nullptr) {
            UIViewGroup::Add(view);
            OnChildAdded(view);
            if (resizeToChildren_) {
                ResizeToChildren();
            }
        }
    } else if (category == SVG_CATEGORY_ANIMATION) {
        SvgAnimation* anim = dynamic_cast<SvgAnimation*>(child);
        if (anim != nullptr) {
            anim->SetTarget(this);
            anim->ResolveTarget();
        }
        if (doc_ != nullptr) {
            doc_->RegisterAnimation(child);
        }
    }
}

void SvgContainerNode::ApplyInheritedState(const Paint& parentPaint, const TransAffine& parentTransform)
{
    inheritedPaint_ = paintState_.Apply(parentPaint);
    accumulatedTransform_ = parentTransform;
    SvgAttributeParser::PostMultiply(accumulatedTransform_, paintState_.GetTransform());
    PushStateToChildren();
    if (resizeToChildren_) {
        ResizeToChildren();
    }
}

void SvgContainerNode::PushStateToChildren()
{
    UIView* child = GetChildrenHead();
    while (child != nullptr) {
        UIViewType type = child->GetViewType();
        if (type == UI_SVG_LEAF) {
            SvgLeafNode* leaf = static_cast<SvgLeafNode*>(child);
            leaf->ApplyInheritedState(inheritedPaint_, accumulatedTransform_);
        } else if (type == UI_SVG_CONTAINER) {
            SvgContainerNode* container = static_cast<SvgContainerNode*>(child);
            container->ApplyInheritedState(inheritedPaint_, accumulatedTransform_);
        }
        child = child->GetNextSibling();
    }
}

void SvgContainerNode::RefreshChildState(SvgElementBase* child)
{
    if (child == nullptr) {
        return;
    }
    SvgLeafNode* leaf = dynamic_cast<SvgLeafNode*>(child);
    if (leaf != nullptr) {
        leaf->ApplyInheritedState(inheritedPaint_, accumulatedTransform_);
        return;
    }
    SvgContainerNode* container = dynamic_cast<SvgContainerNode*>(child);
    if (container != nullptr) {
        container->ApplyInheritedState(inheritedPaint_, accumulatedTransform_);
    }
}

SvgElementBase* SvgContainerNode::Clone() const
{
    return nullptr;
}

void SvgContainerNode::CopyStateFrom(const SvgContainerNode& other)
{
    paintState_.CopyFrom(other.paintState_);
    inheritedPaint_ = other.inheritedPaint_;
    accumulatedTransform_ = other.accumulatedTransform_;
}

void SvgContainerNode::OnDocumentAttached(SvgDocument* doc)
{
    SvgElementBase::OnDocumentAttached(doc);
    ListNode<SvgElementBase*>* node = ownedChildren_.Begin();
    for (; node != ownedChildren_.End(); node = node->next_) {
        if (node->data_ != nullptr) {
            node->data_->OnDocumentAttached(doc);
        }
    }
}

void SvgContainerNode::PropagateDocument(SvgElementBase* element)
{
    if (element == nullptr || doc_ == nullptr) {
        return;
    }
    element->OnDocumentAttached(doc_);
}

void SvgContainerNode::CloneChildrenInto(SvgContainerNode* dest) const
{
    if (dest == nullptr) {
        return;
    }
    if (doc_ != nullptr) {
        dest->SetDocument(doc_);
    }
    ListNode<SvgElementBase*>* node = ownedChildren_.Begin();
    for (; node != ownedChildren_.End(); node = node->next_) {
        SvgElementBase* child = node->data_;
        if (child == nullptr) {
            continue;
        }
        SvgElementBase* childClone = child->Clone();
        if (childClone != nullptr) {
            dest->AppendChild(childClone);
        }
    }
}

void SvgContainerNode::OnChildAdded(UIView* child)
{
    if (child == nullptr) {
        return;
    }
    UIViewType type = child->GetViewType();
    if (type == UI_SVG_LEAF) {
        SvgLeafNode* leaf = static_cast<SvgLeafNode*>(child);
        leaf->ApplyInheritedState(inheritedPaint_, accumulatedTransform_);
    } else if (type == UI_SVG_CONTAINER) {
        SvgContainerNode* container = static_cast<SvgContainerNode*>(child);
        container->ApplyInheritedState(inheritedPaint_, accumulatedTransform_);
    }
}

void SvgContainerNode::ResizeToChildren()
{
    // SVG <g> containers do not clip their children by default. Because the
    // container's own transform (e.g. a rotation animation) is pushed to the
    // children, the children's transformed bounds can extend outside an
    // axis-aligned bbox computed from the untransformed children. Sizing the
    // group to its parent's bounds keeps the clipping region large enough while
    // still anchoring the container at the SVG origin.
    if (ExpandToParentBounds()) {
        SvgContainerNode* parent = dynamic_cast<SvgContainerNode*>(GetParent());
        if (parent != nullptr) {
            SetPosition(0, 0);
            Resize(parent->GetWidth(), parent->GetHeight());
            return;
        }
    }

    int16_t minX = 0;
    int16_t minY = 0;
    int16_t maxX = -1;
    int16_t maxY = -1;
    if (!ComputeChildrenBounds(minX, minY, maxX, maxY)) {
        return;
    }
    ApplyChildrenBounds(minX, minY, maxX, maxY);
}

bool SvgContainerNode::ComputeChildrenBounds(int16_t& minX, int16_t& minY, int16_t& maxX, int16_t& maxY) const
{
    bool hasChild = false;
    UIView* child = GetChildrenHead();
    while (child != nullptr) {
        if (!child->IsVisible()) {
            child = child->GetNextSibling();
            continue;
        }
        // Measure the child's footprint through its own transform, so an offset,
        // scaled or rotated child is not clipped to its untransformed box. This is
        // the general behaviour expected of containers (a <use>/<g> offset
        // must expand the parent clip, not get cut at the origin). GetRelativeRect
        // is still relative to this container (not screen-absolute), so the earlier
        // negative-coordinate scroll regression is not reintroduced.
        SvgElementBase* childBase = dynamic_cast<SvgElementBase*>(child);
        Rect childRect = (childBase != nullptr)
            ? ComputeTransformedBounds(child->GetRelativeRect(), childBase->GetTransform())
            : child->GetRelativeRect();
        if (!hasChild) {
            minX = childRect.GetLeft();
            minY = childRect.GetTop();
            maxX = childRect.GetRight();
            maxY = childRect.GetBottom();
            hasChild = true;
        } else {
            if (childRect.GetLeft() < minX) {
                minX = childRect.GetLeft();
            }
            if (childRect.GetTop() < minY) {
                minY = childRect.GetTop();
            }
            if (childRect.GetRight() > maxX) {
                maxX = childRect.GetRight();
            }
            if (childRect.GetBottom() > maxY) {
                maxY = childRect.GetBottom();
            }
        }
        child = child->GetNextSibling();
    }
    return hasChild;
}

void SvgContainerNode::ApplyChildrenBounds(int16_t minX, int16_t minY, int16_t maxX, int16_t maxY)
{
    // Children are measured in coordinates relative to this container (GetRelativeRect),
    // which are independent of where the SVG sits on screen. An earlier version used
    // GetOrigRect() (screen-absolute), so once the SVG scrolled to a negative screen
    // Y the computed bounds turned negative, the container's clip rect became invalid,
    // and the group/use content was culled to blank. Keeping the container anchored at
    // the SVG origin and sizing only from the local child extents avoids that.
    SetPosition(0, 0);
    int16_t width = (maxX + 1 > 0) ? static_cast<int16_t>(maxX + 1) : 0;
    int16_t height = (maxY + 1 > 0) ? static_cast<int16_t>(maxY + 1) : 0;
    Resize(width, height);
}

// ---------- SvgRootNode ----------

bool SvgRootNode::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "width") == 0) {
        width_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "height") == 0) {
        height_ = SvgAttributeParser::ParseLength(value);
    } else if (strcmp(name, "viewBox") == 0) {
        hasViewBox_ = SvgAttributeParser::ParseViewBox(value, viewBox_);
    } else if (strcmp(name, "preserveAspectRatio") == 0) {
        SvgAttributeParser::ParsePreserveAspectRatio(value, preserveAlign_, preserveSlice_);
    } else {
        return SvgContainerNode::SetAttribute(name, value);
    }
    if (GetWidth() > 0 && GetHeight() > 0) {
        UpdateViewportTransform();
    }
    return true;
}

void SvgRootNode::SyncViewport(int16_t fallbackW, int16_t fallbackH)
{
    int16_t rw = (width_ > 0) ? width_ : fallbackW;
    int16_t rh = (height_ > 0) ? height_ : fallbackH;
    if (rw > 0 && rh > 0) {
        SetPosition(0, 0, rw, rh);
    }
    UpdateViewportTransform();
}

void SvgRootNode::ComputeViewBoxTransform(TransAffine& matrix, int16_t viewportW, int16_t viewportH) const
{
    matrix.Reset();
    if (!hasViewBox_ || viewportW <= 0 || viewportH <= 0) {
        return;
    }
    float vbX = viewBox_[0];
    float vbY = viewBox_[1];
    float vbW = viewBox_[2];
    float vbH = viewBox_[3];
    if (vbW <= 0.0f || vbH <= 0.0f) {
        return;
    }
    if (preserveAlign_ == PRESERVE_ALIGN_NONE) {
        float sx = static_cast<float>(viewportW) / vbW;
        float sy = static_cast<float>(viewportH) / vbH;
        TransAffine scale = TransAffine::TransAffineScaling(sx, sy);
        TransAffine trans = TransAffine::TransAffineTranslation(-vbX * sx, -vbY * sy);
        matrix = trans;
        SvgAttributeParser::PostMultiply(matrix, scale);
        return;
    }
    float sx = static_cast<float>(viewportW) / vbW;
    float sy = static_cast<float>(viewportH) / vbH;
    float scale = preserveSlice_ ? std::max(sx, sy) : std::min(sx, sy);
    float alignX = static_cast<float>(preserveAlign_ % ALIGN_X_DIVISOR) * ALIGN_FACTOR_MID;
    float alignY = static_cast<float>(preserveAlign_ / ALIGN_X_DIVISOR) * ALIGN_FACTOR_MID;
    float tx = -vbX * scale + (static_cast<float>(viewportW) - vbW * scale) * alignX;
    float ty = -vbY * scale + (static_cast<float>(viewportH) - vbH * scale) * alignY;
    TransAffine trans = TransAffine::TransAffineTranslation(tx, ty);
    TransAffine sc = TransAffine::TransAffineScaling(scale, scale);
    matrix = trans;
    SvgAttributeParser::PostMultiply(matrix, sc);
}

void SvgRootNode::UpdateViewportTransform()
{
    int16_t viewportW = (width_ > 0) ? width_ : GetWidth();
    int16_t viewportH = (height_ > 0) ? height_ : GetHeight();
    TransAffine viewBoxMatrix;
    ComputeViewBoxTransform(viewBoxMatrix, viewportW, viewportH);
    accumulatedTransform_ = viewBoxMatrix;
    SvgAttributeParser::PostMultiply(accumulatedTransform_, paintState_.GetTransform());
    PushStateToChildren();
}

void SvgRootNode::ApplyInheritedState(const Paint& parentPaint, const TransAffine& parentTransform)
{
    // The graphics default paint carries a white stroke of width 2, which is not
    // SVG stroke inheritance. Mark it so Apply() can ignore the default stroke
    // without re-inspecting color/width values at every node.
    paintState_.SetIgnoreDefaultStroke(SvgPaintState::IsDefaultParentPaint(parentPaint));
    SvgContainerNode::ApplyInheritedState(parentPaint, parentTransform);
    // Re-apply this root's viewBox mapping on top of the inherited transform, using
    // the same composition order as UpdateViewportTransform (viewBox then own
    // transform) so that a nested root keeps both the ancestors' transform and its
    // own user space to viewport mapping.
    int16_t viewportW = (width_ > 0) ? width_ : GetWidth();
    int16_t viewportH = (height_ > 0) ? height_ : GetHeight();
    TransAffine viewBoxMatrix;
    ComputeViewBoxTransform(viewBoxMatrix, viewportW, viewportH);
    accumulatedTransform_ = parentTransform;
    SvgAttributeParser::PostMultiply(accumulatedTransform_, viewBoxMatrix);
    SvgAttributeParser::PostMultiply(accumulatedTransform_, paintState_.GetTransform());
    PushStateToChildren();
}

SvgElementBase* SvgRootNode::Clone() const
{
    SvgRootNode* clone = new SvgRootNode();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->width_ = width_;
    clone->height_ = height_;
    clone->hasViewBox_ = hasViewBox_;
    for (uint8_t i = 0; i < SvgAttributeParser::VIEWBOX_COMPONENT_COUNT; i++) {
        clone->viewBox_[i] = viewBox_[i];
    }
    clone->preserveAlign_ = preserveAlign_;
    clone->preserveSlice_ = preserveSlice_;
    clone->CopyStateFrom(*this);
    CloneChildrenInto(clone);
    return clone;
}

void SvgRootNode::AttachDocument(SvgDocument* doc)
{
    SetDocument(doc);
    if (GetDocument() != nullptr) {
        GetDocument()->SetRoot(this);
    }
}

// ---------- SvgGroupNode ----------

SvgElementBase* SvgGroupNode::Clone() const
{
    SvgGroupNode* clone = new SvgGroupNode();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->CopyStateFrom(*this);
    CloneChildrenInto(clone);
    return clone;
}

Rect SvgGroupNode::GetContentRect()
{
    // The group's own content rect must cover the whole parent viewport so that
    // transformed children can invalidate pixels outside the group's untransformed
    // bounds. Without this, child invalidations are truncated to the group's bbox
    // and rotating content gets clipped.
    UIView* par = GetParent();
    if (par != nullptr) {
        return par->GetContentRect();
    }
    return SvgContainerNode::GetContentRect();
}

// ---------- SvgUseNode ----------

SvgUseNode::SvgUseNode() : ref_(nullptr), x_(0), y_(0), width_(0), height_(0)
{
    resizeToChildren_ = true;
}

SvgUseNode::~SvgUseNode()
{
    delete[] ref_;
    ref_ = nullptr;
}

bool SvgUseNode::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "x") == 0) {
        x_ = SvgAttributeParser::ParseLength(value);
        return true;
    }
    if (strcmp(name, "y") == 0) {
        y_ = SvgAttributeParser::ParseLength(value);
        return true;
    }
    if (strcmp(name, "width") == 0) {
        width_ = SvgAttributeParser::ParseLength(value);
        return true;
    }
    if (strcmp(name, "height") == 0) {
        height_ = SvgAttributeParser::ParseLength(value);
        return true;
    }
    if (strcmp(name, "xlink:href") == 0 || strcmp(name, "href") == 0) {
        delete[] ref_;
        ref_ = CopyStringWithLimit(value, SVG_MAX_PATH_LEN);
        if (doc_ != nullptr && !expanded_) {
            ResolveAndExpand();
        }
        return true;
    }
    return SvgContainerNode::SetAttribute(name, value);
}

void SvgUseNode::OnDocumentAttached(SvgDocument* doc)
{
    SvgContainerNode::OnDocumentAttached(doc);
    if (doc_ != nullptr && !expanded_) {
        ResolveAndExpand();
    }
}

void SvgUseNode::OnDocumentReady()
{
    ResolveAndExpand();
}

void SvgUseNode::ResolveAndExpand()
{
    if (ref_ == nullptr || doc_ == nullptr || expanded_) {
        return;
    }
    if (g_useDepth >= MAX_USE_DEPTH) {
        return;
    }
    char idBuf[SvgAttributeParser::SVG_URL_ID_LEN] = {0};
    const char* id = ref_;
    if (SvgAttributeParser::ParseUrlReference(ref_, idBuf, sizeof(idBuf))) {
        id = idBuf;
    }
    SvgElementBase* target = doc_->GetResource(id);
    if (target == nullptr) {
        return;
    }
    // Mark this <use> as expanded before cloning so that any clone of this
    // node (including the self-reference case) copies expanded_=true and does
    // not re-enter ResolveAndExpand. This prevents exponential subtree
    // duplication and stack/oom crashes on cyclic <use> references while
    // still honoring the depth guard for chains of unexpanded <use> nodes.
    expanded_ = true;
    UseDepthGuard guard(g_useDepth);
    SvgElementBase* clone = target->Clone();
    if (clone != nullptr) {
        if (x_ != 0 || y_ != 0) {
            char transformBuf[TRANSFORM_BUF_LEN] = {0};
            if (snprintf_s(transformBuf, TRANSFORM_BUF_LEN, TRANSFORM_BUF_LEN - 1,
                           "translate(%d,%d)", x_, y_) >= 0) {
                clone->SetAttribute("transform", transformBuf);
            }
        }
        AppendChild(clone);
    }
    // RM001-RM003 V2 does not apply <use> width/height to the referenced element;
    // the referenced element retains its own dimensions. Keep the attributes parsed
    // for future SVG-symbol/viewport override support.
}

SvgElementBase* SvgUseNode::Clone() const
{
    SvgUseNode* clone = new SvgUseNode();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->CopyStateFrom(*this);
    clone->ref_ = CopyStringWithLimit(ref_, SVG_MAX_PATH_LEN);
    clone->x_ = x_;
    clone->y_ = y_;
    clone->width_ = width_;
    clone->height_ = height_;
    clone->expanded_ = expanded_;
    CloneChildrenInto(clone);
    return clone;
}

// ---------- SvgSwitchNode ----------

bool SvgSwitchNode::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    return SvgContainerNode::SetAttribute(name, value);
}

void SvgSwitchNode::AppendChild(SvgElementBase* child)
{
    if (child == nullptr) {
        return;
    }
    if (child->GetOwner() != nullptr && child->GetOwner() != this) {
        return;
    }
    child->SetOwner(this);
    ownedChildren_.PushBack(child);
    if (doc_ != nullptr) {
        child->OnDocumentAttached(doc_);
    }
    if (GetChildrenHead() != nullptr) {
        return;
    }
    SvgElementCategory category = child->GetCategory();
    if (category == SVG_CATEGORY_VIEW && EvaluateChildConditions(child)) {
        UIView* view = dynamic_cast<UIView*>(child);
        if (view != nullptr) {
            UIViewGroup::Add(view);
            OnChildAdded(view);
        }
    }
}

SvgElementBase* SvgSwitchNode::Clone() const
{
    SvgSwitchNode* clone = new SvgSwitchNode();
    if (clone == nullptr) {
        return nullptr;
    }
    clone->CopyStateFrom(*this);
    CloneChildrenInto(clone);
    return clone;
}

bool SvgSwitchNode::EvaluateChildConditions(SvgElementBase* child) const
{
    (void)child;
    // Deferred: systemLanguage/requiredFeatures evaluation is not implemented in RM001-RM003;
    // the first child is accepted as matching. Full evaluation requires parser-level attribute
    // access and is deferred to a future release.
    return true;
}

} // namespace OHOS
