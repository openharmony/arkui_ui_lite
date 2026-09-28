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
 * @file ui_toggle_button.h
 *
 * @brief Defines the attributes and common functions of a toggle button.
 *
 * Each toggle button contains two images, one for pressing and the other for releasing.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef GRAPHIC_LITE_UI_TOGGLE_BUTTON_H
#define GRAPHIC_LITE_UI_TOGGLE_BUTTON_H

#include "components/ui_checkbox.h"
#include "components/ui_image_view.h"

namespace OHOS {
/**
 * @brief Represents a toggle button.
 *
 * Each toggle button contains two images, one for pressing and the other for releasing.
 *
 * @see UICheckBox
 * @since 1.0
 * @version 1.0
 */
class UIToggleButton : public UICheckBox {
public:
    /**
     * @brief A constructor used to create a <b>UIToggleButton</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    UIToggleButton();

    /**
     * @brief A destructor used to delete the <b>UIToggleButton</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    virtual ~UIToggleButton() {}

    /**
     * @brief Obtains the component type.
     *
     * @return Returns the component type, as defined in {@link UIViewType}.
     * @since 1.0
     * @version 1.0
     */
    UIViewType GetViewType() const override
    {
        return UI_TOGGLE_BUTTON;
    }

    void OnDraw(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea) override;

    /**
     * @brief Sets the state for this toggle button.
     *
     * @param state Indicates the state of this toggle button. The value <b>true</b> indicates the image displayed for
     * <b>On</b>, and <b>false</b> indicates the image displayed for <b>Off</b>. If this function is not called, the
     * image is displayed for <b>Off</b>.
     * @since 1.0
     * @version 1.0
     */
    void SetState(bool state);

    /**
     * @brief Obtains the state of this toggle button.
     *
     * @return Returns <b>true</b> if the image for <b>On</b> is displayed; returns <b>false</b> if the image is
     * displayed for <b>Off</b>.
     * @since 1.0
     * @version 1.0 */
    bool GetState() const
    {
        return (state_ != UNSELECTED);
    }

    /**
     * @brief Sets the layout dirention for this toggle button.
     *
     * @param isRtl Indicates the layout dirention of this toggle button. The value <b>true</b> indicates layout
     * direntionis right-to-left, which means the white circle in button is at left side when the state is ON,
     * at right side when the state is OFF. The value <b>false</b> indicates layout direntionis left-to-right.
     */
    void EnableRtl(bool isRtl);

    bool IsRtl();

#if GRAPHIC_ENABLE_SWITCH_FLAG
    /**
     * @brief Sets the thumb radius when the toggle button is in the ON state.
     *
     * @param radius Indicates the thumb radius for the ON state.
     */
    void SetOnThumbSize(uint16_t radius);

    /**
     * @brief Sets the thumb radius when the toggle button is in the OFF state.
     *
     * @param radius Indicates the thumb radius for the OFF state.
     */
    void SetOffThumbSize(uint16_t radius);

    /**
     * @brief Obtains the thumb radius when the toggle button is in the ON state.
     *
     * @return Returns the thumb radius for the ON state.
     */
    uint16_t GetOnThumbSize() const
    {
        return onThumbRadius_;
    }

    /**
     * @brief Obtains the thumb radius when the toggle button is in the OFF state.
     *
     * @return Returns the thumb radius for the OFF state.
     */
    uint16_t GetOffThumbSize() const
    {
        return offThumbRadius_;
    }

    /**
     * @brief Sets the thumb color when the toggle button is in the ON state.
     *
     * @param color Indicates the thumb color for the ON state.
     */
    void SetOnThumbColor(ColorType color);

    /**
     * @brief Sets the thumb color when the toggle button is in the OFF state.
     *
     * @param color Indicates the thumb color for the OFF state.
     */
    void SetOffThumbColor(ColorType color);

    /**
     * @brief Obtains the thumb color when the toggle button is in the ON state.
     *
     * @return Returns the thumb color for the ON state.
     */
    ColorType GetOnThumbColor() const
    {
        return onThumbColor_;
    }

    /**
     * @brief Obtains the thumb color when the toggle button is in the OFF state.
     *
     * @return Returns the thumb color for the OFF state.
     */
    ColorType GetOffThumbColor() const
    {
        return offThumbColor_;
    }

    /**
     * @brief Sets the thumb border color when the toggle button is in the ON state.
     *
     * @param color Indicates the thumb border color for the ON state.
     */
    void SetOnBorderColor(ColorType color);

    /**
     * @brief Sets the thumb border color when the toggle button is in the OFF state.
     *
     * @param color Indicates the thumb border color for the OFF state.
     */
    void SetOffBorderColor(ColorType color);

    /**
     * @brief Obtains the thumb border color when the toggle button is in the ON state.
     *
     * @return Returns the thumb border color for the ON state.
     */
    ColorType GetOnBorderColor() const
    {
        return onBorderColor_;
    }

    /**
     * @brief Obtains the thumb border color when the toggle button is in the OFF state.
     *
     * @return Returns the thumb border color for the OFF state.
     */
    ColorType GetOffBorderColor() const
    {
        return offBorderColor_;
    }

#endif
protected:
    void CalculateSize() override;
#if DEFAULT_ANIMATION
    void Callback(UIView* view) override;
    void OnStop(UIView& view) override;
#endif

private:
#if GRAPHIC_ENABLE_SWITCH_FLAG
    void UpdateDrawState();
    int16_t GetThumbDrawRadius();
    void DrawThumb(BufferInfo& gfxDstBuffer, const Rect& invalidatedArea, Style& styleUnSelect, int16_t drawRadius);
#endif
    uint16_t corner_ = 0;
    uint16_t radius_ = 0;
    int16_t rectWidth_ = 0;
    Point leftCenter_ = {0, 0};
    Point rightCenter_ = {0, 0};
    Point currentCenter_ = {0, 0};
    ColorType bgColor_ = Color::White();
    Rect rectMid_ = {0, 0, 0, 0};
    bool isRtl_ = false;
#if GRAPHIC_ENABLE_SWITCH_FLAG
    ColorType thumbColor_ = Color::White();
    ColorType onThumbColor_ = Color::White();
    ColorType offThumbColor_ = Color::White();
    ColorType onBorderColor_ = Color::White();
    ColorType offBorderColor_ = Color::White();
    uint16_t onThumbRadius_ = 0;
    uint16_t offThumbRadius_ = 0;
    bool hasOnThumbSize_ : 1;
    bool hasOffThumbSize_ : 1;
    bool hasWarnedOnThumbRadius_ : 1;
    bool hasWarnedOffThumbRadius_ : 1;
    bool hasOnThumbColor_ : 1;
    bool hasOffThumbColor_ : 1;
    bool hasOnBorderColor_ : 1;
    bool hasOffBorderColor_ : 1;
#endif
}; // class UIToggleButton
} // namespace OHOS
#endif // GRAPHIC_LITE_UI_TOGGLE_BUTTON_H
