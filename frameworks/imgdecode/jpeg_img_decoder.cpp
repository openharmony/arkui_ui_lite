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

#include "imgdecode/jpeg_img_decoder.h"
#include "gfx_utils/file.h"
#include "gfx_utils/graphic_log.h"
#include "gfx_utils/mem_api.h"
#include "securec.h"

namespace OHOS {
RetCode JpegImgDecoder::Open(ImgResDsc& dsc)
{
    fp_ = fopen(dsc.path, "rb");
    if (fp_ == nullptr) {
        GRAPHIC_LOGE("open file:%s failed\n", dsc.path);
        return RetCode::FAIL;
    }

    dsc.imgInfo.data = nullptr;
    dsc.inCache = false;

    return RetCode::OK;
}

RetCode JpegImgDecoder::Close(ImgResDsc& dsc)
{
    if (dsc.imgInfo.data != nullptr) {
        ImageCacheFree(dsc.imgInfo);
        dsc.imgInfo.data = nullptr;
    }

    if (fp_ != nullptr) {
        fclose(fp_);
        fp_ = nullptr;
    }

    dsc.inCache = false;
    return RetCode::OK;
}

RetCode JpegImgDecoder::CreateDecoder(void)
{
    if (fp_ == nullptr) {
        GRAPHIC_LOGE("File is not opened, call Open first\n");
        return RetCode::FAIL;
    }

    cinfo_.err = jpeg_std_error(&jerr_);
    jpeg_create_decompress(&cinfo_);
    jpeg_stdio_src(&cinfo_, fp_);
    jpeg_read_header(&cinfo_, TRUE);
    jpeg_start_decompress(&cinfo_);

    return RetCode::OK;
}

void JpegImgDecoder::DestroyDecoder(void)
{
    if (fp_ != nullptr) {
        fclose(fp_);
        fp_ = nullptr;
    }

    jpeg_finish_decompress(&cinfo_);
    jpeg_destroy_decompress(&cinfo_);
}

RetCode JpegImgDecoder::GetHeader(ImgResDsc& dsc)
{
    if (CreateDecoder() != RetCode::OK) {
        GRAPHIC_LOGE("Decoder is created failed\n");
        return RetCode::FAIL;
    }

    dsc.imgInfo.header.width = cinfo_.output_width;
    dsc.imgInfo.header.height = cinfo_.output_height;
    dsc.imgInfo.header.colorMode = ARGB8888;

    return RetCode::OK;
}

RetCode JpegImgDecoder::ReadToCache(ImgResDsc& dsc)
{
    JSAMPARRAY buffer;
    uint8_t pixelByteSize = DrawUtils::GetPxSizeByColorMode(ARGB8888) >> 3; // 3: Shift right 3 bits
    uint16_t width = cinfo_.output_width;
    uint16_t height = cinfo_.output_height;
    uint32_t dataSize = width * height * pixelByteSize;
    uint16_t rowStride = cinfo_.output_width * pixelByteSize;
    buffer = (*cinfo_.mem->alloc_sarray)(reinterpret_cast<j_common_ptr>(&cinfo_),
        JPOOL_IMAGE, rowStride, 1); // 1: one-row-high array

    dsc.imgInfo.dataSize = dataSize;
    uint8_t* srcData = static_cast<uint8_t*>(ImageCacheMalloc(dsc.imgInfo));
    dsc.imgInfo.data = srcData;
    if (srcData == nullptr) {
        DestroyDecoder();
        return RetCode::FAIL;
    }

    uint32_t n = 0;
    while (cinfo_.output_scanline < cinfo_.output_height) {
        jpeg_read_scanlines(&cinfo_, buffer, 1);       // 1: read one line each time
        for (uint16_t x = 0; x < width * 3; x += 3) { // 3: color components per pixel
            srcData[n++] = buffer[0][x + 2];          // 2: B channel
            srcData[n++] = buffer[0][x + 1];          // 1: G channel
            srcData[n++] = buffer[0][x + 0];          // 0: R channel
            srcData[n++] = 255;                       // 255: set alpha channel
        }
    }
    DestroyDecoder();
    dsc.inCache = true;

    return RetCode::OK;
}
} // namespace OHOS
