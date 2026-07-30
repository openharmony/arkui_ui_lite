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

#ifndef UI_FONT_MANAGER
#define UI_FONT_MANAGER

#include "font/base_font.h"
#include "gfx_utils/list.h"

namespace OHOS {
class UIFontManager : public HeapBase {
public:
    typedef struct {
        BaseFont *font;
        uint16_t viewId;
    } AppFontInfo;
    UIFontManager() {}
    ~UIFontManager() {}

    void SetDefaultFont(BaseFont *font);
    int8_t SetAppFont(const char *path, uint16_t viewId);
    int8_t DeleteAppFont(uint16_t viewId);
    BaseFont *GetFont(uint16_t viewId = 0);
    int8_t SetAppLangId(uint8_t langId);
    void SetCreateFontCallback(BaseFont *(cb)(void));
private:
    BaseFont *FindAppFont(uint16_t viewId);

    List<AppFontInfo> fontList_;
    BaseFont *defaultFont_ = nullptr;
    BaseFont *(*createFont_)(void) = nullptr;
};
} // namespace OHOS
#endif // UI_FONT_MANAGER