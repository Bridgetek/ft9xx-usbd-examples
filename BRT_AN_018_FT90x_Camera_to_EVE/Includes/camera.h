/**
 @file camera.h
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

#ifndef SOURCES_CAMERA_H_
#define SOURCES_CAMERA_H_

/**
 @brief Supported camera interfaces.
 */
//@{
#define CAMERA_OV5640 1
#define CAMERA_OV965X 2
//@}

/**
 @brief Resolution definitions for camera interface.
 */
//@{
#define CAMERA_MODE_ANY 0
#define CAMERA_MODE_QVGA 1
#define CAMERA_MODE_VGA 2
#define CAMERA_MODE_SVGA 3
#define CAMERA_MODE_XGA 4
#define CAMERA_MODE_SXGA 5
//@}

/**
 @brief Output format definitions for camera interface.
 */
//@{
#define CAMERA_FORMAT_UNCOMPRESSED 1
#define CAMERA_FORMAT_MJPEG 2
//@}

/**
 * @brief Payload and Frame Type
 * @details Payload and Frame Types to tranmit.
 */
//@{
#define PAYLOAD_TYPE CAMERA_FORMAT_MJPEG

#if (PAYLOAD_TYPE == CAMERA_FORMAT_UNCOMPRESSED)
#define FRAME_PT 112
#define FRAME_TYPE CAMERA_MODE_QVGA
#elif (PAYLOAD_TYPE == CAMERA_FORMAT_MJPEG)
#define FRAME_PT 26
#define FRAME_TYPE CAMERA_MODE_VGA
#else
#error Unsupported payload type
#endif
//@}

/**
 @brief Frame Width and Height definitions for camera interface.
 */
//@{
#define CAMERA_FRAME_WIDTH_QQVGA 160
#define CAMERA_FRAME_WIDTH_QVGA 320
#define CAMERA_FRAME_WIDTH_VGA 640
#define CAMERA_FRAME_WIDTH_SVGA 800
#define CAMERA_FRAME_WIDTH_XGA 1024
#define CAMERA_FRAME_WIDTH_SXGA 1280

#define CAMERA_FRAME_HEIGHT_QQVGA 120
#define CAMERA_FRAME_HEIGHT_QVGA 240
#define CAMERA_FRAME_HEIGHT_VGA 480
#define CAMERA_FRAME_HEIGHT_SVGA 600
#define CAMERA_FRAME_HEIGHT_XGA 768
#define CAMERA_FRAME_HEIGHT_SXGA 1024
//@}

/**
 @brief Frame Rate definitions for camera interface.
 @details
 */
//@{
#define CAMERA_FRAME_RATE_ANY 0
#define CAMERA_FRAME_RATE_7_5 (15/2)
#define CAMERA_FRAME_RATE_10 10
#define CAMERA_FRAME_RATE_15 15
#define CAMERA_FRAME_RATE_30 30
//@}

/**
 @brief Preferred line length for camera interface
 @details This is the line length of data from the camera where the
  line length can be set. This should be small enough to fit inside
  an isochronous transfer (including the UVC header) but not fall on
  a max packet boundary for when bulk transfers are used.
 */
//@{
#define CAMERA_PREFERRED_LINE_LENGTH 512
//@}

/**
 @brief Definition of camera streaming state
 */
#define CAMERA_STREAMING_OFF 0
#define CAMERA_STREAMING_START 1
#define CAMERA_STREAMING_STOP 2

typedef void (*CAMERA_start_stop)(void);
typedef int8_t (*CAMERA_set)(int resolution, int frame_rate, int format);

/**
 @brief Camera Initialisation
 @details Initialises the camera module registers.
 @returns Return the hardware ID of the camera module.
 */
uint16_t camera_init(void);

/**
 @brief Camera Start
 @details Starts streaming data from the Camera.
 */
void camera_start(void);

/**
 @brief Camera Stop
 @details Stops streaming data from the Camera.
 */
void camera_stop(void);

/**
 @brief Camera Supports
 @details Check that the camera module supports a resolution, frame
 	 rate and output format.
 */
int8_t camera_supports(int resolution, int frame_rate, int format);

/**
 @brief Camera Setup
 @details Configures the camera module to a resolution, frame rate and
   output format.
 */
int8_t camera_set(int resolution, int frame_rate, int format);

/**
 @brief      CAMERA state change
 @details    Will flag that there is a state change to the camera.
 **/
void camera_set_state(int8_t state);

/**
 @brief      CAMERA state change
 @details    Will return the current state of the camera.
 **/
int8_t camera_get_state();

/**
 @brief      CAMERA frame setup
 @details    Sets the parameters of the current frame.
 **/
void camera_set_frame(uint16_t width, uint16_t height, uint32_t size);

/**
 @brief      CAMERA sample length setup
 @details    Sets the sampling parameters of the current frame.
 **/
void camera_set_sample(uint16_t length);

/**
 @brief      CAMERA frame size
 @details    Gets the frame size.
 **/
uint32_t camera_get_frame_size();

/**
 @brief      CAMERA sample length
 @details    Gets the sampling parameters of the current frame.
 **/
uint16_t camera_get_sample();


#include "ov5640_camera.h"

#endif /* SOURCES_CAMERA_H_ */
