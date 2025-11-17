// image_to_jpeg.h -Efficient encoding interface for image to JPEG conversion
// JPEG encoding implementation that saves about 8KB SRAM

#pragma once
#include "sdkconfig.h"
#ifndef CONFIG_IDF_TARGET_ESP32

#include <stdint.h>
#include <stddef.h>
#include <linux/videodev2.h>

typedef uint32_t v4l2_pix_fmt_t; // see linux/videodev2.h for details


#ifdef __cplusplus
extern "C" {
#endif

// JPEG output callback function type
// arg: user-defined parameter, index: current data index, data: JPEG data block, len: data block length
// Returns: actual number of bytes processed

typedef size_t (*jpg_out_cb)(void *arg, size_t index, const void *data, size_t len);

/**
 * @brief Efficient conversion of image formats to JPEG
 * 
 *This function uses an optimized JPEG encoder for encoding. Main features:
 *-Save about 8KB of SRAM usage (static variables changed to heap allocation)
 *-Supports input of multiple image formats
 *-High quality JPEG output
 * 
 * @param src source image data
 * @param src_len source image data length
 * @param width image width
 * @param height image height  
 * @param format image format (PIXFORMAT_RGB565, PIXFORMAT_RGB888, etc.)
 * @param quality JPEG quality (1-100)
 * @param out output JPEG data pointer (needs to be released by the caller)
 * @param out_len output JPEG data length
 * 
 * @return true for success, false for failure
 */
bool image_to_jpeg(uint8_t *src, size_t src_len, uint16_t width, uint16_t height, 
                   v4l2_pix_fmt_t format, uint8_t quality, uint8_t **out, size_t *out_len);

/**
 * @brief Convert image format to JPEG (callback version)
 * 
 *Use callback function to process JPEG output data, suitable for streaming or chunked processing:
 *-Save about 8KB of SRAM usage (static variables changed to heap allocation)
 *-Supports streaming output without preallocating large buffers
 *-Process JPEG data block by block through callback function
 * 
 * @param src source image data
 * @param src_len source image data length
 * @param width image width
 * @param height image height
 * @param format image format
 * @param quality JPEG quality (1-100)
 * @param cb output callback function
 * @param arg User parameters passed to the callback function
 * 
 * @return true for success, false for failure
 */
bool image_to_jpeg_cb(uint8_t *src, size_t src_len, uint16_t width, uint16_t height, 
                      v4l2_pix_fmt_t format, uint8_t quality, jpg_out_cb cb, void *arg);

#ifdef __cplusplus
}
#endif

#endif // ndef CONFIG_IDF_TARGET_ESP32

