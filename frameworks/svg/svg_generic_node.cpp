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

#include "svg/svg_generic_node.h"
#include <cstring>

namespace OHOS {

namespace {

bool CopyStringToBuffer(char* dst, uint16_t dstLen, const char* src)
{
    if (src == nullptr || dst == nullptr || dstLen == 0) {
        return false;
    }
    uint32_t srcLen = static_cast<uint32_t>(strlen(src)) + 1;
    uint32_t copyLen = (srcLen < dstLen) ? srcLen : dstLen;
    if (memcpy_s(dst, dstLen, src, copyLen) != EOK) {
        return false;
    }
    dst[dstLen - 1] = '\0';
    return true;
}

} // namespace

SvgGenericNode::SvgGenericNode(SvgElementType tag)
    : tag_(tag), attrs_(), children_(), textContent_(nullptr)
{
}

SvgGenericNode::~SvgGenericNode()
{
    // AttrEntry stores name/value in inline fixed-size buffers inside the list node,
    // so PopFront is sufficient; there is no separate heap allocation to release.
    while (!attrs_.IsEmpty()) {
        attrs_.PopFront();
    }
    while (!children_.IsEmpty()) {
        SvgElementBase* child = children_.Front();
        children_.PopFront();
        UiDelete(child);
    }
    delete[] textContent_;
}

bool SvgGenericNode::SetAttribute(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    if (strcmp(name, "id") == 0) {
        SetIdAndRegister(value);
        return true;
    }
    if (FindAndUpdateAttr(name, value)) {
        return true;
    }
    AppendAttr(name, value);
    return true;
}

void SvgGenericNode::AppendChild(SvgElementBase* child)
{
    if (child == nullptr) {
        return;
    }
    if (child->GetOwner() != nullptr && child->GetOwner() != this) {
        return;
    }
    child->SetOwner(this);
    children_.PushBack(child);
    if (doc_ != nullptr) {
        child->OnDocumentAttached(doc_);
    }
}

void SvgGenericNode::SetTextContent(const char* text)
{
    delete[] textContent_;
    textContent_ = nullptr;
    if (text == nullptr) {
        return;
    }
    uint32_t len = static_cast<uint32_t>(strlen(text)) + 1;
    if (len > SVG_MAX_TEXT_LEN) {
        return;
    }
    textContent_ = new char[len];
    if (textContent_ != nullptr && memcpy_s(textContent_, len, text, len) != EOK) {
        delete[] textContent_;
        textContent_ = nullptr;
    }
}

const char* SvgGenericNode::GetAttribute(const char* name) const
{
    if (name == nullptr) {
        return nullptr;
    }
    ListNode<AttrEntry>* node = attrs_.Begin();
    for (; node != attrs_.End(); node = node->next_) {
        if (strcmp(node->data_.name, name) == 0) {
            return node->data_.value;
        }
    }
    return nullptr;
}

void SvgGenericNode::OnDocumentAttached(SvgDocument* doc)
{
    SvgElementBase::OnDocumentAttached(doc);
    ListNode<SvgElementBase*>* node = children_.Begin();
    for (; node != children_.End(); node = node->next_) {
        if (node->data_ != nullptr) {
            node->data_->OnDocumentAttached(doc);
        }
    }
}

bool SvgGenericNode::FindAndUpdateAttr(const char* name, const char* value)
{
    ListNode<AttrEntry>* node = attrs_.Begin();
    for (; node != attrs_.End(); node = node->next_) {
        if (strcmp(node->data_.name, name) == 0) {
            return CopyStringToBuffer(node->data_.value, SVG_MAX_ATTR_LEN, value);
        }
    }
    return false;
}

void SvgGenericNode::AppendAttr(const char* name, const char* value)
{
    AttrEntry entry;
    if (!CopyStringToBuffer(entry.name, SVG_MAX_ATTR_LEN, name)) {
        return;
    }
    if (!CopyStringToBuffer(entry.value, SVG_MAX_ATTR_LEN, value)) {
        return;
    }
    attrs_.PushBack(entry);
}

} // namespace OHOS
