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
#ifndef SPANNABLE_STRING_H
#define SPANNABLE_STRING_H
#include "font/ui_font_header.h"
#include "gfx_utils/color.h"
#include "gfx_utils/list.h"
#include "gfx_utils/graphic_types.h"
namespace OHOS {
struct FontSizeSpan : HeapBase {
    uint16_t start;
    uint16_t end;
    uint8_t fontSize;
};

/**
 * @brief Stores the attribute information about letter height for full text.
 */
struct LetterHeightSpan : HeapBase {
    uint16_t start;
    uint16_t end;
    int16_t height;
};

struct FontIdSpan : HeapBase {
    uint16_t start;
    uint16_t end;
    uint16_t fontId;
};

struct BackgroundColorSpan : public HeapBase {
    int16_t start;
    int16_t end;
    ColorType backgroundColor;
};

struct ForegroundColorSpan : public HeapBase {
    int16_t start;
    int16_t end;
    ColorType fontColor;
};

struct LineBackgroundColorSpan : public HeapBase {
    int16_t start;
    int16_t end;
    ColorType linebackgroundColor;
};

class StyleSpan : public HeapBase {
public:
    StyleSpan() {};
    ~StyleSpan() {};
    TextStyle textStyle_;
    uint16_t start_;
    uint16_t end_;
    StyleSpan(TextStyle textStyle, uint16_t start, uint16_t end)
    {
        textStyle_ = textStyle;
        start_ = start;
        end_ = end;
    };
};
class SpannableString : public HeapBase {
public:
    SpannableString();
    ~SpannableString();

    bool GetSpannable(uint16_t index);
    void SetTextStyle(TextStyle textStyle, uint16_t startIndex, uint16_t endIndex);
    bool GetTextStyle(uint16_t index, TextStyle& textStyle);
    void SetFontSize(uint8_t inputFontSize, uint16_t startIndex, uint16_t endIndex);
    bool GetFontSize(uint16_t index, uint8_t& outputSize);
    void SetFontId(uint16_t inputFontId, uint16_t startIndex, uint16_t endIndex);
    bool GetFontId(uint16_t index, uint16_t& outputFontId);
    bool GetFontHeight(uint16_t index, int16_t& outputHeight, uint16_t& defaultFontId, uint8_t defaultFontSize);

    void SetBackgroundColor(ColorType inputBackgroundColor, uint16_t startIndex, uint16_t endIndex);
    bool GetBackgroundColor(uint16_t index, ColorType& outputBackgroundColor);
    void SetForegroundColor(ColorType inputForegroundColor, uint16_t startIndex, uint16_t endIndex);
    bool GetForegroundColor(uint16_t index, ColorType& outputForegroundColor);
    void SetLineBackgroundColor(ColorType inputLineBackgroundColor, uint16_t startIndex, uint16_t endIndex);
    bool GetLineBackgroundColor(uint16_t index, ColorType& outputLineBackgroundColor);

    void Reset();
    void SetSpannableString(const SpannableString* input);

private:
    uint16_t isSpannableLen_;

    /* Record each letter having full text setting or not. */
    bool* isSpannable_;
    List<StyleSpan*> styleList_;
    List<FontSizeSpan> sizeList_;
    List<FontIdSpan> fontIdList_;
    List<LetterHeightSpan> heightList_;

    List<BackgroundColorSpan> backgroundColorList_;
    List<ForegroundColorSpan> foregroundColorList_;
    List<LineBackgroundColorSpan> lineBackgroundColorList_;

    /* If endIndex larger than isSpannableLen_, the array isSpannable_ will re-malloc endIndex length memory. */
    bool SetSpannable(bool value, uint16_t startIndex, uint16_t endIndex);
    void SetFontHeight(int16_t inputHeight, uint16_t startIndex, uint16_t endIndex);
    bool ExpandSpannableLen(uint16_t index);

    /* merge same value. */
    template <typename SpanType>
    bool EqualInsert(uint16_t& startIndex,
                     uint16_t& endIndex,
                     uint16_t tempStart,
                     uint16_t tempEnd,
                     ListNode<SpanType>** tempSpan,
                     List<SpanType>& tempList)
    {
        if ((startIndex == tempStart) && endIndex == tempEnd) {
            return false;
        }
        bool needAddNode = true;
        if (startIndex <= tempStart) {
            if (*tempSpan == nullptr) {
                return false;
            }
            if (endIndex < tempStart) {
                // not overlap
            } else if (endIndex <= tempEnd) {
                endIndex = tempEnd;
                *tempSpan = tempList.Remove(*tempSpan)->prev_;
            } else {
                *tempSpan = tempList.Remove(*tempSpan)->prev_;
            }
        } else {
            if (*tempSpan == nullptr) {
                return false;
            }
            if (startIndex > tempEnd) {
                // not overlap
            } else if (endIndex <= tempEnd) {
                needAddNode = false;
            } else {
                startIndex = tempStart;
                *tempSpan = tempList.Remove(*tempSpan)->prev_;
            }
        }
        return needAddNode;
    }

    /* merge different value. */
    template <typename SpanType>
    bool UnequalInsert(uint16_t& startIndex,
                       uint16_t& endIndex,
                       uint16_t tempStart,
                       uint16_t tempEnd,
                       ListNode<SpanType>** tempSpan,
                       List<SpanType>& tempList,
                       SpanType tempLeft,
                       SpanType tempRight)
    {
        bool needAddNode = true;
        if (*tempSpan == nullptr) {
            return false;
        }
        if (startIndex > tempEnd) {
            // case A, no change
        } else if (startIndex > tempStart && endIndex >= tempEnd) {
            // case B
            (*tempSpan)->data_.end = startIndex;
        } else if (startIndex <= tempStart && endIndex >= tempEnd) {
            // case C
            *tempSpan = tempList.Remove(*tempSpan)->prev_;
        } else if (startIndex <= tempStart && endIndex < tempEnd) {
            // case D
            (*tempSpan)->data_.start = endIndex;
        } else if (endIndex < tempStart) {
            // case E, no change
        } else if (startIndex > tempStart && endIndex < tempEnd) {
            // case F
            tempList.Insert(*tempSpan, tempLeft);
            tempList.Insert(*tempSpan, tempRight);
            *tempSpan = tempList.Remove(*tempSpan)->prev_;
        } else {
            /* it shall not go here */
        }
        return needAddNode;
    }
};
} // namespace OHOS
#endif
