// LUFA Library Configuration Header File. Used to configure LUFA's compile time options, as an alternative to the compile-time defines.
#ifndef _LUFA_CONFIG_H_
#define _LUFA_CONFIG_H_

	/* USB Device descriptor parameter */
	#define VENDOR_ID       0xFEED
	#define PRODUCT_ID      0x6060
	#define DEVICE_VER      0x0001
	#define MANUFACTURER    geekhack
	#define PRODUCT         GH60
	#define DESCRIPTION     t.m.k. keyboard firmware for GH60

	/* key matrix size */
	#define MATRIX_ROWS 3
	#define MATRIX_COLS 10

	/* define if matrix has ghost */
	//#define MATRIX_HAS_GHOST

	/* Set 0 if debouncing isn't needed */
	#define DEBOUNCE    5

	/* Mechanical locking support. Use KC_LCAP, KC_LNUM or KC_LSCR instead in keymap */
	#define LOCKING_SUPPORT_ENABLE
	/* Locking resynchronize hack */
	#define LOCKING_RESYNC_ENABLE

	/* key combination for command */
	#define IS_COMMAND() ( \
	    keyboard_report->mods == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_RSHIFT)) \
	)



	/*
	 * Feature disable options
	 *  These options are also useful to firmware size reduction.
	 */

	/* disable debug print */
	//#define NO_DEBUG

	/* disable print */
	//#define NO_PRINT

	/* disable action features */
	//#define NO_ACTION_LAYER
	//#define NO_ACTION_TAPPING
	//#define NO_ACTION_ONESHOT
	//#define NO_ACTION_MACRO
	//#define NO_ACTION_FUNCTION


	#if (ARCH == ARCH_AVR8)
		// Non-USB Related Configuration Tokens
		// #define DISABLE_TERMINAL_CODES

		// USB Class Driver Related Tokens
		// #define HID_HOST_BOOT_PROTOCOL_ONLY
		// #define HID_STATETABLE_STACK_DEPTH       {Insert Value Here}
		// #define HID_USAGE_STACK_DEPTH            {Insert Value Here}
		// #define HID_MAX_COLLECTIONS              {Insert Value Here}
		// #define HID_MAX_REPORTITEMS              {Insert Value Here}
		// #define HID_MAX_REPORT_IDS               {Insert Value Here}
		// #define NO_CLASS_DRIVER_AUTOFLUSH

		// General USB Driver Related Tokens
		// #define ORDERED_EP_CONFIG
		#define USE_STATIC_OPTIONS               (USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)
		#define USB_DEVICE_ONLY
		// #define USB_HOST_ONLY
		// #define USB_STREAM_TIMEOUT_MS            {Insert Value Here}
		// #define NO_LIMITED_CONTROLLER_CONNECT
		// #define NO_SOF_EVENTS

		// USB Device Mode Driver Related Tokens
		// #define USE_RAM_DESCRIPTORS
		#define USE_FLASH_DESCRIPTORS
		// #define USE_EEPROM_DESCRIPTORS
		// #define NO_INTERNAL_SERIAL
		#define FIXED_CONTROL_ENDPOINT_SIZE      64
		// #define DEVICE_STATE_AS_GPIOR            {Insert Value Here}
		#define FIXED_NUM_CONFIGURATIONS         1
		// #define CONTROL_ONLY_DEVICE
		// #define INTERRUPT_CONTROL_ENDPOINT
		// #define NO_DEVICE_REMOTE_WAKEUP
		// #define NO_DEVICE_SELF_POWER

		// USB Host Mode Driver Related Tokens
		// #define HOST_STATE_AS_GPIOR              {Insert Value Here}
		// #define USB_HOST_TIMEOUT_MS              {Insert Value Here}
		// #define HOST_DEVICE_SETTLE_DELAY_MS	    {Insert Value Here}
		// #define NO_AUTO_VBUS_MANAGEMENT
		// #define INVERTED_VBUS_ENABLE_LINE

	#elif (ARCH == ARCH_XMEGA)
		// Non-USB Related Configuration Tokens
		// #define DISABLE_TERMINAL_CODES

		// USB Class Driver Related Tokens
		// #define HID_HOST_BOOT_PROTOCOL_ONLY
		// #define HID_STATETABLE_STACK_DEPTH       {Insert Value Here}
		// #define HID_USAGE_STACK_DEPTH            {Insert Value Here}
		// #define HID_MAX_COLLECTIONS              {Insert Value Here}
		// #define HID_MAX_REPORTITEMS              {Insert Value Here}
		// #define HID_MAX_REPORT_IDS               {Insert Value Here}
		// #define NO_CLASS_DRIVER_AUTOFLUSH

		// General USB Driver Related Tokens
		#define USE_STATIC_OPTIONS               (USB_DEVICE_OPT_FULLSPEED | USB_OPT_RC32MCLKSRC | USB_OPT_BUSEVENT_PRIHIGH)
		// #define USB_STREAM_TIMEOUT_MS            {Insert Value Here}
		// #define NO_LIMITED_CONTROLLER_CONNECT
		// #define NO_SOF_EVENTS

		// USB Device Mode Driver Related Tokens
		// #define USE_RAM_DESCRIPTORS
		#define USE_FLASH_DESCRIPTORS
		// #define USE_EEPROM_DESCRIPTORS
		// #define NO_INTERNAL_SERIAL
		#define FIXED_CONTROL_ENDPOINT_SIZE      64
		// #define DEVICE_STATE_AS_GPIOR            {Insert Value Here}
		#define FIXED_NUM_CONFIGURATIONS         1
		// #define CONTROL_ONLY_DEVICE
		#define MAX_ENDPOINT_INDEX               1
		// #define NO_DEVICE_REMOTE_WAKEUP
		// #define NO_DEVICE_SELF_POWER

	#else
		#error Unsupported architecture for this LUFA configuration file.
	#endif
#endif
