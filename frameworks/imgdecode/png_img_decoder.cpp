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

#include "imgdecode/png_img_decoder.h"
#include "gfx_utils/mem_api.h"
#include "gfx_utils/file.h"
#include "gfx_utils/graphic_log.h"

namespace OHOS {

template<typename T>
void FreeResource(T*& ptr)
{
    UIFree(ptr);
    ptr = nullptr;
}

// 为了兼容数组指针类型
template<typename T>
void FreeResource(T**& ptr)
{
    UIFree(ptr);
    ptr = nullptr;
}

RetCode PngImgDecoder::Open(ImgResDsc& dsc)
{
    fp_ = fopen(dsc.path, "rb");
    if (fp_ == nullptr) {
        GRAPHIC_LOGE("open file:%s failed\n", dsc.path);
        return RetCode::FAIL;
    }

    dsc.imgInfo.data = nullptr;
    dsc.inCache = false;
    dsc.fd = -1; // don't use fd here

    return RetCode::OK;
}

RetCode PngImgDecoder::Close(ImgResDsc& dsc)
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

RetCode PngImgDecoder::CreateDecoder(void)
{
    if (fp_ == nullptr) {
        GRAPHIC_LOGE("File is not opened, call Open first\n");
        return RetCode::FAIL;
    }

    png_ = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ == nullptr) {
        return RetCode::FAIL;
    }

    info_ = png_create_info_struct(png_);
    if (info_ == nullptr) {
        png_destroy_read_struct(&png_, nullptr, nullptr);
        return RetCode::FAIL;
    }

    png_init_io(png_, fp_);
    png_read_info(png_, info_);

    return RetCode::OK;
}

void PngImgDecoder::DestroyDecoder(void)
{
    if (fp_ != nullptr) {
        fclose(fp_);
        fp_ = nullptr;
    }

    if (png_ != nullptr && info_ != nullptr) {
        png_destroy_read_struct(&png_, &info_, nullptr);
    } else if (png_ != nullptr && info_ == nullptr) {
        png_destroy_read_struct(&png_, nullptr, nullptr);
    }
}

RetCode PngImgDecoder::GetHeader(ImgResDsc& dsc)
{
    if (CreateDecoder() != RetCode::OK) {
        GRAPHIC_LOGE("Decoder is created failed\n");
        return RetCode::FAIL;
    }

    dsc.imgInfo.header.width = png_get_image_width(png_, info_);
    dsc.imgInfo.header.height = png_get_image_height(png_, info_);
    dsc.imgInfo.header.colorMode = ARGB8888;

    return RetCode::OK;
}

void PngImgDecoder::UpdateInfo(void)
{
    uint8_t colorType = png_get_color_type(png_, info_);
    uint8_t bitDepth = png_get_bit_depth(png_, info_);
    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) { // 8: Expand grayscale images to the full 8 bits
        png_set_expand_gray_1_2_4_to_8(png_);
    }
    if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_);
    }
    if (colorType == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_);
    }
    if (bitDepth == 16) { // 16: Chop 16-bit depth images to 8-bit depth
        png_set_strip_16(png_);
    }

    if (png_get_valid(png_, info_, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_);
    }
    if (!(colorType & PNG_COLOR_MASK_ALPHA)) {
        png_set_add_alpha(png_, 0xFF, PNG_FILLER_AFTER);
    }
    png_set_interlace_handling(png_);
    png_read_update_info(png_, info_);
}

RetCode PngImgDecoder::ReadToCache(ImgResDsc& dsc)
{
    uint8_t pixelByteSize = DrawUtils::GetPxSizeByColorMode(ARGB8888) >> 3; // 3: Shift right 3 bits
    uint16_t width = dsc.imgInfo.header.width;
    uint16_t height = dsc.imgInfo.header.height;
    uint32_t dataSize = height * width * pixelByteSize;

    UpdateInfo();
    png_bytep* rowPointer = static_cast<png_bytep*>(UIMalloc(sizeof(png_bytep) * height));
    if (rowPointer == nullptr) {
        DestroyDecoder();
        return RetCode::FAIL;
    }
    for (uint16_t y = 0; y < height; y++) {
        rowPointer[y] = static_cast<png_byte*>(UIMalloc(png_get_rowbytes(png_, info_)));
        if (rowPointer[y] == nullptr) {
            for (uint16_t i = 0; i < y; i++) {
                FreeResource(rowPointer[i]);
            }
            FreeResource(rowPointer);
            DestroyDecoder();
            return RetCode::FAIL;
        }
    }
    png_read_image(png_, rowPointer);

    dsc.imgInfo.dataSize = dataSize;
    uint8_t* srcData = static_cast<uint8_t*>(ImageCacheMalloc(dsc.imgInfo));
    dsc.imgInfo.data = srcData;
    if (srcData == nullptr) {
        for (uint16_t i = 0; i < height; i++) {
            FreeResource(rowPointer[i]);
        }
        FreeResource(rowPointer);
        DestroyDecoder();
        return RetCode::FAIL;
    }
    uint32_t n = 0;
    for (uint16_t y = 0; y < height; y++) {
        png_bytep row = rowPointer[y];
        for (uint16_t x = 0; x < width * pixelByteSize; x += pixelByteSize) {
            srcData[n++] = row[x + 2]; // 2: B channel
            srcData[n++] = row[x + 1]; // 1: G channel
            srcData[n++] = row[x + 0]; // 0: R channel
            srcData[n++] = row[x + 3]; // 3: Alpha channel
        }
        FreeResource(row);
    }
    FreeResource(rowPointer);

    DestroyDecoder();
    dsc.inCache = true;
    return RetCode::OK;
}
} // namespace OHOS