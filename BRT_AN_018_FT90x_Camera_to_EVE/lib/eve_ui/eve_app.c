/**
  @file eve.c
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

#include <ft900.h>

#include "EVE_config.h"
#include "EVE.h"
#include "HAL.h"

#include "eve_ui.h"
#include "camera.h"

#include "tinyprintf.h"

/* CONSTANTS ***********************************************************************/

/**
 * @brief Show the built-in logo animation on the FT8xx.
 */
#undef SHOW_LOGO

/**
 * @brief Enable a screenshot to be taken.
 */
#undef ENABLE_SCREENSHOT

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

#ifdef SHOW_LOGO
/**
 * @brief Information string for the splash screen.
 * @details this is 4 lines long and each line is added by a Ft_Gpu_CoCmd_Text.
 */
char *info[] = { "FT8xx/FT9xx Camera to Eve",
		"BRT_AN_018",
		"Application Note to demonstrate interactive display device.",
		"Image sent via D2XX and rendered on display."
};
#endif // SHOW_LOGO

/* MACROS **************************************************************************/

#ifdef EVE_ZOOM_QVGA
#define IMAGE_SCALE 2
#else
#define IMAGE_SCALE 1
#endif

#if CAMERA_RESOLUTION == CAMERA_USE_QVGA
#define IMAGE_WIDTH CAMERA_FRAME_WIDTH_QVGA
#define IMAGE_HEIGHT CAMERA_FRAME_HEIGHT_QVGA
#elif CAMERA_RESOLUTION == CAMERA_USE_VGA
#define IMAGE_WIDTH CAMERA_FRAME_WIDTH_VGA
#define IMAGE_HEIGHT CAMERA_FRAME_HEIGHT_VGA
#endif

#define IMAGE_DISP_WIDTH (IMAGE_WIDTH * IMAGE_SCALE)
#define IMAGE_DISP_HEIGHT (IMAGE_HEIGHT * IMAGE_SCALE)

#define LEFT_X ((EVE_DISP_WIDTH / 2) - (IMAGE_DISP_WIDTH / 2))
#define RIGHT_X ((EVE_DISP_WIDTH / 2) + (IMAGE_DISP_WIDTH / 2))
#define TOP_Y ((EVE_DISP_HEIGHT / 2) - (IMAGE_DISP_HEIGHT / 2))
#define BOTTOM_Y ((EVE_DISP_HEIGHT / 2) + (IMAGE_DISP_HEIGHT / 2))

/* LOCAL FUNCTIONS / INLINES *******************************************************/

void eve_splash(char *toast)
{
	// Load BridgeTek logo image.
	eve_load_images(0);

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_CLEAR_COLOR_RGB(0, 0, 0);
	EVE_CLEAR(1,1,1);
	//EVE_CLEAR_TAG(TAG_NO_ACTION);
	EVE_COLOR_RGB(255, 255, 255);
	EVE_BEGIN(BITMAPS);
	EVE_VERTEX_TRANSLATE_X(((EVE_DISP_WIDTH/2)-((uint32_t)eve_img_bridgetek_logo_width/2)) * 16);
	EVE_VERTEX2II(0, 0, BITMAP_BRIDGETEK_LOGO, 0);
	EVE_VERTEX_TRANSLATE_X(0);
	EVE_CMD_TEXT(EVE_DISP_WIDTH/2, EVE_DISP_HEIGHT/2,
			FONT_HEADER, OPT_CENTERX|OPT_CENTERY, toast);
	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();
}

/* FUNCTIONS ***********************************************************************/

void eve_setup()
{
	EVE_Init();

	eve_calibrate();
}

void eve_wait(void)
{
	uint8_t key_code = 0;
	uint8_t key_detect = 0;

	eve_splash("Waiting for camera...");

	key_detect = eve_read_tag(&key_code);
	if (key_detect)
	{
	}
	delayms(100);
}

void eve_screenshot()
{
#ifdef ENABLE_SCREENSHOT
	uint8_t buffer[256];
	int i, j;

	CRITICAL_SECTION_BEGIN

	// Write screenshot into RAM_G
	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_CMD_SNAPSHOT(img_end_address);
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	tfp_printf("ARGB start\n"); // Use this marker to identify the start of the image.
	for (i = 0; i < (EVE_DISP_WIDTH * 2) * EVE_DISP_HEIGHT; i += sizeof(buffer))
	{
		EVE_LIB_ReadDataFromRAMG(buffer, sizeof(buffer), img_end_address+ i);
		for (j = 0; j < sizeof(buffer); j++)
		{
			tfp_printf("%c", buffer[j]);
		}
	}
	tfp_printf("ARGB end\n"); // Marker to identify the end of the image.

	eve_ui_splash("Screenshot completed...", 0);
	delayms(2000);

	CRITICAL_SECTION_END

#endif // ENABLE_SCREENSHOT
}

void eve_start(void)
{
	eve_splash("Waiting for images...");
}

int8_t eve_loop(uint8_t *buffer, int32_t length)
{
	const uint32_t image_start_address = 0;
	static uint8_t eof_pending = 0;
	static uint8_t frame_active = 0;

#ifdef EVE_SHOW_FRAME_INFO
	static uint32_t image_len = 0;
	static uint32_t count = 0;
	static char infoLen[64];
	static char infoCount[64];
	static char infoFrame[64];
#endif // EVE_SHOW_FRAME_INFO

	uint8_t *pmark;
	uint8_t *pstart = buffer;

	if (!frame_active)
	{
		if ((buffer[0] == 0xff) && (buffer[1] == 0xd8))
		{
			// Start of JPEG marker found.
			image_len = 0;
			frame_active = 1;

			EVE_LIB_BeginCoProList();
			EVE_CMD_LOADIMAGE(image_start_address, 0);
		}
	}

	if (frame_active)
	{
		length = (length + 3) & (~3);

		// Write JPEG data to command buffer
		EVE_LIB_WriteDataToCMD(buffer, length);

		// Find 0xff 0xd9 for end of image!
		if (eof_pending)
		{
			if (*pstart == 0xd9)
			{
				frame_active = 0;
				length = 1;
			}
			eof_pending = 0;
		}

		pmark = pstart;
		while (pmark)
		{
			pmark = memchr(pmark, 0xff, length - (pmark - pstart));
			if (pmark)
			{
				pmark++;
				if ((pmark - pstart) == length)
				{
					// The last byte of the sample is 0xff so there might be
					// and end of frame marker continued on the next sample.
					eof_pending = 1;
				}
				else if (*pmark == 0xd9)
				{
					frame_active = 0;
					length = pmark - pstart + 1;
				}
			}
		}

		image_len += length;

		if (!frame_active)
		{
#ifdef EVE_SHOW_FRAME_INFO
			sprintf(infoFrame, "Camera Frame: %dx%d", IMAGE_WIDTH, IMAGE_HEIGHT);
			sprintf(infoLen, "Image Size: %lu", image_len);
			sprintf(infoCount, "Frame count: %lu", count++);
#endif // EVE_SHOW_FRAME_INFO

			// Finalise decoding of JPEG data
			EVE_LIB_EndCoProList();

			// New display list to show JPEG image
			EVE_LIB_BeginCoProList();
			EVE_CMD_DLSTART();
			EVE_CLEAR(1,1,1);

			EVE_BEGIN(BITMAPS);
			EVE_BITMAP_HANDLE(BITMAP_JPEG);
			EVE_BITMAP_SOURCE(image_start_address);
			EVE_BITMAP_LAYOUT(RGB565, IMAGE_WIDTH * 2, IMAGE_HEIGHT);
			EVE_BITMAP_LAYOUT_H((IMAGE_WIDTH * 2) >> 10, IMAGE_HEIGHT >> 9);

#if CAMERA_RESOLUTION == CAMERA_USE_QVGA
#ifdef EVE_ZOOM_QVGA
			// Zoom QVGA image to VGA image
			EVE_CMD_LOADIDENTITY();
			EVE_CMD_SCALE(2 << 16, 2 << 16);
			EVE_CMD_SETMATRIX();
#endif // EVE_ZOOM_QVGA
			EVE_BITMAP_SIZE(NEAREST, BORDER, BORDER,
					IMAGE_WIDTH << 1, IMAGE_HEIGHT << 1);
			EVE_BITMAP_SIZE_H(IMAGE_WIDTH >> 8, IMAGE_HEIGHT >> 8);

#elif CAMERA_RESOLUTION == CAMERA_USE_VGA
			EVE_BITMAP_SIZE(NEAREST, BORDER, BORDER,
					IMAGE_WIDTH, IMAGE_HEIGHT);
			EVE_BITMAP_SIZE_H(IMAGE_WIDTH >> 9, IMAGE_HEIGHT >> 9);
#endif
			// Display image centred on screen
			EVE_VERTEX2II(LEFT_X, TOP_Y, BITMAP_JPEG, 0);
			EVE_END();

#ifdef EVE_ZOOM_QVGA
			// Return scale to 1:1
			EVE_CMD_LOADIDENTITY();
			EVE_CMD_SCALE(1 << 16, 1 << 16);
			EVE_CMD_SETMATRIX();
#endif // EVE_ZOOM_QVGA

#ifdef EVE_USE_OVERLAY
			EVE_BEGIN(LINES);
			EVE_COLOR_RGB(128, 0, 0);
			EVE_LINE_WIDTH(2 * 16);
			// Draw overlay lines
			EVE_VERTEX2F(LEFT_X * 16, BOTTOM_Y * 16);
			EVE_VERTEX2F((LEFT_X + (IMAGE_DISP_WIDTH / 3)) * 16, (BOTTOM_Y / 2) * 16);
			EVE_VERTEX2F((LEFT_X + (IMAGE_DISP_WIDTH / 3)) * 16, (BOTTOM_Y / 2) * 16) ;
			EVE_VERTEX2F((LEFT_X + (IMAGE_DISP_WIDTH * 2 / 3)) * 16, (BOTTOM_Y / 2) * 16);
			EVE_VERTEX2F((LEFT_X + (IMAGE_DISP_WIDTH * 2 / 3)) * 16, (BOTTOM_Y / 2) * 16);
			EVE_VERTEX2F((LEFT_X + (IMAGE_DISP_WIDTH)) * 16, BOTTOM_Y * 16);
#endif // EVE_USE_OVERLAY

#ifdef EVE_SHOW_FRAME_INFO
			EVE_COLOR_RGB(255, 255, 255);
			EVE_CMD_TEXT(LEFT_X, TOP_Y, 27, 0, infoFrame);
			EVE_CMD_TEXT(LEFT_X, TOP_Y + 20, 27, 0, infoLen);
			EVE_CMD_TEXT(LEFT_X, TOP_Y + 40, 27, 0, infoCount);
#endif // EVE_SHOW_FRAME_INFO

			EVE_DISPLAY();
			EVE_CMD_SWAP();
			EVE_LIB_EndCoProList();
		}
	}

	return frame_active;
}

void eve_close(void)
{
}

void eve_tick(void)
{
}
