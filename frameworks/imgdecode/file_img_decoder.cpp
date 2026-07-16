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

#include "gfx_utils/file.h"
#include "gfx_utils/mem_api.h"
#include "imgdecode/file_img_decoder.h"
#include "imgdecode/image_load.h"
#include "securec.h"

namespace OHOS {
FileImgDecoder& FileImgDecoder::GetInstance()
{
    static FileImgDecoder instance;
    return instance;
}

RetCode FileImgDecoder::Open(ImgResDsc& dsc)
{
#ifdef _WIN32
    int32_t fd = open(dsc.path, O_RDONLY | O_BINARY);
#else
    int32_t fd = open(dsc.path, O_RDONLY);
#endif
    if (fd == -1) {
        return RetCode::FAIL;
    }
    dsc.fd = fd;

    dsc.imgInfo.data = nullptr;
    dsc.inCache = false;
    return RetCode::OK;
}

RetCode FileImgDecoder::Close(ImgResDsc& dsc)
{
    if (dsc.imgInfo.data != nullptr) {
        ImageCacheFree(dsc.imgInfo);
        dsc.imgInfo.data = nullptr;
    }
    if (dsc.fd && (dsc.fd != -1)) {
        close(dsc.fd);
        dsc.fd = -1;
    }
    dsc.inCache = false;
    return RetCode::OK;
}

RetCode FileImgDecoder::GetHeader(ImgResDsc& dsc)
{
    if (dsc.fd == -1) {
        return RetCode::FAIL;
    }
    int32_t readCount = read(dsc.fd, &dsc.imgInfo.header, sizeof(ImageHeader));
    if (readCount != sizeof(ImageHeader)) {
        dsc.imgInfo.header.width = 0;
        dsc.imgInfo.header.height = 0;
        dsc.imgInfo.header.colorMode = UNKNOWN;
        return RetCode::FAIL;
    }
    uint8_t colorMode = dsc.imgInfo.header.colorMode;
    if (!IsImgValidMode(colorMode)) {
        return RetCode::FAIL;
    }
    return RetCode::OK;
}

RetCode FileImgDecoder::ReadLine(ImgResDsc& dsc, const Point& start, int16_t len, uint8_t* buf)
{
    if (IsImgValidMode(dsc.imgInfo.header.colorMode)) {
        return ReadLineTrueColor(dsc, start, len, buf);
    }
    return RetCode::FAIL;
}

RetCode FileImgDecoder::ReadToCache(ImgResDsc& dsc)
{
    if (!dsc.inCache) {
        int32_t readCount = 0;
        RetCode ret = ReadFileHeader(dsc, readCount);
        if (ret != RetCode::OK) {
            return ret;
        }

        uint32_t fileSize = 0;
        ret = GetFileSize(dsc.fd, fileSize);
        if (ret != RetCode::OK) {
            return ret;
        }

        uint32_t pxCount = fileSize - readCount;
        ret = ReadImageData(dsc, pxCount);
        if (ret != RetCode::OK) {
            HandleReadFailure(dsc);
            return RetCode::OK;
        }

        dsc.inCache = true;
        close(dsc.fd);
        dsc.fd = -1;
    }

    return RetCode::OK;
}

RetCode FileImgDecoder::ReadFileHeader(ImgResDsc& dsc, int32_t& readCount)
{
    lseek(dsc.fd, 0, SEEK_SET);
    readCount = read(dsc.fd, &dsc.imgInfo.header, sizeof(ImageHeader));
    if (readCount != sizeof(ImageHeader)) {
        return RetCode::FAIL;
    }
    return RetCode::OK;
}

RetCode FileImgDecoder::GetFileSize(int32_t fd, uint32_t& fileSize)
{
    struct stat info;
    int32_t ret = fstat(fd, &info);
    if (ret != 0) {
        return RetCode::FAIL;
    }
    fileSize = info.st_size;
    return RetCode::OK;
}

RetCode FileImgDecoder::ReadImageData(ImgResDsc& dsc, uint32_t pxCount)
{
    if (dsc.imgInfo.data != nullptr) {
        ImageCacheFree(dsc.imgInfo);
        dsc.imgInfo.data = nullptr;
    }

    bool readSuccess = false;
    if (dsc.imgInfo.header.compressMode != COMPRESS_MODE_NONE) {
        readSuccess = ImageLoad::GetImageInfo(dsc.fd, pxCount, dsc.imgInfo);
    } else {
#if defined(ENABLE_GFX_ENGINES) && ENABLE_GFX_ENGINES
        int rowSize = dsc.imgInfo.header.width * DrawUtils::GetByteSizeByColorMode(dsc.imgInfo.header.colorMode);
        dsc.imgInfo.dataSize = rowSize * dsc.imgInfo.header.height;
#else
        dsc.imgInfo.dataSize = pxCount;
#endif
        dsc.imgInfo.data = reinterpret_cast<uint8_t*>(ImageCacheMalloc(dsc.imgInfo));
        if (dsc.imgInfo.data == nullptr) {
            return RetCode::OK;
        }
        uint8_t* tmp = const_cast<uint8_t*>(dsc.imgInfo.data);
#if defined(ENABLE_GFX_ENGINES) && ENABLE_GFX_ENGINES
        int32_t readTotalSize = 0;
        memset_s(reinterpret_cast<void*>(tmp), dsc.imgInfo.dataSize, 0, dsc.imgInfo.dataSize);
        for (int i = 0; i < dsc.imgInfo.header.height; i++) {
            readTotalSize += read(dsc.fd, reinterpret_cast<void*>(tmp), rowSize);
            tmp += rowSize;
        }
        readSuccess = (readTotalSize == rowSize * dsc.imgInfo.header.height);
#else
        readSuccess = (static_cast<int32_t>(pxCount) == read(dsc.fd, reinterpret_cast<void*>(tmp), pxCount));
#endif
    }

    if (!readSuccess) {
        return RetCode::FAIL;
    }
    return RetCode::OK;
}

void FileImgDecoder::HandleReadFailure(ImgResDsc& dsc)
{
    ImageCacheFree(dsc.imgInfo);
    dsc.imgInfo.data = nullptr;
    dsc.imgInfo.dataSize = 0;
    close(dsc.fd);
    dsc.fd = -1;
}

RetCode FileImgDecoder::ReadLineTrueColor(const ImgResDsc& dsc, const Point& start, int16_t len, uint8_t* buf)
{
    uint8_t pxSizeInBit = DrawUtils::GetPxSizeByColorMode(dsc.imgInfo.header.colorMode);
    off_t res;

    uint32_t pos = ((start.y * dsc.imgInfo.header.width + start.x) * pxSizeInBit) >> BYTE_TO_BIT_SHIFT;
    pos += sizeof(ImageHeader); /* Skip the header */
    res = lseek(const_cast<int32_t&>(dsc.fd), pos, SEEK_SET);
    if (res == -1) {
        return RetCode::FAIL;
    }
    uint32_t btr = len * (pxSizeInBit >> BYTE_TO_BIT_SHIFT);
    int32_t br = read(const_cast<int32_t&>(dsc.fd), buf, btr);
    if ((br == -1) || (btr != static_cast<uint32_t>(br))) {
        return RetCode::FAIL;
    }

    return RetCode::OK;
}
} // namespace OHOS
