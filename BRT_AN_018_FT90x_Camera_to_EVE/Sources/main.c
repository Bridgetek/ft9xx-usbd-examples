/**
  @file main.c
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
#include <ft900_uart_simple.h>

#include <ft900_usb.h>
#include <ft900_usbd.h>
#include <ft900_usb_uvc.h>
#include <ft900_startup_dfu.h>

/* UART support for printf output. */
#include "tinyprintf.h"

#include "camera.h"
#include "eve_app.h"

#define BRIDGE_DEBUG
#ifdef BRIDGE_DEBUG
#define BRIDGE_DEBUG_PRINTF(...) do {tfp_printf(__VA_ARGS__);} while (0)
#else
#define BRIDGE_DEBUG_PRINTF(...)
#endif

/* For MikroC const qualifier will place variables in Flash
 * not just make them constant.
 */
#if defined(__GNUC__)
#define DESCRIPTOR_QUALIFIER const
#elif defined(__MIKROC_PRO_FOR_FT90x__)
#define DESCRIPTOR_QUALIFIER data
#endif

/* CONSTANTS ***********************************************************************/

/**
 @brief Number of lines of image data to buffer.
 */
#define CAMERA_BUFFER_MAX_LINES 50
#define FORMAT_MAX_LINE_BYTES_ALIGNED ((CAMERA_PREFERRED_LINE_LENGTH + 3) & (~3))

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

// Make this a multiple of the largest line of data received.
static uint8_t camera_buffer[CAMERA_BUFFER_MAX_LINES][FORMAT_MAX_LINE_BYTES_ALIGNED]  __attribute__((aligned(4)));

/** @brief Camera and VSYNC ISR.
 * @details Keep state of camera interface and communicate with
 * the bottom half.
 */
//@{
/// Count of number of lines available from camera.
static volatile uint16_t camera_rx_data_avail = 0;
/// Buffer line within the camera_buffer array.
static volatile uint16_t camera_rx_buffer_line = 0;
//@}

/** @brief Current threshold for data received from the camera module.
 */
static uint16_t sample_threshold;

/** @brief Used to wait for vsync (GPIO)
 */
static volatile uint8_t camera_vsync = 0;

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/**
 @brief Millisecond counter
 @details Count-up timer to provide the elapsed time for network operations.
 */
static uint32_t milliseconds = 0;

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

/** @name tfp_putc
 *  @details Machine dependent putc function for tfp_printf (tinyprintf) library.
 *  @param p Parameters (machine dependent)
 *  @param c The character to write
 */
void tfp_putc(void* p, char c)
{
	uart_write((ft900_uart_regs_t*)p, (uint8_t)c);
}

/** @brief Returns the current millisecond country
 *  @returns A count of milliseconds
 */
uint32_t millis(void)
{
	return milliseconds;
}

void timer_ISR(void)
{
	if (timer_is_interrupted(timer_select_a))
	{
		milliseconds++;
	}
	if (timer_is_interrupted(timer_select_b))
	{
		eve_tick();
	}
}

static int check =0;

void cam_ISR(void)
{
	static uint8_t *pbuffer;
	static uint16_t len;

	// Synchronise on the start of a frame.
	// If we are waiting for the VSYNC signal then flush all data.
	if (camera_vsync != 0)
	{
		// Read in a line of data from the camera.
		len = cam_available();
		if (len >= sample_threshold)
		{
			// Point to the current line in the camera_buffer.
			pbuffer = camera_buffer[camera_rx_buffer_line];

			// Stream data from the camera to camera_buffer.
			// This must be aligned to and be a multiple of 4 bytes.
			asm("streamin.l %0,%1,%2" \
					: \
					  :"r"(pbuffer), "r"(&(CAM->CAM_REG3)), "r"(sample_threshold));

			// Increment the number of lines available to read.
			// This will signal data is ready to transmit.
			camera_rx_data_avail++;
			camera_rx_buffer_line++;
			if (camera_rx_buffer_line == CAMERA_BUFFER_MAX_LINES)
			{
				// Wrap around in camera_buffer.
				camera_rx_buffer_line = 0;
			}
		}
	}
	else
	{
		cam_flush();
	}
}

void vsync_ISR(void)
{
	if (gpio_is_interrupted(8))
	{
		// Signal start of frame received. Will now wait for line data.
		camera_vsync = 1;
		check = 0;
	}
}

void wait_for_vsync()
{
	camera_vsync = 0;
	while (!camera_vsync){
		// Flush all waiting data in the camera buffer.
		cam_flush();
	}

	camera_rx_buffer_line = 0;
	camera_rx_data_avail = 0;
}

uint8_t eve_testing(uint16_t module)
{
	uint16_t camera_tx_buffer_line = 0;
	uint16_t camera_tx_data_avail;
	uint8_t *pstart = NULL;

	// Length of data packet.
	uint16_t len;

	// Setup the display.
	eve_setup();

	// Draw a waiting for host message.
	eve_start();

	if (module == OV5640_PID)
	{
		tfp_printf("Starting OV5640 in JPEG mode.\r\n");
		eve_splash("Starting OV5640 in JPEG mode.");

#if CAMERA_RESOLUTION == CAMERA_USE_QVGA
		camera_set_frame(CAMERA_FRAME_WIDTH_QVGA, CAMERA_FRAME_HEIGHT_QVGA, OV5640_FRAME_SIZE_MJPEG_QVGA);
		camera_set(CAMERA_MODE_QVGA, CAMERA_FRAME_RATE_15, CAMERA_FORMAT_MJPEG);
#elif CAMERA_RESOLUTION == CAMERA_USE_VGA
		camera_set_frame(CAMERA_FRAME_WIDTH_VGA, CAMERA_FRAME_HEIGHT_VGA, OV5640_FRAME_SIZE_MJPEG_VGA);
		camera_set(CAMERA_MODE_VGA, CAMERA_FRAME_RATE_15, CAMERA_FORMAT_MJPEG);
#endif
		camera_set_sample(CAMERA_PREFERRED_LINE_LENGTH);

		camera_set_state(CAMERA_STREAMING_START);

		sample_threshold = camera_get_sample();

		tfp_printf("Camera starting (sample length %d frame %ld)\r\n", sample_threshold, camera_get_frame_size());

		cam_set_threshold(sample_threshold);
		camera_tx_buffer_line = 0;

		cam_start(sample_threshold);
		camera_start();
		wait_for_vsync();

		cam_enable_interrupt();
#if 0
		camera_stop();

		cam_disable_interrupt();
		cam_stop();

		tfp_printf("Camera stopping\r\n");

		camera_set_state(CAMERA_STREAMING_OFF);
#endif

		while (1)
		{
			cam_disable_interrupt();
			// Number of lines buffered.
			camera_tx_data_avail = camera_rx_data_avail;
			if (camera_rx_data_avail)
			{
				camera_rx_data_avail--;
			}
			cam_enable_interrupt();

			// Send a full line of data if there is data available.
			if (camera_tx_data_avail)
			{
				len = camera_get_sample();

				pstart = (uint8_t *)&camera_buffer[camera_tx_buffer_line];

				// Loop around camera_buffer.
				camera_tx_buffer_line++;
				if (camera_tx_buffer_line == CAMERA_BUFFER_MAX_LINES)
				{
					camera_tx_buffer_line = 0;
				}

				if (eve_loop(pstart, len))
				{
					//wait_for_vsync();
				}
			}
		}
	}
	else
	{
		tfp_printf("Camera not found - must be an OV5640 module.\n");
		eve_splash("Camera not found - must be an OV5640 module.");
	}
	return 0;
}

/* FUNCTIONS ***********************************************************************/

int main(void)
{
	/** 16 bit Camera Module ID	 */
	uint16_t module;

#ifdef USB_INTERFACE_USE_STARTUPDFU
	STARTUP_DFU();
#endif // USB_INTERFACE_USE_STARTUPDFU

	sys_reset_all();
	sys_disable(sys_device_camera);
	sys_disable(sys_device_i2c_master);

	/* Enable the UART Device... */
	sys_enable(sys_device_uart0);
	/* Make GPIO48 function as UART0_TXD and GPIO49 function as UART0_RXD... */
	gpio_function(48, pad_uart0_txd); /* UART0 TXD */
	gpio_function(49, pad_uart0_rxd); /* UART0 RXD */

	/* Enable tfp_printf() functionality... */
	init_printf(UART0, tfp_putc);

#ifdef BRIDGE_DEBUG
	// Open the UART using the coding required.
	uart_open(UART0, 1, UART_DIVIDER_19200_BAUD, uart_data_bits_8, uart_parity_none, uart_stop_bits_1);

	/* Print out a welcome message... */
	BRIDGE_DEBUG_PRINTF("\x1B[2J" /* ANSI/VT100 - Clear the Screen */
			"\x1B[H"  /* ANSI/VT100 - Move Cursor to Home */
	);

	BRIDGE_DEBUG_PRINTF("(C) Copyright 2016, Bridgetek Pte Ltd. \r\n");
	BRIDGE_DEBUG_PRINTF("--------------------------------------------------------------------- \r\n");
	BRIDGE_DEBUG_PRINTF("Welcome to the Camera to EVE application note BRT_AN_018. \r\n");
	BRIDGE_DEBUG_PRINTF("\r\n");
	BRIDGE_DEBUG_PRINTF("Take an MJPEG stream from a camera and display it on the EVE.\r\n");
	BRIDGE_DEBUG_PRINTF("--------------------------------------------------------------------- \r\n");
#endif // BRIDGE_DEBUG

	/* Timer A = 1ms */
	timer_prescaler(1000);
	timer_init(timer_select_a, 100, timer_direction_down, timer_prescaler_select_on, timer_mode_continuous);
	timer_enable_interrupt(timer_select_a);
	timer_start(timer_select_a);

	interrupt_attach(interrupt_timers, (int8_t)interrupt_timers, timer_ISR);

	/* Set up the Camera Interface */
	cam_disable_interrupt();
	cam_stop();
	cam_flush();

	gpio_function(6, pad_cam_xclk);  /* XCLK */
	gpio_function(7, pad_cam_pclk);  /* PCLK */
	//gpio_function(8, pad_cam_vd);  /* VD */
	gpio_function(9, pad_cam_hd);  /* HD */
	gpio_function(10, pad_cam_d7); /* D7 */
	gpio_function(11, pad_cam_d6); /* D6 */
	gpio_function(12, pad_cam_d5); /* D5 */
	gpio_function(13, pad_cam_d4); /* D4 */
	gpio_function(14, pad_cam_d3); /* D3 */
	gpio_function(15, pad_cam_d2); /* D2 */
	gpio_function(16, pad_cam_d1); /* D1 */
	gpio_function(17, pad_cam_d0); /* D0 */
	sys_enable(sys_device_camera);

	/* Set VD as a GPIO input */
	gpio_function(8, pad_gpio8);
	gpio_dir(8, pad_dir_input);
	gpio_interrupt_enable(8, gpio_int_edge_falling); /* VD */
	interrupt_attach(interrupt_gpio, (uint8_t)interrupt_gpio, vsync_ISR);

	/* Set up I2C */
	sys_enable(sys_device_i2c_master);

	gpio_function(44, pad_i2c0_scl); /* I2C0_SCL */
	gpio_function(45, pad_i2c0_sda); /* I2C0_SDA */
	gpio_function(46, pad_i2c1_scl); /* I2C1_SCL */
	gpio_function(47, pad_i2c1_sda); /* I2C1_SDA */

	i2cm_init(I2CM_NORMAL_SPEED, 100000);

	// Initialise the camera hardware.
	module = camera_init();
	if (module > 0)
	{
		/* Clock data in when VREF is low and HREF is high */
		cam_init(cam_trigger_mode_1, cam_clock_pol_raising);
		interrupt_attach(interrupt_camera, (uint8_t)interrupt_camera, cam_ISR);
	}

	interrupt_enable_globally();

	eve_testing(module);

	interrupt_disable_globally();

	// Wait forever...
	for (;;);

	return 0;
}
