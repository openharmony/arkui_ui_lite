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

#include "svg/svg_document.h"
#include "svg/svg_animator.h"
#include "svg/svg_container_nodes.h"
#include "svg/svg_paint_servers.h"
#include "svg/svg_string_util.h"
#include <cstring>

namespace OHOS {

SvgDocument::SvgDocument() : root_(nullptr), hostView_(nullptr) {}

SvgDocument::~SvgDocument()
{
    if (animator_ != nullptr) {
        animator_->Stop();
        UiDelete(animator_);
    }
    ListNode<IdEntry>* idNode = ids_.Begin();
    while (idNode != ids_.End()) {
        idNode->data_.Release();
        idNode = ids_.Next(idNode);
    }
    ids_.Clear();
    while (!animations_.IsEmpty()) {
        animations_.PopFront();
    }
    UiDelete(root_);
}

void SvgDocument::UnregisterResource(const char* id, SvgElementBase* res)
{
    if (id == nullptr || id[0] == '\0' || res == nullptr) {
        return;
    }
    ListNode<IdEntry>* node = ids_.Begin();
    while (node != ids_.End()) {
        if (node->data_.id != nullptr && strcmp(node->data_.id, id) == 0 && node->data_.node == res) {
            node->data_.Release();
            node = ids_.Remove(node);
        } else {
            node = node->next_;
        }
    }
}

void SvgDocument::SetRoot(SvgElementBase* root)
{
    if (root_ == root) {
        return;
    }
    SvgElementBase* oldRoot = root_;
    root_ = root;
    if (oldRoot != nullptr) {
        UiDelete(oldRoot);
    }
    if (root_ == nullptr) {
        return;
    }
    root_->OnDocumentAttached(this);
}

SvgElementBase* SvgDocument::GetRoot() const
{
    return root_;
}

SvgRootNode* SvgDocument::GetRootView() const
{
    return dynamic_cast<SvgRootNode*>(root_);
}

SvgElementBase* SvgDocument::ReleaseRoot()
{
    SvgElementBase* released = root_;
    root_ = nullptr;
    return released;
}

void SvgDocument::RegisterResource(const char* id, SvgElementBase* res)
{
    if (id == nullptr || id[0] == '\0' || res == nullptr) {
        return;
    }
    ListNode<IdEntry>* node = ids_.Begin();
    for (; node != ids_.End(); node = node->next_) {
        if (node->data_.id != nullptr && strcmp(node->data_.id, id) == 0) {
            node->data_.node = res;
            return;
        }
    }
    IdEntry entry;
    entry.id = CopyStringWithLimit(id, SVG_MAX_ATTR_LEN);
    entry.node = res;
    if (entry.id != nullptr) {
        ids_.PushBack(entry);
    }
}

SvgElementBase* SvgDocument::GetResource(const char* id) const
{
    if (id == nullptr || id[0] == '\0') {
        return nullptr;
    }
    ListNode<IdEntry>* node = ids_.Begin();
    for (; node != ids_.End(); node = node->next_) {
        if (node->data_.id != nullptr && strcmp(node->data_.id, id) == 0) {
            return node->data_.node;
        }
    }
    return nullptr;
}

void SvgDocument::RegisterAnimation(SvgElementBase* anim)
{
    if (anim == nullptr) {
        return;
    }
    ListNode<SvgElementBase*>* node = animations_.Begin();
    for (; node != animations_.End(); node = node->next_) {
        if (node->data_ == anim) {
            return;
        }
    }
    animations_.PushBack(anim);
}

bool SvgDocument::ResolvePaintServer(const char* id,
                                     Paint& paint,
                                     const Rect& localBounds,
                                     const TransAffine* localToRecordSpace,
                                     bool isFill) const
{
    if (id == nullptr || id[0] == '\0') {
        return false;
    }
    SvgElementBase* res = GetResource(id);
    if (res == nullptr) {
        return false;
    }
    SvgPaintServerResource* server = dynamic_cast<SvgPaintServerResource*>(res);
    if (server == nullptr) {
        return false;
    }
    server->ApplyToPaint(paint, localBounds, localToRecordSpace, isFill);
    return true;
}

void SvgDocument::SetHostView(UIView* host)
{
    hostView_ = host;
}

UIView* SvgDocument::GetHostView() const
{
    return hostView_;
}

void SvgDocument::ClearIds()
{
    ListNode<IdEntry>* idNode = ids_.Begin();
    while (idNode != ids_.End()) {
        idNode->data_.Release();
        idNode = ids_.Next(idNode);
    }
    ids_.Clear();
}

} // namespace OHOS
