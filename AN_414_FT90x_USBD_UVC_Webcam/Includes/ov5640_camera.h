/**
 @file ov5640_camera.h
 */
/*
 * ============================================================================
 * History
 * =======
 * 2017-03-15 : Created
 *
 * (C) Copyright Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://www.ftdichip.com/FTSourceCodeLicenceTerms.htm ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */

#ifndef SOURCES_OV5640_CAMERA_H_
#define SOURCES_OV5640_CAMERA_H_

/**
 @brief Product Identifier for the OV5640
 @details The PID is read from registers on the module to positively
   identify the module as an OV5640.
 */
#define OV5640_PID	0x5640

/**
 @brief MJPEG frame size estimates
 @details The MJPEG frame is output as a compressed image. This has a
   variable size. The registers on the module are programmed to output
   a data set large enough to hold the biggest JPEG image file. This is
   organised into lines. Lines must be smaller than the buffer size on
   the FT900 camera interface (2kB). The preferred line length is a
   value designed to give good USB transfer rates with configured max
   packet sizes. It is defined in the USB section of the header file.
 */
//@{
#define OV5640_LINE_COUNT_MJPEG_QVGA (((6 * 1024)/OV5640_PREFERRED_LINE_LENGTH) + 1)
#define OV5640_LINE_COUNT_MJPEG_VGA (((16 * 1024)/OV5640_PREFERRED_LINE_LENGTH) + 1)
#define OV5640_LINE_COUNT_MJPEG_SVGA (((20 * 1024)/OV5640_PREFERRED_LINE_LENGTH) + 1)
#define OV5640_LINE_COUNT_MJPEG_XGA (((28 * 1024)/OV5640_PREFERRED_LINE_LENGTH) + 1)
#define OV5640_LINE_COUNT_MJPEG_SXGA (((36 * 1024)/OV5640_PREFERRED_LINE_LENGTH) + 1)

#define OV5640_FRAME_SIZE_MJPEG_QVGA    (OV5640_PREFERRED_LINE_LENGTH * OV5640_LINE_COUNT_MJPEG_QVGA)
#define OV5640_FRAME_SIZE_MJPEG_VGA	    (OV5640_PREFERRED_LINE_LENGTH * OV5640_LINE_COUNT_MJPEG_VGA)
#define OV5640_FRAME_SIZE_MJPEG_SVGA    (OV5640_PREFERRED_LINE_LENGTH * OV5640_LINE_COUNT_MJPEG_SVGA)
#define OV5640_FRAME_SIZE_MJPEG_XGA	    (OV5640_PREFERRED_LINE_LENGTH * OV5640_LINE_COUNT_MJPEG_XGA)
#define OV5640_FRAME_SIZE_MJPEG_SXGA    (OV5640_PREFERRED_LINE_LENGTH * OV5640_LINE_COUNT_MJPEG_SXGA)
//@}

/**
 @brief Preferred line length for camera interface
 @details This is the line length of data from the camera where the
  line length can be set. This should be small enough to fit inside
  an isochronous transfer (including the UVC header) but not fall on
  a max packet boundary for when bulk transfers are used.
 */
//@{
#define OV5640_PREFERRED_LINE_LENGTH 0x580
//@}

/**
 @brief Format Bytes Per Pixel definition for camera uncompressed output.
 @details Derived from the image type.
 */
#define OV5640_BBP (16 >> 3)

/**
 @brief OV5640 Initialisation
 @details Initialises the OV5640 module registers and checks the module is
   an OV5640.
 */
uint16_t ov5640_init(void);

/**
 @brief OV5640 Start
 @details Starts streaming data from the OV5640.
 */
void ov5640_start(void);

/**
 @brief OV5640 Stop
 @details Stops streaming data from the OV5640.
 */
void ov5640_stop(void);

/**
 @brief Camera Supports
 @details Check that the OV5640 supports a resolution, frame
 	 rate and output format.
 */
int8_t ov5640_supports(uint16_t width, uint16_t height, int8_t frame_rate, int8_t format);

/**
 @brief OV5640 Setup
 @details Configures the OV5640 module to a resolution, frame rate and
   output format.
 @returns 0 for success. -1 for not supported.
 	 Sample size (line length) for setting. Frame size for setting.
 */
int8_t ov5640_set(uint16_t width, uint16_t height, int8_t format,
		int8_t *frame_rate, uint16_t *sample_size, uint32_t *frame_size);

#endif /* SOURCES_OV5640_CAMERA_H_ */
