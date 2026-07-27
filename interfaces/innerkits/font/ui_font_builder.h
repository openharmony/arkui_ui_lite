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
#ifndef GRAPHIC_LITE_UI_FONT_BUILDER_H
#define GRAPHIC_LITE_UI_FONT_BUILDER_H

#include "font/ui_font_header.h"
#include "gfx_utils/heap_base.h"
#if (defined ENABLE_SPLIT_FONT)
#include "gfx_utils/list.h"
#endif

namespace OHOS {
class UIFontBuilder : public HeapBase {
public:
    typedef struct {
        uint16_t viewId;
        uint32_t totalTextId;
    } AppTotalTextId;

    static UIFontBuilder* GetInstance();

    void SetTextLangFontsTable(const UITextLanguageFontParam* uiTextLangFontsTable, uint16_t totalFontId);

    void SetLangTextDefaultParamTable(const LangTextParam* langTextDefaultParamTable, uint8_t totalLangId);

    void SetMaxTextId(uint32_t totalTextId);

    UITextLanguageFontParam* GetTextLangFontsTable(uint16_t langFontId);

    uint8_t GetTotalLangId() const;

    uint16_t GetTotalFontId() const;

    uint16_t GetBitmapFontIdMax() const;

    uint32_t GetTotalTextId(uint16_t viewId = 0) const;

    LangTextParam* GetLangTextDefaultParamTable();

    UITextLanguageFontParam* uiTextLangFontsTable_;
    LangTextParam* langTextDefaultParamTable_;

#if (defined ENABLE_SPLIT_FONT)
    void SetAppMaxTextId(uint32_t totalTextId);
    void DeleteAppMaxTextId(uint32_t totalTextId);
    List<AppTotalTextId> appTotalTextId_;
#endif
private:
    UIFontBuilder();
    ~UIFontBuilder() {}
    uint8_t totalLangId_;
    uint16_t totalFontId_;
    uint32_t totalTextId_;
};
}
#endif // GRAPHIC_LITE_UI_FONT_BUILDER_H
