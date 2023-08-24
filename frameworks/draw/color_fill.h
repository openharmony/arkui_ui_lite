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

#ifndef GRAPHIC_LITE_COLOR_FILL_H
#define GRAPHIC_LITE_COLOR_FILL_H
namespace OHOS {
/* cover mode, src alpha is 255 */
#define COLOR_FILL_COVER(d, dm, r2, g2, b2, sm)               \
    if ((dm) == ARGB8888 || (dm) == XRGB8888) {               \
        reinterpret_cast<Color32*>(d)->alpha = OPA_OPAQUE;    \
        if ((sm) == RGB565) {                                   \
            reinterpret_cast<Color32*>(d)->red = (r2) << 3;   \
            reinterpret_cast<Color32*>(d)->green = (g2) << 2; \
            reinterpret_cast<Color32*>(d)->blue = (b2) << 3;  \
        } else {                                              \
            reinterpret_cast<Color32*>(d)->red = (r2);        \
            reinterpret_cast<Color32*>(d)->green = (g2);      \
            reinterpret_cast<Color32*>(d)->blue = (b2);       \
        }                                                     \
    } else if ((dm) == RGB888) {                              \
        if ((sm) == RGB565) {                                   \
            reinterpret_cast<Color24*>(d)->red = (r2) << 3;   \
            reinterpret_cast<Color24*>(d)->green = (g2) << 2; \
            reinterpret_cast<Color24*>(d)->blue = (b2) << 3;  \
        } else {                                              \
            reinterpret_cast<Color24*>(d)->red = (r2);        \
            reinterpret_cast<Color24*>(d)->green = (g2);      \
            reinterpret_cast<Color24*>(d)->blue = (b2);       \
        }                                                     \
    } else if ((dm) == RGB565) {                              \
        if ((sm) == ARGB8888 || (sm) == RGB888 || (sm) == XRGB8888) {  \
            reinterpret_cast<Color16*>(d)->red = (r2) >> 3;   \
            reinterpret_cast<Color16*>(d)->green = (g2) >> 2; \
            reinterpret_cast<Color16*>(d)->blue = (b2) >> 3;  \
        } else {                                              \
            reinterpret_cast<Color16*>(d)->red = (r2);        \
            reinterpret_cast<Color16*>(d)->green = (g2);      \
            reinterpret_cast<Color16*>(d)->blue = (b2);       \
        }                                                     \
    } else {                                                  \
        ASSERT(0);                                            \
    }

#if defined(ENABLE_FIXED_POINT) && ENABLE_FIXED_POINT
#define COLOR_BLEND_RGBA(r1, g1, b1, a1, r2, g2, b2, a2)                                                      \
    const uint16_t Alpha3 = 65025 - (OPA_OPAQUE - (a1)) * (OPA_OPAQUE - (a2));                                \
    if ( Alpha3 != 0 )                                                                                             \
    {                                                                                                         \
        (r1) = static_cast<uint8_t>(((a2) * (r2) * OPA_OPAQUE + (OPA_OPAQUE - (a2)) * (a1) * (r1)) / Alpha3); \
        (g1) = static_cast<uint8_t>(((a2) * (g2) * OPA_OPAQUE + (OPA_OPAQUE - (a2)) * (a1) * (g1)) / Alpha3); \
        (b1) = static_cast<uint8_t>(((a2) * (b2) * OPA_OPAQUE + (OPA_OPAQUE - (a2)) * (a1) * (b1)) / Alpha3); \
        (a1) = static_cast<uint8_t>(Alpha3 / OPA_OPAQUE);                                                     \
    }
#else
#define COLOR_BLEND_RGBA(r1, g1, b1, a1, r2, g2, b2, a2)                                  \
    const float Alpha1 = static_cast<float>(a1) / OPA_OPAQUE;                             \
    const float Alpha2 = static_cast<float>(a2) / OPA_OPAQUE;                             \
    const float Alpha3 = 1 - (1 - Alpha1) * (1 - Alpha2);                                 \
    (r1) = static_cast<uint8_t>((Alpha2 * (r2) + (1 - Alpha2) * Alpha1 * (r1)) / Alpha3); \
    (g1) = static_cast<uint8_t>((Alpha2 * (g2) + (1 - Alpha2) * Alpha1 * (g1)) / Alpha3); \
    (b1) = static_cast<uint8_t>((Alpha2 * (b2) + (1 - Alpha2) * Alpha1 * (b1)) / Alpha3); \
    (a1) = static_cast<uint8_t>(Alpha3 * OPA_OPAQUE)
#endif

#define COLOR_BLEND_XRGB(r1, g1, b1, a1, r2, g2, b2, a2)                               \
    (r1) = (((r2) * (a2)) / OPA_OPAQUE) + (((r1) * (OPA_OPAQUE - (a2))) / OPA_OPAQUE); \
    (g1) = (((g2) * (a2)) / OPA_OPAQUE) + (((g1) * (OPA_OPAQUE - (a2))) / OPA_OPAQUE); \
    (b1) = (((b2) * (a2)) / OPA_OPAQUE) + (((b1) * (OPA_OPAQUE - (a2))) / OPA_OPAQUE); \
    (a1) = static_cast<uint8_t>(OPA_OPAQUE)

#define COLOR_BLEND_RGB(r1, g1, b1, r2, g2, b2, a2)                                    \
    (r1) = (((r2) * (a2)) / OPA_OPAQUE) + (((r1) * (OPA_OPAQUE - (a2))) / OPA_OPAQUE); \
    (g1) = (((g2) * (a2)) / OPA_OPAQUE) + (((g1) * (OPA_OPAQUE - (a2))) / OPA_OPAQUE); \
    (b1) = (((b2) * (a2)) / OPA_OPAQUE) + (((b1) * (OPA_OPAQUE - (a2))) / OPA_OPAQUE)

// 565
#define COLOR_FILL_BLEND(d, dm, s, sm, a)                                                                           \
    if ((dm) == ARGB8888) {                                                                                         \
        Color32* p = reinterpret_cast<Color32*>(d);                                                                 \
        if ((sm) == ARGB8888 || (sm) == XRGB8888) {                                                                 \
            Color32* sTmp = reinterpret_cast<Color32*>(s);                                                          \
            uint8_t alpha = (sTmp->alpha * (a)) / OPA_OPAQUE;                                                       \
            COLOR_BLEND_RGBA(p->red, p->green, p->blue, p->alpha, sTmp->red, sTmp->green, sTmp->blue, alpha);       \
        } else if ((sm) == RGB888) {                                                                                \
            Color24* sTmp = reinterpret_cast<Color24*>(s);                                                          \
            COLOR_BLEND_RGBA(p->red, p->green, p->blue, p->alpha, sTmp->red, sTmp->green, sTmp->blue, a);           \
        } else if ((sm) == RGB565) {                                                                                \
            Color16* sTmp = reinterpret_cast<Color16*>(s);                                                          \
            COLOR_BLEND_RGBA(p->red, p->green, p->blue, p->alpha, (sTmp->red) << 3, (sTmp->green) << 2,             \
                             (sTmp->blue) << 3, a);                                                                 \
        }                                                                                                           \
    } else if ((dm) == XRGB8888) {                                                                                  \
        Color32* p = reinterpret_cast<Color32*>(d);                                                                 \
        if ((sm) == ARGB8888 || (sm) == XRGB8888) {                                                                 \
            Color32* sTmp = reinterpret_cast<Color32*>(s);                                                          \
            uint8_t alpha = (sTmp->alpha * (a)) / OPA_OPAQUE;                                                       \
            COLOR_BLEND_XRGB(p->red, p->green, p->blue, p->alpha, sTmp->red, sTmp->green, sTmp->blue, alpha);       \
        } else if ((sm) == RGB888) {                                                                                \
            Color24* sTmp = reinterpret_cast<Color24*>(s);                                                          \
            COLOR_BLEND_XRGB(p->red, p->green, p->blue, p->alpha, sTmp->red, sTmp->green, sTmp->blue, a);           \
        } else if ((sm) == RGB565) {                                                                                \
            Color16* sTmp = reinterpret_cast<Color16*>(s);                                                          \
            COLOR_BLEND_XRGB(p->red, p->green, p->blue, p->alpha, (sTmp->red) << 3, (sTmp->green) << 2,             \
                             (sTmp->blue) << 3, a);                                                                 \
        }                                                                                                           \
    } else if ((dm) == RGB888) {                                                                                    \
        Color24* p = reinterpret_cast<Color24*>(d);                                                                 \
        if ((sm) == ARGB8888 || (sm) == XRGB8888) {                                                                 \
            Color32* sTmp = reinterpret_cast<Color32*>(s);                                                          \
            uint8_t alpha = (sTmp->alpha * (a)) / OPA_OPAQUE;                                                       \
            COLOR_BLEND_RGB(p->red, p->green, p->blue, sTmp->red, sTmp->green, sTmp->blue, alpha);                  \
        } else if ((sm) == RGB888) {                                                                                \
            Color24* sTmp = reinterpret_cast<Color24*>(s);                                                          \
            COLOR_BLEND_RGB(p->red, p->green, p->blue, sTmp->red, sTmp->green, sTmp->blue, a);                      \
        } else if ((sm) == RGB565) {                                                                                \
            Color16* sTmp = reinterpret_cast<Color16*>(s);                                                          \
            COLOR_BLEND_RGB(p->red, p->green, p->blue, (sTmp->red) << 3, (sTmp->green) << 2, (sTmp->blue) << 3, a); \
        }                                                                                                           \
    } else if ((dm) == RGB565) {                                                                                    \
        Color16* p = reinterpret_cast<Color16*>(d);                                                                 \
        if ((sm) == ARGB8888 || (sm) == XRGB8888) {                                                                 \
            Color32* sTmp = reinterpret_cast<Color32*>(s);                                                          \
            uint8_t alpha = (sTmp->alpha * (a)) / OPA_OPAQUE;                                                       \
            COLOR_BLEND_RGB(p->red, p->green, p->blue, (sTmp->red) >> 3, (sTmp->green) >> 2, (sTmp->blue) >> 3,     \
                            alpha);                                                                                 \
        } else if ((sm) == RGB888) {                                                                                \
            Color24* sTmp = reinterpret_cast<Color24*>(s);                                                          \
            COLOR_BLEND_RGB(p->red, p->green, p->blue, (sTmp->red) >> 3, (sTmp->green) >> 2, (sTmp->blue) >> 3, a); \
        } else if ((sm) == RGB565) {                                                                                \
            Color16* sTmp = reinterpret_cast<Color16*>(s);                                                          \
            COLOR_BLEND_RGB(p->red, p->green, p->blue, sTmp->red, sTmp->green, sTmp->blue, a);                      \
        }                                                                                                           \
    } else {                                                                                                        \
        ASSERT(0);                                                                                                  \
    }
}
#endif