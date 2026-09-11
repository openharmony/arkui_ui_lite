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

#ifndef GRAPHIC_LITE_SVG_DOCUMENT_H
#define GRAPHIC_LITE_SVG_DOCUMENT_H

#include "gfx_utils/heap_base.h"
#include "gfx_utils/list.h"
#include "gfx_utils/rect.h"
#include "gfx_utils/trans_affine.h"
#include "svg_element_base.h"

namespace OHOS {

class Paint;
class UIView;
class SvgRootNode;
class SvgPaintServerResource;

class SvgAnimator;

class SvgDocument : public HeapBase {
public:
    SvgDocument();
    ~SvgDocument();
    SvgDocument(const SvgDocument&) = delete;
    SvgDocument& operator=(const SvgDocument&) = delete;

    void SetRoot(SvgElementBase* root); // SvgDocument takes ownership; deletes old root if any

    SvgElementBase* GetRoot() const;

    SvgRootNode* GetRootView() const;

    // Transfers ownership of the root out of this document. The document forgets the
    // root and will not delete it; the caller becomes responsible for freeing it.
    // Used when a subtree built into a temporary document is re-parented into another
    // tree, which would otherwise double free the root.
    SvgElementBase* ReleaseRoot();

    void RegisterResource(const char* id, SvgElementBase* res);

    void UnregisterResource(const char* id, SvgElementBase* res);

    SvgElementBase* GetResource(const char* id) const;

    void RegisterAnimation(SvgElementBase* anim);

    const List<SvgElementBase*>& GetAnimations() const { return animations_; }

    bool ResolvePaintServer(const char* id,
                            Paint& paint,
                            const Rect& localBounds,
                            const TransAffine* localToRecordSpace,
                            bool isFill) const;

    void SetHostView(UIView* host);

    UIView* GetHostView() const;

    void SetAnimator(SvgAnimator* animator) { animator_ = animator; }

    SvgAnimator* GetAnimator() const { return animator_; }

    void ClearIds();

private:
    struct IdEntry : public HeapBase {
        char* id = nullptr;
        SvgElementBase* node = nullptr;

        void Release()
        {
            delete[] id;
            id = nullptr;
        }
    };

    SvgElementBase* root_ = nullptr;
    List<SvgElementBase*> animations_;
    List<IdEntry> ids_;
    UIView* hostView_ = nullptr;
    SvgAnimator* animator_ = nullptr;
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_DOCUMENT_H
