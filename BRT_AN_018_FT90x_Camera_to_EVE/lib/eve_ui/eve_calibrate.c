/**
 @file eve_calibrate.c
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

#include <stdint.h>
#include <string.h>

#include <ft900.h>
#include <ft900_dlog.h>

#include "EVE_config.h"
#include "EVE.h"
#include "HAL.h"

#include "eve_ui.h"

/* CONSTANTS ***********************************************************************/

/**
 @brief Page number in datalogger memory in Flash for touchscreen calibration
 values.
 */
#define CONFIG_PAGE_TOUCHSCREEN 0
/**
 @brief Key for identifying if touchscreen calibration values are programmed into
 datalogger memory in the Flash.
 */
#define VALID_KEY_TOUCHSCREEN 0xd72f91a3

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/**
 @brief Structure to hold touchscreen calibration settings.
 @details This is used to store the touchscreen calibration settings persistently
 in Flash and identify if the calibration needs to be re-performed.
 */
struct touchscreen_calibration {
	uint32_t key; // VALID_KEY_TOUCHSCREEN
	uint32_t transform[6];
};

/**
 @brief Link to datalogger area defined in crt0.S file.
 @details Must be passed to dlog library functions to initialise and use
        datalogger functions. We use the datalogger area for persistent
        configuration storage.
 */
extern __flash__ uint32_t __dlog_partition[];

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

/**
 * @brief Functions used to store calibration data in flash.
 */
//@{
int8_t flash_calib_init(void)
{
	int	pgsz;
	int	i;
	int ret;

	ret = dlog_init(__dlog_partition, &pgsz, &i);
	if (ret < 0)
	{
		// Project settings incorrect. Require dlog support with modified
		// linker script and crt0.S file.
		// See AN_398 for examples.
		return -1;
	}
	return 0;
}

int8_t flash_calib_write(struct touchscreen_calibration *calib)
{
	uint8_t	flashbuf[260] __attribute__((aligned(4)));
	dlog_erase();

	calib->key = VALID_KEY_TOUCHSCREEN;
	memset(flashbuf, 0xff, sizeof(flashbuf));
	memcpy(flashbuf, calib, sizeof(struct touchscreen_calibration));
	if (dlog_prog(CONFIG_PAGE_TOUCHSCREEN, (uint32_t *)flashbuf) < 0)
	{
		// Flash not written.
		return -1;
	}

	return 0;
}

int8_t flash_calib_read(struct touchscreen_calibration *calib)
{
	uint8_t	flashbuf[260] __attribute__((aligned(4)));
	memset(flashbuf, 0x00, sizeof(flashbuf));
	if (dlog_read(CONFIG_PAGE_TOUCHSCREEN, (uint32_t *)flashbuf) < 0)
	{
		return -1;
	}

	if (((struct touchscreen_calibration *)flashbuf)->key == VALID_KEY_TOUCHSCREEN)
	{
		memcpy(calib, flashbuf, sizeof(struct touchscreen_calibration));
		return 0;
	}
	return -2;
}
//@}

/* FUNCTIONS ***********************************************************************/

void eve_calibrate(void)
{
	struct touchscreen_calibration calib;
	uint8_t dummy;

	flash_calib_init();

	// If no store of calibration or current screen touch.
	if ((flash_calib_read(&calib) != 0) || (eve_read_tag(&dummy)))
	{
		// Wait for end of touch.
		while (eve_read_tag(&dummy));

		EVE_LIB_BeginCoProList();
		EVE_CMD_DLSTART();
		EVE_CLEAR_COLOR_RGB(0, 0, 0);
		EVE_CLEAR(1,1,1);
		EVE_COLOR_RGB(255, 255, 255);
		EVE_CMD_TEXT(EVE_DISP_WIDTH/2, EVE_DISP_HEIGHT/2,
				28, OPT_CENTERX|OPT_CENTERY,"Please tap on the dots");
		EVE_CMD_CALIBRATE(0);
		EVE_LIB_EndCoProList();
		EVE_LIB_AwaitCoProEmpty();

		calib.transform[0] = HAL_MemRead32(REG_TOUCH_TRANSFORM_A);
		calib.transform[1] = HAL_MemRead32(REG_TOUCH_TRANSFORM_B);
		calib.transform[2] = HAL_MemRead32(REG_TOUCH_TRANSFORM_C);
		calib.transform[3] = HAL_MemRead32(REG_TOUCH_TRANSFORM_D);
		calib.transform[4] = HAL_MemRead32(REG_TOUCH_TRANSFORM_E);
		calib.transform[5] = HAL_MemRead32(REG_TOUCH_TRANSFORM_F);
		flash_calib_write(&calib);
	}
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_A, calib.transform[0]);
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_B, calib.transform[1]);
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_C, calib.transform[2]);
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_D, calib.transform[3]);
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_E, calib.transform[4]);
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_F, calib.transform[5]);
}

