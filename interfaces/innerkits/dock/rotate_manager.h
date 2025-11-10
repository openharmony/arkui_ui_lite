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
 
#ifndef ROTATE_MANAGER_H
#define ROTATE_MANAGER_H
 
#include "gfx_utils/list.h"
#include "rotate_event_listener.h"
#include "events/rotate_event.h"
 
namespace OHOS {
class RotateManager : public HeapBase {
public:
    /**
     * @brief Get the singleton instance of RotateManager.
     *
     * This function returns the global unique instance of RotateManager.
     * It is thread-safe and should be used to access the rotation event manager.
     *
     * @return RotateManager& Reference to the singleton instance.
     */
    static RotateManager& GetInstance(void);

    /**
     * @brief Register a listener for rotation events.
     *
     * Adds a new listener to the internal list. The listener will receive
     * rotation events once registered.
     *
     * @param listener Pointer to the RotateEventListener to register.
     * @note The listener must remain valid for the duration of registration.
     */
    void Add(RotateEventListener* listener);

    /**
     * @brief Remove a listener from rotation event notifications.
     *
     * Removes the specified listener from the list of active listeners.
     * After this call, the listener will no longer receive rotation events.
     *
     * @param listener Pointer to the RotateEventListener to remove.
     * @note The listener must have been previously added.
     */
    void Remove(RotateEventListener* listener);

    /**
     * @brief Remove all registered listeners.
     *
     * Clears the list of registered listeners, stopping all listeners
     * from receiving rotation events.
     */
    void Clear();

    /**
     * @brief Get the list of currently registered listeners.
     *
     * Returns a const reference to the internal list of registered listeners.
     * The returned list is read-only and should not be modified directly.
     *
     * @return const List<RotateEventListener*>& Reference to the list of listeners.
     */
    const List<RotateEventListener*>& GetRegisteredListeners() const;

    /**
     * @brief Notify all registered listeners that a rotation has started.
     *
     * This method triggers the OnRotateStart callback on all registered listeners.
     *
     * @param event The rotation event data.
     */
    void OnRotateStart(const RotateEvent& event);

    /**
     * @brief Notify all registered listeners of a rotation update.
     *
     * This method triggers the OnRotate callback on all registered listeners.
     *
     * @param event The rotation event data.
     */
    void OnRotate(const RotateEvent& event);

    /**
     * @brief Notify all registered listeners that a rotation has ended.
     *
     * This method triggers the OnRotateEnd callback on all registered listeners.
     *
     * @param event The rotation event data.
     */
    void OnRotateEnd(const RotateEvent& event);

private:
    List<RotateEventListener*> rotateList_;
    RotateManager() = default;
    RotateManager(const RotateManager&) = delete;
    RotateManager& operator=(const RotateManager&) = delete;
};
}
#endif // ROTATE_MANAGER_H
