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

/**
 * @addtogroup UI_Layout
 * @{
 *
 * @brief Defines UI layouts such as <b>FlexLayout</b> and <b>GridLayout</b>.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file flex_layout.h
 *
 * @brief Declares a flexible layout container. You can perform simple adaptive layout on child views that the
 *        container holds, for example, to evenly arrange all child views in the same row or column.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef GRAPHIC_LITE_FLEX_LAYOUT_H
#define GRAPHIC_LITE_FLEX_LAYOUT_H

#include "layout.h"

namespace OHOS {
/**
 * @brief Defines a flexible layout container. You can perform simple adaptive layout on child views that the
 *        container holds, for example, to evenly arrange all child views in the same row or column.
 *
 * @since 1.0
 * @version 1.0
 */
class FlexLayout : public Layout {
public:
    static constexpr uint8_t NOWRAP = 0;
    static constexpr uint8_t WRAP = 1;

    /**
     * @brief A default constructor used to create a <b>FlexLayout</b> instance.
     * @since 1.0
     * @version 1.0
     */
#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
    FlexLayout()
        : majorAlign_(ALIGN_START), secondaryAlign_(ALIGN_CENTER), wrap_(NOWRAP), rowCount_(1), columnCount_(1),
          alignContent_(ALIGN_CONTENT_STRETCH), lastAlignContent_(ALIGN_CONTENT_STRETCH), rowGap_(0), columnGap_(0),
          rowGapPercent_(0), columnGapPercent_(0),
          isRowGapPercent_(false), isColumnGapPercent_(false), gapBaseWidthAuto_(false), gapBaseHeightAuto_(false),
          lastDirection_(LAYOUT_HOR)
    {
    }
#else
    FlexLayout()
        : majorAlign_(ALIGN_START), secondaryAlign_(ALIGN_CENTER), wrap_(NOWRAP), rowCount_(1), columnCount_(1)
    {
    }
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT

    /**
     * @brief A destructor used to delete the <b>FlexLayout</b> instance.
     * @since 1.0
     * @version 1.0
     */
    virtual ~FlexLayout() {}

#if defined(CONFIG_DYNAMIC_LAYOUT) && (CONFIG_DYNAMIC_LAYOUT == 1)
    /**
     * @brief Obtains the view type.
     *
     * @return Returns <b>UI_FLEXLAYOUT</b>, as defined in {@link UIViewType}.
     * @since 1.0
     * @version 1.0
     */
    UIViewType GetViewType() const override
    {
        return UI_FLEXLAYOUT;
    }
#endif

    /**
     * @brief Sets the alignment mode of the primary axis (the axis where the layout direction is located).
     *        The child views in the layout are placed in this mode in the direction of the primary axis.
     * @param align Indicates the alignment mode to set. The value can be <b>ALIGN_START</b>, <b>ALIGN_END</b>,
     *              <b>ALIGN_CENTER</b>, <b>ALIGN_EVENLY</b>, <b>ALIGN_AROUND</b>, or <b>ALIGN_BETWEEN</b>.
     * @since 1.0
     * @version 1.0
     */
    void SetMajorAxisAlign(const AlignType& align)
    {
        majorAlign_ = align;
    }

    /**
     * @brief Sets the alignment mode of the secondary axis (the axis perpendicular to the set layout direction).
     * @param align Indicates the alignment mode to set. The value can be <b>ALIGN_START</b>, <b>ALIGN_CENTER</b>,
     *              <b>ALIGN_END</b>, or <b>ALIGN_STRETCH</b>.
     * @since 1.0
     * @version 1.0
     */
    void SetSecondaryAxisAlign(const AlignType& align)
    {
        secondaryAlign_ = align;
    }

    /**
     * @brief Sets whether to support word wrap.
     * @param wrap Indicates the word wrap attribute.
     * @since 1.0
     * @version 1.0
     */
    void SetFlexWrap(uint8_t wrap)
    {
        wrap_ = wrap;
    }

#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
    void SetGap(int16_t gap)
    {
        SetRowGap(gap);
        SetColumnGap(gap);
    }

    void SetRowGap(int16_t gap)
    {
        rowGap_ = (gap < 0) ? 0 : gap;
        isRowGapPercent_ = false;
    }

    int16_t GetRowGap() const
    {
        return rowGap_;
    }

    void SetColumnGap(int16_t gap)
    {
        columnGap_ = (gap < 0) ? 0 : gap;
        isColumnGapPercent_ = false;
    }

    int16_t GetColumnGap() const
    {
        return columnGap_;
    }

    void SetRowGapPercent(float percent)
    {
        rowGapPercent_ = percent;
        isRowGapPercent_ = true;
    }

    void SetColumnGapPercent(float percent)
    {
        columnGapPercent_ = percent;
        isColumnGapPercent_ = true;
    }

    void SetGapBaseAuto(bool widthAuto, bool heightAuto)
    {
        gapBaseWidthAuto_ = widthAuto;
        gapBaseHeightAuto_ = heightAuto;
    }

    /**
     * @brief Sets the alignment mode for multi-line cross-axis distribution (CSS align-content).
     * @param align Indicates the alignment mode. The value can be <b>ALIGN_CONTENT_START</b>,
     *              <b>ALIGN_CONTENT_CENTER</b>, <b>ALIGN_CONTENT_END</b>, <b>ALIGN_CONTENT_STRETCH</b>,
     *              or <b>ALIGN_CONTENT_BETWEEN</b>.
     */
    void SetAlignContent(AlignContentType align)
    {
        if (alignContent_ == align) {
            return;
        }
        alignContent_ = align;
        Invalidate();
    }

    /**
     * @brief Obtains the current align-content mode.
     * @return Returns the align-content mode.
     */
    AlignContentType GetAlignContent() const
    {
        return alignContent_;
    }
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT

    /**
     * @brief Lays out all child views according to the preset arrangement mode.
     * @param needInvalidate Specifies whether to refresh the invalidated area after the layout is complete.
     *                       Value <b>true</b> means to refresh the invalidated area after the layout is complete,
     *                       and <b>false</b> means the opposite.
     * @since 1.0
     * @version 1.0
     */
    void LayoutChildren(bool needInvalidate = false) override;

private:
    // Legacy layout methods (used when all flex item props are defaults)
    void LayoutHorizontal();
    void LayoutVertical();
    void CalValidLength(uint16_t& totalValidLength, uint16_t& allChildNum);
    void GetStartPos(const int16_t& length, int16_t& pos, int16_t& interval, int16_t count,
        uint16_t* validLengths, uint16_t* childsNum);
    void GetNoWrapStartPos(const int16_t& length, int16_t& majorPos, int16_t& interval);
    void CalRowCount();
    void GetRowMaxHeight(uint16_t size, uint16_t* maxRosHegiht);
    void GetRowsWidth(uint16_t rowNum, uint16_t* rowsWidth, uint16_t* rowsChildNum);
    void GetRowStartPos(int16_t& pos, int16_t& interval, int16_t count,
        uint16_t* rowsWidth, uint16_t* rowsChildNum);
    void CalColumnCount();
    void GetColumnMaxWidth(uint16_t size, uint16_t* maxColumnsWidth);
    void GetColumnsHeight(uint16_t columnNum, uint16_t* columnsHeight, uint16_t* columnsChildNum);
    void GetColumnStartPos(int16_t& pos, int16_t& interval, int16_t count,
        uint16_t* columnsHeight, uint16_t* columnsChildNum);
    void GetCrossAxisPosY(int16_t& posY, uint16_t& count, uint16_t* rowsMaxHeight, UIView* child);
    void GetCrossAxisPosX(int16_t& posX, uint16_t& count, uint16_t* columnsMaxWidth, UIView* child);
#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
    void ResolveGaps();
    static int16_t ResolveGapPercent(float percent, bool isAuto, int16_t base);
    void LayoutChildrenWithoutFlexSizing(bool needInvalidate);
    void LayoutChildrenWithFlexSizing(bool needInvalidate, bool restoredCrossSize);
    void CountVisibleChildren(uint16_t& inFlowCount, uint16_t& absCount) const;
    void FillLayoutChildren(UIView** sorted, UIView** absChildren);
    bool HasPhasedLayoutNeed(bool hasMultiLineAlignContent) const;
    bool HasAdvancedLayoutNeed(UIView** sorted, uint16_t inFlowCount,
                               uint16_t absCount, bool forceAdvancedLayout) const;
    void LayoutPreparedChildren(UIView** sorted, uint16_t inFlowCount, UIView** absChildren,
                                uint16_t absCount, bool forceAdvancedLayout, bool isHorizontal);
    void LayoutAdvancedChildren(UIView** sorted, uint16_t inFlowCount, UIView** absChildren,
                                uint16_t absCount, bool isHorizontal);
    void LayoutFlexBasis(UIView* child, int16_t basis, bool isHorizontal);
    bool ResolveFlexBasis(UIView* child, int16_t& basis) const;
    void LayoutAspectRatioBasis(UIView* child, bool isHorizontal);
    void LayoutAutoBasis(UIView* child, bool isHorizontal);
    // Internal structure representing a flex line (row or column)
    struct FlexLine {
        uint16_t lineItemCount;
        int32_t totalMainSize;
        int16_t crossSize;
        int16_t startPos;
        int16_t crossPos;
        int32_t freeSpace;
        uint16_t itemStartIndex;

        FlexLine()
            : lineItemCount(0), totalMainSize(0), crossSize(0),
              startPos(0), crossPos(0), freeSpace(0), itemStartIndex(0) {}
    };

    // Phased layout algorithm methods
    void LayoutChildrenPhased(UIView** sorted, uint16_t count, bool isHorizontal);
    void BeginFlexLayoutSizing(UIView** sorted, uint16_t count);
    void ComputeFlexBasis(UIView** sorted, uint16_t count, bool isHorizontal);
    uint16_t CollectFlexLines(UIView** sorted, uint16_t count, FlexLine* lines,
                              int16_t containerMainSize, int16_t containerCrossSize, bool isHorizontal);
    void DistributeFlexSpace(FlexLine* lines, uint16_t lineCount, UIView** sorted,
                             int16_t containerMainSize, bool isHorizontal);
    void GrowFlexLine(FlexLine& line, UIView** sorted, int16_t containerMainSize,
                      int16_t mainGap, bool isHorizontal);
    void ShrinkFlexLine(FlexLine& line, UIView** sorted, int16_t containerMainSize,
                        int16_t mainGap, bool isHorizontal);
    bool PrepareFlexLineFrozenItems(FlexLine& line, bool* frozenBuf,
                                    bool*& frozen, bool*& frozenAlloc) const;
    uint32_t FreezeGrowItems(FlexLine& line, UIView** sorted, bool* frozen, bool isHorizontal);
    bool DistributeGrowToUnfrozen(FlexLine& line, UIView** sorted, bool* frozen,
                                  int32_t freeSpace, uint32_t totalGrow, bool isHorizontal);
    int64_t FreezeShrinkItems(FlexLine& line, UIView** sorted, bool* frozen, bool isHorizontal);
    bool DistributeShrinkToUnfrozen(FlexLine& line, UIView** sorted, bool* frozen,
                                    int32_t overflow, int64_t totalScaledShrink, bool isHorizontal);
    void RecalculateLineMainSize(FlexLine& line, UIView** sorted, bool isHorizontal);
    void ResizeItemMain(UIView* item, int16_t mainSize, bool isHorizontal) const;
    int16_t ClampItemMainSize(UIView* item, int32_t mainSize, bool isHorizontal) const;
    int16_t ClampItemCrossSize(UIView* item, int16_t crossSize, bool isHorizontal) const;
    void ResizeItemCross(UIView* item, int16_t crossSize, bool isHorizontal) const;
    int16_t GetItemCrossPosition(FlexLine& line, UIView* item, uint8_t effectiveAlign,
                                 bool isHorizontal);
    void PositionFlexItem(UIView* item, int16_t mainPos, int16_t crossPos,
                          int16_t itemMainSize, int16_t mainAutoOffset,
                          int16_t containerMainSize, bool isHorizontal);
    void PositionFlexLines(FlexLine* lines, uint16_t lineCount,
                           int16_t containerCrossSize, int16_t crossGap);
    uint16_t CountAutoMargins(FlexLine& line, UIView** sorted, bool isHorizontal) const;
    int16_t GetJustifyInterval(FlexLine& line, int32_t freeSpace) const;
    void PositionFlexLineItems(FlexLine& line, uint16_t lineCount, UIView** sorted,
                               int16_t containerMainSize, int16_t mainGap, bool isHorizontal);
    void PositionItems(FlexLine* lines, uint16_t lineCount, UIView** sorted,
                       int16_t containerMainSize, int16_t containerCrossSize, bool isHorizontal);
    void ApplyJustifyContent(FlexLine& line, int16_t containerMainSize, uint16_t itemCount);
    int16_t GetItemMainSize(UIView* child, bool isHorizontal) const;
    int16_t GetItemCrossSize(UIView* child, bool isHorizontal) const;
    int16_t GetItemMainMarginStart(UIView* child, bool isHorizontal) const;
    int16_t GetItemMainMarginEnd(UIView* child, bool isHorizontal) const;
    int16_t GetItemCrossMarginStart(UIView* child, bool isHorizontal) const;
    int16_t GetItemCrossMarginEnd(UIView* child, bool isHorizontal) const;
    bool HasFlexItemProperties(UIView** sorted, uint16_t count) const;
    // Full CSS Flex support additions
    int16_t GetMainGap(bool isHorizontal) const;
    int16_t GetCrossGap(bool isHorizontal) const;
    int16_t GetItemMinMain(UIView* child, bool isHorizontal) const;
    int16_t GetItemMaxMain(UIView* child, bool isHorizontal) const;
    int16_t GetItemMinCross(UIView* child, bool isHorizontal) const;
    int16_t GetItemMaxCross(UIView* child, bool isHorizontal) const;
    void ClampMainSizes(UIView** sorted, uint16_t count, bool isHorizontal);
    void RestoreAutoCrossSizes(UIView** sorted, uint16_t count, bool isHorizontal);
    void ClampCrossSizes(UIView** sorted, uint16_t count, bool isHorizontal);
    bool IsAbsoluteContainingBlock(UIView* view) const;
    UIView* FindAbsoluteContainingBlock(UIView* item) const;
    UIView* GetInitialContainingBlock(UIView* item) const;
    void PositionAbsoluteChildren(UIView** absChildren, uint16_t absCount);
    bool IsCrossStretchItem(UIView* item) const;
    void SaveOriginalCrossSizeIfNeeded(UIView* item, bool isHorizontal) const;
    void RestoreOriginalCrossSizeIfNeeded(UIView* item, bool isHorizontal) const;
    bool RestoreChildrenOriginalSizeIfNeeded();
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT
    static constexpr uint16_t MAX_COUNT_DEFAULT = 100;
    AlignType majorAlign_;
    AlignType secondaryAlign_;
    uint8_t wrap_;
    uint16_t rowCount_;
    uint16_t columnCount_;
#if (GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT == 1)
    AlignContentType alignContent_;
    AlignContentType lastAlignContent_;
    int16_t rowGap_;
    int16_t columnGap_;
    float rowGapPercent_;
    float columnGapPercent_;
    bool isRowGapPercent_ : 1;
    bool isColumnGapPercent_ : 1;
    bool gapBaseWidthAuto_ : 1;
    bool gapBaseHeightAuto_ : 1;
    uint8_t lastDirection_;
#endif // GRAPHIC_ENABLE_FLEX_LAYOUT_ENHANCEMENT
};
} // namespace OHOS
#endif // GRAPHIC_LITE_FLEX_LAYOUT_H
