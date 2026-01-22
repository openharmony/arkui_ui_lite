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

#ifndef GRAPHIC_LITE_ROTATE_EVENT_H
#define GRAPHIC_LITE_ROTATE_EVENT_H
/**
 * @addtogroup Graphic
 * @{
 *
 * @brief Defines a lightweight graphics system that provides basic UI and container views,
 *        including buttons, images, labels, lists, animators, scroll views, swipe views, and layouts.
 *        This system also provides the Design for X (DFX) capability to implement features such as
 *        view rendering, animation, and input event distribution.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file rotate_event.h
 *
 * @brief Declares a <b>RotateEvent</b>, which usually occurs on a knob.
 *
 * @since 5.0
 * @version 3.0
 */

#include "event.h"

namespace OHOS {
/**
 * @brief Defines a <b>RotateEvent</b>, which usually occurs on a knob.
 *
 * @since 5.0
 * @version 3.0
 */
class RotateEvent : public Event {
public:
    RotateEvent() = delete;

    /**
     * @brief A constructor used to create a <b>RotateEvent</b> instance.
     * @param rotate Indicates the short data representing the number reported by a <b>RotateEvent</b>.
     *
     * @since 5.0
     * @version 3.0
     */
    RotateEvent(int16_t rotate)
        : rotate_(static_cast<int16_t>(rotate * ROTATE_SENSITIVITY)), // Rotation direction
          angularVelocity_(0),
          rotateVelocity_(0),
          rotateDegree_(0),
          timestamp_(0) {}

    /**
     * @brief A constructor used to create a <b>RotateEvent</b> instance.
     * @param rotate Indicates the short data representing the number reported by a <b>RotateEvent</b>.
     * @param angularVelocity_  Indicates the angular velocity by a <b>RotateEvent</b>.
     * @param rotateVelocity_  Indicates the rotate velocity in degrees per second by a <b>RotateEvent</b>.
     * @param rotateDegree_  Indicates the current rotation angle in degrees by a <b>RotateEvent</b>.
     *
     * @since 1.0
     * @version 1.0
     */
    RotateEvent(int16_t rotate, float angularVelocity, float rotateVelocity, float rotateDegree)
        : rotate_(static_cast<int16_t>(rotate * ROTATE_SENSITIVITY)), // Rotation direction
          angularVelocity_(static_cast<float>(angularVelocity)),
          rotateVelocity_(static_cast<float>(rotateVelocity)),
          rotateDegree_(static_cast<float>(rotateDegree)),
          timestamp_(0) {}

    /**
     * @brief A constructor used to create a <b>RotateEvent</b> instance.
     * @param rotate Indicates the short data representing the number reported by a <b>RotateEvent</b>.
     * @param angularVelocity_  Indicates the angular velocity by a <b>RotateEvent</b>.
     * @param rotateVelocity_  Indicates the rotate velocity in degrees per second by a <b>RotateEvent</b>.
     * @param rotateDegree_  Indicates the current rotation angle in degrees by a <b>RotateEvent</b>.
     * @param timestamp_  Indicates the reported event stamp. by a <b>RotateEvent</b>.
     *
     * @since 1.0
     * @version 1.0
     */
    RotateEvent(int16_t rotate, float angularVelocity, float rotateVelocity, float rotateDegree, uint64_t timestamp)
        : rotate_(static_cast<int16_t>(rotate * ROTATE_SENSITIVITY)), // Rotation direction
          angularVelocity_(static_cast<float>(angularVelocity)),
          rotateVelocity_(static_cast<float>(rotateVelocity)),
          rotateDegree_(static_cast<float>(rotateDegree)),
          timestamp_(static_cast<uint64_t>(timestamp)) {}

    ~RotateEvent() {}

    /**
     * @brief Obtains the number reported by a <b>RotateEvent</b>.
     * @return Returns the number reported by a <b>RotateEvent</b>.
     *
     * @since 5.0
     * @version 3.0
     */
    int16_t GetRotate() const
    {
        return rotate_;
    }

    /**
     * @brief Obtains the angular velocity by a <b>RotateEvent</b>.
     * @return Returns the angular velocity by a <b>RotateEvent</b>.
     *
     * @since 1.0
     * @version 1.0
     */
    float GetAngularVelocity() const
    {
        return angularVelocity_;
    }

    /**
     * @brief Obtains the rotate velocity in degrees per second by a <b>RotateEvent</b>.
     * @return Returns the rotate velocity in degrees per second by a <b>RotateEvent</b>.
     *
     * @since 1.0
     * @version 1.0
     */
    float GetRotateVelocity() const
    {
        return rotateVelocity_;
    }

    /**
     * @brief Obtains the current rotation angle in degrees by a <b>RotateEvent</b>.
     * @return Returns the current rotation angle in degrees by a <b>RotateEvent</b>.
     *
     * @since 1.0
     * @version 1.0
     */
    float GetRotateDegree() const
    {
        return rotateDegree_;
    }

    /**
     * @brief Obtains the current rotation angle in degrees by a <b>RotateEvent</b>.
     * @return Returns the current rotation angle in degrees by a <b>RotateEvent</b>.
     *
     * @since 1.0
     * @version 1.0
     */
    uint64_t GetTimestamp() const
    {
        return timestamp_;
    }

    /**
     * @brief Obtains the current rotation angle in degrees by a <b>RotateEvent</b>.
     * @return Returns the current rotation angle in degrees by a <b>RotateEvent</b>.
     *
     * @since 1.0
     * @version 1.0
     */
    float GetAngularVelocityJs() const
    {
        if (rotate_ > 0) {
            return angularVelocity_;
        } else {
            return 0 - angularVelocity_;
        }
    }

    /**
     * @brief Obtains the current rotation angle in degrees by a <b>RotateEvent</b>.
     * @return Returns the current rotation angle in degrees by a <b>RotateEvent</b>.
     *
     * @since 1.0
     * @version 1.0
     */
    float GetRotateVelocityJs() const
    {
        if (rotate_ > 0) {
            return rotateVelocity_;
        } else {
            return 0 - rotateVelocity_;
        }
    }

private:
    int16_t rotate_;
    float angularVelocity_;
    float rotateVelocity_;
    float rotateDegree_;
    uint64_t timestamp_;
};
} // namespace OHOS
#endif // GRAPHIC_LITE_ROTATE_EVENT_H
