/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef GRAPHIC_LITE_INPUT_METHOD_MANAGER_H
#define GRAPHIC_LITE_INPUT_METHOD_MANAGER_H

#include <string>

#include "components/ui_view.h"
#include "gfx_utils/graphic_types.h"

namespace OHOS {
class InputMethodManager : public HeapBase {
public:
    struct InputMethodParam {
        InputType inputType;
        std::string text;
        UIView* view; /* view param for mini system without Window feature */
    };
    /**
     * @brief Defines a input method listener. You need to register this listener when you use keyboard to input text.
     *        This listener will invoke when the edit text view get focused.
     */
    class InputMethodListener : public HeapBase {
    public:
        /**
         * @brief A destructor used to delete the <b>InputMethodListener</b> instance.
         */
        virtual ~InputMethodListener() {}

        /**
         * @brief Invoke this method when edit text view get focused.
         * @param param  the param passed, see InputMethodParam.
         */
        virtual void OnShow(InputMethodParam& param) = 0;

        /**
         * @brief Invoke this method when edit text view get blured.
         */
        virtual void OnHide() = 0;
    };

    /**
     * @brief return InputMethodManager's singleton
     * @return InputMethodManager*
     */
    static InputMethodManager& GetInstance();

    /**
     * @brief Called to show input method when the edit view focuse
     * @param UIView  the edit view
     */
    void ShowInputMethod(UIView* view);

    /**
     * @brief Called to hide input method when the edit view blure
     */
    void HideInputMethod();

    /**
     * @brief Sets a input method listener.
     * @param listener the input method listener.
     */
    void SetInputMethodListener(InputMethodListener* listener);

    /**
     * @brief Call to insert text when keyboard select new input text.
     * @param text the input method listener.
     */
    void InsertText(std::string text);

    /**
     * @brief Call to delete text when keyboard press delete button.
     * @param length the length of charactor to delete
     */
    void DeleteBackward(uint16_t length);

    /**
     * @brief Sets the input type.
     * @param type the input type, see InputType
     */
    void SetInputType(InputType type);

    /**
     * @brief Call function invoke after the keyboard showed.
     */
    void OnKeyboardShow();

    /**
     * @brief Call function invoke after the keyboard hided.
     */
    void OnKeyboardHide();

    /**
     * @brief Sets the cursor index.
     */
    void SetCursorIndex(uint16_t cursorIndex_);

    uint16_t GetCursorIndex();

private:
    InputMethodManager() {}
    ~InputMethodManager() {}

    InputMethodManager(const InputMethodManager&) = delete;
    InputMethodManager& operator=(const InputMethodManager&) = delete;
    InputMethodManager(InputMethodManager&&) = delete;
    InputMethodManager& operator=(InputMethodManager&&) = delete;

    InputMethodListener* inputMethodListener_ = nullptr;
    UIView* inputView_ = nullptr;
};
} // namespace OHOS
#endif // GRAPHIC_LITE_INPUT_METHOD_MANAGER_H
