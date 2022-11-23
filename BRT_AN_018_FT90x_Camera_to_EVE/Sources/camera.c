/**
  @file camera.c
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

/* INCLUDES ************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "camera.h"

#include "tinyprintf.h"

#define CAMERA_DEBUG
#ifdef CAMERA_DEBUG
#define CAMERA_DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define CAMERA_DEBUG_PRINTF(...)
#endif

static CAMERA_start_stop CAMERA_start_fn;
static CAMERA_start_stop CAMERA_stop_fn;
static CAMERA_set CAMERA_set_fn;
static CAMERA_set CAMERA_supports_fn;

/** @brief Camera state change flag.
 * @details Signals bottom half that a commit has changed something about the camera image.
 */
static uint8_t camera_state_change = CAMERA_STREAMING_OFF;

/** @brief Current set image information.
 *  @details Updated by a commit and used to time frames.
 */
//@{
/// @brief Total pixels in a line or sample received from the camera module.
/// @details This is for reporting information only.
static uint16_t frame_width = 0;
/// @brief Total lines in a frame received from the camera module.
/// @details This is for reporting information only.
static uint16_t frame_height = 0;
/// @brief Total size of the frame received from the camera module.
static uint32_t frame_size = 0;
/// @brief Total bytes in a line or sample received from the camera module.
static uint16_t sample_length = 0;

uint16_t camera_init(void)
{
	uint16_t type;
	CAMERA_DEBUG_PRINTF("Camera Test ");

	type = ov5640_init();
	if (type != 0)
	{
		CAMERA_DEBUG_PRINTF("OV5640\r\n");

		CAMERA_start_fn = ov5640_start;
		CAMERA_stop_fn = ov5640_stop;
		CAMERA_set_fn = ov5640_set;
		CAMERA_supports_fn = ov5640_supports;

		return type;
	}

	CAMERA_DEBUG_PRINTF("none!\r\n");

	return 0;
}

/**
 * @brief CAMERA start.
 */
void camera_start(void)
{
	return CAMERA_start_fn();
}

/**
 * @brief CAMERA stop.
 */
void camera_stop(void)
{
	return CAMERA_stop_fn();
}

/**
 * @brief CAMERA supports.
 */
int8_t camera_supports(int resolution, int frame_rate, int format)
{
	return CAMERA_supports_fn(resolution, frame_rate, format);
}

/**
 * @brief CAMERA setup.
 */
int8_t camera_set(int resolution, int frame_rate, int format)
{
	return CAMERA_set_fn(resolution, frame_rate, format);
}

/**
 @brief      CAMERA state change
 @details    Will flag that there is a state change to the camera.
 **/
void camera_set_state(int8_t state)
{
	camera_state_change = state;
}

/**
 @brief      CAMERA state change
 @details    Will return the current state of the camera.
 **/
int8_t camera_get_state()
{
	return camera_state_change;
}

/**
 @brief      CAMERA frame setup
 @details    Sets the parameters of the current frame.
 **/
void camera_set_frame(uint16_t width, uint16_t height, uint32_t size)
{
	frame_width = width;
	frame_height = height;
	frame_size = size;
}

/**
 @brief      CAMERA frame size
 @details    Gets the frame size.
 **/
uint32_t camera_get_frame_size()
{
	return frame_size;
}

/**
 @brief      CAMERA sample length setup
 @details    Sets the sampling parameters of the current frame.
 **/
void camera_set_sample(uint16_t length)
{
	sample_length = length;
}

/**
 @brief      CAMERA sample length
 @details    Gets the sampling parameters of the current frame.
 **/
uint16_t camera_get_sample()
{
	return sample_length;
}
