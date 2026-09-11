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

#include "svg/svg_engine.h"
#include "svg/svg_document.h"
#include "svg/svg_element_factory.h"
#include "svg/svg_container_nodes.h"
#include "svg/svg_shape_nodes.h"
#include "svg/svg_text_nodes.h"
#include "svg/svg_image_node.h"
#include "svg/svg_paint_servers.h"
#include "svg/svg_generic_node.h"
#include "svg/svg_animator.h"
#include "components/root_view.h"
#include "gfx_utils/graphic_log.h"

namespace OHOS {

namespace {

void DetachSvgSubtree(UIViewGroup* parent, UIView* child)
{
    if (parent == nullptr || child == nullptr) {
        return;
    }
    UIViewGroup* childGroup = dynamic_cast<UIViewGroup*>(child);
    if (childGroup != nullptr) {
        UIView* grand = childGroup->GetChildrenHead();
        while (grand != nullptr) {
            UIView* next = grand->GetNextSibling();
            DetachSvgSubtree(childGroup, grand);
            grand = next;
        }
    }
    parent->Remove(child);
#if defined(LOCAL_RENDER) && LOCAL_RENDER
    // UIViewGroup::Remove adds a new dirty-map entry keyed by the removed view.
    // Since this view is being destroyed, delete that entry to avoid a dangling
    // pointer in RootView's invalid map on the next render pass.
    RootView::GetInstance()->RemoveViewFromInvalidMap(child);
#endif
}

void NotifyDocumentReady(SvgElementBase* element)
{
    if (element == nullptr) {
        return;
    }
    element->OnDocumentReady();
    UIViewGroup* group = dynamic_cast<UIViewGroup*>(element);
    if (group == nullptr) {
        return;
    }
    UIView* child = group->GetChildrenHead();
    while (child != nullptr) {
        SvgElementBase* childBase = dynamic_cast<SvgElementBase*>(child);
        if (childBase != nullptr) {
            NotifyDocumentReady(childBase);
        }
        child = child->GetNextSibling();
    }
}

} // namespace

SvgDocumentHandle SvgEngine::CreateDocument()
{
    return static_cast<SvgDocumentHandle>(new SvgDocument());
}

void SvgEngine::DestroyDocument(SvgDocumentHandle doc)
{
    SvgDocument* document = static_cast<SvgDocument*>(doc);
    if (document == nullptr) {
        return;
    }
    // Running animators hold raw pointers to the element tree and to the host view.
    // They must be torn down while both are still alive, otherwise the stop callback
    // dereferences freed memory.
    SvgAnimator* animator = document->GetAnimator();
    if (animator != nullptr) {
        animator->Stop();
        document->SetAnimator(nullptr);
        UiDelete(animator);
    }
    SvgRootNode* root = document->GetRootView();
    UIViewGroup* host = dynamic_cast<UIViewGroup*>(document->GetHostView());
    if (root != nullptr) {
        // Detach the whole SVG view tree before freeing it.  UIViewGroup::Remove
        // clears each view from RootView's invalid map; without this the children
        // of the root would remain in the map as dangling pointers after deletion.
        UIView* child = root->GetChildrenHead();
        while (child != nullptr) {
            UIView* next = child->GetNextSibling();
            DetachSvgSubtree(root, child);
            child = next;
        }
    }
    if (host != nullptr) {
        // Remove the SVG root from its host and clear both from the invalid map.
        // The framework detached hostView_ from its visual parent before
        // ReleaseNativeViews runs, which added a dirty-map entry keyed by hostView_;
        // clean it now to avoid a dangling key on the next render pass.
        if (root != nullptr) {
            host->Remove(root);
#if defined(LOCAL_RENDER) && LOCAL_RENDER
            RootView::GetInstance()->RemoveViewFromInvalidMap(root);
#endif
        }
#if defined(LOCAL_RENDER) && LOCAL_RENDER
        RootView::GetInstance()->RemoveViewFromInvalidMap(host);
#endif
    }
    document->SetRoot(nullptr);
    document->SetHostView(nullptr);
    UiDelete(document);
}

SvgElementHandle SvgEngine::CreateElement(SvgDocumentHandle doc, SvgElementType type)
{
    SvgDocument* document = static_cast<SvgDocument*>(doc);
    SvgElementBase* element = CreateSvgElementByType(type);
    if (element != nullptr && document != nullptr) {
        element->OnDocumentAttached(document);
    }
    return static_cast<SvgElementHandle>(element);
}

void SvgEngine::SetAttribute(SvgElementHandle elem, const char* name, const char* value)
{
    SvgElementBase* element = static_cast<SvgElementBase*>(elem);
    if (element == nullptr || name == nullptr || value == nullptr) {
        return;
    }
    element->SetAttribute(name, value);
}

void SvgEngine::AppendChild(SvgElementHandle parent, SvgElementHandle child)
{
    SvgElementBase* parentElem = static_cast<SvgElementBase*>(parent);
    SvgElementBase* childElem = static_cast<SvgElementBase*>(child);
    if (parentElem == nullptr || childElem == nullptr) {
        return;
    }
    // BuildViewTree may be invoked more than once for the same component subtree
    // (e.g. when an if/for descriptor changes and HandleChildrenChange reattaches
    // children). Make AppendChild idempotent so already-attached children are not
    // duplicated; otherwise gradient resources end up with repeated stops and
    // their destructor performs a double-free.
    if (childElem->GetOwner() == parentElem) {
        return;
    }
    parentElem->AppendChild(childElem);
}

void SvgEngine::SetRoot(SvgDocumentHandle doc, SvgElementHandle root)
{
    SvgDocument* document = static_cast<SvgDocument*>(doc);
    SvgElementBase* rootElem = static_cast<SvgElementBase*>(root);
    if (document == nullptr || rootElem == nullptr) {
        return;
    }
    document->SetRoot(rootElem);
}

UIView* SvgEngine::AttachToView(SvgDocumentHandle doc, UIView* parentView)
{
    SvgDocument* document = static_cast<SvgDocument*>(doc);
    if (document == nullptr || parentView == nullptr) {
        return nullptr;
    }
    SvgRootNode* root = document->GetRootView();
    if (root == nullptr) {
        return nullptr;
    }
    UIViewGroup* parent = dynamic_cast<UIViewGroup*>(parentView);
    if (parent == nullptr) {
        return nullptr;
    }
    parent->Add(root);
    document->SetHostView(parentView);
    root->AttachDocument(document);
    Rect contentRect = parentView->GetContentRect();
    root->SyncViewport(contentRect.GetWidth(), contentRect.GetHeight());
    NotifyDocumentReady(root);
    SvgAnimator initialAnimator;
    initialAnimator.ApplyInitialStates(*document, parentView);
    return root;
}

void SvgEngine::UpdateRootViewport(SvgDocumentHandle doc, int16_t width, int16_t height)
{
    SvgDocument* document = static_cast<SvgDocument*>(doc);
    if (document == nullptr) {
        return;
    }
    SvgRootNode* root = document->GetRootView();
    if (root == nullptr) {
        return;
    }
    root->SyncViewport(width, height);
}

void SvgEngine::Render(SvgDocumentHandle doc)
{
    SvgDocument* document = static_cast<SvgDocument*>(doc);
    if (document == nullptr) {
        return;
    }
    SvgRootNode* root = document->GetRootView();
    if (root != nullptr) {
        root->Invalidate();
    }
}

void SvgEngine::Invalidate(SvgDocumentHandle doc)
{
    Render(doc);
}

void SvgEngine::StartAnimation(SvgDocumentHandle doc)
{
    SvgDocument* document = static_cast<SvgDocument*>(doc);
    if (document == nullptr) {
        return;
    }
    SvgAnimator* animator = document->GetAnimator();
    if (animator == nullptr) {
        animator = new SvgAnimator();
        if (animator == nullptr) {
            GRAPHIC_LOGE("SvgEngine::StartAnimation failed to allocate SvgAnimator");
            return;
        }
        document->SetAnimator(animator);
    }
    animator->Start(*document);
}

void SvgEngine::StopAnimation(SvgDocumentHandle doc)
{
    SvgDocument* document = static_cast<SvgDocument*>(doc);
    if (document == nullptr) {
        return;
    }
    SvgAnimator* animator = document->GetAnimator();
    if (animator != nullptr) {
        animator->Stop();
    }
}

void SvgEngine::PauseAnimations(SvgDocumentHandle doc)
{
    SvgDocument* document = static_cast<SvgDocument*>(doc);
    if (document == nullptr) {
        return;
    }
    SvgAnimator* animator = document->GetAnimator();
    if (animator != nullptr) {
        animator->PauseAnimations();
    }
}

void SvgEngine::UnpauseAnimations(SvgDocumentHandle doc)
{
    SvgDocument* document = static_cast<SvgDocument*>(doc);
    if (document == nullptr) {
        return;
    }
    SvgAnimator* animator = document->GetAnimator();
    if (animator != nullptr) {
        animator->UnpauseAnimations();
    }
}

} // namespace OHOS
