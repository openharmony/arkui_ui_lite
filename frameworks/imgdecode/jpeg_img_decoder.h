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

#ifndef GRAPHIC_LITE_JPEG_IMG_DECODER_H
#define GRAPHIC_LITE_JPEG_IMG_DECODER_H

#include "file_img_decoder.h"
#include "draw/draw_image.h"
#include "draw/draw_utils.h"
#include "jpeglib.h"

namespace OHOS {
class JpegImgDecoder : public FileImgDecoder {
public:
    static JpegImgDecoder& GetInstance()
    {
        static JpegImgDecoder instance;
        return instance;
    }

    RetCode Open(ImgResDsc& dsc) override;

    RetCode Close(ImgResDsc& dsc) override;

    RetCode GetHeader(ImgResDsc& dsc) override;

    RetCode ReadToCache(ImgResDsc& dsc) override;

    RetCode ReadLine(ImgResDsc& dsc, const Point& start, int16_t len, uint8_t* buf) override
    {
        return RetCode::FAIL;
    }

private:
    FILE* fp_ {nullptr};
    struct jpeg_decompress_struct cinfo_ {0};
    struct jpeg_error_mgr jerr_ {0};
    JpegImgDecoder() {};
    ~JpegImgDecoder() override {};

    RetCode CreateDecoder(void);

    void DestroyDecoder(void);

    JpegImgDecoder(const JpegImgDecoder&) = delete;
    JpegImgDecoder& operator=(const JpegImgDecoder&) = delete;
    JpegImgDecoder(JpegImgDecoder&&) = delete;
    JpegImgDecoder& operator=(JpegImgDecoder&&) = delete;
};
}
#endif