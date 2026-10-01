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

#ifndef GRAPHIC_LITE_SVG_ENGINE_H
#define GRAPHIC_LITE_SVG_ENGINE_H

#include <cstdint>
#include "svg_element_type.h"

namespace OHOS {

class UIView;

using SvgDocumentHandle = void*;
using SvgElementHandle = void*;

/**
 * @brief SvgEngine is the sole public facade for SVG document creation.
 *
 * Ownership rules:
 *   - SvgDocumentHandle owns the root SvgElementBase (and thus the whole tree).
 *   - DestroyDocument removes the root view from its host, deletes the root element
 *     (whose destructor recursively cleans ownedChildren_), then deletes the document.
 *   - Resources registered in the document's id table are owned by their tree position;
 *     the table only stores pointers, so clearing it does not double-free.
 */
class SvgEngine {
public:
    static SvgDocumentHandle CreateDocument();
    static void DestroyDocument(SvgDocumentHandle doc);
    static SvgElementHandle CreateElement(SvgDocumentHandle doc, SvgElementType type);
    static void SetAttribute(SvgElementHandle elem, const char* name, const char* value);
    static void AppendChild(SvgElementHandle parent, SvgElementHandle child);
    static void SetRoot(SvgDocumentHandle doc, SvgElementHandle root);
    static UIView* AttachToView(SvgDocumentHandle doc, UIView* parentView);
    static void UpdateRootViewport(SvgDocumentHandle doc, int16_t width, int16_t height);
    static void Render(SvgDocumentHandle doc);
    static void Invalidate(SvgDocumentHandle doc);
    static void StartAnimation(SvgDocumentHandle doc);
    static void StopAnimation(SvgDocumentHandle doc);
    // Standard timeline primitives: Pause freezes in place (entries kept), Unpause resumes from frozen point.
    static void PauseAnimations(SvgDocumentHandle doc);
    static void UnpauseAnimations(SvgDocumentHandle doc);
};

} // namespace OHOS

#endif // GRAPHIC_LITE_SVG_ENGINE_H
