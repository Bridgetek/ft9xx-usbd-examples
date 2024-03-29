/**
 @file eve_helper.c
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

#include "EVE.h"
#include "HAL.h"

#include "eve_ui.h"

uint8_t eve_read_tag(uint8_t *key)
{
	uint8_t Read_tag;
	uint8_t key_detect = 0;

	Read_tag = HAL_MemRead8(REG_TOUCH_TAG);

	if (!(HAL_MemRead16(REG_TOUCH_RAW_XY) & 0x8000))
	{
		key_detect = 1;
		*key = Read_tag;
	}

	return key_detect;
}

uint8_t eve_read_touchscreen(uint16_t *x, uint16_t *y)
{
	uint8_t key_detect = 0;
	uint32_t touch;

	touch = HAL_MemRead32(REG_TOUCH_SCREEN_XY);

	if (touch & 0x8000)
	{
		// no touch
		key_detect = 0;
	}
	else
	{
		key_detect = 1;
		*x = (touch >> 16) & 0x7fff;
		*y = touch & 0x7fff;
	}

	return key_detect;
}

