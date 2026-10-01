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

#ifndef GRAPHIC_LITE_SVG_ELEMENT_BASE_H
#define GRAPHIC_LITE_SVG_ELEMENT_BASE_H

#include "gfx_utils/heap_base.h"
#include "gfx_utils/list.h"
#include "gfx_utils/trans_affine.h"
#include <cstdint>
#include <cstring>
#include "securec.h"


namespace OHOS {

template<typename T>
inline void UiDelete(T*& pointer)
{
    if (pointer != nullptr) {
        delete pointer;
        pointer = nullptr;
    }
}

class SvgElementBase;

class SvgDocument;

enum SvgElementCategory : uint8_t {
    SVG_CATEGORY_VIEW = 0,
    SVG_CATEGORY_RESOURCE,
    SVG_CATEGORY_ANIMATION,
    SVG_CATEGORY_GENERIC
};

class SvgElementBase : public HeapBase {
public:
    virtual ~SvgElementBase();

    virtual SvgElementCategory GetCategory() const = 0;

    virtual bool SetAttribute(const char* name, const char* value) = 0;

    // Returns the element's own (non-inherited) transform. The default returns
    // identity so non-graphical targets remain unaffected. Animation callbacks
    // use this to compose an animated transform onto the element's base transform
    // when the animation declares additive="sum"; the default additive="replace"
    // means the animated value replaces the base value.
    virtual const TransAffine& GetTransform() const
    {
        static const TransAffine kIdentity;
        return kIdentity;
    }

    virtual void SetTransform(const TransAffine& transform)
    {
        (void)transform;
    }

    virtual void AppendChild(SvgElementBase* child) = 0;

    virtual void OnDocumentReady() {}

    virtual void OnDocumentAttached(SvgDocument* doc);

    virtual SvgElementBase* Clone() const
    {
        return nullptr;
    }

    // Releases this element. The default implementation uses delete; subclasses
    // that are allocated from a pool may override to return themselves to the pool.
    virtual void Destroy() { delete this; }

    void SetDocument(SvgDocument* doc)
    {
        doc_ = doc;
    }

    SvgDocument* GetDocument() const
    {
        return doc_;
    }

    void SetOwner(SvgElementBase* owner)
    {
        owner_ = owner;
    }

    SvgElementBase* GetOwner() const
    {
        return owner_;
    }

    const char* GetId() const
    {
        return id_;
    }

protected:
    void SetId(const char* value)
    {
        delete[] id_;
        id_ = nullptr;
        if (value == nullptr) {
            return;
        }
        uint32_t len = strlen(value) + 1;
        id_ = new char[len];
        if (id_ != nullptr && memcpy_s(id_, len, value, len) != EOK) {
            delete[] id_;
            id_ = nullptr;
        }
    }

    void SetIdAndRegister(const char* value);

    SvgDocument* doc_ = nullptr;
    SvgElementBase* owner_ = nullptr;
    char* id_ = nullptr;
};

inline void UiDelete(SvgElementBase*& pointer)
{
    if (pointer != nullptr) {
        pointer->Destroy();
        pointer = nullptr;
    }
}

template<typename T>
inline bool SvgListContains(const List<T>& list, const T& value)
{
    for (ListNode<T>* node = list.Begin(); node != list.End(); node = node->next_) {
        if (node->data_ == value) {
            return true;
        }
    }
    return false;
}

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_ELEMENT_BASE_H
