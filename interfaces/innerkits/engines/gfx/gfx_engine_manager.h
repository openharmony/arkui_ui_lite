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

#ifndef GRAPHIC_LITE_GFX_ENGINE_MANAGER_H
#define GRAPHIC_LITE_GFX_ENGINE_MANAGER_H

#include "gfx_utils/color.h"
#include "gfx_utils/diagram/common/paint.h"
#include "gfx_utils/graphic_buffer.h"
#include "gfx_utils/graphic_types.h"
#include "gfx_utils/image_info.h"
#include "gfx_utils/style.h"
#include "gfx_utils/transform.h"

namespace OHOS {
class BaseGfxEngine;
enum BlendMode {
    BLEND_MODE,     /* no blending */
    BLEND_SRC,      /* S */
    BLEND_DST,      /* D */
    BLEND_SRC_OVER, /* S + (1 - Sa) * D */
    BLEND_DST_OVER, /* (1 - Da) * S + D */
    BLEND_SRC_IN,   /* Da * S */
    BLEND_DST_IN,   /* Sa * D */
    BLEND_SRC_OUT,  /* S * (1 - Da) */
    BLEND_DST_OUT,  /* D * (1 - Sa) */
    BLEND_SCREEN,   /* S + D - S * D */
    BLEND_MULTIPLY, /* S * (1 - Da) + D * (1 - Sa) + S * D */
    BLEND_ADDITIVE, /* S + D */
    BLEND_SUBTRACT, /* D * (1 - S) */
};

#ifndef TRANSFORMOPTION
#define TRANSFORMOPTION
struct TransformOption {
    TransformAlgorithm algorithm;
};
#endif

struct BlendOption {
    TransformMap transMap;
    BlendMode mode;
    TransformOption option;
    OpacityType opacity;
};

class Image;
struct ArcInfo {
    Point center;
    Point imgPos;
    uint16_t radius;
    float startAngle;
    float endAngle;
    const Image* imgSrc;
};

struct TransformDataInfo {
    ImageHeader header;
    const uint8_t* data;
    uint8_t pxSize;
    BlurLevel blurLevel;
    TransformAlgorithm algorithm;
};

struct LetterDataInfo {
    const uint8_t* fontMap;
    Rect fontRect;
    const uint8_t fontWeight;
    const ColorType color;
    const OpacityType opa;
    const int16_t fontStrokeWidth;
    const ColorType fontStrokeColor;
};

struct LetterPathDataInfo {
    uint32_t cmdLen;
    uint8_t* cmds;
    uint32_t pathLen;
    float* points;
    Rect fontRect;
    const ColorType color;
    const OpacityType opa;
    const int16_t fontStrokeWidth;
    const ColorType fontStrokeColor;
    float x;
    float y;
};

enum BufferInfoUsage { BUFFER_FB_SURFACE, BUFFER_MAP_SURFACE, BUFFER_SNAPSHOT_SURFACE };

class BaseGfxEngine : public HeapBase {
public:
    virtual void DrawArc(BufferInfo& dst,
                         ArcInfo& arcInfo,
                         const Rect& mask,
                         const Style& style,
                         OpacityType opacity,
                         uint8_t cap) = 0;

    virtual void MemoryBarrier(void* addr = nullptr, int len = 0) {}

    virtual void DrawLine(BufferInfo& dst,
                          const Point& start,
                          const Point& end,
                          const Rect& mask,
                          int16_t width,
                          ColorType color,
                          OpacityType opacity) = 0;

    virtual void DrawLetter(BufferInfo& gfxDstBuffer,
                            const uint8_t* fontMap,
                            const Rect& fontRect,
                            const Rect& subRect,
                            const uint8_t fontWeight,
                            const ColorType& color,
                            const OpacityType opa) = 0;

    virtual void DrawLetter(BufferInfo& gfxDstBuffer,
                            const Rect& subRect,
                            LetterDataInfo& dataInfo) = 0;

    virtual void DrawLetterPath(BufferInfo& gfxDstBuffer,
                                const Rect& subRect,
                                LetterPathDataInfo& dataInfo,
                                const TransformMap* transMap,
                                const TransformDataInfo* transDataInfo) = 0;

    virtual void DrawLetterPathWithClip(BufferInfo& dst,
                                        const Rect& subRect,
                                        const Rect& clipRect,
                                        LetterPathDataInfo& dataInfo,
                                        const TransformMap* transMap,
                                        const TransformDataInfo* transDataInfo) = 0;

    virtual void DrawCubicBezier(BufferInfo& dst,
                                 const Point& start,
                                 const Point& control1,
                                 const Point& control2,
                                 const Point& end,
                                 const Rect& mask,
                                 int16_t width,
                                 ColorType color,
                                 OpacityType opacity) = 0;

    virtual void
        DrawRect(BufferInfo& dst, const Rect& rect, const Rect& dirtyRect, const Style& style, OpacityType opacity) = 0;

    virtual void DrawTransform(BufferInfo& dst,
                               const Rect& mask,
                               const Point& position,
                               ColorType color,
                               OpacityType opacity,
                               const TransformMap& transMap,
                               const TransformDataInfo& dataInfo) = 0;

    // x/y: center of a circle
    virtual void ClipCircle(const ImageInfo* info, float x, float y, float radius) = 0;

    virtual void ClipScreenShape(const ImageInfo* info, float x, float y, float width, float height) = 0;

    virtual void ClipRadius(const ImageInfo* info, float radius)
    {
        return;
    }

    virtual void Blit(BufferInfo& dst,
                      const Point& dstPos,
                      const BufferInfo& src,
                      const Rect& subRect,
                      const BlendOption& blendOption) = 0;

    virtual void Fill(BufferInfo& dst, const Rect& fillArea, const ColorType color, const OpacityType opacity) = 0;

    virtual void DrawPath(BufferInfo& dst,
                          void* param,
                          const Paint& paint,
                          const Rect& rect,
                          const Rect& invalidatedArea,
                          const Style& style) = 0;

    virtual void FillPath(BufferInfo& dst,
                          void* param,
                          const Paint& paint,
                          const Rect& rect,
                          const Rect& invalidatedArea,
                          const Style& style) = 0;

    virtual void FlushPath(BufferInfo& dst, const Rect& mask, const ColorType color,
        const OpacityType opacity, int16_t lineWidth = 0)
    {
        return;
    }

    virtual uint8_t* AllocBuffer(uint32_t size, uint32_t usage) = 0;

    virtual void FreeBuffer(uint8_t* buffer, uint32_t usage) = 0;

    virtual BufferInfo* GetFBBufferInfo()
    {
        return nullptr;
    }

    virtual void AdjustLineStride(BufferInfo& info) {}

    virtual void CalcImageDataSize(ImageInfo& info) {}

    virtual void *GetRenderBufferAddress()
    {
        return nullptr;
    }

    virtual void Flush(const Rect& flushRect)
    {
        (void)flushRect;
    }

    virtual CompressMode GetSnapShotDefaultCompressMode()
    {
        return COMPRESS_MODE_NONE;
    }

    virtual ColorMode GetSnapShotDefaultColorMode()
    {
        return ARGB8888;
    }

    virtual void CheckComposeTexture() {}

    virtual uint16_t GetScreenWidth()
    {
        return screenWidth_;
    }

    virtual uint16_t GetScreenHeight()
    {
        return screenHeight_;
    }

    virtual void SetScreenShape(ScreenShape screenShape)
    {
        screenShape_ = screenShape;
    }

    virtual ScreenShape GetScreenShape()
    {
        return screenShape_;
    }

    virtual void StartDoubleBufferRender() {}

    virtual void StopDoubleBufferRender() {}

    virtual int16_t AlignImageWidth(int16_t imageWidth)
    {
        return imageWidth;
    }

    static BaseGfxEngine* GetInstance()
    {
        return baseEngine_;
    }

    static void InitGfxEngine(BaseGfxEngine* gfxEngine)
    {
        baseEngine_ = gfxEngine;
    }

protected:
    static BaseGfxEngine* baseEngine_;
    uint16_t screenWidth_ = HORIZONTAL_RESOLUTION;
    uint16_t screenHeight_ = VERTICAL_RESOLUTION;
    ScreenShape screenShape_ = RECTANGLE;
};
} // namespace OHOS

#endif // GRAPHIC_LITE_GFX_ENGINE_MANAGER_H
