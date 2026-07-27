/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include "font/ui_font_manager.h"

namespace OHOS {
void UIFontManager::SetDefaultFont(BaseFont *font)
{
    defaultFont_ = font;

    return;
}

BaseFont *UIFontManager::FindAppFont(uint16_t viewId)
{
    ListNode<AppFontInfo> *node = fontList_.Begin();
    for (uint16_t i = 0; i < fontList_.Size(); i++) {
        if (viewId == node->data_.viewId) {
            return node->data_.font;
        }
        node = node->next_;
    }

    return nullptr;
}

int8_t UIFontManager::SetAppFont(const char *path, uint16_t viewId)
{
    if (viewId == 0) {
        GRAPHIC_LOGE("UIFontManager::SetAppFont err viewId:%d", viewId);
        return INVALID_RET_VALUE;
    }
    if (FindAppFont(viewId) != nullptr) {
        return RET_VALUE_OK;
    }
    if (createFont_ == nullptr) {
        GRAPHIC_LOGE("UIFontManager::SetAppFont createFont_ nullptr");
        return INVALID_RET_VALUE;
    }
    BaseFont *font = createFont_();
    if (font == nullptr) {
        GRAPHIC_LOGE("UIFontManager::SetAppFont new font fail viewid:%d", viewId);
        return INVALID_RET_VALUE;
    }
    font->SetAppFontPath(path, viewId);
    AppFontInfo fontInfo = { font, viewId };
    fontList_.PushBack(fontInfo);

    return RET_VALUE_OK;
}

int8_t UIFontManager::DeleteAppFont(uint16_t viewId)
{
    ListNode<AppFontInfo> *node = fontList_.Begin();
    for (uint16_t i = 0; i < fontList_.Size(); i++) {
        if (viewId == node->data_.viewId) {
            if (node->data_.font != nullptr) {
                delete node->data_.font;
                node->data_.font = nullptr;
            }
            fontList_.Remove(node);
            return RET_VALUE_OK;
        }
        node = node->next_;
    }

    return RET_VALUE_OK;
}

BaseFont *UIFontManager::GetFont(uint16_t viewId)
{
    if ((viewId == 0) || fontList_.IsEmpty()) {
        return defaultFont_;
    }

    BaseFont *appFont = FindAppFont(viewId);
    if (appFont != nullptr) {
        return appFont;
    }

    return defaultFont_;
}

int8_t UIFontManager::SetAppLangId(uint8_t langId)
{
    if (fontList_.IsEmpty()) {
        return RET_VALUE_OK;
    }

    ListNode<AppFontInfo> *node = fontList_.Begin();
    for (uint16_t i = 0; i < fontList_.Size(); i++) {
        if (node->data_.font == nullptr || node->data_.font->SetAppLangId(langId) == INVALID_RET_VALUE) {
            return INVALID_RET_VALUE;
        }
        node = node->next_;
    }

    return RET_VALUE_OK;
}

void UIFontManager::SetCreateFontCallback(BaseFont *(cb)(void))
{
    createFont_ = cb;

    return;
}
} // namespace OHOS