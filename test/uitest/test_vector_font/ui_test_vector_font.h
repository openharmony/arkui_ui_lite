/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
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

#ifndef UI_TEST_VECTOR_FONT_H
#define UI_TEST_VECTOR_FONT_H

#include "graphic_config.h"
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
#include "ui_test.h"
#include "components/ui_scroll_view.h"
#include "font/ui_font.h"
#include <string>

namespace OHOS {
class UITestVectorFont : public UITest {
public:
    UITestVectorFont() {}
    ~UITestVectorFont() {}
    void SetUp() override;
    void TearDown() override;
    const UIView* GetTestView() override;
    void FontFontEngineVectorTestCHLang001();
    void FontFontEngineVectorTestCHLang002();
    void FontFontEngineVectorTestJALang001();
    void FontFontEngineVectorTestJALang002();
    void FontFontEngineVectorTestKRLang001();
    void FontFontEngineVectorTestVILang001();
#if ENABLE_MULTI_FONT
    /**
     * @brief Test JA language
     */
    void FontFontEngineMultiLanguageTestJALang001();

    /**
     * @brief Test UnRegister JA language
     */
    void FontFontEngineMultiLanguageTestJALang002();

    /**
     * @brief Test KR language
     */
    void FontFontEngineMultiLanguageTestKRLang001();

    /**
     * @brief Test VI language
     */
    void FontFontEngineMultiLanguageTestVILang001();

    /**
     * @brief Test CS language
     */
    void FontFontEngineMultiLanguageTestCSLang001();

    /**
     * @brief Test EL language
     */
    void FontFontEngineMultiLanguageTestELLang001();

    /**
     * @brief Test RO language
     */
    void FontFontEngineMultiLanguageTestROLang001();
#endif

#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    void TestDrawTextITALYBOLD();
#endif
private:
    UIScrollView* container_ = nullptr;
    void InnerTestTitle(const char* title);
    int16_t xPos = 50; // 50: x initial position
    int16_t yPos = 5; // 5 : y initial position
};
} // namespace OHOS
#endif // ENABLE_VECTOR_FONT
#endif // UI_TEST_VECTOR_FONT_H
