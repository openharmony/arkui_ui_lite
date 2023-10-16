/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "common/spannable_string.h"
#include "font/ui_font.h"
#include "gfx_utils/graphic_log.h"
#include "securec.h"
namespace OHOS {
namespace {
constexpr uint16_t DEFAULT_IS_SPANNABLE_LEN = 10;
constexpr uint16_t DEFAULT_EXPAND_EDGE = 1024;
constexpr uint16_t DEFAULT_EXPAND_TIMES = 2;
constexpr uint16_t DEFAULT_EXPAND_OFFSET = 1;
} // namespace

SpannableString::SpannableString() : isSpannableLen_(0), isSpannable_(nullptr) {}

SpannableString::~SpannableString()
{
    Reset();
}

void SpannableString::SetTextStyle(TextStyle inputTextStyle, uint16_t startIndex, uint16_t endIndex)
{
    StyleSpan* style = new StyleSpan(inputTextStyle, startIndex, endIndex);
    styleList_.PushBack(style);
    SetSpannable(true, startIndex, endIndex);
}
bool SpannableString::GetTextStyle(uint16_t index, TextStyle& textStyle)
{
    bool hasFind = false;
    ListNode<StyleSpan*>* tempSpan = styleList_.Begin();
    for (; tempSpan != styleList_.End(); tempSpan = tempSpan->next_) {
        uint16_t tempStart = tempSpan->data_->start_;
        uint16_t tempEnd = tempSpan->data_->end_;
        if ((tempStart <= index) && (index < tempEnd)) {
            textStyle = tempSpan->data_->textStyle_;
            hasFind = true;
            break;
        }
    }
    return hasFind;
}

void SpannableString::Reset()
{
    if (isSpannable_ != nullptr) {
        UIFree(isSpannable_);
    }
    isSpannable_ = nullptr;
    isSpannableLen_ = 0;
    if (styleList_.Size() > 0) {
        for (auto iter = styleList_.Begin(); iter != styleList_.End(); iter = iter->next_) {
            delete iter->data_;
            iter->data_ = nullptr;
        }
        styleList_.Clear();
    }
    if (sizeList_.Size() > 0) {
        sizeList_.Clear();
    }
    if (fontIdList_.Size() > 0) {
        fontIdList_.Clear();
    }
    if (heightList_.Size() > 0) {
        heightList_.Clear();
    }
}

void SpannableString::SetSpannableString(const SpannableString* input)
{
    Reset();
    SetSpannable(true, 0, DEFAULT_IS_SPANNABLE_LEN);

    ListNode<FontSizeSpan>* node = input->sizeList_.Begin();
    while (node != input->sizeList_.End()) {
        SetFontSize(node->data_.fontSize, node->data_.start, node->data_.end);
        node = node->next_;
    }
    ListNode<FontIdSpan>* node_id = input->fontIdList_.Begin();
    while (node_id != input->fontIdList_.End()) {
        SetFontId(node_id->data_.fontId, node_id->data_.start, node_id->data_.end);
        node_id = node_id->next_;
    }
    ListNode<LetterHeightSpan>* node_height = input->heightList_.Begin();
    while (node_height != input->heightList_.End()) {
        SetFontHeight(node_height->data_.height, node_height->data_.start, node_height->data_.end);
        node_height = node_height->next_;
    }
    ListNode<StyleSpan*>* node_span = input->styleList_.Begin();
    while (node_span != input->styleList_.End()) {
        SetTextStyle(node_span->data_->textStyle_, node_span->data_->start_, node_span->data_->end_);
        node_span = node_span->next_;
    }
    ListNode<BackgroundColorSpan>* node_backColor = input->backgroundColorList_.Begin();
    while (node_backColor != input->backgroundColorList_.End()) {
        SetBackgroundColor(node_backColor->data_.backgroundColor,
                           node_backColor->data_.start,
                           node_backColor->data_.end);
        node_backColor = node_backColor->next_;
    }
    ListNode<ForegroundColorSpan>* node_foreColor = input->foregroundColorList_.Begin();
    while (node_foreColor != input->foregroundColorList_.End()) {
        SetForegroundColor(node_foreColor->data_.fontColor, node_foreColor->data_.start, node_foreColor->data_.end);
        node_foreColor = node_foreColor->next_;
    }
    ListNode<LineBackgroundColorSpan>* node_lineBackColor = input->lineBackgroundColorList_.Begin();
    while (node_lineBackColor != input->lineBackgroundColorList_.End()) {
        SetLineBackgroundColor(node_lineBackColor->data_.linebackgroundColor,
                               node_lineBackColor->data_.start,
                               node_lineBackColor->data_.end);
        node_lineBackColor = node_lineBackColor->next_;
    }
}

bool SpannableString::ExpandSpannableLen(uint16_t index)
{
    if (isSpannableLen_ < index) {
        uint16_t preLens = isSpannableLen_;
        while (isSpannableLen_ < index && isSpannableLen_ != 0 && isSpannableLen_ < DEFAULT_EXPAND_EDGE) {
            isSpannableLen_ = isSpannableLen_ * DEFAULT_EXPAND_TIMES + DEFAULT_EXPAND_OFFSET;
        }
        bool* tempIsSpannable_ = static_cast<bool*>(UIMalloc(isSpannableLen_ * sizeof(bool)));
        if (tempIsSpannable_ == nullptr) {
            GRAPHIC_LOGE("SpannableString::InitSpannable() isSpannable_ == nullptr");
            return false;
        }
        if (isSpannable_ != nullptr) {
            if (memcpy_s(tempIsSpannable_, isSpannableLen_, isSpannable_, isSpannableLen_) != EOK) {
                UIFree(tempIsSpannable_);
                tempIsSpannable_ = nullptr;
                return false;
            }
            UIFree(isSpannable_);
            isSpannable_ = nullptr;
        }
        for (uint16_t i = preLens; i < isSpannableLen_; i++) {
            tempIsSpannable_[i] = false;
        }
        isSpannable_ = tempIsSpannable_;
    }
    return true;
}

bool SpannableString::SetSpannable(bool value, uint16_t startIndex, uint16_t endIndex)
{
    if (isSpannable_ == nullptr) {
        isSpannableLen_ = DEFAULT_IS_SPANNABLE_LEN;
        isSpannable_ = static_cast<bool*>(UIMalloc(isSpannableLen_ * sizeof(bool)));
        for (uint16_t i = 0; i < isSpannableLen_; i++) {
            isSpannable_[i] = false;
        }
    }
    bool isSuccess = ExpandSpannableLen(endIndex);
    if (isSuccess && (isSpannable_ != nullptr)) {
        for (uint16_t i = startIndex; ((i < endIndex) && (i < isSpannableLen_)); i++) {
            isSpannable_[i] = value;
        }
        isSuccess = true;
    }
    return isSuccess;
}

bool SpannableString::GetSpannable(uint16_t index)
{
    bool result = false;
    if ((isSpannable_ != nullptr) && (index < isSpannableLen_)) {
        result = isSpannable_[index];
    }
    return result;
}

/*
 * this function merge samge value when add node
 */
void SpannableString::SetFontSize(uint8_t inputFontSize, uint16_t startIndex, uint16_t endIndex)
{
    if (sizeList_.IsEmpty()) {
        FontSizeSpan inputSpan;
        inputSpan.start = startIndex;
        inputSpan.end =  endIndex;
        inputSpan.fontSize = inputFontSize;
        sizeList_.PushFront(inputSpan);
        SetSpannable(true, startIndex, endIndex);
        return;
    } else {
        ListNode<FontSizeSpan>* tempSpan = sizeList_.Begin();
        for (; tempSpan != sizeList_.End(); tempSpan = tempSpan->next_) {
            bool needAddNode = true;
            uint16_t tempStart = tempSpan->data_.start;
            uint16_t tempEnd = tempSpan->data_.end;
            uint8_t tempSize = tempSpan->data_.fontSize;
            if (inputFontSize == tempSize) {
                needAddNode = EqualInsert<FontSizeSpan>(startIndex, endIndex, tempStart, tempEnd, &tempSpan, sizeList_);
            } else {
                FontSizeSpan tempLeft;
                tempLeft.start = tempStart;
                tempLeft.end = startIndex;
                tempLeft.fontSize = tempSize;
                FontSizeSpan tempRight;
                tempRight.start = endIndex;
                tempRight.end = tempEnd;
                tempRight.fontSize = tempSize;
                needAddNode = UnequalInsert<FontSizeSpan>(startIndex, endIndex, tempStart, tempEnd, &tempSpan,
                                                          sizeList_, tempLeft, tempRight);
            }
            if (needAddNode) {
                FontSizeSpan inputSpan;
                inputSpan.start = startIndex;
                inputSpan.end = endIndex;
                inputSpan.fontSize = inputFontSize;
                sizeList_.PushBack(inputSpan);
                SetSpannable(true, startIndex, endIndex);
            }
        }
    }
}

bool SpannableString::GetFontSize(uint16_t index, uint8_t& outputSize)
{
    bool hasFind = false;
    ListNode<FontSizeSpan>* tempSpan = sizeList_.Begin();
    for (; tempSpan != sizeList_.End(); tempSpan = tempSpan->next_) {
        uint16_t tempStart = tempSpan->data_.start;
        uint16_t tempEnd = tempSpan->data_.end;
        if ((tempStart <= index) && (index < tempEnd)) {
            outputSize = tempSpan->data_.fontSize;
            hasFind = true;
            break;
        }
    }
    return hasFind;
}

void SpannableString::SetFontId(uint16_t inputFontId, uint16_t startIndex, uint16_t endIndex)
{
    if (fontIdList_.IsEmpty()) {
        FontIdSpan inputSpan;
        inputSpan.start = startIndex;
        inputSpan.end = endIndex;
        inputSpan.fontId =  inputFontId;
        fontIdList_.PushFront(inputSpan);
        SetSpannable(true, startIndex, endIndex);
        return;
    }
    ListNode<FontIdSpan>* tempSpan = fontIdList_.Begin();
    for (; tempSpan != fontIdList_.End(); tempSpan = tempSpan->next_) {
        bool needAddNode = true;
        uint16_t tempStart = tempSpan->data_.start;
        uint16_t tempEnd = tempSpan->data_.end;
        uint16_t tempId = tempSpan->data_.fontId;
        if (inputFontId == tempId) {
            needAddNode = EqualInsert<FontIdSpan>(startIndex, endIndex, tempStart, tempEnd, &tempSpan, fontIdList_);
        } else {
            FontIdSpan tempLeft;
            tempLeft.start = tempStart;
            tempLeft.end = startIndex;
            tempLeft.fontId = tempId;
            FontIdSpan tempRight;
            tempRight.start = endIndex;
            tempRight.end = tempEnd;
            tempRight.fontId = tempId;
            needAddNode = UnequalInsert<FontIdSpan>(startIndex, endIndex, tempStart, tempEnd, &tempSpan, fontIdList_,
                                                    tempLeft, tempRight);
        }
        if (needAddNode) {
            FontIdSpan inputSpan;
            inputSpan.start = startIndex;
            inputSpan.end = endIndex;
            inputSpan.fontId = inputFontId;
            fontIdList_.PushBack(inputSpan);
            SetSpannable(true, startIndex, endIndex);
        }
    }
}

bool SpannableString::GetFontId(uint16_t index, uint16_t& outputFontId)
{
    bool hasFind = false;
    ListNode<FontIdSpan>* tempSpan = fontIdList_.Begin();
    for (; tempSpan != fontIdList_.End(); tempSpan = tempSpan->next_) {
        uint16_t tempStart = tempSpan->data_.start;
        uint16_t tempEnd = tempSpan->data_.end;
        if ((tempStart <= index) && (index < tempEnd)) {
            outputFontId = tempSpan->data_.fontId;
            hasFind = true;
            break;
        }
    }
    return hasFind;
}

void SpannableString::SetFontHeight(int16_t inputHeight, uint16_t startIndex, uint16_t endIndex)
{
    if (heightList_.IsEmpty()) {
        LetterHeightSpan inputSpan;
        inputSpan.start = startIndex;
        inputSpan.end = endIndex;
        inputSpan.height = inputHeight;
        heightList_.PushFront(inputSpan);
        SetSpannable(true, startIndex, endIndex);
        return;
    }
    ListNode<LetterHeightSpan>* tempSpan = heightList_.Begin();
    for (; tempSpan != heightList_.End(); tempSpan = tempSpan->next_) {
        bool needAddNode = true;
        uint16_t tempStart = tempSpan->data_.start;
        uint16_t tempEnd = tempSpan->data_.end;
        int16_t tempHeight = tempSpan->data_.height;
        if (inputHeight == tempHeight) {
            needAddNode =
                EqualInsert<LetterHeightSpan>(startIndex, endIndex, tempStart, tempEnd, &tempSpan, heightList_);
        } else {
            LetterHeightSpan tempLeft;
            tempLeft.start = tempStart;
            tempLeft.end = startIndex;
            tempLeft.height = tempHeight;
            LetterHeightSpan tempRight;
            tempRight.start = endIndex;
            tempRight.end = tempEnd;
            tempRight.height = tempHeight;
            needAddNode = UnequalInsert<LetterHeightSpan>(startIndex, endIndex, tempStart, tempEnd, &tempSpan,
                                                          heightList_, tempLeft, tempRight);
        }
        if (needAddNode) {
            LetterHeightSpan inputSpan;
            inputSpan.start = startIndex;
            inputSpan.end = endIndex;
            inputSpan.height = inputHeight;
            heightList_.PushBack(inputSpan);
            SetSpannable(true, startIndex, endIndex);
        }
    }
}

bool SpannableString::GetFontHeight(uint16_t index,
                                    int16_t& outputHeight,
                                    uint16_t& defaultFontId,
                                    uint8_t defaultFontSize)
{
    bool hasFind = false;
    ListNode<LetterHeightSpan>* tempSpan = heightList_.Begin();
    for (; tempSpan != heightList_.End(); tempSpan = tempSpan->next_) {
        uint16_t tempStart = tempSpan->data_.start;
        uint16_t tempEnd = tempSpan->data_.end;
        if ((tempStart <= index) && (index < tempEnd)) {
            hasFind = true;
            outputHeight = tempSpan->data_.height;
            break;
        }
    }
    if (!hasFind) {
        GetFontId(index, defaultFontId);
        GetFontSize(index, defaultFontSize);
        UIFont* uifont = UIFont::GetInstance();
        outputHeight = uifont->GetHeight(defaultFontId, defaultFontSize);
        SetFontHeight(outputHeight, index, index + 1);
    }
    return hasFind;
}

void SpannableString::SetBackgroundColor(ColorType inputBackgroundColor, uint16_t startIndex, uint16_t endIndex)
{
    if (backgroundColorList_.IsEmpty()) {
        BackgroundColorSpan inputSpan;
        inputSpan.start = startIndex;
        inputSpan.end =  endIndex;
        inputSpan.backgroundColor.full = inputBackgroundColor.full;
        backgroundColorList_.PushFront(inputSpan);
        SetSpannable(true, startIndex, endIndex);
        return;
    } else {
        ListNode<BackgroundColorSpan>* tempSpan = backgroundColorList_.Begin();
        for (; tempSpan != backgroundColorList_.End(); tempSpan = tempSpan->next_) {
            bool needAddNode = true;
            uint16_t tempStart = tempSpan->data_.start;
            uint16_t tempEnd = tempSpan->data_.end;
            ColorType tempSize;
            tempSize.full = tempSpan->data_.backgroundColor.full;
            if (inputBackgroundColor.full == tempSize.full) {
                needAddNode = EqualInsert<BackgroundColorSpan>(
                    startIndex, endIndex, tempStart, tempEnd, &tempSpan,
                    backgroundColorList_);
            } else {
                BackgroundColorSpan tempLeft;
                tempLeft.start = tempStart;
                tempLeft.end = startIndex;
                tempLeft.backgroundColor.full = tempSize.full;
                BackgroundColorSpan tempRight;
                tempRight.start = endIndex;
                tempRight.end = tempEnd;
                tempRight.backgroundColor.full = tempSize.full;
                needAddNode = UnequalInsert<BackgroundColorSpan>(
                    startIndex, endIndex, tempStart, tempEnd, &tempSpan,
                    backgroundColorList_, tempLeft, tempRight);
            }
            if (needAddNode) {
                BackgroundColorSpan inputSpan;
                inputSpan.start = startIndex;
                inputSpan.end = endIndex;
                inputSpan.backgroundColor.full = inputBackgroundColor.full;
                backgroundColorList_.PushBack(inputSpan);
                SetSpannable(true, startIndex, endIndex);
            }
        }
    }
}

bool SpannableString::GetBackgroundColor(uint16_t index, ColorType& outputBackgroundColor)
{
    bool hasFind = false;
    ListNode<BackgroundColorSpan>* tempSpan = backgroundColorList_.Begin();
    for (; tempSpan != backgroundColorList_.End(); tempSpan = tempSpan->next_) {
        uint16_t tempStart = tempSpan->data_.start;
        uint16_t tempEnd = tempSpan->data_.end;
        if ((tempStart <= index) && (index < tempEnd)) {
            outputBackgroundColor.full = tempSpan->data_.backgroundColor.full;
            hasFind = true;
            break;
        }
    }
    return hasFind;
}

void SpannableString::SetForegroundColor(ColorType inputForegroundColor, uint16_t startIndex, uint16_t endIndex)
{
    if (foregroundColorList_.IsEmpty()) {
        ForegroundColorSpan inputSpan;
        inputSpan.start = startIndex;
        inputSpan.end =  endIndex;
        inputSpan.fontColor.full = inputForegroundColor.full;
        foregroundColorList_.PushFront(inputSpan);
        SetSpannable(true, startIndex, endIndex);
        return;
    } else {
        ListNode<ForegroundColorSpan>* tempSpan = foregroundColorList_.Begin();
        for (; tempSpan != foregroundColorList_.End(); tempSpan = tempSpan->next_) {
            bool needAddNode = true;
            uint16_t tempStart = tempSpan->data_.start;
            uint16_t tempEnd = tempSpan->data_.end;
            ColorType tempSize;
            tempSize.full= tempSpan->data_.fontColor.full;
            if (inputForegroundColor.full == tempSize.full) {
                needAddNode = EqualInsert<ForegroundColorSpan>(
                    startIndex, endIndex, tempStart, tempEnd, &tempSpan,
                    foregroundColorList_);
            } else {
                ForegroundColorSpan tempLeft;
                tempLeft.start = tempStart;
                tempLeft.end = startIndex;
                tempLeft.fontColor.full = tempSize.full;
                ForegroundColorSpan tempRight;
                tempRight.start = endIndex;
                tempRight.end = tempEnd;
                tempRight.fontColor.full = tempSize.full;
                needAddNode = UnequalInsert<ForegroundColorSpan>(startIndex, endIndex, tempStart, tempEnd, &tempSpan,
                                                          foregroundColorList_, tempLeft, tempRight);
            }
            if (needAddNode) {
                ForegroundColorSpan inputSpan;
                inputSpan.start = startIndex;
                inputSpan.end = endIndex;
                inputSpan.fontColor.full = inputForegroundColor.full;
                foregroundColorList_.PushBack(inputSpan);
                SetSpannable(true, startIndex, endIndex);
            }
        }
    }
}

bool SpannableString::GetForegroundColor(uint16_t index, ColorType& outputForegroundColor)
{
    bool hasFind = false;
    ListNode<ForegroundColorSpan>* tempSpan = foregroundColorList_.Begin();
    for (; tempSpan != foregroundColorList_.End(); tempSpan = tempSpan->next_) {
        uint16_t tempStart = tempSpan->data_.start;
        uint16_t tempEnd = tempSpan->data_.end;
        if ((tempStart <= index) && (index < tempEnd)) {
            outputForegroundColor.full = tempSpan->data_.fontColor.full;
            hasFind = true;
            break;
        }
    }
    return hasFind;
}

void SpannableString::SetLineBackgroundColor(ColorType inputLineBackgroundColor, uint16_t startIndex, uint16_t endIndex)
{
    if (lineBackgroundColorList_.IsEmpty()) {
        LineBackgroundColorSpan inputSpan;
        inputSpan.start = startIndex;
        inputSpan.end =  endIndex;
        inputSpan.linebackgroundColor.full = inputLineBackgroundColor.full;
        lineBackgroundColorList_.PushFront(inputSpan);
        SetSpannable(true, startIndex, endIndex);
        return;
    } else {
        ListNode<LineBackgroundColorSpan>* tempSpan = lineBackgroundColorList_.Begin();
        for (; tempSpan != lineBackgroundColorList_.End(); tempSpan = tempSpan->next_) {
            bool needAddNode = true;
            uint16_t tempStart = tempSpan->data_.start;
            uint16_t tempEnd = tempSpan->data_.end;
            ColorType tempSize;
            tempSize.full = tempSpan->data_.linebackgroundColor.full;
            if (inputLineBackgroundColor.full == tempSize.full) {
                needAddNode = EqualInsert<LineBackgroundColorSpan>(
                    startIndex, endIndex, tempStart, tempEnd, &tempSpan,
                    lineBackgroundColorList_);
            } else {
                LineBackgroundColorSpan tempLeft;
                tempLeft.start = tempStart;
                tempLeft.end = startIndex;
                tempLeft.linebackgroundColor.full = tempSize.full;
                LineBackgroundColorSpan tempRight;
                tempRight.start = endIndex;
                tempRight.end = tempEnd;
                tempRight.linebackgroundColor.full = tempSize.full;
                needAddNode = UnequalInsert<LineBackgroundColorSpan>(
                    startIndex, endIndex, tempStart, tempEnd, &tempSpan,
                    lineBackgroundColorList_, tempLeft, tempRight);
            }
            if (needAddNode) {
                LineBackgroundColorSpan inputSpan;
                inputSpan.start = startIndex;
                inputSpan.end = endIndex;
                inputSpan.linebackgroundColor.full = inputLineBackgroundColor.full;
                lineBackgroundColorList_.PushBack(inputSpan);
                SetSpannable(true, startIndex, endIndex);
            }
        }
    }
}

bool SpannableString::GetLineBackgroundColor(uint16_t index, ColorType& outputLineBackgroundColor)
{
    bool hasFind = false;
    ListNode<LineBackgroundColorSpan>* tempSpan = lineBackgroundColorList_.Begin();
    for (; tempSpan != lineBackgroundColorList_.End(); tempSpan = tempSpan->next_) {
        uint16_t tempStart = tempSpan->data_.start;
        uint16_t tempEnd = tempSpan->data_.end;
        if ((tempStart <= index) && (index < tempEnd)) {
            outputLineBackgroundColor.full = tempSpan->data_.linebackgroundColor.full;
            hasFind = true;
            break;
        }
    }
    return hasFind;
}
} // namespace OHOS
