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

#ifndef GRAPHIC_LITE_SOFT_ENGINE_H
#define GRAPHIC_LITE_SOFT_ENGINE_H

#include "engines/gfx/gfx_engine_manager.h"

namespace OHOS {
class SoftEngine : public BaseGfxEngine {
public:
    void DrawArc(BufferInfo& dst,
                 ArcInfo& arcInfo,
                 const Rect& mask,
                 const Style& style,
                 OpacityType opacity,
                 uint8_t cap) override;

    void DrawLine(BufferInfo& dst,
                  const Point& start,
                  const Point& end,
                  const Rect& mask,
                  int16_t width,
                  ColorType color,
                  OpacityType opacity) override;

    void DrawLetter(BufferInfo& gfxDstBuffer,
                    const uint8_t* fontMap,
                    const Rect& fontRect,
                    const Rect& subRect,
                    const uint8_t fontWeight,
                    const ColorType& color,
                    const OpacityType opa) override;

    void DrawLetter(BufferInfo& gfxDstBuffer,
                    const Rect& subRect,
                    LetterDataInfo& dataInfo) override;

    void DrawLetterPath(BufferInfo& gfxDstBuffer,
                        const Rect& subRect,
                        LetterPathDataInfo& dataInfo,
                        const TransformMap* transMap,
                        const TransformDataInfo* dataInfo) override;

    void DrawLetterPathWithClip(BufferInfo& gfxDstBuffer,
                                const Rect& subRect,
                                const Rect& clipRect,
                                LetterPathDataInfo& dataInfo,
                                const TransformMap* transMap,
                                const TransformDataInfo* dataInfo) override;

    void DrawCubicBezier(BufferInfo& dst,
                         const Point& start,
                         const Point& control1,
                         const Point& control2,
                         const Point& end,
                         const Rect& mask,
                         int16_t width,
                         ColorType color,
                         OpacityType opacity) override;

    void DrawRect(BufferInfo& dst,
                  const Rect& rect,
                  const Rect& dirtyRect,
                  const Style& style,
                  OpacityType opacity) override;

    void DrawTransform(BufferInfo& dst,
                       const Rect& mask,
                       const Point& position,
                       ColorType color,
                       OpacityType opacity,
                       const TransformMap& transMap,
                       const TransformDataInfo& dataInfo) override;

    // x/y: center of a circle
    void ClipCircle(const ImageInfo* info, float x, float y, float radius) override;

    void ClipScreenShape(const ImageInfo* info, float x, float y, float width, float height) override;

    void Blit(BufferInfo& dst,
              const Point& dstPos,
              const BufferInfo& src,
              const Rect& subRect,
              const BlendOption& blendOption) override;

    void Fill(BufferInfo& dst, const Rect& fillArea, const ColorType color, const OpacityType opacity) override;

    void DrawPath(BufferInfo& dst,
                  void* param,
                  const Paint& paint,
                  const Rect& rect,
                  const Rect& invalidatedArea,
                  const Style& style) override;

    void FillPath(BufferInfo& dst,
                  void* param,
                  const Paint& paint,
                  const Rect& rect,
                  const Rect& invalidatedArea,
                  const Style& style) override;

    uint8_t* AllocBuffer(uint32_t size, uint32_t usage) override;

    void FreeBuffer(uint8_t* buffer, uint32_t usage) override;
};
} // namespace OHOS
#endif
