/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#include "font/ui_font_builder.h"
#include "font/ui_font.h"

namespace OHOS {
UIFontBuilder::UIFontBuilder() : uiTextLangFontsTable_(nullptr), langTextDefaultParamTable_(nullptr),
                                 totalLangId_(0), totalFontId_(0), totalTextId_(0) {}

UIFontBuilder* UIFontBuilder::GetInstance()
{
    static UIFontBuilder uiFontBuilder;
    return &uiFontBuilder;
}

void UIFontBuilder::SetTextLangFontsTable(const UITextLanguageFontParam* uiTextLangFontsTable,
                                          uint16_t totalFontId)
{
    if ((uiTextLangFontsTable != nullptr) && (totalFontId > 0)) {
        uiTextLangFontsTable_ = const_cast<UITextLanguageFontParam*>(uiTextLangFontsTable);
        totalFontId_ = totalFontId;
    }
}

void UIFontBuilder::SetLangTextDefaultParamTable(const LangTextParam* langTextDefaultParamTable,
                                                 uint8_t totalLangId)
{
    if ((langTextDefaultParamTable != nullptr) && (totalLangId > 0)) {
        langTextDefaultParamTable_ = const_cast<LangTextParam*>(langTextDefaultParamTable);
        totalLangId_ = totalLangId;
    }
}

void UIFontBuilder::SetMaxTextId(uint32_t totalTextId)
{
    totalTextId_ = totalTextId;
}

UITextLanguageFontParam* UIFontBuilder::GetTextLangFontsTable(uint16_t langFontId)
{
    if ((langFontId >= totalFontId_) || (uiTextLangFontsTable_ == nullptr)) {
        return nullptr;
    }
    return &(uiTextLangFontsTable_[langFontId]);
}

uint8_t UIFontBuilder::GetTotalLangId() const
{
    return totalLangId_;
}

uint16_t UIFontBuilder::GetTotalFontId() const
{
    uint16_t fontIdMax = 0xFF;
    if (!UIFont::GetInstance()->IsVectorFont()) {
        fontIdMax = totalFontId_;
    }
    return fontIdMax;
}

uint16_t UIFontBuilder::GetBitmapFontIdMax() const
{
    return totalFontId_;
}

uint32_t UIFontBuilder::GetTotalTextId(uint16_t viewId) const
{
#if (defined ENABLE_SPLIT_FONT)
    if ((viewId == 0) || appTotalTextId_.IsEmpty()) {
        return totalTextId_;
    }
    ListNode<AppTotalTextId> *node = appTotalTextId_.Begin();
    for (uint16_t i = 0; i < appTotalTextId_.Size(); i++) {
        if (viewId == node->data_.viewId) {
            return node->data_.totalTextId;
        }
        node = node->next_;
    }
#endif

    return totalTextId_;
}

LangTextParam* UIFontBuilder::GetLangTextDefaultParamTable()
{
    if (langTextDefaultParamTable_ == nullptr) {
        return nullptr;
    }
    return langTextDefaultParamTable_;
}

#if (defined ENABLE_SPLIT_FONT)
void UIFontBuilder::SetAppMaxTextId(uint32_t totalTextId)
{
    uint16_t viewId = (totalTextId & 0x7fffffff) >> 16; // 取高16位
    if (viewId == 0) {
        return;
    }
    ListNode<AppTotalTextId> *node = appTotalTextId_.Begin();
    for (uint16_t i = 0; i < appTotalTextId_.Size(); i++) {
        if (viewId == node->data_.viewId) {
            return;
        }
        node = node->next_;
    }
    AppTotalTextId id = { viewId, totalTextId };
    appTotalTextId_.PushBack(id);

    return;
}

void UIFontBuilder::DeleteAppMaxTextId(uint32_t totalTextId)
{
    uint16_t viewId = (totalTextId & 0x7fffffff) >> 16; // 取高16位
    ListNode<AppTotalTextId> *node = appTotalTextId_.Begin();
    for (uint16_t i = 0; i < appTotalTextId_.Size(); i++) {
        if (viewId == node->data_.viewId) {
            appTotalTextId_.Remove(node);
            return;
        }
        node = node->next_;
    }

    return;
}
#endif
} // namespace OHOS
