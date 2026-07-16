/*
 * Copyright (c) 2026 HiSilicon (Shanghai) Technologies Co., Ltd.
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

#ifndef GRAPHIC_LITE_HI3519_ENGINE_H
#define GRAPHIC_LITE_HI3519_ENGINE_H

#include "engines/gfx/soft_engine.h"
#include "gfx_utils/pixel_format_utils.h"

namespace OHOS {
class Hi3519GfxEngine : public SoftEngine {
public:
    void Blit(BufferInfo& dst,
                const Point& dstPos,
                const BufferInfo& src,
                const Rect& subRect,
                const BlendOption& blendOption) override;

    void Fill(BufferInfo& dst,
              const Rect& fillArea,
              const ColorType color,
              const OpacityType opacity) override;

    void DrawTransform(BufferInfo& dst,
                            const Rect& mask,
                            const Point& position,
                            ColorType color,
                            OpacityType opacity,
                            const TransformMap& transMap,
                            const TransformDataInfo& dataInfo) override;
private:
    ImagePixelFormat GetPixelFormatByColorMode(ColorMode colorMode);
};
}
#endif // GRAPHIC_LITE_HI3519_ENGINE_H