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
 * @addtogroup UI_Components
 * @{
 *
 * @brief Defines UI components such as buttons, texts, images, lists, and progress bars.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file ui_slider.h
 *
 * @brief Defines the attributes and common functions of a slider.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef GRAPHIC_LITE_UI_SLIDER_H
#define GRAPHIC_LITE_UI_SLIDER_H

#include "common/image.h"
#include "components/ui_box_progress.h"

namespace OHOS {
/**
 * @brief Represents a slider.
 *
 * Users can drag or click the knob to adjust the progress of an event.
 *
 * @see UIBoxProgress
 * @since 1.0
 * @version 1.0
 */
class UISlider : public UIBoxProgress {
public:
    /**
     * @brief A constructor used to create a <b>UISlider</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    UISlider();

    /**
     * @brief A destructor used to delete the <b>UISlider</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    virtual ~UISlider();

    /**
     * @brief Obtains the view type.
     *
     * @return Returns the view type, as defined in {@link UIViewType}.
     * @since 1.0
     * @version 1.0
     */
    UIViewType GetViewType() const override
    {
        return UI_SLIDER;
    }

    /**
     * @brief Sets the width for this knob.
     *
     * The width of this knob is the same as its height. \n
     * By default, the width of this knob is the same as the height of the horizontal slider
     * or the width of the vertical slider. \n
     *
     * @param width Indicates the knob width to set.
     * @see GetKnobWidth
     * @since 1.0
     * @version 1.0
     */
    void SetKnobWidth(int16_t width)
    {
        knobWidth_ = width;
    }

    /**
     * @brief Obtains the knob width.
     *
     * @return Returns the knob width.
     * @see SetKnobWidth
     * @since 1.0
     * @version 1.0
     */
    int16_t GetKnobWidth()
    {
        return knobWidth_;
    }

    /**
     * @brief Sets the image as pixel maps for this slider's knob.
     *
     * @param knobImage Indicates the knob image to set.
     * @since 6
     */
    void SetKnobImage(const ImageInfo* knobImage);

    /**
     * @brief Sets the image for this slider's knob.
     *
     * @param knobImage Indicates the knob image to set.
     * @since 6
     */
    void SetKnobImage(const char* knobImage);

    /**
     * @brief Sets the color for this slider's knob.
     *
     * @param knobColor Indicates the knob color to set.
     * @since 6
     */
    void SetKnobColor(const ColorType knobColor)
    {
        SetKnobStyle(STYLE_BACKGROUND_COLOR, knobColor.full);
    }

    /**
     * @brief Sets the corner radius for this slider's knob.
     *
     * @param knobRadius Indicates the knob corner radius to set.
     * @since 6
     */
    void SetKnobRadius(int16_t knobRadius)
    {
        SetKnobStyle(STYLE_BORDER_RADIUS, knobRadius);
    }

    /**
     * @brief Sets the knob style.
     *
     * @param style Indicates the knob style to set. For details, see {@link Style}.
     * @see GetKnobStyle
     * @since 1.0
     * @version 1.0
     */
    void SetKnobStyle(const Style& style);

    /**
     * @brief Sets a knob style.
     *
     * @param key Indicates the key of the style to set.
     * @param value Indicates the value matching the key.
     * @since 1.0
     * @version 1.0
     */
    void SetKnobStyle(uint8_t key, int64_t value);

    /**
     * @brief Obtains the knob style.
     *
     * @return Returns the knob style.
     * @since 1.0
     * @version 1.0
     */
    const Style& GetKnobStyle() const;

    /**
     * @brief Obtains the value of a knob style.
     *
     * @param key Indicates the key of the style.
     * @return Returns the value of the style.
     * @since 1.0
     * @version 1.0
     */
    int64_t GetKnobStyle(uint8_t key) const;

    /**
     * @brief Sets the images as pixel maps for this slider, including the background, foreground images.
     *
     * @param backgroundImage Indicates the background image to set.
     * @param foregroundImage Indicates the foreground image to set.
     * @since 1.0
     * @version 1.0
     */
    void SetImage(const ImageInfo* backgroundImage, const ImageInfo* foregroundImage);

    /**
     * @brief Sets the images for this slider, including the background, foreground images.
     *
     * @param backgroundImage Indicates the background image to set.
     * @param foregroundImage Indicates the foreground image to set.
     * @since 1.0
     * @version 1.0
     */
    void SetImage(const char* backgroundImage, const char* foregroundImage);

    /**
     * @brief Sets the colors for this slider, including the background, foreground colors.
     *
     * @param backgroundColor Indicates the background color to set.
     * @param foregroundColor Indicates the foreground color to set.
     * @since 1.0
     * @version 1.0
     */
    void SetSliderColor(const ColorType backgroundColor, const ColorType foregroundColor)
    {
        SetBackgroundStyle(STYLE_BACKGROUND_COLOR, backgroundColor.full);
        SetForegroundStyle(STYLE_BACKGROUND_COLOR, foregroundColor.full);
    }

    /**
     * @brief Sets the corner radiuses for this slider, including the background, foreground corner radiuses.
     *
     * @param backgroundRadius Indicates the background corner radius to set.
     * @param foregroundRadius Indicates the foreground corner radius to set.
     * @since 1.0
     * @version 1.0
     */
    void SetSliderRadius(int16_t backgroundRadius, int16_t foregroundRadius)
    {
        SetBackgroundStyle(STYLE_BORDER_RADIUS, backgroundRadius);
        SetForegroundStyle(STYLE_BORDER_RADIUS, foregroundRadius);
    }

#if ENABLE_ROTATE_INPUT
    /**
     * @brief Obtains the rotation factor.
     *
     * @return Returns the rotation factor.
     * @since 5.0
     * @version 3.0
     */
    float GetRotateFactor()
    {
        return rotateFactor_;
    }

    /**
     * @brief Sets the rotation factor.
     *
     * @param factor Indicates the rotation factor to set.
     * @since 5.0
     * @version 3.0
     */
    void SetRotateFactor(float factor)
    {
        if (MATH_ABS(factor) > MAX_ROTATE_FACTOR) {
            rotateFactor_ = (factor > 0) ? MAX_ROTATE_FACTOR : -MAX_ROTATE_FACTOR;
            return;
        }
        rotateFactor_ = factor;
    }

    bool OnRotateEvent(const RotateEvent& event) override;

    bool OnRotateEndEvent(const RotateEvent& event) override;
#endif
    bool OnClickEvent(const ClickEvent& event) override;

#if GRAPHIC_ENABLE_SLIDER_FLAG
    bool OnPressEvent(const PressEvent& event) override;

    bool OnReleaseEvent(const ReleaseEvent& event) override;
#endif

    bool OnDragEvent(const DragEvent& event) override;

    bool OnDragEndEvent(const DragEvent& event) override;

    bool OnPreDraw(Rect& invalidatedArea) const override
    {
        return false;
    }

    void OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea) override;

#if GRAPHIC_ENABLE_SLIDER_FLAG
    void ReMeasure() override;
#endif

    /**
     * @brief Represents the listener for a slider change.
     *
     * This is an inner class of <b>UISlider</b> used to listen for slider events and invoke the callback function.
     *
     * @see UISlider
     * @since 1.0
     * @version 1.0
     */
    class UISliderEventListener : public HeapBase {
    public:
        /**
         * @brief A destructor used to delete the <b> UISliderEventListener </b> instance.
         *
         * @since 1.0
         * @version 1.0
         */
        virtual ~UISliderEventListener() {}
        /**
         * @brief Called when the slider is dragged or clicked. This is a virtual function, which needs your
         *        implementation.
         *
         * @param value Indicates the current value of the slider.
         * @since 1.0
         * @version 1.0
         */
        virtual void OnChange(int32_t value) {}
        /**
         * @brief Called when the slider is released. This is a virtual function, which needs your implementation.
         *
         * @param value Indicates the current value of the slider.
         * @since 1.0
         * @version 1.0
         */
        virtual void OnRelease(int32_t value) {}
    };

    /**
     * @brief Sets the listener for a slider change.
     *
     * When a user drags or clicks the slider, listening is triggered and the <b>OnChange</b> callback is invoked.
     * When a user releases the slider, the <b>OnRelease</b> callback is invoked.
     *
     * @param listener Indicates the listener to set. For details, see {@link UISliderEventListener}.
     * @since 1.0
     * @version 1.0
     */
    void SetSliderEventListener(UISliderEventListener* listener)
    {
        listener_ = listener;
    }

#if GRAPHIC_ENABLE_SLIDER_FLAG
    static constexpr uint16_t MAX_GRADIENT_COLOR_COUNT = 8;
    static constexpr uint16_t MAX_MARK_VALUE_COUNT = 32;
    static constexpr uint8_t MAX_DISABLED_TOAST_MSG_LEN = 64;
    static constexpr int16_t MAX_MARKINGS_SIZE = 64;
    /**
     * @brief Enumerates the marker shape drawn on the slider track.
     */
    enum MarkingsType : uint8_t {
        /** Draws each marker as a short line. */
        MARKINGS_LINE = 0,
        /** Draws each marker as a dot. */
        MARKINGS_DOT,
    };

    /**
     * @brief Sets whether the slider is disabled.
     *
     * A disabled slider does not update its value in click, drag, or rotate events. If a disabled toast message is
     * configured, clicking or dragging the disabled slider displays the message.
     *
     * @param disabled Specifies whether the slider is disabled.
     */
    void SetDisabled(bool disabled);

    /**
     * @brief Sets the toast message displayed when a disabled slider is operated.
     *
     * Passing <b>nullptr</b> or an empty string clears the message and disables the disabled-toast display. The message
     * is truncated to <b>MAX_DISABLED_TOAST_MSG_LEN</b> characters.
     *
     * @param msg Indicates the disabled toast message.
     */
    void SetDisabledToastMsg(const char* msg);

    /**
     * @brief Sets the values used for tick positions and snapping.
     *
     * The values are copied, sorted, and deduplicated. At most <b>MAX_MARK_VALUE_COUNT</b> values are retained. Passing
     * <b>nullptr</b> or a count of 0 clears the configured values.
     *
     * @param values Indicates the value array to set.
     * @param count Indicates the number of values in the array.
     */
    void SetValues(const int32_t* values, uint16_t count);

    /**
     * @brief Sets the gradient colors for the slider background track.
     *
     * At least two colors are required to draw a gradient. At most <b>MAX_GRADIENT_COLOR_COUNT</b> colors are retained.
     * Passing <b>nullptr</b> or a count of 0 clears the background gradient.
     *
     * @param colors Indicates the gradient color array.
     * @param count Indicates the number of colors in the array.
     */
    void SetBgGradientColors(const ColorType* colors, uint16_t count);

    /**
     * @brief Sets the gradient colors for the selected progress track.
     *
     * At least two colors are required to draw a gradient. At most <b>MAX_GRADIENT_COLOR_COUNT</b> colors are retained.
     * Passing <b>nullptr</b> or a count of 0 clears the selected-track gradient.
     *
     * @param colors Indicates the gradient color array.
     * @param count Indicates the number of colors in the array.
     */
    void SetOnTintGradientColors(const ColorType* colors, uint16_t count);

    /**
     * @brief Sets whether markers are drawn on the slider track.
     *
     * Marker positions come from configured values. If no values are configured, marker positions are generated from
     * the slider step.
     *
     * @param enable Specifies whether to draw markers.
     */
    void SetShowMarkings(bool enable);

    /**
     * @brief Sets the marker size.
     *
     * A value less than or equal to 0 or greater than <b>MAX_MARKINGS_SIZE</b> is treated as invalid. The default
     * marker size is restored and a warning log is printed.
     *
     * @param size Indicates the marker size in pixels.
     */
    void SetMarkingsSize(int16_t size);

    /**
     * @brief Sets whether marker text is drawn.
     *
     * Marker text uses the same positions as markers.
     *
     * @param enable Specifies whether to draw marker text.
     */
    void SetShowMarkText(bool enable);

    /**
     * @brief Sets whether the whole slider bounds can accept click and drag events.
     *
     * When disabled, only events close to the track are handled by the slider.
     *
     * @param enable Specifies whether to expand the interactive area.
     */
    void SetExpandClickArea(bool enable);

    /**
     * @brief Sets whether to display the current value as a toast.
     *
     * The toast is displayed during drag and click events. Disabling this option clears any visible value toast.
     *
     * @param enable Specifies whether to enable the value toast.
     */
    void EnableToast(bool enable);

    /**
     * @brief Sets whether the slider value snaps to tick values during user interaction.
     *
     * If values are configured by {@link SetValues}, snapping uses those values. Otherwise snapping uses the slider
     * step.
     *
     * @param enable Specifies whether to enable tick snapping.
     */
    void EnableTicks(bool enable);

    /**
     * @brief Sets whether markers are drawn on the slider track.
     *
     * This function is equivalent to {@link SetShowMarkings}.
     *
     * @param enable Specifies whether to draw markers.
     */
    void EnableMarkings(bool enable);

    /**
     * @brief Sets whether marker text is drawn.
     *
     * This function is equivalent to {@link SetShowMarkText}.
     *
     * @param enable Specifies whether to draw marker text.
     */
    void EnableMarkText(bool enable);

    /**
     * @brief Sets the marker shape.
     *
     * Invalid values restore the default marker shape and print a warning log.
     *
     * @param type Indicates the marker shape.
     */
    void SetMarkingsType(MarkingsType type);

    /**
     * @brief Checks whether markers are enabled.
     *
     * @return Returns <b>true</b> if markers are enabled; returns <b>false</b> otherwise.
     */
    bool IsMarkingsEnabled() const
    {
        return showMarkings_;
    }

    /**
     * @brief Checks whether the value toast is enabled.
     *
     * @return Returns <b>true</b> if the value toast is enabled; returns <b>false</b> otherwise.
     */
    bool IsToastEnabled() const
    {
        return enableToast_;
    }

    /**
     * @brief Checks whether tick snapping is enabled.
     *
     * @return Returns <b>true</b> if tick snapping is enabled; returns <b>false</b> otherwise.
     */
    bool IsTicksEnabled() const
    {
        return enableTicks_;
    }

    /**
     * @brief Obtains the current marker shape.
     *
     * @return Returns the current marker shape.
     */
    MarkingsType GetMarkingsType() const
    {
        return markingsType_;
    }

    /**
     * @brief Checks whether the slider is disabled.
     *
     * @return Returns <b>true</b> if the slider is disabled; returns <b>false</b> otherwise.
     */
    bool IsDisabled() const
    {
        return disabled_;
    }

    /**
     * @brief Checks whether marker text is enabled.
     *
     * @return Returns <b>true</b> if marker text is enabled; returns <b>false</b> otherwise.
     */
    bool IsMarkTextEnabled() const
    {
        return showMarkText_;
    }

    /**
     * @brief Checks whether the interactive area is expanded.
     *
     * @return Returns <b>true</b> if the whole slider bounds accept interaction; returns <b>false</b> otherwise.
     */
    bool IsExpandClickAreaEnabled() const
    {
        return expandClickArea_;
    }

    /**
     * @brief Obtains the marker size.
     *
     * @return Returns the marker size in pixels.
     */
    int16_t GetMarkingsSize() const
    {
        return markingsSize_;
    }

    /**
     * @brief Obtains the number of background gradient colors.
     *
     * @return Returns the number of configured background gradient colors.
     */
    uint16_t GetBgGradientColorsCount() const
    {
        return bgGradientColorsCount_;
    }

    /**
     * @brief Obtains the number of selected-track gradient colors.
     *
     * @return Returns the number of configured selected-track gradient colors.
     */
    uint16_t GetOnTintGradientColorsCount() const
    {
        return onTintGradientColorsCount_;
    }

    /**
     * @brief Obtains the disabled toast message.
     *
     * @return Returns the disabled toast message, or <b>nullptr</b> if no message is configured.
     */
    const char* GetDisabledToastMsg() const
    {
        return (disabledToastMsg_[0] == '\0') ? nullptr : disabledToastMsg_;
    }

    /**
     * @brief Obtains the number of configured values.
     *
     * @return Returns the number of configured values retained by the slider.
     */
    uint16_t GetValuesCount() const
    {
        return valuesCount_;
    }

#endif

protected:
    bool InitImage() override;

private:
    static constexpr uint8_t MAX_ROTATE_FACTOR = 128;
#if GRAPHIC_ENABLE_SLIDER_FLAG
    static constexpr uint16_t MAX_TICK_VALUE_COUNT = 20;
    static constexpr int16_t DEFAULT_MARKINGS_SIZE = 4;
    static constexpr MarkingsType DEFAULT_MARKINGS_TYPE = MARKINGS_LINE;
    static constexpr uint8_t TOAST_FONT_SIZE = 12;
    static constexpr uint8_t TEXT_BUFFER_SIZE = 12;
#endif

    int16_t knobWidth_;
    bool knobStyleAllocFlag_;
    Style* knobStyle_;
    Image* knobImage_;

    void DrawKnob(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const Rect& foregroundRect);
#if GRAPHIC_ENABLE_SLIDER_FLAG
    void DrawBackground(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea);
    bool CalculateForegroundCoords(const Point& startPoint, int16_t progressWidth, int16_t progressHeight,
                                   uint16_t radius, Rect& coords, Rect& foregroundRect);
#endif
    void DrawForeground(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, Rect& coords);
#if GRAPHIC_ENABLE_SLIDER_FLAG
    struct MarkTextParam {
        Point startPoint;
        int16_t progressWidth;
        int16_t progressHeight;
        uint16_t fontId;
        uint16_t lineHeight;
        Style textStyle;
        Rect viewRect;
        int32_t range;
    };

    struct ToastParam {
        Rect viewRect;
        Point startPoint;
        int16_t progressWidth;
        int16_t progressHeight;
        uint16_t fontId;
        Style textStyle;
        uint16_t textLength;
        int16_t textWidth;
        int16_t textHeight;
        int16_t toastWidth;
        int16_t toastHeight;
    };

    void DrawMarkings(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea);
    void DrawMarkingItem(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const Point& startPoint,
                         int16_t progressWidth, int16_t progressHeight, int32_t value, int32_t range);
    void DrawMarkText(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea);
    void DrawMarkTextItem(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const MarkTextParam& param,
                          int32_t value);
    void DrawToast(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const char* text);
    void DrawDisabledToast(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea);
    bool InitToastParam(const char* text, ToastParam& param);
    void LocateToast(const ToastParam& param, int16_t& left, int16_t& top) const;
    void DrawToastRect(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, int16_t left, int16_t top,
                       const ToastParam& param);
    void DrawLabelText(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const Point& pos,
                       int16_t textWidth, int16_t textHeight, uint16_t fontId, const Style& textStyle,
                       const char* text, uint16_t textLength);
    uint16_t BuildMarkValues(int32_t* values, uint16_t maxCount) const;
    void EnsureLastMarkValue(int32_t* values, uint16_t count, uint16_t maxCount) const;
    bool ShouldAcceptClick(const Point& pos);
    void DrawGradientRect(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, const Rect& rect,
                          const Style& style, const ColorType* colors, uint16_t count);
    ColorType InterpolateColor(const ColorType& start, const ColorType& end, uint8_t mix) const;

    int32_t CalculateHorizontalValueByPosition(const Point& knobPosition, const Point& startPoint);
    int32_t CalculateVerticalValueByPosition(const Point& knobPosition, const Point& startPoint);
    int32_t CalculateValueByPosition(const Point& knobPosition);
#endif
    int32_t CalculateCurrentValue(int16_t length, int16_t totalLength);
    int32_t UpdateCurrentValue(const Point& knobPosition);
#if GRAPHIC_ENABLE_SLIDER_FLAG
    int32_t SnapToMarkValue(int32_t value) const;
    bool HandleDisabledEvent();
    void ShowValueToast(int32_t value, bool autoHide);
    void HideValueToast();
#endif
#if ENABLE_ROTATE_INPUT
    float rotateFactor_;
    float cachedRotation_;
#endif
    UISliderEventListener* listener_;

#if GRAPHIC_ENABLE_SLIDER_FLAG
    char disabledToastMsg_[MAX_DISABLED_TOAST_MSG_LEN + 1] = {0};
    int32_t values_[MAX_MARK_VALUE_COUNT] = {0};
    int32_t toastValue_ = 0;
    uint16_t valuesCount_ = 0;
    uint16_t bgGradientColorsCount_ = 0;
    uint16_t onTintGradientColorsCount_ = 0;
    ColorType bgGradientColors_[MAX_GRADIENT_COLOR_COUNT] = {};
    ColorType onTintGradientColors_[MAX_GRADIENT_COLOR_COUNT] = {};
    ColorType markingsColor_ = Color::Gray();
    ColorType markTextColor_ = Color::Black();
    int16_t markingsSize_ = DEFAULT_MARKINGS_SIZE;
    MarkingsType markingsType_ = DEFAULT_MARKINGS_TYPE;
    bool disabled_ : 1;
    bool showDisabledToast_ : 1;
    bool enableToast_ : 1;
    bool toastVisible_ : 1;
    bool toastAutoHide_ : 1;
    bool needEraseToast_ : 1;
    bool enableTicks_ : 1;
    bool showMarkings_ : 1;
    bool showMarkText_ : 1;
    bool expandClickArea_ : 1;
#endif
}; // class UISlider
} // namespace OHOS
#endif // GRAPHIC_LITE_UI_SLIDER_H
