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
 
#ifndef ROTATE_EVENT_LISTENER_H
#define ROTATE_EVENT_LISTENER_H
 
#include "events/rotate_event.h"
 
namespace OHOS {
class RotateEventListener : public HeapBase {
public:
    /**
     * @brief Virtual destructor for proper cleanup.
     *
     * Ensures derived classes are destroyed correctly.
     */
    virtual ~RotateEventListener() {};

    /**
     * @brief Handle the start of a rotation gesture.
     *
     * This method is called when a rotation gesture begins (e.g., two fingers
     * start rotating on the screen).
     *
     * @param event The rotation event data containing initial touch positions,
     *                angle, and other relevant information.
     *
     * @return true if the event was fully processed and should not be propagated
     *         further; false otherwise, allowing other listeners to handle it.
     */
    virtual bool OnRotateStartEvent(const RotateEvent& event)
    {
        return false;
    }

    /**
     * @brief Handle an ongoing rotation gesture.
     *
     * This method is called repeatedly during a rotation gesture to report
     * updated rotation angle, distance, and touch positions.
     *
     * @param event The rotation event data with current gesture state.
     *
     * @return true if the event was handled and should not be propagated;
     *         false otherwise, allowing other listeners to process it.
     */
    virtual bool OnRotateEvent(const RotateEvent& event)
    {
        return false;
    }

    /**
     * @brief Handle the end of a rotation gesture.
     *
     * This method is called when a rotation gesture ends (e.g., fingers lifted
     * from the screen).
     *
     * @param event The final rotation event data, including total rotation angle,
     *                velocity, and end coordinates.
     *
     * @return true if the event was processed and no further action is needed;
     *         false otherwise, allowing other components to react.
     */
    virtual bool OnRotateEndEvent(const RotateEvent& event)
    {
        return false;
    }
};
}
#endif // ROTATE_EVENT_LISTENER_H
