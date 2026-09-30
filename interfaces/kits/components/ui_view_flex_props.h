/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef GRAPHIC_LITE_UI_VIEW_FLEX_PROPS_H
#define GRAPHIC_LITE_UI_VIEW_FLEX_PROPS_H

#include <cstdint>

struct FlexItemProps {
    uint16_t flexGrow;
    uint16_t flexShrink;
    int16_t flexBasis;
    int16_t minWidth;
    int16_t maxWidth;
    int16_t minHeight;
    int16_t maxHeight;
    uint16_t aspectRatio;
    uint8_t positionType;
    int16_t flexLeft;
    int16_t flexRight;
    int16_t flexTop;
    int16_t flexBottom;
    float flexLeftPercent;
    float flexRightPercent;
    float flexTopPercent;
    float flexBottomPercent;
    uint8_t marginAutoFlags;
    uint8_t flexInsetFlags;
    uint8_t flexInsetPercentFlags;
    int16_t autoBasisWidth;
    int16_t autoBasisHeight;
    uint8_t autoBasisWidthValid : 1;
    uint8_t autoBasisHeightValid : 1;
    uint8_t layoutSizing : 1;
    uint8_t flexShrinkSet : 1;

    FlexItemProps()
        : flexGrow(0),
          flexShrink(0),
          flexBasis(-1),
          minWidth(-1),
          maxWidth(-1),
          minHeight(-1),
          maxHeight(-1),
          aspectRatio(0),
          positionType(0),
          flexLeft(-1),
          flexRight(-1),
          flexTop(-1),
          flexBottom(-1),
          flexLeftPercent(-1.0f),
          flexRightPercent(-1.0f),
          flexTopPercent(-1.0f),
          flexBottomPercent(-1.0f),
          marginAutoFlags(0),
          flexInsetFlags(0),
          flexInsetPercentFlags(0),
          autoBasisWidth(0),
          autoBasisHeight(0),
          autoBasisWidthValid(false),
          autoBasisHeightValid(false),
          layoutSizing(false),
          flexShrinkSet(false)
    {}
};

#endif // GRAPHIC_LITE_UI_VIEW_FLEX_PROPS_H
