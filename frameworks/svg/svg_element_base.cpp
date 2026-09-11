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

#include "svg/svg_element_base.h"
#include "svg/svg_document.h"

namespace OHOS {

SvgElementBase::~SvgElementBase()
{
    if (doc_ != nullptr && id_ != nullptr) {
        doc_->UnregisterResource(id_, this);
    }
    delete[] id_;
}

void SvgElementBase::OnDocumentAttached(SvgDocument* doc)
{
    SetDocument(doc);
    if (doc != nullptr && id_ != nullptr) {
        doc->RegisterResource(id_, this);
    }
}

void SvgElementBase::SetIdAndRegister(const char* value)
{
    if (doc_ != nullptr && id_ != nullptr) {
        doc_->UnregisterResource(id_, this);
    }
    SetId(value);
    if (doc_ != nullptr && value != nullptr) {
        doc_->RegisterResource(value, this);
    }
}

} // namespace OHOS
