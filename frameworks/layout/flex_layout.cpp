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

#include "layout/flex_layout.h"
#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
#include <climits>
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT

namespace OHOS {

#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
namespace {
constexpr float FLEX_PERCENT_BASE = 100.0f;
constexpr uint16_t ASPECT_RATIO_BASE = 100; // aspect-ratio base unit: ratio = width / height * 100

inline int16_t ClampToInt16(int32_t value)
{
    if (value > INT16_MAX) {
        return INT16_MAX;
    }
    if (value < INT16_MIN) {
        return INT16_MIN;
    }
    return static_cast<int16_t>(value);
}

int16_t ResolveAbsoluteInset(int16_t pixelValue, float percentValue, bool isPercent, int16_t base)
{
    if (isPercent) {
        return static_cast<int16_t>(percentValue * base / FLEX_PERCENT_BASE);
    }
    return pixelValue;
}

int16_t ResolveAbsoluteX(UIView* item, int16_t containerWidth, int16_t& width, int16_t height)
{
    bool hasLeft = item->HasFlexLeft();
    bool hasRight = item->HasFlexRight();
    int16_t left = ResolveAbsoluteInset(item->GetFlexLeft(), item->GetFlexLeftPercent(),
                                        item->IsFlexLeftPercent(), containerWidth);
    int16_t right = ResolveAbsoluteInset(item->GetFlexRight(), item->GetFlexRightPercent(),
                                         item->IsFlexRightPercent(), containerWidth);
    int16_t borderPad = item->GetStyle(STYLE_BORDER_WIDTH) * 2 + /* 2: left and right border */
                        item->GetStyle(STYLE_PADDING_LEFT) + item->GetStyle(STYLE_PADDING_RIGHT);
    int16_t marginLeft = item->GetStyle(STYLE_MARGIN_LEFT);
    int16_t marginRight = item->GetStyle(STYLE_MARGIN_RIGHT);
    if (hasLeft && hasRight) {
        width = containerWidth - left - right - borderPad - marginLeft - marginRight;
        if (width < 0) {
            width = 0;
        }
        item->Resize(width, height);
        return left;
    }
    if (hasLeft) {
        return left;
    }
    if (hasRight) {
        return containerWidth - width - right - borderPad - marginLeft - marginRight;
    }
    return 0;
}

int16_t ResolveAbsoluteY(UIView* item, int16_t containerHeight, int16_t width)
{
    bool hasTop = item->HasFlexTop();
    bool hasBottom = item->HasFlexBottom();
    int16_t top = ResolveAbsoluteInset(item->GetFlexTop(), item->GetFlexTopPercent(),
                                       item->IsFlexTopPercent(), containerHeight);
    int16_t bottom = ResolveAbsoluteInset(item->GetFlexBottom(), item->GetFlexBottomPercent(),
                                          item->IsFlexBottomPercent(), containerHeight);
    int16_t borderPad = item->GetStyle(STYLE_BORDER_WIDTH) * 2 + /* 2: top and bottom border */
                        item->GetStyle(STYLE_PADDING_TOP) + item->GetStyle(STYLE_PADDING_BOTTOM);
    int16_t marginTop = item->GetStyle(STYLE_MARGIN_TOP);
    int16_t marginBottom = item->GetStyle(STYLE_MARGIN_BOTTOM);
    if (hasTop && hasBottom) {
        int16_t height = containerHeight - top - bottom - borderPad - marginTop - marginBottom;
        if (height < 0) {
            height = 0;
        }
        item->Resize(width, height);
        return top;
    }
    if (hasTop) {
        return top;
    }
    if (hasBottom) {
        return containerHeight - item->GetHeight() - bottom - borderPad - marginTop - marginBottom;
    }
    return 0;
}

Rect GetScrollAdjustedParentRect(UIView* view, const Rect& parentRect)
{
    UIView* childInScroll = view;
    UIView* ancestor = (view == nullptr) ? nullptr : view->GetParent();
    while (ancestor != nullptr) {
        if (ancestor->GetViewType() == UI_SCROLL_VIEW) {
            Rect adjustedRect = parentRect;
            Rect scrollChildRect = childInScroll->GetRelativeRect();
            adjustedRect.SetX(parentRect.GetX() - scrollChildRect.GetX());
            adjustedRect.SetY(parentRect.GetY() - scrollChildRect.GetY());
            return adjustedRect;
        }
        childInScroll = ancestor;
        ancestor = ancestor->GetParent();
    }
    return parentRect;
}
} // namespace
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT

/*
 * Entry point: dispatches to legacy path or phased algorithm.
 */
void FlexLayout::LayoutChildren(bool needInvalidate)
{
    if (childrenHead_ == nullptr) {
        return;
    }

#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
    ResolveGaps();
    bool restoredCrossSize = RestoreChildrenOriginalSizeIfNeeded();
    bool restoreFromStretch = (lastAlignContent_ == ALIGN_CONTENT_STRETCH) &&
                              (alignContent_ == ALIGN_CONTENT_START) && (wrap_ == WRAP);
    LayoutChildrenWithFlexSizing(needInvalidate, restoredCrossSize || restoreFromStretch);
    lastAlignContent_ = alignContent_;
#else
    // No Flex item sizing support: use legacy algorithm.
    if ((direction_ == LAYOUT_HOR) || (direction_ == LAYOUT_HOR_R)) {
        LayoutHorizontal();
    } else {
        LayoutVertical();
    }
    if (needInvalidate) {
        Invalidate();
    }
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT
}

#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
int16_t FlexLayout::ResolveGapPercent(float percent, bool isAuto, int16_t base)
{
    if (isAuto || percent <= 0) {
        return 0;
    }
    float pixel = percent * base / FLEX_PERCENT_BASE;
    return (pixel >= INT16_MAX) ? INT16_MAX : static_cast<int16_t>(pixel);
}

void FlexLayout::ResolveGaps()
{
    if (isColumnGapPercent_) {
        columnGap_ = ResolveGapPercent(columnGapPercent_, gapBaseWidthAuto_, GetWidth());
    }
    if (isRowGapPercent_) {
        rowGap_ = ResolveGapPercent(rowGapPercent_, gapBaseHeightAuto_, GetHeight());
    }
}

void FlexLayout::LayoutChildrenWithoutFlexSizing(bool needInvalidate)
{
    if ((direction_ == LAYOUT_HOR) || (direction_ == LAYOUT_HOR_R)) {
        LayoutHorizontal();
    } else {
        LayoutVertical();
    }
    if (needInvalidate) {
        Invalidate();
    }
}

void FlexLayout::LayoutChildrenWithFlexSizing(bool needInvalidate, bool restoredCrossSize)
{
    // W3C flexbox: the initial value of align-content is stretch, so a wrapped container
    // distributes its lines through the phased path by default. An explicit
    // align-content: flex-start keeps the legacy path unless another enhancement needs it.
    const bool hasMultiLineAlignContent = (wrap_ == WRAP) && (alignContent_ != ALIGN_CONTENT_START);
    const bool hasGap = (rowGap_ != 0) || (columnGap_ != 0);
    const bool hasCrossStretch = (secondaryAlign_ == ALIGN_STRETCH);
    const bool isHorizontal = (direction_ == LAYOUT_HOR) || (direction_ == LAYOUT_HOR_R);
    const bool forceAdvancedLayout = hasMultiLineAlignContent || restoredCrossSize;
    if (!HasPhasedLayoutNeed(forceAdvancedLayout) && !hasGap && !hasCrossStretch && !restoredCrossSize) {
        LayoutChildrenWithoutFlexSizing(needInvalidate);
        return;
    }

    uint16_t inFlowCount = 0;
    uint16_t absCount = 0;
    CountVisibleChildren(inFlowCount, absCount);
    if ((inFlowCount == 0) && (absCount == 0)) {
        return;
    }

    UIView* sortedBuf[MAX_COUNT_DEFAULT] = {nullptr};
    UIView** sorted = sortedBuf;
    UIView** sortedAlloc = nullptr;
    if (inFlowCount > MAX_COUNT_DEFAULT) {
        sortedAlloc = new UIView* [inFlowCount]();
        sorted = sortedAlloc;
    }

    UIView* absBuf[MAX_COUNT_DEFAULT] = {nullptr};
    UIView** absChildren = absBuf;
    UIView** absAlloc = nullptr;
    if (absCount > MAX_COUNT_DEFAULT) {
        absAlloc = new UIView* [absCount]();
        absChildren = absAlloc;
    }

    if ((sorted != nullptr) && (absChildren != nullptr)) {
        FillLayoutChildren(sorted, absChildren);
        LayoutPreparedChildren(sorted, inFlowCount, absChildren, absCount, forceAdvancedLayout, isHorizontal);
    }

    if (needInvalidate) {
        Invalidate();
    }
    if (sortedAlloc != nullptr) {
        delete[] sortedAlloc;
    }
    if (absAlloc != nullptr) {
        delete[] absAlloc;
    }
}

bool FlexLayout::HasPhasedLayoutNeed(bool hasMultiLineAlignContent) const
{
    for (UIView* view = childrenHead_; view != nullptr; view = view->GetNextSibling()) {
        if (view->IsVisible() && view->IsPhasedLayoutNeed()) {
            return true;
        }
    }
    return hasMultiLineAlignContent;
}

void FlexLayout::CountVisibleChildren(uint16_t& inFlowCount, uint16_t& absCount) const
{
    for (UIView* child = childrenHead_; child != nullptr; child = child->GetNextSibling()) {
        if (!child->IsVisible()) {
            continue;
        }
        if (child->GetPositionType() == POSITION_ABSOLUTE) {
            absCount++;
        } else {
            inFlowCount++;
        }
    }
}

void FlexLayout::FillLayoutChildren(UIView** sorted, UIView** absChildren)
{
    uint16_t idx = 0;
    uint16_t aidx = 0;
    for (UIView* child = childrenHead_; child != nullptr; child = child->GetNextSibling()) {
        if (!child->IsVisible()) {
            continue;
        }
        child->ReMeasure();
        if (child->GetPositionType() == POSITION_ABSOLUTE) {
            absChildren[aidx++] = child;
        } else {
            sorted[idx++] = child;
        }
    }
}

bool FlexLayout::HasAdvancedLayoutNeed(UIView** sorted, uint16_t inFlowCount,
                                       uint16_t absCount, bool forceAdvancedLayout) const
{
    return HasFlexItemProperties(sorted, inFlowCount) || forceAdvancedLayout ||
           (secondaryAlign_ == ALIGN_STRETCH) || (rowGap_ != 0) || (columnGap_ != 0) || (absCount > 0);
}

void FlexLayout::LayoutPreparedChildren(UIView** sorted, uint16_t inFlowCount, UIView** absChildren,
                                        uint16_t absCount, bool forceAdvancedLayout, bool isHorizontal)
{
    if (HasAdvancedLayoutNeed(sorted, inFlowCount, absCount, forceAdvancedLayout)) {
        LayoutAdvancedChildren(sorted, inFlowCount, absChildren, absCount, isHorizontal);
        return;
    }

    if (isHorizontal) {
        LayoutHorizontal();
    } else {
        LayoutVertical();
    }
}

void FlexLayout::LayoutAdvancedChildren(UIView** sorted, uint16_t inFlowCount, UIView** absChildren,
                                        uint16_t absCount, bool isHorizontal)
{
    LayoutChildrenPhased(sorted, inFlowCount, isHorizontal);
    if (absCount == 0) {
        return;
    }

    PositionAbsoluteChildren(absChildren, absCount);
}

bool FlexLayout::RestoreChildrenOriginalSizeIfNeeded()
{
    bool directionChanged = (lastDirection_ != direction_);
    bool isHorizontal = (direction_ == LAYOUT_HOR) || (direction_ == LAYOUT_HOR_R);
    bool restored = false;
    for (UIView* child = childrenHead_; child != nullptr; child = child->GetNextSibling()) {
        if (!child->IsVisible()) {
            continue;
        }
        if (directionChanged) {
            RestoreOriginalCrossSizeIfNeeded(child, true);
            RestoreOriginalCrossSizeIfNeeded(child, false);
            restored = true;
            continue;
        }
        if (!IsCrossStretchItem(child)) {
            bool wasSaved = isHorizontal ? child->IsOriginalHeightSaved() : child->IsOriginalWidthSaved();
            RestoreOriginalCrossSizeIfNeeded(child, isHorizontal);
            restored = restored || wasSaved;
        }
    }
    if (directionChanged) {
        lastDirection_ = direction_;
    }
    return restored;
}

bool FlexLayout::IsCrossStretchItem(UIView* item) const
{
    if (item == nullptr) {
        return false;
    }
    uint8_t align = item->GetAlignSelf();
    if (align == UIView::ALIGN_SELF_AUTO) {
        align = secondaryAlign_;
    }
    return (align == ALIGN_STRETCH) || (align == UIView::ALIGN_SELF_STRETCH);
}

void FlexLayout::SaveOriginalCrossSizeIfNeeded(UIView* item, bool isHorizontal) const
{
    if (item == nullptr) {
        return;
    }
    if (isHorizontal) {
        if (!item->IsOriginalHeightSaved()) {
            item->SetOriginalHeight(item->GetHeight());
            item->SetOriginalHeightSaved(true);
        }
        return;
    }
    if (!item->IsOriginalWidthSaved()) {
        item->SetOriginalWidth(item->GetWidth());
        item->SetOriginalWidthSaved(true);
    }
}

void FlexLayout::RestoreOriginalCrossSizeIfNeeded(UIView* item, bool isHorizontal) const
{
    if (item == nullptr) {
        return;
    }
    if (isHorizontal) {
        if (item->IsOriginalHeightSaved()) {
            item->Resize(item->GetWidth(), item->GetOriginalHeight());
            item->SetOriginalHeightSaved(false);
        }
        return;
    }
    if (item->IsOriginalWidthSaved()) {
        item->Resize(item->GetOriginalWidth(), item->GetHeight());
        item->SetOriginalWidthSaved(false);
    }
}
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT

void FlexLayout::GetStartPos(const int16_t& length,
                             int16_t& pos,
                             int16_t& interval,
                             int16_t count,
                             uint16_t* validLengths,
                             uint16_t* childsNum)
{
    if (!validLengths || !childsNum) {
        return;
    }
    pos = 0;
    interval = 0;

    if (majorAlign_ == ALIGN_START) {
        pos = 0;
    } else if (majorAlign_ == ALIGN_END) {
        pos = length - validLengths[count];
        /* if total length of children is too long or only one child, layout them centerly no matter what key word set.
         */
    } else if ((majorAlign_ == ALIGN_CENTER) || (validLengths[count] >= length) || (childsNum[count] == 1)) {
        pos = (length - validLengths[count]) / 2; // 2: half
    } else if (majorAlign_ == ALIGN_AROUND) {
        if (childsNum[count] == 0) {
            return;
        }
        interval = (length - validLengths[count]) / childsNum[count];
        pos = interval / 2; // 2: half
    } else if (majorAlign_ == ALIGN_EVENLY) {
        interval = (length - validLengths[count]) / (childsNum[count] + 1);
        pos = interval;
    } else {
        interval = (length - validLengths[count]) / (childsNum[count] - 1);
        pos = 0;
    }
}

void FlexLayout::GetNoWrapStartPos(const int16_t& length, int16_t& majorPos, int16_t& interval)
{
    uint16_t childrenNum = 0;
    uint16_t totalValidLength = 0;

    CalValidLength(totalValidLength, childrenNum);
    GetStartPos(length, majorPos, interval, 0, &totalValidLength, &childrenNum);
}

void FlexLayout::GetRowStartPos(int16_t& pos,
                                int16_t& interval,
                                int16_t count,
                                uint16_t* rowsWidth,
                                uint16_t* rowsChildNum)
{
    GetStartPos(GetWidth(), pos, interval, count, rowsWidth, rowsChildNum);
}

void FlexLayout::GetColumnStartPos(int16_t& pos,
                                   int16_t& interval,
                                   int16_t count,
                                   uint16_t* columnsHeight,
                                   uint16_t* columnsChildNum)
{
    GetStartPos(GetHeight(), pos, interval, count, columnsHeight, columnsChildNum);
}

void FlexLayout::CalValidLength(uint16_t& totalValidLength, uint16_t& allChildNum)
{
    UIView* child = childrenHead_;
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;

    /* calculate valid length of all children views */
    while (child != nullptr) {
        if (child->IsVisible()) {
            child->ReMeasure();
            if ((direction_ == LAYOUT_HOR) || (direction_ == LAYOUT_HOR_R)) {
                left = child->GetStyle(STYLE_MARGIN_LEFT);
                right = child->GetStyle(STYLE_MARGIN_RIGHT);
                totalValidLength += (child->GetRelativeRect().GetWidth() + left + right);
            } else {
                top = child->GetStyle(STYLE_MARGIN_TOP);
                bottom = child->GetStyle(STYLE_MARGIN_BOTTOM);
                totalValidLength += (child->GetRelativeRect().GetHeight() + top + bottom);
            }
            allChildNum++;
        }
        child = child->GetNextSibling();
    }
}

void FlexLayout::CalRowCount()
{
    UIView* child = childrenHead_;
    int16_t pos = 0;
    int16_t left;
    int16_t right;

    rowCount_ = 1;
    while (child != nullptr) {
        if (child->IsVisible()) {
            child->ReMeasure();
            left = child->GetStyle(STYLE_MARGIN_LEFT);
            right = child->GetStyle(STYLE_MARGIN_RIGHT);
            pos += left;
            if ((pos + child->GetRelativeRect().GetWidth() + right) > GetWidth()) {
                pos = left;
                rowCount_++;
            }
            pos += child->GetRelativeRect().GetWidth() + right;
        }
        child = child->GetNextSibling();
    }
}

void FlexLayout::GetRowMaxHeight(uint16_t size, uint16_t* maxRosHegiht)
{
    UIView* child = childrenHead_;
    int16_t pos = 0;
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
    uint16_t i = 0;
    uint16_t height = 0;

    if ((maxRosHegiht == nullptr) || (size > rowCount_)) {
        return;
    }

    while (child != nullptr) {
        if (child->IsVisible()) {
            left = child->GetStyle(STYLE_MARGIN_LEFT);
            right = child->GetStyle(STYLE_MARGIN_RIGHT);
            top = child->GetStyle(STYLE_MARGIN_TOP);
            bottom = child->GetStyle(STYLE_MARGIN_BOTTOM);
            pos += left;
            if ((pos + child->GetRelativeRect().GetWidth() + right) > GetWidth()) {
                pos = left;
                maxRosHegiht[i] = height;
                height = 0;
                i++;
            }
            height = MATH_MAX(height, child->GetRelativeRect().GetHeight() + top + bottom);
            maxRosHegiht[i] = height;
            pos += child->GetRelativeRect().GetWidth() + right;
        }
        child = child->GetNextSibling();
    }
}

void FlexLayout::GetRowsWidth(uint16_t rowNum, uint16_t* rowsWidth, uint16_t* rowsChildNum)
{
    UIView* child = childrenHead_;
    int16_t pos = 0;
    int16_t left;
    int16_t right;
    uint16_t rowChildNum = 0;
    uint16_t rowCount = 0;
    uint16_t width = 0;

    if ((rowsWidth == nullptr) || (rowsChildNum == nullptr) || (rowNum > rowCount_)) {
        return;
    }

    while (child != nullptr) {
        if (child->IsVisible()) {
            left = child->GetStyle(STYLE_MARGIN_LEFT);
            right = child->GetStyle(STYLE_MARGIN_RIGHT);
            pos += left;
            if ((pos + child->GetRelativeRect().GetWidth() + right) > GetWidth()) {
                pos = left;
                rowsWidth[rowCount] = width;
                width = 0;
                rowsChildNum[rowCount] = rowChildNum;
                rowChildNum = 0;
                rowCount++;
            }
            width += child->GetRelativeRect().GetWidth() + right + left;
            rowsWidth[rowCount] = width;
            rowChildNum++;
            rowsChildNum[rowCount] = rowChildNum;
            pos += child->GetRelativeRect().GetWidth() + right;
        }
        child = child->GetNextSibling();
    }
}

void FlexLayout::GetCrossAxisPosY(int16_t& posY, uint16_t& count, uint16_t* rowsMaxHeight, UIView* child)
{
    if ((rowsMaxHeight == nullptr) || (child == nullptr)) {
        return;
    }

    uint16_t i = 0;
    uint16_t offset = 0;
    int16_t top = child->GetStyle(STYLE_MARGIN_TOP);
    int16_t bottom = child->GetStyle(STYLE_MARGIN_BOTTOM);

    if (secondaryAlign_ == ALIGN_START) {
        for (i = 0; i < count; i++) {
            offset += rowsMaxHeight[i];
        }
        posY = top + offset;
    } else if (secondaryAlign_ == ALIGN_END) {
        for (i = rowCount_ - 1; i > count; i--) {
            offset += rowsMaxHeight[i];
        }
        posY = GetHeight() - child->GetRelativeRect().GetHeight() - bottom - offset;
    } else {
        for (i = 0; i < rowCount_; i++) {
            offset += rowsMaxHeight[i];
        }
        offset = (rowsMaxHeight[0] - offset) / 2; // 2: half
        for (i = 1; i <= count; i++) {
            offset += (rowsMaxHeight[i - 1] + rowsMaxHeight[i]) / 2; // 2: half
        }
        posY = (GetHeight() - child->GetRelativeRect().GetHeight() - top - bottom) / 2 + top + offset; // 2: half
    }
}

void FlexLayout::LayoutHorizontal()
{
    UIView* child = childrenHead_;
    int16_t interval = 0;
    int16_t posX = 0;
    int16_t posY = 0;
    uint16_t count = 0;
    uint16_t widthsBuf[MAX_COUNT_DEFAULT] = {0};
    uint16_t maxHeightsBuf[MAX_COUNT_DEFAULT] = {0};
    uint16_t childsNumBuf[MAX_COUNT_DEFAULT] = {0};
    uint16_t* rowsWidth = widthsBuf;
    uint16_t* rowsMaxHeight = maxHeightsBuf;
    uint16_t* rowsChildNum = childsNumBuf;
    bool allocFlag = false;

    if (wrap_ == WRAP) {
        CalRowCount();
        if (rowCount_ > MAX_COUNT_DEFAULT) {
            rowsWidth = new uint16_t[rowCount_]();
            rowsMaxHeight = new uint16_t[rowCount_]();
            rowsChildNum = new uint16_t[rowCount_]();
            allocFlag = true;
        }
        GetRowMaxHeight(rowCount_, rowsMaxHeight);
        GetRowsWidth(rowCount_, rowsWidth, rowsChildNum);
        GetRowStartPos(posX, interval, count, rowsWidth, rowsChildNum);
    } else {
        GetNoWrapStartPos(GetWidth(), posX, interval);
    }

    while (child != nullptr) {
        if (child->IsVisible()) {
            int16_t left = child->GetStyle(STYLE_MARGIN_LEFT);
            int16_t right = child->GetStyle(STYLE_MARGIN_RIGHT);
            posX += left;
            if (((posX + child->GetRelativeRect().GetWidth() + right) > GetWidth()) && (wrap_ == WRAP)) {
                GetRowStartPos(posX, interval, ++count, rowsWidth, rowsChildNum);
                posX += left;
            }

            GetCrossAxisPosY(posY, count, rowsMaxHeight, child);
            if (direction_ == LAYOUT_HOR_R) {
                child->SetPosition(GetWidth() - posX - child->GetRelativeRect().GetWidth() - right,
                                   posY - child->GetStyle(STYLE_MARGIN_TOP));
            } else {
                child->SetPosition(posX - left, posY - child->GetStyle(STYLE_MARGIN_TOP));
            }
            posX += child->GetRelativeRect().GetWidth() + right + interval;
            child->LayoutChildren();
        }
        child = child->GetNextSibling();
    }

    if (allocFlag) {
        delete[] rowsWidth;
        delete[] rowsMaxHeight;
        delete[] rowsChildNum;
    }
}

void FlexLayout::CalColumnCount()
{
    UIView* child = childrenHead_;
    int16_t pos = 0;
    int16_t top;
    int16_t bottom;

    columnCount_ = 1;
    while (child != nullptr) {
        if (child->IsVisible()) {
            child->ReMeasure();
            top = child->GetStyle(STYLE_MARGIN_TOP);
            bottom = child->GetStyle(STYLE_MARGIN_BOTTOM);
            pos += top;
            if ((pos + child->GetRelativeRect().GetHeight() + bottom) > GetHeight()) {
                pos = top;
                columnCount_++;
            }
            pos += child->GetRelativeRect().GetHeight() + bottom;
        }
        child = child->GetNextSibling();
    }
}

void FlexLayout::GetColumnMaxWidth(uint16_t size, uint16_t* maxColumnsWidth)
{
    UIView* child = childrenHead_;
    int16_t pos = 0;
    int16_t left;
    int16_t right;
    int16_t bottom;
    uint16_t i = 0;
    uint16_t width = 0;

    if ((maxColumnsWidth == nullptr) || (size > columnCount_)) {
        return;
    }

    while (child != nullptr) {
        if (child->IsVisible()) {
            left = child->GetStyle(STYLE_MARGIN_LEFT);
            right = child->GetStyle(STYLE_MARGIN_RIGHT);
            bottom = child->GetStyle(STYLE_MARGIN_BOTTOM);
            pos += left;
            if ((pos + child->GetRelativeRect().GetHeight() + bottom) > GetHeight()) {
                pos = left;
                maxColumnsWidth[i] = width;
                width = 0;
                i++;
            }
            width = MATH_MAX(width, child->GetRelativeRect().GetWidth() + left + right);
            maxColumnsWidth[i] = width;
            pos += child->GetRelativeRect().GetHeight() + bottom;
        }
        child = child->GetNextSibling();
    }
}

void FlexLayout::GetColumnsHeight(uint16_t columnNum, uint16_t* columnsHeight, uint16_t* columnsChildNum)
{
    UIView* child = childrenHead_;
    int16_t pos = 0;
    int16_t top;
    int16_t bottom;
    uint16_t columnChildNum = 0;
    uint16_t columnCount = 0;
    uint16_t height = 0;

    if ((columnsHeight == nullptr) || (columnsChildNum == nullptr) || (columnNum > columnCount_)) {
        return;
    }

    while (child != nullptr) {
        if (child->IsVisible()) {
            top = child->GetStyle(STYLE_MARGIN_TOP);
            bottom = child->GetStyle(STYLE_MARGIN_BOTTOM);
            pos += top;
            if ((pos + child->GetRelativeRect().GetHeight() + bottom) > GetHeight()) {
                pos = top;
                columnsHeight[columnCount] = height;
                height = 0;
                columnsChildNum[columnCount] = columnChildNum;
                columnChildNum = 0;
                columnCount++;
            }
            height += child->GetRelativeRect().GetHeight() + top + bottom;
            columnsHeight[columnCount] = height;
            columnChildNum++;
            columnsChildNum[columnCount] = columnChildNum;
            pos += child->GetRelativeRect().GetHeight() + bottom;
        }
        child = child->GetNextSibling();
    }
}

void FlexLayout::GetCrossAxisPosX(int16_t& posX, uint16_t& count, uint16_t* columnsMaxWidth, UIView* child)
{
    if ((columnsMaxWidth == nullptr) || (child == nullptr)) {
        return;
    }

    uint16_t i = 0;
    uint16_t offset = 0;
    int16_t left = child->GetStyle(STYLE_MARGIN_LEFT);
    int16_t right = child->GetStyle(STYLE_MARGIN_RIGHT);

    if (secondaryAlign_ == ALIGN_START) {
        for (i = 0; i < count; i++) {
            offset += columnsMaxWidth[i];
        }
        posX = left + offset;
    } else if (secondaryAlign_ == ALIGN_END) {
        for (i = columnCount_ - 1; i > count; i--) {
            offset += columnsMaxWidth[i];
        }
        posX = GetWidth() - child->GetRelativeRect().GetWidth() - right - offset;
    } else {
        for (i = 0; i < columnCount_; i++) {
            offset += columnsMaxWidth[i];
        }
        offset = (columnsMaxWidth[0] - offset) / 2; // 2: half
        for (i = 1; i <= count; i++) {
            offset += (columnsMaxWidth[i - 1] + columnsMaxWidth[i]) / 2; // 2: half
        }
        posX = (GetWidth() - child->GetRelativeRect().GetWidth() - left - right) / 2 + left + offset; // 2: half
    }
}

void FlexLayout::LayoutVertical()
{
    UIView* child = childrenHead_;
    int16_t interval = 0;
    int16_t posX = 0;
    int16_t posY = 0;
    uint16_t count = 0;
    uint16_t heightsBuf[MAX_COUNT_DEFAULT] = {0};
    uint16_t maxWidthsBuf[MAX_COUNT_DEFAULT] = {0};
    uint16_t childsNumBuf[MAX_COUNT_DEFAULT] = {0};
    uint16_t* columnsHeight = heightsBuf;
    uint16_t* columnsMaxWidth = maxWidthsBuf;
    uint16_t* columnsChildNum = childsNumBuf;
    bool allocFlag = false;

    if (wrap_ == WRAP) {
        CalColumnCount();
        if (columnCount_ > MAX_COUNT_DEFAULT) {
            columnsHeight = new uint16_t[columnCount_]();
            columnsMaxWidth = new uint16_t[columnCount_]();
            columnsChildNum = new uint16_t[columnCount_]();
            allocFlag = true;
        }
        GetColumnMaxWidth(columnCount_, columnsMaxWidth);
        GetColumnsHeight(columnCount_, columnsHeight, columnsChildNum);
        GetColumnStartPos(posY, interval, count, columnsHeight, columnsChildNum);
    } else {
        GetNoWrapStartPos(GetHeight(), posY, interval);
    }

    while (child != nullptr) {
        if (child->IsVisible()) {
            int16_t top = child->GetStyle(STYLE_MARGIN_TOP);
            int16_t bottom = child->GetStyle(STYLE_MARGIN_BOTTOM);
            posY += top;
            if (((posY + child->GetRelativeRect().GetHeight() + bottom) > GetHeight()) && (wrap_ == WRAP)) {
                GetColumnStartPos(posY, interval, ++count, columnsHeight, columnsChildNum);
                posY += top;
            }

            GetCrossAxisPosX(posX, count, columnsMaxWidth, child);
            if (direction_ == LAYOUT_VER_R) {
                child->SetPosition(posX - child->GetStyle(STYLE_MARGIN_LEFT),
                                   GetHeight() - posY - child->GetRelativeRect().GetHeight() - bottom);
            } else {
                child->SetPosition(posX - child->GetStyle(STYLE_MARGIN_LEFT), posY - top);
            }
            posY += child->GetRelativeRect().GetHeight() + bottom + interval;
            child->LayoutChildren();
        }
        child = child->GetNextSibling();
    }

    if (allocFlag) {
        delete[] columnsHeight;
        delete[] columnsMaxWidth;
        delete[] columnsChildNum;
    }
}

#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
bool FlexLayout::HasFlexItemProperties(UIView** sorted, uint16_t count) const
{
    for (uint16_t i = 0; i < count; i++) {
        if (sorted[i]->HasFlexItemProperties()) {
            return true;
        }
    }
    return false;
}

bool FlexLayout::IsAbsoluteContainingBlock(UIView* view) const
{
    return (view != nullptr) && (view->GetPositionType() == POSITION_ABSOLUTE);
}

UIView* FlexLayout::FindAbsoluteContainingBlock(UIView* item) const
{
    UIView* ancestor = (item == nullptr) ? nullptr : item->GetParent();
    while (ancestor != nullptr) {
        if (IsAbsoluteContainingBlock(ancestor)) {
            return ancestor;
        }
        ancestor = ancestor->GetParent();
    }
    return nullptr;
}

UIView* FlexLayout::GetInitialContainingBlock(UIView* item) const
{
    UIView* containingBlock = (item == nullptr) ? nullptr : item->GetParent();
    UIView* parent = (containingBlock == nullptr) ? nullptr : containingBlock->GetParent();
    while (parent != nullptr) {
        containingBlock = parent;
        parent = parent->GetParent();
    }
    return containingBlock;
}

int16_t FlexLayout::GetMainGap(bool isHorizontal) const
{
    return isHorizontal ? columnGap_ : rowGap_;
}

int16_t FlexLayout::GetCrossGap(bool isHorizontal) const
{
    return isHorizontal ? rowGap_ : columnGap_;
}

int16_t FlexLayout::GetItemMinMain(UIView* child, bool isHorizontal) const
{
    int16_t v = isHorizontal ? child->GetMinWidth() : child->GetMinHeight();
    return (v < 0) ? 0 : v;
}

int16_t FlexLayout::GetItemMaxMain(UIView* child, bool isHorizontal) const
{
    return isHorizontal ? child->GetMaxWidth() : child->GetMaxHeight();
}

int16_t FlexLayout::GetItemMinCross(UIView* child, bool isHorizontal) const
{
    int16_t v = isHorizontal ? child->GetMinHeight() : child->GetMinWidth();
    return (v < 0) ? 0 : v;
}

int16_t FlexLayout::GetItemMaxCross(UIView* child, bool isHorizontal) const
{
    return isHorizontal ? child->GetMaxHeight() : child->GetMaxWidth();
}

void FlexLayout::ClampMainSizes(UIView** sorted, uint16_t count, bool isHorizontal)
{
    // Clamp each item's main size into [min, max]. -1 max means unlimited.
    for (uint16_t i = 0; i < count; i++) {
        UIView* child = sorted[i];
        int16_t minMain = GetItemMinMain(child, isHorizontal);
        int16_t maxMain = GetItemMaxMain(child, isHorizontal);
        int16_t curMain = isHorizontal ? child->GetWidth() : child->GetHeight();
        int16_t newMain = curMain;
        if (newMain < minMain) {
            newMain = minMain;
        }
        if (maxMain >= 0 && newMain > maxMain) {
            newMain = maxMain;
        }
        if (newMain != curMain) {
            if (isHorizontal) {
                child->Resize(newMain, child->GetHeight());
            } else {
                child->Resize(child->GetWidth(), newMain);
            }
        }
    }
}

void FlexLayout::ClampCrossSizes(UIView** sorted, uint16_t count, bool isHorizontal)
{
    // Cross-axis min/max constraints apply independently of align-items. In
    // particular, an item with height=0 and min-height must become visible even
    // when the container uses center/start/end rather than stretch.
    for (uint16_t i = 0; i < count; i++) {
        UIView* child = sorted[i];
        int16_t minCross = GetItemMinCross(child, isHorizontal);
        int16_t maxCross = GetItemMaxCross(child, isHorizontal);
        int16_t curCross = isHorizontal ? child->GetHeight() : child->GetWidth();
        int16_t newCross = curCross;
        if (newCross < minCross) {
            newCross = minCross;
        }
        if (maxCross >= 0 && newCross > maxCross) {
            newCross = maxCross;
        }
        if (newCross != curCross) {
            if (isHorizontal) {
                child->Resize(child->GetWidth(), newCross);
            } else {
                child->Resize(newCross, child->GetHeight());
            }
        }
    }
}

void FlexLayout::RestoreAutoCrossSizes(UIView** sorted, uint16_t count, bool isHorizontal)
{
    for (uint16_t i = 0; i < count; i++) {
        UIView* child = sorted[i];
        int16_t crossSize = child->GetFlexAutoBasis(!isHorizontal);
        if (isHorizontal) {
            child->Resize(child->GetWidth(), crossSize);
        } else {
            child->Resize(crossSize, child->GetHeight());
        }
    }
}

void FlexLayout::PositionAbsoluteChildren(UIView** absChildren, uint16_t absCount)
{
    for (uint16_t i = 0; i < absCount; i++) {
        UIView* item = absChildren[i];
        UIView* containingBlock = FindAbsoluteContainingBlock(item);
        bool useInitialContainingBlock = false;
        if (containingBlock == nullptr) {
            containingBlock = GetInitialContainingBlock(item);
            useInitialContainingBlock = true;
        }
        if (containingBlock == nullptr) {
            continue;
        }

        Rect blockRect = containingBlock->GetPaddingBoxRect();
        Rect parentRect = GetPaddingBoxRect();
        if (useInitialContainingBlock) {
            parentRect = GetScrollAdjustedParentRect(this, parentRect);
        }
        item->ReMeasure();
        int16_t w = item->GetWidth();
        int16_t h = item->GetHeight();
        int16_t x = ResolveAbsoluteX(item, blockRect.GetWidth(), w, h);
        int16_t y = ResolveAbsoluteY(item, blockRect.GetHeight(), w);
        item->SetPosition(blockRect.GetX() + x - parentRect.GetX(), blockRect.GetY() + y - parentRect.GetY());
        item->LayoutChildren();
    }
}

int16_t FlexLayout::GetItemMainSize(UIView* child, bool isHorizontal) const
{
    if (isHorizontal) {
        return child->GetRelativeRect().GetWidth() +
               static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_LEFT)) +
               static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_RIGHT));
    } else {
        return child->GetRelativeRect().GetHeight() +
               static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_TOP)) +
               static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_BOTTOM));
    }
}

int16_t FlexLayout::GetItemCrossSize(UIView* child, bool isHorizontal) const
{
    if (isHorizontal) {
        return child->GetRelativeRect().GetHeight() +
               static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_TOP)) +
               static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_BOTTOM));
    } else {
        return child->GetRelativeRect().GetWidth() +
               static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_LEFT)) +
               static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_RIGHT));
    }
}

int16_t FlexLayout::GetItemMainMarginStart(UIView* child, bool isHorizontal) const
{
    if (isHorizontal) {
        return static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_LEFT));
    } else {
        return static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_TOP));
    }
}

int16_t FlexLayout::GetItemMainMarginEnd(UIView* child, bool isHorizontal) const
{
    if (isHorizontal) {
        return static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_RIGHT));
    } else {
        return static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_BOTTOM));
    }
}

int16_t FlexLayout::GetItemCrossMarginStart(UIView* child, bool isHorizontal) const
{
    if (isHorizontal) {
        return static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_TOP));
    } else {
        return static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_LEFT));
    }
}

int16_t FlexLayout::GetItemCrossMarginEnd(UIView* child, bool isHorizontal) const
{
    if (isHorizontal) {
        return static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_BOTTOM));
    } else {
        return static_cast<int16_t>(child->GetStyle(STYLE_MARGIN_RIGHT));
    }
}

void FlexLayout::BeginFlexLayoutSizing(UIView** sorted, uint16_t count)
{
    for (uint16_t i = 0; i < count; i++) {
        sorted[i]->BeginFlexLayoutSizing();
    }
}

void FlexLayout::ComputeFlexBasis(UIView** sorted, uint16_t count, bool isHorizontal)
{
    for (uint16_t i = 0; i < count; i++) {
        UIView* child = sorted[i];
        int16_t basis = 0;
        if (ResolveFlexBasis(child, basis)) {
            LayoutFlexBasis(child, basis, isHorizontal);
        } else if (child->GetAspectRatio() != 0) {
            LayoutAspectRatioBasis(child, isHorizontal);
        } else {
            LayoutAutoBasis(child, isHorizontal);
        }
    }
}

void FlexLayout::LayoutFlexBasis(UIView* child, int16_t basis, bool isHorizontal)
{
    if (isHorizontal) {
        child->Resize(basis, child->GetHeight());
    } else {
        child->Resize(child->GetWidth(), basis);
    }
}

bool FlexLayout::ResolveFlexBasis(UIView* child, int16_t& basis) const
{
    basis = child->GetFlexBasis();
    return basis >= 0;
}

void FlexLayout::LayoutAspectRatioBasis(UIView* child, bool isHorizontal)
{
    uint16_t ratio = child->GetAspectRatio();
    if (ratio == 0) {
        return;
    }
    if (child->HasExplicitWidth() && !child->HasExplicitHeight()) {
        int16_t width = child->GetWidth();
        int16_t height = static_cast<int16_t>(static_cast<int64_t>(width) * ASPECT_RATIO_BASE / ratio);
        child->Resize(width, height);
        return;
    }
    if (child->HasExplicitHeight() && !child->HasExplicitWidth()) {
        int16_t height = child->GetHeight();
        int16_t width = static_cast<int16_t>(static_cast<int64_t>(height) * ratio / ASPECT_RATIO_BASE);
        child->Resize(width, height);
        return;
    }
    if (child->HasExplicitWidth() && child->HasExplicitHeight()) {
        return;
    }
    if (isHorizontal) {
        int16_t height = child->GetHeight();
        int16_t width = static_cast<int16_t>(static_cast<int64_t>(height) * ratio / ASPECT_RATIO_BASE);
        child->Resize(width, height);
    } else {
        int16_t width = child->GetWidth();
        int16_t height = static_cast<int16_t>(static_cast<int64_t>(width) * ASPECT_RATIO_BASE / ratio);
        child->Resize(width, height);
    }
}

void FlexLayout::LayoutAutoBasis(UIView* child, bool isHorizontal)
{
    int16_t autoBasis = child->GetFlexAutoBasis(isHorizontal);
    if (isHorizontal) {
        child->Resize(autoBasis, child->GetHeight());
    } else {
        child->Resize(child->GetWidth(), autoBasis);
    }
}

uint16_t FlexLayout::CollectFlexLines(UIView** sorted, uint16_t count, FlexLine* lines,
                                      int16_t containerMainSize, int16_t containerCrossSize, bool isHorizontal)
{
    int16_t mainGap = GetMainGap(isHorizontal);
    uint16_t lineCount = 0;
    lines[lineCount].itemStartIndex = 0;

    for (uint16_t i = 0; i < count; i++) {
        int16_t mainSize = GetItemMainSize(sorted[i], isHorizontal);
        int16_t crossSize = GetItemCrossSize(sorted[i], isHorizontal);
        int16_t gapBefore = (lines[lineCount].lineItemCount > 0) ? mainGap : 0;

        if ((wrap_ == WRAP) && (lines[lineCount].lineItemCount > 0) &&
            (lines[lineCount].totalMainSize + gapBefore + mainSize > containerMainSize)) {
            lineCount++;
            lines[lineCount].itemStartIndex = i;
            gapBefore = 0;
        }

        lines[lineCount].lineItemCount++;
        lines[lineCount].totalMainSize += gapBefore + mainSize;
        if (crossSize > lines[lineCount].crossSize) {
            lines[lineCount].crossSize = crossSize;
        }
    }
    lineCount++;

    if (lineCount == 1 && wrap_ == NOWRAP) {
        lines[0].crossSize = containerCrossSize;
    }
    return lineCount;
}

void FlexLayout::ResizeItemMain(UIView* item, int16_t mainSize, bool isHorizontal) const
{
    if (isHorizontal) {
        item->Resize(mainSize, item->GetHeight());
    } else {
        item->Resize(item->GetWidth(), mainSize);
    }
}

void FlexLayout::RecalculateLineMainSize(FlexLine& line, UIView** sorted, bool isHorizontal)
{
    line.totalMainSize = 0;
    for (uint16_t j = 0; j < line.lineItemCount; j++) {
        line.totalMainSize += GetItemMainSize(sorted[line.itemStartIndex + j], isHorizontal);
    }
}

int16_t FlexLayout::ClampItemMainSize(UIView* item, int32_t mainSize, bool isHorizontal) const
{
    int16_t minMain = GetItemMinMain(item, isHorizontal);
    int16_t maxMain = GetItemMaxMain(item, isHorizontal);
    int32_t clamped = mainSize;
    if (clamped < minMain) {
        clamped = minMain;
    }
    if (maxMain >= 0 && clamped > maxMain) {
        clamped = maxMain;
    }
    if (clamped < 0) {
        return 0;
    }
    return (clamped > INT16_MAX) ? INT16_MAX : static_cast<int16_t>(clamped);
}

bool FlexLayout::PrepareFlexLineFrozenItems(FlexLine& line, bool* frozenBuf,
                                            bool*& frozen, bool*& frozenAlloc) const
{
    frozen = frozenBuf;
    frozenAlloc = nullptr;
    if (line.lineItemCount > MAX_COUNT_DEFAULT) {
        frozenAlloc = new bool[line.lineItemCount]();
        frozen = frozenAlloc;
    }
    return true;
}

uint32_t FlexLayout::FreezeGrowItems(FlexLine& line, UIView** sorted, bool* frozen, bool isHorizontal)
{
    uint32_t totalGrow = 0;
    for (uint16_t j = 0; j < line.lineItemCount; j++) {
        UIView* item = sorted[line.itemStartIndex + j];
        int16_t curMain = isHorizontal ? item->GetWidth() : item->GetHeight();
        int16_t clampedMain = ClampItemMainSize(item, curMain, isHorizontal);
        if (clampedMain != curMain) {
            ResizeItemMain(item, clampedMain, isHorizontal);
            frozen[j] = true;
            continue;
        }
        if (!frozen[j]) {
            totalGrow += item->GetFlexGrow();
        }
    }
    return totalGrow;
}

bool FlexLayout::DistributeGrowToUnfrozen(FlexLine& line, UIView** sorted, bool* frozen,
                                          int32_t freeSpace, uint32_t totalGrow, bool isHorizontal)
{
    bool hasViolation = false;
    int32_t remainingFree = freeSpace;
    uint32_t remainingGrow = totalGrow;
    for (uint16_t j = 0; j < line.lineItemCount; j++) {
        UIView* item = sorted[line.itemStartIndex + j];
        uint16_t grow = item->GetFlexGrow();
        if (frozen[j] || grow == 0) {
            continue;
        }
        int32_t extra = (static_cast<int64_t>(remainingFree) * grow) / remainingGrow;
        remainingFree -= extra;
        remainingGrow -= grow;
        int16_t curMain = isHorizontal ? item->GetWidth() : item->GetHeight();
        int16_t newMain = ClampItemMainSize(item, static_cast<int32_t>(curMain) + extra, isHorizontal);
        if (newMain != curMain + extra) {
            frozen[j] = true;
            hasViolation = true;
        }
        ResizeItemMain(item, newMain, isHorizontal);
    }
    return hasViolation;
}

void FlexLayout::GrowFlexLine(FlexLine& line, UIView** sorted, int16_t containerMainSize,
                              int16_t mainGap, bool isHorizontal)
{
    bool frozenBuf[MAX_COUNT_DEFAULT] = {false};
    bool* frozen = nullptr;
    bool* frozenAlloc = nullptr;
    if (!PrepareFlexLineFrozenItems(line, frozenBuf, frozen, frozenAlloc)) {
        return;
    }

    int32_t gapTotal = (line.lineItemCount > 1) ?
        static_cast<int32_t>(mainGap) * (line.lineItemCount - 1) : 0;
    for (uint16_t loop = 0; loop < line.lineItemCount; loop++) {
        uint32_t totalGrow = FreezeGrowItems(line, sorted, frozen, isHorizontal);
        RecalculateLineMainSize(line, sorted, isHorizontal);
        int32_t freeSpace = static_cast<int32_t>(containerMainSize) - gapTotal - line.totalMainSize;
        if (freeSpace <= 0 || totalGrow == 0) {
            break;
        }
        bool hasViolation = DistributeGrowToUnfrozen(line, sorted, frozen, freeSpace, totalGrow, isHorizontal);
        if (!hasViolation) {
            break;
        }
    }
    RecalculateLineMainSize(line, sorted, isHorizontal);
    if (frozenAlloc != nullptr) {
        delete[] frozenAlloc;
    }
}

int64_t FlexLayout::FreezeShrinkItems(FlexLine& line, UIView** sorted, bool* frozen, bool isHorizontal)
{
    int64_t totalScaledShrink = 0;
    for (uint16_t j = 0; j < line.lineItemCount; j++) {
        UIView* item = sorted[line.itemStartIndex + j];
        int16_t curMain = isHorizontal ? item->GetWidth() : item->GetHeight();
        int16_t clampedMain = ClampItemMainSize(item, curMain, isHorizontal);
        if (clampedMain != curMain) {
            ResizeItemMain(item, clampedMain, isHorizontal);
            frozen[j] = true;
            continue;
        }
        if (!frozen[j] && item->GetFlexShrink() != 0) {
            totalScaledShrink += static_cast<int64_t>(item->GetFlexShrink()) * curMain;
        }
    }
    return totalScaledShrink;
}

bool FlexLayout::DistributeShrinkToUnfrozen(FlexLine& line, UIView** sorted, bool* frozen,
                                            int32_t overflow, int64_t totalScaledShrink, bool isHorizontal)
{
    bool hasViolation = false;
    int32_t remainingOverflow = overflow;
    int64_t remainingScaledShrink = totalScaledShrink;
    for (uint16_t j = 0; j < line.lineItemCount; j++) {
        UIView* item = sorted[line.itemStartIndex + j];
        int16_t curMain = isHorizontal ? item->GetWidth() : item->GetHeight();
        int64_t scaledShrink = static_cast<int64_t>(item->GetFlexShrink()) * curMain;
        if (frozen[j] || scaledShrink == 0) {
            continue;
        }
        int32_t reduction = static_cast<int32_t>(
            (static_cast<int64_t>(remainingOverflow) * scaledShrink) / remainingScaledShrink);
        remainingOverflow -= reduction;
        remainingScaledShrink -= scaledShrink;
        int32_t targetMain = static_cast<int32_t>(curMain) - reduction;
        int16_t newMain = ClampItemMainSize(item, targetMain, isHorizontal);
        if (newMain != targetMain) {
            frozen[j] = true;
            hasViolation = true;
        }
        ResizeItemMain(item, newMain, isHorizontal);
    }
    return hasViolation;
}

void FlexLayout::ShrinkFlexLine(FlexLine& line, UIView** sorted, int16_t containerMainSize,
                                int16_t mainGap, bool isHorizontal)
{
    bool frozenBuf[MAX_COUNT_DEFAULT] = {false};
    bool* frozen = nullptr;
    bool* frozenAlloc = nullptr;
    if (!PrepareFlexLineFrozenItems(line, frozenBuf, frozen, frozenAlloc)) {
        return;
    }

    int32_t gapTotal = (line.lineItemCount > 1) ?
        static_cast<int32_t>(mainGap) * (line.lineItemCount - 1) : 0;
    for (uint16_t loop = 0; loop < line.lineItemCount; loop++) {
        int64_t totalScaledShrink = FreezeShrinkItems(line, sorted, frozen, isHorizontal);
        RecalculateLineMainSize(line, sorted, isHorizontal);
        int32_t overflow = line.totalMainSize + gapTotal - static_cast<int32_t>(containerMainSize);
        if (overflow <= 0 || totalScaledShrink == 0) {
            break;
        }
        bool hasViolation = DistributeShrinkToUnfrozen(line, sorted, frozen, overflow,
                                                       totalScaledShrink, isHorizontal);
        if (!hasViolation) {
            break;
        }
    }
    RecalculateLineMainSize(line, sorted, isHorizontal);
    if (frozenAlloc != nullptr) {
        delete[] frozenAlloc;
    }
}

void FlexLayout::DistributeFlexSpace(FlexLine* lines, uint16_t lineCount, UIView** sorted,
                                     int16_t containerMainSize, bool isHorizontal)
{
    int16_t mainGap = GetMainGap(isHorizontal);
    for (uint16_t li = 0; li < lineCount; li++) {
        lines[li].freeSpace = static_cast<int32_t>(containerMainSize) - lines[li].totalMainSize;
        if (lines[li].freeSpace > 0) {
            GrowFlexLine(lines[li], sorted, containerMainSize, mainGap, isHorizontal);
        } else if (lines[li].freeSpace < 0) {
            ShrinkFlexLine(lines[li], sorted, containerMainSize, mainGap, isHorizontal);
        }
        int32_t gapTotal = (lines[li].lineItemCount > 1) ?
            static_cast<int32_t>(mainGap) * (lines[li].lineItemCount - 1) : 0;
        lines[li].freeSpace = static_cast<int32_t>(containerMainSize) - gapTotal - lines[li].totalMainSize;
    }
}

void FlexLayout::LayoutChildrenPhased(UIView** sorted, uint16_t count, bool isHorizontal)
{
    int16_t containerMainSize = isHorizontal ? GetWidth() : GetHeight();
    int16_t containerCrossSize = isHorizontal ? GetHeight() : GetWidth();

    // Phase 1: Restore auto cross-axis size, then compute flex-basis (and aspect-ratio).
    BeginFlexLayoutSizing(sorted, count);
    RestoreAutoCrossSizes(sorted, count, isHorizontal);
    ComputeFlexBasis(sorted, count, isHorizontal);
    ClampCrossSizes(sorted, count, isHorizontal);

    // Phase 2: Collect items into lines
    // Allocate line storage
    uint16_t maxLines = (wrap_ == WRAP) ? count : 1;
    FlexLine linesBuf[MAX_COUNT_DEFAULT] = {};
    FlexLine* lines = linesBuf;
    FlexLine* linesAlloc = nullptr;
    if (maxLines > MAX_COUNT_DEFAULT) {
        linesAlloc = new FlexLine[maxLines]();
        lines = linesAlloc;
    }

    if (lines != nullptr) {
        uint16_t lineCount = CollectFlexLines(sorted, count, lines, containerMainSize,
                                              containerCrossSize, isHorizontal);

        // Phase 3a: Distribute free space (grow) or shrink overflow
        DistributeFlexSpace(lines, lineCount, sorted, containerMainSize, isHorizontal);

        // Phase 3b: Clamp main sizes as a final safety net after grow/shrink freeze loops.
        ClampMainSizes(sorted, count, isHorizontal);

        // Phase 4: Position items
        PositionItems(lines, lineCount, sorted, containerMainSize, containerCrossSize, isHorizontal);
    }

    for (uint16_t i = 0; i < count; i++) {
        sorted[i]->EndFlexLayoutSizing();
    }
    if (linesAlloc != nullptr) {
        delete[] linesAlloc;
    }
}

void FlexLayout::ApplyJustifyContent(FlexLine& line, int16_t containerMainSize, uint16_t itemCount)
{
    int32_t freeSpace = static_cast<int32_t>(containerMainSize) - line.totalMainSize;
    if (itemCount == 0) {
        line.startPos = 0;
        return;
    }

    if (majorAlign_ == ALIGN_START) {
        line.startPos = 0;
    } else if (majorAlign_ == ALIGN_END) {
        line.startPos = ClampToInt16(freeSpace);
    } else if ((majorAlign_ == ALIGN_CENTER) || (freeSpace < 0) || (itemCount == 1)) {
        line.startPos = ClampToInt16(freeSpace / 2); // 2: half
    } else if (majorAlign_ == ALIGN_EVENLY) {
        int16_t interval = ClampToInt16(freeSpace / static_cast<int32_t>(itemCount + 1));
        line.startPos = interval;
    } else if (majorAlign_ == ALIGN_AROUND) {
        int16_t interval = ClampToInt16(freeSpace / static_cast<int32_t>(itemCount));
        line.startPos = interval / 2; // 2: half
    } else { // ALIGN_BETWEEN
        line.startPos = 0;
    }
}

int16_t FlexLayout::ClampItemCrossSize(UIView* item, int16_t crossSize, bool isHorizontal) const
{
    int16_t minCross = GetItemMinCross(item, isHorizontal);
    int16_t maxCross = GetItemMaxCross(item, isHorizontal);
    if (crossSize < minCross) {
        crossSize = minCross;
    }
    if (maxCross >= 0 && crossSize > maxCross) {
        crossSize = maxCross;
    }
    return (crossSize > 0) ? crossSize : 0;
}

void FlexLayout::ResizeItemCross(UIView* item, int16_t crossSize, bool isHorizontal) const
{
    if (isHorizontal) {
        item->Resize(item->GetWidth(), crossSize);
    } else {
        item->Resize(crossSize, item->GetHeight());
    }
}

int16_t FlexLayout::GetItemCrossPosition(FlexLine& line, UIView* item, uint8_t effectiveAlign,
                                         bool isHorizontal)
{
    int16_t crossMarginStart = GetItemCrossMarginStart(item, isHorizontal);
    int16_t crossMarginEnd = GetItemCrossMarginEnd(item, isHorizontal);
    int16_t itemCrossSize = isHorizontal ? item->GetHeight() : item->GetWidth();
    int16_t crossPaddingBorder = isHorizontal ?
        item->GetStyle(STYLE_PADDING_TOP) + item->GetStyle(STYLE_PADDING_BOTTOM) +
        item->GetStyle(STYLE_BORDER_WIDTH) * 2 : // 2: start and end border
        item->GetStyle(STYLE_PADDING_LEFT) + item->GetStyle(STYLE_PADDING_RIGHT) +
        item->GetStyle(STYLE_BORDER_WIDTH) * 2; // 2: start and end border

    if ((effectiveAlign == ALIGN_STRETCH) || (effectiveAlign == UIView::ALIGN_SELF_STRETCH)) {
        const bool hasExplicitCrossSize = isHorizontal ? item->HasExplicitHeight() : item->HasExplicitWidth();
        if (!hasExplicitCrossSize) {
            int16_t newCrossSize = ClampItemCrossSize(item,
                                                      line.crossSize - crossMarginStart - crossMarginEnd -
                                                      crossPaddingBorder,
                                                      isHorizontal);
            SaveOriginalCrossSizeIfNeeded(item, isHorizontal);
            ResizeItemCross(item, newCrossSize, isHorizontal);
        }
        return line.crossPos;
    }
    if (effectiveAlign == ALIGN_CENTER) {
        return line.crossPos + (line.crossSize - itemCrossSize - crossMarginStart - crossMarginEnd) /
               2 + crossMarginStart; // 2: half
    }
    if (effectiveAlign == ALIGN_END) {
        return line.crossPos + line.crossSize - itemCrossSize - crossMarginEnd;
    }
    return line.crossPos + crossMarginStart;
}

void FlexLayout::PositionFlexItem(UIView* item, int16_t mainPos, int16_t crossPos,
                                  int16_t itemMainSize, int16_t mainAutoOffset,
                                  int16_t containerMainSize, bool isHorizontal)
{
    if (isHorizontal) {
        int16_t actualMainPos = (direction_ == LAYOUT_HOR_R) ?
            containerMainSize - mainPos - itemMainSize - mainAutoOffset : mainPos + mainAutoOffset;
        item->SetPosition(actualMainPos, crossPos);
        return;
    }

    int16_t actualMainPos = (direction_ == LAYOUT_VER_R) ?
        containerMainSize - mainPos - itemMainSize - mainAutoOffset : mainPos + mainAutoOffset;
    item->SetPosition(crossPos, actualMainPos);
}

void FlexLayout::PositionFlexLines(FlexLine* lines, uint16_t lineCount,
                                   int16_t containerCrossSize, int16_t crossGap)
{
    if (lineCount == 0) {
        return;
    }

    int32_t totalCrossSize = 0;
    for (uint16_t li = 0; li < lineCount; li++) {
        totalCrossSize += lines[li].crossSize;
    }
    totalCrossSize += static_cast<int32_t>(crossGap) * (lineCount - 1);
    int32_t freeCrossSpace = static_cast<int32_t>(containerCrossSize) - totalCrossSize;

    if (alignContent_ == ALIGN_CONTENT_END) {
        lines[lineCount - 1].crossPos = containerCrossSize - lines[lineCount - 1].crossSize;
        for (int16_t li = static_cast<int16_t>(lineCount) - 2; li >= 0; li--) {
            lines[li].crossPos = lines[li + 1].crossPos - lines[li].crossSize - crossGap;
        }
        return;
    }

    int16_t gap = 0;
    lines[0].crossPos = 0;
    if (alignContent_ == ALIGN_CONTENT_CENTER) {
        lines[0].crossPos = static_cast<int16_t>(freeCrossSpace / 2); // 2: half
    } else if (alignContent_ == ALIGN_CONTENT_BETWEEN) {
        // W3C: space-between falls back to flex-start on negative free space or a single line.
        if ((freeCrossSpace >= 0) && (lineCount > 1)) {
            gap = static_cast<int16_t>(freeCrossSpace / static_cast<int32_t>(lineCount - 1));
        }
    } else if (alignContent_ == ALIGN_CONTENT_STRETCH) {
        // W3C: stretch splits positive free space equally between all lines; a wrapped
        // container stays multi-line even with one resulting line, so no line-count check
        // here. Negative free space falls back to flex-start and lines are not shrunk.
        if (freeCrossSpace > 0) {
            int32_t extraPerLine = freeCrossSpace / static_cast<int32_t>(lineCount);
            for (uint16_t li = 0; li < lineCount; li++) {
                lines[li].crossSize += static_cast<int16_t>(extraPerLine);
            }
            // The division remainder goes to the last line so the line cross sizes sum
            // exactly to the container inner cross size.
            lines[lineCount - 1].crossSize +=
                static_cast<int16_t>(freeCrossSpace - extraPerLine * static_cast<int32_t>(lineCount));
        }
    }

    for (uint16_t li = 1; li < lineCount; li++) {
        lines[li].crossPos = lines[li - 1].crossPos + lines[li - 1].crossSize + crossGap + gap;
    }
}

uint16_t FlexLayout::CountAutoMargins(FlexLine& line, UIView** sorted, bool isHorizontal) const
{
    uint16_t autoCount = 0;
    for (uint16_t j = 0; j < line.lineItemCount; j++) {
        UIView* item = sorted[line.itemStartIndex + j];
        autoCount += (isHorizontal && item->IsMarginLeftAuto()) ? 1 : 0;
    }
    return autoCount;
}

int16_t FlexLayout::GetJustifyInterval(FlexLine& line, int32_t freeSpace) const
{
    if (line.lineItemCount == 0) {
        return 0;
    }
    if ((freeSpace < 0) || (line.lineItemCount == 1)) {
        return 0;
    }
    if (majorAlign_ == ALIGN_EVENLY) {
        return static_cast<int16_t>(freeSpace / static_cast<int32_t>(line.lineItemCount + 1));
    }
    if (majorAlign_ == ALIGN_AROUND) {
        return static_cast<int16_t>(freeSpace / static_cast<int32_t>(line.lineItemCount));
    }
    if (majorAlign_ == ALIGN_BETWEEN && line.lineItemCount > 1) {
        return static_cast<int16_t>(freeSpace / static_cast<int32_t>(line.lineItemCount - 1));
    }
    return 0;
}

void FlexLayout::PositionFlexLineItems(FlexLine& line, uint16_t lineCount, UIView** sorted,
                                       int16_t containerMainSize, int16_t mainGap, bool isHorizontal)
{
    RecalculateLineMainSize(line, sorted, isHorizontal);
    if (line.lineItemCount > 1) {
        line.totalMainSize += static_cast<int32_t>(mainGap) * (line.lineItemCount - 1);
    }

    int32_t freeSpace = static_cast<int32_t>(containerMainSize) - line.totalMainSize;
    uint16_t autoCount = CountAutoMargins(line, sorted, isHorizontal);
    bool autoConsumed = (freeSpace > 0) && (autoCount > 0);
    int16_t autoValue = autoConsumed ? static_cast<int16_t>(freeSpace / static_cast<int32_t>(autoCount)) : 0;
    int16_t interval = 0;
    int16_t mainPos = 0;
    if (!autoConsumed) {
        ApplyJustifyContent(line, containerMainSize, line.lineItemCount);
        mainPos = line.startPos;
        interval = GetJustifyInterval(line, freeSpace);
    }

    for (uint16_t j = 0; j < line.lineItemCount; j++) {
        UIView* item = sorted[line.itemStartIndex + j];
        int16_t mainMarginStart = GetItemMainMarginStart(item, isHorizontal);
        int16_t mainMarginEnd = GetItemMainMarginEnd(item, isHorizontal);

        bool mainStartAuto = isHorizontal && item->IsMarginLeftAuto();
        int16_t effMainMarginStart = (autoConsumed && mainStartAuto) ? autoValue : mainMarginStart;
        int16_t effMainMarginEnd = mainMarginEnd;
        int16_t mainAutoOffset = (autoConsumed && mainStartAuto) ? autoValue : 0;

        int16_t itemMainSize = isHorizontal ? item->GetWidth() : item->GetHeight();

        uint8_t effectiveAlign = item->GetAlignSelf();
        if (effectiveAlign == UIView::ALIGN_SELF_AUTO) {
            effectiveAlign = secondaryAlign_;
        }

        int16_t crossPos = GetItemCrossPosition(line, item, effectiveAlign, isHorizontal);
        PositionFlexItem(item, mainPos, crossPos, itemMainSize, mainAutoOffset,
                         containerMainSize, isHorizontal);

        mainPos += itemMainSize + effMainMarginStart + effMainMarginEnd + interval;
        if (j + 1 < line.lineItemCount) {
            mainPos += mainGap;
        }
        item->LayoutChildren();
    }
}

void FlexLayout::PositionItems(FlexLine* lines, uint16_t lineCount, UIView** sorted,
                               int16_t containerMainSize, int16_t containerCrossSize, bool isHorizontal)
{
    int16_t mainGap = GetMainGap(isHorizontal);
    int16_t crossGap = GetCrossGap(isHorizontal);

    PositionFlexLines(lines, lineCount, containerCrossSize, crossGap);

    for (uint16_t li = 0; li < lineCount; li++) {
        PositionFlexLineItems(lines[li], lineCount, sorted, containerMainSize, mainGap, isHorizontal);
    }
}

#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT

} // namespace OHOS
