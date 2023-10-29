/*
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "graphic_config.h"
#include "ui_test_spannable_string.h"

#include "common/screen.h"
#if ENABLE_VECTOR_FONT
#else
#include "common/ui_text_language.h"
#endif
#include "components/ui_label.h"
#include "font/ui_font.h"
#if ENABLE_MULTI_FONT
#include "font/ui_multi_font_manager.h"
#endif

namespace OHOS {
namespace {
const int16_t GAP = 5;
const int16_t TITLE_HEIGHT = 20;
const uint16_t LABEL_WIDTH = 400;
const uint16_t LABEL_HEIGHT = 50;
const uint16_t FONT_SIZE = 30;
const uint16_t FONT_EMOJI_SIZE = 100;
const char *SOURCE_HAN_SANS_SC_REGULAR = "SourceHanSansSC-Regular.otf";
const char *ROBOTO_CONDENSED_REGULAR = "RobotoCondensed-Regular.ttf";
const char *DEFAULT_EMOJI_FILENAME = "NotoColorEmojiCompat.ttf";
const char *Amiri_Regular = "../../../../../../../third_party/harfbuzz/perf/fonts/Amiri-Regular.ttf";
const char *Roboto_Regular = "../../../../../../../third_party/harfbuzz/perf/fonts/Roboto-Regular.ttf";
const char *Source_HWSC = "../../../../../../../foundation/window/window_window_manager_lite/test/"
    "SourceHanSansHWSC-VF.ttf";
const char *Source_SC = "../../../../../../../foundation/window/window_window_manager_lite/test/"
    "SourceHanSansSC-VF.ttf";

// foundation\window\window_window_manager_lite\test\sample_ui.cpp

#if ENABLE_MULTI_FONT
const int16_t F_HWEMOJI_REGULAR_30_32 = 99;
#endif
} // namespace

void UITestSpannableString::SetUp()
{
    if (container_ == nullptr) {
        container_ = new UIScrollView();
        container_->SetThrowDrag(true);
        container_->SetHorizontalScrollState(false);
        container_->Resize(Screen::GetInstance().GetWidth(), Screen::GetInstance().GetHeight() - BACK_BUTTON_HEIGHT);
        positionX_ = 50; // 50: init position x
        positionY_ = 5;  // 5: init position y
    }
}

void UITestSpannableString::TearDown()
{
    DeleteChildren(container_);
    container_ = nullptr;
}

void UITestSpannableString::InnerTestTitle(const char *title)
{
    UILabel *titleLabel = new UILabel();
    titleLabel->SetPosition(TEXT_DISTANCE_TO_LEFT_SIDE, positionY_, Screen::GetInstance().GetWidth(),
        TITLE_LABEL_DEFAULT_HEIGHT);
#if ENABLE_VECTOR_FONT
    titleLabel->SetFont(DEFAULT_VECTOR_FONT_FILENAME, DEFAULT_VECTOR_FONT_SIZE);
#else
    titleLabel->SetFontId(F_SOURCEHANSANSSC_REGULAR_18_4);
#endif
    titleLabel->SetText(title);
    container_->Add(titleLabel);
    positionY_ += TITLE_HEIGHT + GAP;
}

const UIView *UITestSpannableString::GetTestView()
{
    UIKitSpannableStringTestNormal001();
    UIKitSpannableStringTestTextStyle001();
    UIKitSpannableStringTestFontSize001();
    UIKitSpannableStringTestFontSize002();
    UIKitSpannableStringTestFontId001();
    UIKitSpannableStringTestFontId002();
    return container_;
}

void UITestSpannableString::UIKitSpannableStringTestNormal001()
{
    if (container_ == nullptr) {
        return;
    }
    InnerTestTitle(" Display singleline Chinese");
    UILabel *label = new UILabel();
    label->SetPosition(positionX_, positionY_);
    label->Resize(LABEL_WIDTH, LABEL_HEIGHT * 2); // 2 : double
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
    UIFont::GetInstance()->RegisterFontInfo(SOURCE_HAN_SANS_SC_REGULAR);
    label->SetFont(SOURCE_HAN_SANS_SC_REGULAR, FONT_SIZE);
#else
    label->SetFontId(F_SOURCEHANSANSSC_REGULAR_30_4);
#endif
    label->SetText(" 轻量图形子系统");
    container_->Add(label);
    positionY_ += LABEL_HEIGHT * 2 + GAP; // 2 : double
}

void UITestSpannableString::UIKitSpannableStringTestTextStyle001()
{
    if (container_ == nullptr) {
        return;
    }
    InnerTestTitle(" Display textStyle ");
    UILabel *label = new UILabel();
    label->SetPosition(positionX_, positionY_);
    label->Resize(LABEL_WIDTH, LABEL_HEIGHT * 2); // 2 : double
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
    UIFont::GetInstance()->RegisterFontInfo(SOURCE_HAN_SANS_SC_REGULAR);
    label->SetFont(SOURCE_HAN_SANS_SC_REGULAR, FONT_SIZE);
#else
    label->SetFontId(F_SOURCEHANSANSSC_REGULAR_30_4);
#endif
    label->SetText("正常粗体斜体粗斜体");

    SpannableString *ss = new SpannableString();
    TextStyle style = TextStyle::TEXT_STYLE_ITALIC;
    ss->SetTextStyle(style, 4, 6); // 4:left edge; 6:right edge;
    TextStyle style_bold = TextStyle::TEXT_STYLE_BOLD;
    ss->SetTextStyle(style_bold, 2, 4); // 2:left edge; 4:right edge;
    label->SetText(ss);
    delete ss;
    ss = nullptr;
    container_->Add(label);
    positionY_ += LABEL_HEIGHT * 2 + GAP; // 2 : double
}

void UITestSpannableString::UIKitSpannableStringTestFontSize001()
{
    if (container_ == nullptr) {
        return;
    }
    InnerTestTitle(" Display SetFontSize, shall be same with SetAbsoluteSizeSpan");
    UILabel *label = new UILabel();
    label->SetPosition(positionX_, positionY_);
    label->Resize(LABEL_WIDTH, LABEL_HEIGHT * 2); // 2 : double
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
    UIFont::GetInstance()->RegisterFontInfo(SOURCE_HAN_SANS_SC_REGULAR);
    label->SetFont(SOURCE_HAN_SANS_SC_REGULAR, FONT_SIZE);
#else
    label->SetFontId(F_SOURCEHANSANSSC_REGULAR_30_4);
#endif
    label->SetText("轻量图形子系统");
    SpannableString *ss = new SpannableString();
    ss->SetFontSize(38, 1, 3); // 38:fontsize; 1:left edge; 3:right edge;
    ss->SetFontSize(18, 2, 4); // 18:fontsize; 2:left edge; 4:right edge;
    ss->SetFontSize(24, 3, 5); // 24:fontsize; 3:left edge; 5:right edge;
    ss->SetFontSize(14, 10, 12); // 14:fontsize; 10:left edge; 12:right edge;
    label->SetText(ss);
    container_->Add(label);
    positionY_ += LABEL_HEIGHT * 2 + GAP; // 2 : double
    delete ss;
    ss = nullptr;
}

void UITestSpannableString::UIKitSpannableStringTestFontSize002()
{
    if (container_ == nullptr) {
        return;
    }
    InnerTestTitle(" Display SetAbsoluteSizeSpan, shall be same with SetFontSize");
    UILabel *label = new UILabel();
    label->SetPosition(positionX_, positionY_);
    label->Resize(LABEL_WIDTH, LABEL_HEIGHT * 2); // 2 : double
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
    UIFont::GetInstance()->RegisterFontInfo(SOURCE_HAN_SANS_SC_REGULAR);
    label->SetFont(SOURCE_HAN_SANS_SC_REGULAR, FONT_SIZE);
#else
    label->SetFontId(F_SOURCEHANSANSSC_REGULAR_30_4);
#endif
    label->SetText("轻量图形子系统");
    label->SetAbsoluteSizeSpan(1, 2, 38);   // 1 ： start index; 2 : end index; 38 absolute font size
    label->SetAbsoluteSizeSpan(2, 3, 18);   // 2 ： start index; 3 : end index; 18 absolute font size
    label->SetAbsoluteSizeSpan(3, 4, 24);   // 3 ： start index; 4 : end index; 24 absolute font size
    label->SetAbsoluteSizeSpan(10, 11, 14); // 10 ： start index; 11 : end index; 14 font size

    container_->Add(label);
    positionY_ += LABEL_HEIGHT * 2 + GAP; // 2 : double
}

void UITestSpannableString::UIKitSpannableStringTestFontId001()
{
    if (container_ == nullptr) {
        return;
    }
    InnerTestTitle(" Display SetFontId, show two fontId can be found in different labels");
    UILabel *label = new UILabel();
    label->SetPosition(positionX_, positionY_);
    label->Resize(LABEL_WIDTH * 2, LABEL_HEIGHT * 2); // 2 : double
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
    UIFont::GetInstance()->RegisterFontInfo(SOURCE_HAN_SANS_SC_REGULAR);
    label->SetFont(SOURCE_HAN_SANS_SC_REGULAR, FONT_SIZE);
#else
    label->SetFontId(F_SOURCEHANSANSSC_REGULAR_30_4);
#endif
    UIFont::GetInstance()->RegisterFontInfo(Amiri_Regular);
    UILabel *label_2 = new UILabel();
    label_2->SetPosition(positionX_, positionY_);
    label_2->Resize(LABEL_WIDTH, LABEL_HEIGHT * 2); // 2: double
    label_2->SetFont(Amiri_Regular, FONT_SIZE);
    label_2->SetText("IJK4 IJK8 IJK12 注意不同字体");
    container_->Add(label_2);
    positionY_ += LABEL_HEIGHT * 1 + GAP;

    UIFont::GetInstance()->RegisterFontInfo(Roboto_Regular);
    UILabel *label_3 = new UILabel();
    label_3->SetPosition(positionX_, positionY_);
    label_3->Resize(LABEL_WIDTH, LABEL_HEIGHT * 2); // 2: double
    label_3->SetFont(Roboto_Regular, FONT_SIZE);
    label_3->SetText("IJK4 IJK8 IJK12 注意不同字体");
    container_->Add(label_3);
    positionY_ += LABEL_HEIGHT * 2 + GAP; // 2: double
}

void UITestSpannableString::UIKitSpannableStringTestFontId002()
{
    if (container_ == nullptr) {
        return;
    }
    InnerTestTitle(" Display SetFontId, two fontIds in one label");
    UILabel *label = new UILabel();
    label->SetPosition(positionX_, positionY_);
    label->Resize(LABEL_WIDTH, LABEL_HEIGHT * 2); // 2 : double
#if defined(ENABLE_VECTOR_FONT) && ENABLE_VECTOR_FONT
    UIFont::GetInstance()->RegisterFontInfo(SOURCE_HAN_SANS_SC_REGULAR);
    label->SetFont(SOURCE_HAN_SANS_SC_REGULAR, FONT_SIZE);
#else
    label->SetFontId(F_SOURCEHANSANSSC_REGULAR_30_4);
#endif
    label->SetText("字体一 字体二 注意不同字体");
    SpannableString *ss = new SpannableString();
    UIFont::GetInstance()->RegisterFontInfo(Source_HWSC);
    UIFont::GetInstance()->RegisterFontInfo(Source_SC);
    const uint16_t id_hwsc = UIFont::GetInstance()->GetFontId(Source_HWSC);
    const uint16_t id_sc = UIFont::GetInstance()->GetFontId(Source_SC);
    uint16_t findPath[] = {id_hwsc};
    UIMultiFontManager::GetInstance()->SetSearchFontList(id_sc, findPath, sizeof(findPath));
    ss->SetFontId(id_hwsc, 0, 4); // 0:left edge; 4: right edge;
    ss->SetFontId(id_sc, 4, 8); // 4:left edge; 8: right edge;
    label->SetText(ss);
    container_->Add(label);
    positionY_ += LABEL_HEIGHT * 2 + GAP; // 2: double
    delete ss;
    ss = nullptr;
}
} // namespace OHOS
