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

#include "draw/draw_label.h"
#include "common/typed_text.h"
#include "draw/draw_utils.h"
#include "engines/gfx/gfx_engine_manager.h"
#include "font/ui_font.h"
#include "font/ui_font_header.h"
#include "gfx_utils/graphic_log.h"

namespace OHOS {
uint16_t DrawLabel::DrawTextOneLine(BufferInfo& gfxDstBuffer, const LabelLineInfo& labelLine, uint16_t& letterIndex)
{
    if (labelLine.text == nullptr) {
        return 0;
    }
    UIFont* fontEngine = UIFont::GetInstance();
    if (labelLine.direct == TEXT_DIRECT_RTL) {
        labelLine.pos.x -= labelLine.offset.x;
    } else {
        labelLine.pos.x += labelLine.offset.x;
    }

    uint32_t i = 0;
    uint16_t retOffsetY = 0; // ret value elipse offsetY
    uint16_t offsetPosY = 0;
    uint8_t maxLetterSize = GetLineMaxLetterSize(labelLine.text, labelLine.lineLength, labelLine.fontId,
                                                 labelLine.fontSize, letterIndex, labelLine.spannableString);
    GlyphNode glyphNode;
    while (i < labelLine.lineLength) {
        uint32_t letter = TypedText::GetUTF8Next(labelLine.text, i, i);
        uint16_t fontId = labelLine.fontId;
        uint8_t fontSize = labelLine.fontSize;
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
        TextStyle textStyle = TEXT_STYLE_NORMAL;
        if (labelLine.textStyles) {
            textStyle = labelLine.textStyles[letterIndex];
        }
#endif
        bool haveLineBackgroundColor = false;
        ColorType lineBackgroundColor;
        bool havebackgroundColor = false;
        ColorType backgroundColor;
        ColorType foregroundColor = labelLine.style.textColor_;

        if (labelLine.spannableString != nullptr && labelLine.spannableString->GetSpannable(letterIndex)) {
            labelLine.spannableString->GetFontId(letterIndex, fontId);
            labelLine.spannableString->GetFontSize(letterIndex, fontSize);
            havebackgroundColor = labelLine.spannableString->GetBackgroundColor(letterIndex, backgroundColor);
            labelLine.spannableString->GetForegroundColor(letterIndex, foregroundColor);
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
            labelLine.spannableString->GetTextStyle(letterIndex, textStyle);
#endif
            haveLineBackgroundColor =
                labelLine.spannableString->GetLineBackgroundColor(letterIndex, lineBackgroundColor);
        }
        LabelLetterInfo letterInfo{labelLine.pos,
                                   labelLine.mask,
                                   foregroundColor,
                                   labelLine.opaScale,
                                   0,
                                   0,
                                   letter,
                                   labelLine.direct,
                                   fontId,
                                   0,
                                   fontSize,
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
                                   textStyle,
#endif
                                   labelLine.baseLine,
                                   labelLine.style.letterSpace_,
                                   labelLine.style.lineSpace_,
                                   havebackgroundColor,
                                   backgroundColor,
                                   haveLineBackgroundColor,
                                   lineBackgroundColor
                                   };
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
        glyphNode.textStyle = letterInfo.textStyle;
#endif
        glyphNode.advance = 0;
        uint8_t* fontMap = fontEngine->GetBitmap(letterInfo.letter, glyphNode, letterInfo.fontId, letterInfo.fontSize,
                                                 letterInfo.shapingId);
        if (fontMap != nullptr) {
            uint8_t weight = fontEngine->GetFontWeight(glyphNode.fontId);
            // 16: rgb565->16 rgba8888->32 font with rgba
            if (weight >= 16) {
                DrawUtils::GetInstance()->DrawColorLetter(gfxDstBuffer, letterInfo, fontMap,
                                                          glyphNode, labelLine.lineHeight);
            } else {
                letterInfo.offsetY = labelLine.ellipsisOssetY == 0 ? offsetPosY : labelLine.ellipsisOssetY;
                retOffsetY = offsetPosY;
                DrawUtils::GetInstance()->DrawNormalLetter(gfxDstBuffer, letterInfo, fontMap, glyphNode, maxLetterSize);
            }
        }
        if (labelLine.direct == TEXT_DIRECT_RTL) {
            labelLine.pos.x -= (glyphNode.advance + labelLine.style.letterSpace_);
        } else {
            labelLine.pos.x += (glyphNode.advance + labelLine.style.letterSpace_);
        }
        letterIndex++;
    }
    return retOffsetY;
}

uint8_t DrawLabel::GetLineMaxLetterSize(const char* text, uint16_t lineLength, uint16_t fontId, uint8_t fontSize,
                                        uint16_t letterIndex, SpannableString* spannableString)
{
    if (spannableString == nullptr) {
        return fontSize;
    }
    uint32_t i = 0;
    uint8_t maxLetterSize = fontSize;
    while (i < lineLength) {
        uint32_t unicode = TypedText::GetUTF8Next(text, i, i);
        if (TypedText::IsColourWord(unicode, fontId, fontSize)) {
            letterIndex++;
            continue;
        }
        if (spannableString != nullptr && spannableString->GetSpannable(letterIndex)) {
            uint8_t tempSize = fontSize;
            spannableString->GetFontSize(letterIndex, tempSize);
            if (tempSize > maxLetterSize) {
                maxLetterSize = tempSize;
            }
        }
        letterIndex++;
    }
    return maxLetterSize;
}

void DrawLabel::DrawArcText(BufferInfo& gfxDstBuffer,
                            const Rect& mask,
                            const char* text,
                            const Point& arcCenter,
                            uint16_t fontId,
                            uint8_t fontSize,
                            const ArcTextInfo arcTextInfo,
                            const float changeAngle,
                            TextOrientation orientation,
                            const Style& style,
                            OpacityType opaScale,
                            bool compatibilityMode)
{
    if ((text == nullptr) || (arcTextInfo.lineStart == arcTextInfo.lineEnd) || (arcTextInfo.radius == 0)) {
        GRAPHIC_LOGE("DrawLabel::DrawArcText invalid parameter\n");
        return;
    }
    OpacityType opa = DrawUtils::GetMixOpacity(opaScale, style.textOpa_);
    if (opa == OPA_TRANSPARENT) {
        return;
    }
    uint16_t letterWidth;
    UIFont* fontEngine = UIFont::GetInstance();

    uint16_t letterHeight = fontEngine->GetHeight(fontId, fontSize);
    uint32_t i = arcTextInfo.lineStart;
    bool orientationFlag = (orientation == TextOrientation::INSIDE);
    bool directFlag = (arcTextInfo.direct == TEXT_DIRECT_LTR);
    bool xorFlag = !directFlag;
    if (compatibilityMode) {
        xorFlag = !((orientationFlag && directFlag) || (!orientationFlag && !directFlag));
    }
    float angle = directFlag ? (arcTextInfo.startAngle + changeAngle) : (arcTextInfo.startAngle - changeAngle);

    float posX;
    float posY;
    float rotateAngle;
    while (i < arcTextInfo.lineEnd) {
        uint32_t tmp = i;
        uint32_t letter = TypedText::GetUTF8Next(text, tmp, i);
        if (letter == 0) {
            continue;
        }
        if ((letter == '\r') || (letter == '\n')) {
            break;
        }
        letterWidth = fontEngine->GetWidth(letter, fontId, fontSize, 0);
        if (!DrawLabel::CalculateAngle(letterWidth, letterHeight, style.letterSpace_,
                                       arcTextInfo, xorFlag, tmp, orientation,
                                       posX, posY, rotateAngle, angle,
                                       arcCenter, compatibilityMode)) {
            continue;
        }

        ArcLetterInfo letterInfo;
        letterInfo.InitData(fontId, fontSize, letter, { MATH_ROUND(posX), MATH_ROUND(posY) },
            static_cast<int16_t>(rotateAngle), style.textColor_, opaScale, arcTextInfo.startAngle,
            arcTextInfo.endAngle, angle, arcTextInfo.radius, compatibilityMode,
            directFlag, orientationFlag, arcTextInfo.hasAnimator);

        DrawLetterWithRotate(gfxDstBuffer, mask, letterInfo, posX, posY);
    }
}

bool DrawLabel::CalculateAngle(uint16_t letterWidth,
                               uint16_t letterHeight,
                               int16_t letterSpace,
                               const ArcTextInfo arcTextInfo,
                               bool xorFlag,
                               uint32_t index,
                               TextOrientation orientation,
                               float& posX,
                               float& posY,
                               float& rotateAngle,
                               float& angle,
                               const Point& arcCenter,
                               bool compatibilityMode)
{
    const int DIVIDER_BY_TWO = 2;
    if (compatibilityMode) {
        if ((index == arcTextInfo.lineStart) && xorFlag) {
            angle += TypedText::GetAngleForArcLen(static_cast<float>(letterWidth), letterHeight, arcTextInfo.radius,
                                                  arcTextInfo.direct, orientation);
        }
        uint16_t arcLen = letterWidth + letterSpace;
        if (arcLen == 0) {
            return false;
        }
        float incrementAngle = TypedText::GetAngleForArcLen(static_cast<float>(arcLen), letterHeight,
                                                            arcTextInfo.radius, arcTextInfo.direct, orientation);

        rotateAngle = (orientation == TextOrientation::INSIDE) ? angle : (angle - SEMICIRCLE_IN_DEGREE);

        // 2: half
        float fineTuningAngle = incrementAngle * (static_cast<float>(letterWidth) / (DIVIDER_BY_TWO * arcLen));
        rotateAngle += (xorFlag ? -fineTuningAngle : fineTuningAngle);
        TypedText::GetArcLetterPos(arcCenter, arcTextInfo.radius, angle, posX, posY);
        angle += incrementAngle;
    } else {
        float incrementAngle = TypedText::GetAngleForArcLen(letterWidth, letterSpace, arcTextInfo.radius);
        if (incrementAngle >= -EPSINON && incrementAngle <= EPSINON) {
            return false;
        }

        float fineTuningAngle = incrementAngle / DIVIDER_BY_TWO;
        rotateAngle = xorFlag ? (angle - SEMICIRCLE_IN_DEGREE - fineTuningAngle) : (angle + fineTuningAngle);
        TypedText::GetArcLetterPos(arcCenter, arcTextInfo.radius, angle, posX, posY);
        angle = xorFlag ? (angle - incrementAngle) : (angle + incrementAngle);
    }

    return true;
}

void DrawLabel::DrawLetterWithRotate(BufferInfo& gfxDstBuffer,
                                     const Rect& mask,
                                     const ArcLetterInfo& letterInfo,
                                     float posX,
                                     float posY)
{
    UIFont* fontEngine = UIFont::GetInstance();
    FontHeader head;
    GlyphNode node;
#if defined(ENABLE_TEXT_STYLE) && ENABLE_TEXT_STYLE
    node.textStyle = TEXT_STYLE_NORMAL;
#endif
    if (fontEngine->GetFontHeader(head, letterInfo.fontId, letterInfo.fontSize) != 0) {
        return;
    }

    const uint8_t* fontMap = fontEngine->GetBitmap(letterInfo.letter, node,
        letterInfo.fontId, letterInfo.fontSize, 0);
    if (fontMap == nullptr) {
        return;
    }
    uint8_t fontWeight = fontEngine->GetFontWeight(letterInfo.fontId);
    ColorMode colorMode = fontEngine->GetColorType(letterInfo.fontId);

    int16_t offset = letterInfo.compatibilityMode ? head.ascender : 0;
    Rect rectLetter;
    rectLetter.Resize(node.cols, node.rows);
    TransformMap transMap(rectLetter);
    // Avoiding errors caused by rounding calculations
    transMap.Translate(Vector2<float>(posX + node.left, posY + offset - node.top));
    transMap.Rotate(letterInfo.rotateAngle, Vector2<float>(posX, posY));

    TransformDataInfo letterTranDataInfo = {ImageHeader{colorMode, 0, 0, 0, node.cols, node.rows}, fontMap, fontWeight,
                                            BlurLevel::LEVEL0, TransformAlgorithm::BILINEAR};

    uint8_t* buffer = nullptr;
    if (letterInfo.hasAnimator) {
        bool inRange = DrawLabel::CalculatedTransformDataInfo(&buffer, letterTranDataInfo, letterInfo);
        if (inRange == false) {
            if (buffer != nullptr) {
                UIFree(buffer);
                buffer = nullptr;
            }
            return;
        }
    }

    BaseGfxEngine::GetInstance()->DrawTransform(gfxDstBuffer, mask, Point { 0, 0 }, letterInfo.color,
        letterInfo.opaScale, transMap, letterTranDataInfo);
    if (buffer != nullptr) {
        UIFree(buffer);
        buffer = nullptr;
    }
}

bool DrawLabel::CalculatedClipAngle(const ArcLetterInfo& letterInfo, float& angle)
{
    if (letterInfo.directFlag) {
        if ((letterInfo.compatibilityMode && letterInfo.orientationFlag) || !letterInfo.compatibilityMode) {
            if (letterInfo.currentAngle > letterInfo.endAngle) {
                angle = letterInfo.currentAngle - letterInfo.endAngle;
            } else if (letterInfo.currentAngle > letterInfo.startAngle) {
                angle = letterInfo.currentAngle - letterInfo.startAngle;
            } else {
                return false;
            }
        } else {
            if (letterInfo.currentAngle > letterInfo.endAngle) {
                angle = letterInfo.currentAngle - letterInfo.endAngle;
            } else if (letterInfo.currentAngle > letterInfo.startAngle) {
                angle = letterInfo.currentAngle - letterInfo.startAngle;
            } else {
                return false;
            }
        }
    } else {
        if (letterInfo.compatibilityMode && letterInfo.orientationFlag) {
            if (letterInfo.currentAngle < letterInfo.endAngle) {
                angle = letterInfo.endAngle - letterInfo.currentAngle;
            } else if (letterInfo.currentAngle < letterInfo.startAngle) {
                angle = letterInfo.startAngle - letterInfo.currentAngle;
            } else {
                return false;
            }
        } else if ((letterInfo.compatibilityMode && !letterInfo.orientationFlag) || !letterInfo.compatibilityMode) {
            if (letterInfo.currentAngle < letterInfo.endAngle) {
                angle = letterInfo.endAngle - letterInfo.currentAngle;
            } else if (letterInfo.currentAngle < letterInfo.startAngle) {
                angle = letterInfo.startAngle - letterInfo.currentAngle;
            } else {
                return false;
            }
        }
    }

    return true;
}

void DrawLabel::OnCalculatedClockwise(const ArcLetterInfo& letterInfo, const uint16_t sizePerPx,
                                      const uint16_t cols, const int16_t offsetX, uint16_t& begin,
                                      uint16_t& copyCols, TextInRange& range)
{
    if (!letterInfo.directFlag) {
        return;
    }
    if ((letterInfo.compatibilityMode && letterInfo.orientationFlag) || !letterInfo.compatibilityMode) {
        if (letterInfo.currentAngle > letterInfo.endAngle) {
            if (offsetX >= cols) {
                range = TextInRange::OUT_RANGE;
            }
            copyCols = cols - offsetX;
        } else if (letterInfo.currentAngle > letterInfo.startAngle) {
            if (offsetX >= cols) {
                range = TextInRange::IN_RANGE;
            }
            copyCols = offsetX;
            begin = (cols - offsetX) * sizePerPx;
        }
    } else {
        if (letterInfo.currentAngle > letterInfo.endAngle) {
            if (offsetX >= cols) {
                range = TextInRange::OUT_RANGE;
            }
            copyCols = cols - offsetX;
            begin = offsetX * sizePerPx;
        } else if (letterInfo.currentAngle > letterInfo.startAngle) {
            if (offsetX >= cols) {
                range = TextInRange::IN_RANGE;
            }
            copyCols = offsetX;
        }
    }
}

void DrawLabel::OnCalculatedAnticlockwise(const ArcLetterInfo& letterInfo, const uint16_t sizePerPx,
                                          const uint16_t cols, const int16_t offsetX, uint16_t& begin,
                                          uint16_t& copyCols, TextInRange& range)
{
    if (letterInfo.directFlag) {
        return;
    }
    if (letterInfo.compatibilityMode && letterInfo.orientationFlag) {
        if (letterInfo.currentAngle < letterInfo.endAngle) {
            if (offsetX >= cols) {
                range = TextInRange::OUT_RANGE;
            }
            copyCols = cols - offsetX;
            begin = offsetX * sizePerPx;
        } else if (letterInfo.currentAngle < letterInfo.startAngle) {
            if (offsetX >= cols) {
                range = TextInRange::IN_RANGE;
            }
            copyCols = offsetX;
        }
    } else if ((letterInfo.compatibilityMode && !letterInfo.orientationFlag) || !letterInfo.compatibilityMode) {
        if (letterInfo.currentAngle < letterInfo.endAngle) {
            if (offsetX >= cols) {
                range = TextInRange::OUT_RANGE;
            }
            copyCols = cols - offsetX;
        } else if (letterInfo.currentAngle < letterInfo.startAngle) {
            if (offsetX >= cols) {
                range = TextInRange::IN_RANGE;
            }
            copyCols = offsetX;
            begin = (cols - offsetX) * sizePerPx;
        }
    }
}

void DrawLabel::CalculatedBeginAndCopySize(const ArcLetterInfo& letterInfo, const uint16_t sizePerPx,
                                           const uint16_t cols, const int16_t offsetX, uint16_t& begin,
                                           uint16_t& copyCols, TextInRange& range)
{
    if (letterInfo.directFlag) {
        OnCalculatedClockwise(letterInfo, sizePerPx, cols, offsetX, begin, copyCols, range);
    } else {
        OnCalculatedAnticlockwise(letterInfo, sizePerPx, cols, offsetX, begin, copyCols, range);
    }
}

bool DrawLabel::CalculatedTransformDataInfo(uint8_t** buffer, TransformDataInfo& letterTranDataInfo,
    const ArcLetterInfo& letterInfo)
{
    float angle = 0.0f;
    if (DrawLabel::CalculatedClipAngle(letterInfo, angle) == false) {
        return false;
    }
    if (angle >= -EPSINON && angle <= EPSINON) {
        return true;
    }

    int16_t offsetX = static_cast<uint16_t>(angle * letterInfo.radius * UI_PI / SEMICIRCLE_IN_DEGREE);
    uint16_t copyCols = 0;
    uint16_t begin = 0;
    uint16_t sizePerPx = letterTranDataInfo.pxSize / 8; // 8 bit
    TextInRange range = TextInRange::NEED_CLIP;
    uint16_t cols = letterTranDataInfo.header.width;
    uint16_t rows = letterTranDataInfo.header.height;
    DrawLabel::CalculatedBeginAndCopySize(letterInfo, sizePerPx, cols, offsetX, begin, copyCols, range);
    if (range == TextInRange::IN_RANGE) {
        return true;
    } else if (range == TextInRange::OUT_RANGE) {
        return false;
    }

    const uint8_t* fontMap = letterTranDataInfo.data;

    uint32_t size = cols * rows * sizePerPx;
    *buffer = static_cast<uint8_t*>(UIMalloc(size));
    if (*buffer == nullptr) {
        return false;
    }

    if (memset_s(*buffer, size, 0, size) != EOK) {
        UIFree(*buffer);
        return false;
    }

    for (uint16_t i = 0; i < rows; i++) {
        uint16_t beginSize = i * cols * sizePerPx + begin;
        uint16_t copySize = copyCols * sizePerPx;
        if (memcpy_s(*buffer + beginSize, copySize, fontMap + beginSize, copySize) != EOK) {
            UIFree(*buffer);
            return false;
        }
    }
    letterTranDataInfo.data = *buffer;
    return true;
}

void DrawLabel::GetLineBackgroundColor(uint16_t letterIndex, List<LineBackgroundColor>* linebackgroundColor,
                                       bool& havelinebackground, ColorType& linebgColor)
{
    if (linebackgroundColor->Size() > 0) {
        ListNode<LineBackgroundColor>* lbColor = linebackgroundColor->Begin();
        for (; lbColor != linebackgroundColor->End(); lbColor = lbColor->next_) {
            uint32_t start = lbColor->data_.start;
            uint32_t end = lbColor->data_.end;
            if (letterIndex >= start && letterIndex <= end) {
                havelinebackground = true;
                linebgColor = lbColor->data_.linebackgroundColor ;
            }
        }
    }
};

void DrawLabel::GetBackgroundColor(uint16_t letterIndex, List<BackgroundColor>* backgroundColor,
                                   bool& havebackground, ColorType& bgColor)
{
    if (backgroundColor->Size() > 0) {
        ListNode<BackgroundColor>* bColor = backgroundColor->Begin();
        for (; bColor != backgroundColor->End(); bColor = bColor->next_) {
            uint16_t start = bColor->data_.start;
            uint16_t end = bColor->data_.end;
            if (letterIndex >= start && letterIndex <= end) {
                havebackground = true;
                bgColor = bColor->data_.backgroundColor ;
            }
        }
    }
};

void DrawLabel::GetForegroundColor(uint16_t letterIndex, List<ForegroundColor>* foregroundColor, ColorType& fgColor)
{
    if (foregroundColor->Size() > 0) {
        ListNode<ForegroundColor>* fColor = foregroundColor->Begin();
        for (; fColor != foregroundColor->End(); fColor = fColor->next_) {
            uint32_t start = fColor->data_.start;
            uint32_t end = fColor->data_.end;
            if (letterIndex >= start && letterIndex <= end) {
                fgColor = fColor->data_.fontColor;
            }
        }
    }
};

void DrawLabel::DrawLineBackgroundColor(BufferInfo& gfxDstBuffer, uint16_t letterIndex, const LabelLineInfo& labelLine)
{
    uint32_t i = 0;
    while (i < labelLine.lineLength) {
        TypedText::GetUTF8Next(labelLine.text, i, i);
        bool havelinebackground = false;
        ColorType linebackgroundColor;
        if (labelLine.spannableString != nullptr &&
            labelLine.spannableString->GetSpannable(letterIndex)) {
            havelinebackground =
                labelLine.spannableString->GetLineBackgroundColor(
                    letterIndex, linebackgroundColor);
        }
        if (havelinebackground) {
            Style style;
            style.bgColor_ = linebackgroundColor;
            Rect linebackground(labelLine.mask.GetLeft(), labelLine.pos.y,
                                labelLine.mask.GetRight(),
                                labelLine.pos.y + labelLine.lineHeight);
            BaseGfxEngine::GetInstance()->DrawRect(gfxDstBuffer, labelLine.mask,
                                                   linebackground, style,
                                                   linebackgroundColor.alpha);
        }
        letterIndex++;
    }
};
} // namespace OHOS
