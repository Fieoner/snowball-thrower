/*
Nintendo Switch Fightstick - Proof-of-Concept

Based on the LUFA library's Low-Level Joystick Demo
	(C) Dean Camera
Based on the HORI's Pokken Tournament Pro Pad design
	(C) HORI

This project implements a modified version of HORI's Pokken Tournament Pro Pad
USB descriptors to allow for the creation of custom controllers for the
Nintendo Switch. This also works to a limited degree on the PS3.

Since System Update v3.0.0, the Nintendo Switch recognizes the Pokken
Tournament Pro Pad as a Pro Controller. Physical design limitations prevent
the Pokken Controller from functioning at the same level as the Pro
Controller. However, by default most of the descriptors are there, with the
exception of Home and Capture. Descriptor modification allows us to unlock
these buttons for our use.
*/

#include <avr/io.h>
#include "Joystick.h"
#include "timer.h"
#include "print.h"
#include "debug.h"

#define MATRIX_ROWS 3
#define MATRIX_COLS 10
#define DEBOUNCE 5
#define CONSOLE_ENABLE


typedef uint16_t matrix_row_t;

typedef enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	X,
	Y,
	A,
	B,
	L,
	R,
	THROW,
	NOTHING,
	TRIGGERS
} Buttons_t;

typedef struct {
	Buttons_t button;
	uint16_t duration;
} command; 

typedef struct {
	bool UP;
	bool DOWN;
	bool LEFT;
	bool RIGHT;
	bool X;
	bool Y;
	bool A;
	bool B;
	bool L;
	bool R;
	bool ZL;
	bool ZR;
	bool CAPTURE;
	bool HOME;
	bool MINUS;
	bool PLUS;
} keystate;

static keystate ks = { false, false, false, false, false, false, false, false, false, false };
static bool debouncing = false;
static uint16_t debouncing_time = 0;


/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];

static matrix_row_t read_cols(void);
static void init_cols(void);
void unselect_rows(void);
void select_row(uint8_t row);
uint16_t matrix_get_row(uint16_t row);

// Main entry point.
int main(void) {
	// We'll start by performing hardware and peripheral setup.
	SetupHardware();
	// We'll then enable global interrupts for our use.
	GlobalInterruptEnable();
	matrix_init();
	// Once that's done, we'll enter an infinite loop.
	for (;;)
	{
		keyboard_task();
		// We need to run our task to process and deliver data for our IN and OUT endpoints.
		HID_Task();
		// We also need to run the main USB management task.
		USB_USBTask();
	}
}

void matrix_print(void)
{
    print("r/c 0123456789ABCDEF\n");

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {

#if (MATRIX_COLS <= 8)
        xprintf("%02X: %08b%s\n", row, bitrev(matrix_get_row(row)),
#elif (MATRIX_COLS <= 16)
        xprintf("%02X: %016b%s\n", row, bitrev16(matrix_get_row(row)),
#elif (MATRIX_COLS <= 32)
        xprintf("%02X: %032b%s\n", row, bitrev32(matrix_get_row(row)),
#endif
#ifdef MATRIX_HAS_GHOST
        matrix_has_ghost_in_row(row) ?  " <ghost" : ""
#else
        ""
#endif
        );
    }
}

uint16_t matrix_get_row(uint16_t row) {
	return matrix[row];
}

void keyboard_task(void){
	static matrix_row_t matrix_prev[MATRIX_ROWS];
#ifdef MATRIX_HAS_GHOST
	static matrix_row_t matrix_ghost[MATRIX_ROWS];
#endif
	matrix_row_t matrix_row = 0;
	matrix_row_t matrix_change = 0;

	matrix_scan();
	for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
		matrix_row = matrix_get_row(r);
		matrix_change = matrix_row ^ matrix_prev[r];
		if (matrix_change) {
/*
#ifdef MATRIX_HAS_GHOST
			if (has_ghost_in_row(r)) {
				matrix_ghost[r] = matrix_row;
				continue;
			}
			matrix_ghost[r] = matrix_row;
#endif
*/
			//if (true) matrix_print();
			matrix_row_t col_mask = 1;
			for (uint8_t c = 0; c < MATRIX_COLS; c++, col_mask <<=1) {
				if (matrix_change & col_mask) {
					ks.A = true;
					matrix_prev[r] ^= col_mask;
				}
			}
		}
	}
	
}

void matrix_init(void) {
	unselect_rows();
	init_cols();
	for (uint8_t i=0; i < MATRIX_ROWS; i++) {
		matrix[i] = 0;
		matrix_debouncing[i] = 0;
	}
}

void  init_cols(void)
{
    // Input with pull-up(DDR:0, PORT:1)
    DDRD  &= ~(1<<3 | 1<<2 | 1<<1 | 1<<0 | 1<<4);
    PORTD |=  (1<<3 | 1<<2 | 1<<1 | 1<<0 | 1<<4);
    DDRC  &= ~(1<<6);
    PORTC |=  (1<<6);
    DDRD  &= ~(1<<7);
    PORTD |=  (1<<7);
    DDRE  &= ~(1<<6);
    PORTE |=  (1<<6);
    DDRB  &= ~(1<<4 | 1<<5);
    PORTB |=  (1<<4 | 1<<5);
}

matrix_row_t read_cols(void){
	return (PIND&(1<<3) ? 0 : (1<<0)) |
           (PIND&(1<<2) ? 0 : (1<<1)) |
           (PIND&(1<<1) ? 0 : (1<<2)) |
           (PIND&(1<<0) ? 0 : (1<<3)) |
           (PIND&(1<<4) ? 0 : (1<<4)) |
           (PINC&(1<<6) ? 0 : (1<<5)) |
           (PIND&(1<<7) ? 0 : (1<<6)) |
           (PINE&(1<<6) ? 0 : (1<<7)) |
           (PINB&(1<<4) ? 0 : (1<<8)) |
           (PINB&(1<<5) ? 0 : (1<<9));
}

void matrix_scan(void) {
	ks.UP = false;
	ks.DOWN = false;
	ks.LEFT = false;
	ks.RIGHT = false;
	ks.X = false;
	ks.B = false;
	ks.Y = false;
	ks.A = false;
	ks.R = false;
	ks.L = false;
	ks.ZR = false;
	ks.ZL = false;
	ks.CAPTURE = false;
	ks.HOME = false;
	ks.MINUS = false;
	ks.PLUS = false;
	for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
 		select_row(i);
		_delay_us(1);
		matrix_row_t cols = read_cols();
		if (i == 2 && cols&(1<<2)) ks.UP = true;
		if (i == 1 && cols&(1<<2)) ks.DOWN = true;
		if (i == 1 && cols&(1<<1)) ks.LEFT = true;
		if (i == 1 && cols&(1<<3)) ks.RIGHT = true;
		if (i == 2 && cols&(1<<7)) ks.X = true;
		if (i == 1 && cols&(1<<6)) ks.B = true;
		if (i == 2 && cols&(1<<6)) ks.Y = true;
		if (i == 1 && cols&(1<<7)) ks.A = true;
		if (i == 1 && cols&(1<<8)) ks.R = true;
		if (i == 2 && cols&(1<<8)) ks.L = true;
		if (i == 1 && cols&(1<<9)) ks.ZR = true;
		if (i == 2 && cols&(1<<9)) ks.ZL = true;
		if (i == 0 && cols&(1<<4)) ks.CAPTURE = true;
		if (i == 0 && cols&(1<<5)) ks.HOME = true;
		if (i == 1 && cols&(1<<4)) ks.MINUS = true;
		if (i == 1 && cols&(1<<5)) ks.PLUS = true;
		if (matrix_debouncing[i] != cols) {
			if (debouncing) {
				dprintf("bounce: %d %d@%02X\n", timer_elapsed(debouncing_time), i, matrix_debouncing[i]^cols);
			}
			matrix_debouncing[i] = cols;
			debouncing = true;
			debouncing_time = timer_read();
		}
		unselect_rows();
	}

	if (debouncing && timer_elapsed(debouncing_time) >= DEBOUNCE) {
		for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
			matrix[i] = matrix_debouncing[i];
		}
		debouncing = false;
	}
	return 1;
}
void unselect_rows(void)
{
    // Hi-Z(DDR:0, PORT:0) to unselect
    DDRB  &= ~0b01001010;
    PORTB &= ~0b01001010;
}

void select_row(uint8_t row)
{
    // Output low(DDR:1, PORT:0) to select
    switch (row) {
        case 0:
            DDRB  |= (1<<3);
            PORTB &= ~(1<<3);
            break;
        case 1:
            DDRB  |= (1<<1);
            PORTB &= ~(1<<1);
            break;
        case 2:
            DDRB  |= (1<<6);
            PORTB &= ~(1<<6);
            break;
    }
}


// Configures hardware and peripherals, such as the USB peripherals.
void SetupHardware(void) {
	// We need to disable watchdog if enabled by bootloader/fuses.
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// We need to disable clock division before initializing the USB hardware.
	clock_prescale_set(clock_div_1);
	// We can then initialize our hardware and peripherals, including the USB stack.

	#ifdef ALERT_WHEN_DONE
	// Both PORTD and PORTB will be used for the optional LED flashing and buzzer.
	#warning LED and Buzzer functionality enabled. All pins on both PORTB and \
PORTD will toggle when printing is done.
	DDRD  = 0xFF; //Teensy uses PORTD
	PORTD =  0x0;
                  //We'll just flash all pins on both ports since the UNO R3
	DDRB  = 0xFF; //uses PORTB. Micro can use either or, but both give us 2 LEDs
	PORTB =  0x0; //The ATmega328P on the UNO will be resetting, so unplug it?
	#endif
	// The USB stack should be initialized last.
	USB_Init();
}

// Fired to indicate that the device is enumerating.
void EVENT_USB_Device_Connect(void) {
	// We can indicate that we're enumerating here (via status LEDs, sound, etc.).
}

// Fired to indicate that the device is no longer connected to a host.
void EVENT_USB_Device_Disconnect(void) {
	// We can indicate that our device is not ready (via status LEDs, sound, etc.).
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;

	// We setup the HID report endpoints.
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

	// We can read ConfigSuccess to indicate a success or failure at this point.
}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void) {
	// We can handle two control requests: a GetReport and a SetReport.

	// Not used here, it looks like we don't receive control request from the Switch.
}

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void) {
	// If the device isn't connected and properly configured, we can't do anything here.
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	// We'll start with the OUT endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
	// We'll check to see if we received something on the OUT endpoint.
	if (Endpoint_IsOUTReceived())
	{
		// If we did, and the packet has data, we'll react to it.
		if (Endpoint_IsReadWriteAllowed())
		{
			// We'll create a place to store our data received from the host.
			USB_JoystickReport_Output_t JoystickOutputData;
			// We'll then take in that data, setting it up in our storage.
			while(Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError);
			// At this point, we can react to this data.

			// However, since we're not doing anything with this data, we abandon it.
		}
		// Regardless of whether we reacted to the data, we acknowledge an OUT packet on this endpoint.
		Endpoint_ClearOUT();
	}

	// We'll then move on to the IN endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
	// We first check to see if the host is ready to accept data.
	if (Endpoint_IsINReady())
	{
		// We'll create an empty report.
		USB_JoystickReport_Input_t JoystickInputData;
		// We'll then populate this report with what we want to send to the host.
		GetNextReport(&JoystickInputData);
		// Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
		while(Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL) != ENDPOINT_RWSTREAM_NoError);
		// We then send an IN packet on this endpoint.
		Endpoint_ClearIN();
	}
}

typedef enum {
	SYNC_CONTROLLER,
	SYNC_POSITION,
	BREATHE,
	PROCESS,
	CLEANUP,
	DONE
} State_t;
State_t state = SYNC_CONTROLLER;

#define ECHOES 2
int echoes = 0;
USB_JoystickReport_Input_t last_report;

int report_count = 0;
int xpos = 0;
int ypos = 0;
int bufindex = 0;
int duration_count = 0;
int portsval = 0;

// Prepare the next report for the host.
void GetNextReport(USB_JoystickReport_Input_t* const ReportData) {

	// Prepare an empty report
	memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));
	ReportData->LX = STICK_CENTER;
	ReportData->LY = STICK_CENTER;
	ReportData->RX = STICK_CENTER;
	ReportData->RY = STICK_CENTER;
	ReportData->HAT = HAT_CENTER;

	// Repeat ECHOES times the last report
	/*
	if (echoes > 0)
	{
		memcpy(ReportData, &last_report, sizeof(USB_JoystickReport_Input_t));
		echoes--;
		return;
	}
	*/

	// States and moves management
	/*
	switch (state)
	{

		case SYNC_CONTROLLER:
			state = BREATHE;
			break;

		// case SYNC_CONTROLLER:
		// 	if (report_count > 550)
		// 	{
		// 		report_count = 0;
		// 		state = SYNC_POSITION;
		// 	}
		// 	else if (report_count == 250 || report_count == 300 || report_count == 325)
		// 	{
		// 		ReportData->Button |= SWITCH_L | SWITCH_R;
		// 	}
		// 	else if (report_count == 350 || report_count == 375 || report_count == 400)
		// 	{
		// 		ReportData->Button |= SWITCH_A;
		// 	}
		// 	else
		// 	{
		// 		ReportData->Button = 0;
		// 		ReportData->LX = STICK_CENTER;
		// 		ReportData->LY = STICK_CENTER;
		// 		ReportData->RX = STICK_CENTER;
		// 		ReportData->RY = STICK_CENTER;
		// 		ReportData->HAT = HAT_CENTER;
		// 	}
		// 	report_count++;
		// 	break;

		case SYNC_POSITION:
			bufindex = 0;


			ReportData->Button = 0;
			ReportData->LX = STICK_CENTER;
			ReportData->LY = STICK_CENTER;
			ReportData->RX = STICK_CENTER;
			ReportData->RY = STICK_CENTER;
			ReportData->HAT = HAT_CENTER;


			state = BREATHE;
			break;

		case BREATHE:
			state = PROCESS;
			break;

		case PROCESS:
*/
			ReportData->Button = 0;
			if (ks.X) ReportData->Button += SWITCH_X;
			if (ks.B) ReportData->Button += SWITCH_B;
			if (ks.Y) ReportData->Button += SWITCH_Y;
			if (ks.A) ReportData->Button += SWITCH_A;
			if (ks.R) ReportData->Button += SWITCH_R;
			if (ks.L) ReportData->Button += SWITCH_L;
			if (ks.ZR) ReportData->Button += SWITCH_ZR;
			if (ks.ZL) ReportData->Button += SWITCH_ZL;
			if (ks.CAPTURE) ReportData->Button += SWITCH_CAPTURE;
			if (ks.HOME) ReportData->Button += SWITCH_HOME;
			if (ks.MINUS) ReportData->Button += SWITCH_MINUS;
			if (ks.PLUS) ReportData->Button += SWITCH_PLUS;
			if (ks.UP) ReportData->LY = STICK_MIN;
			if (ks.DOWN) ReportData->LY = STICK_MAX;
			if (ks.LEFT) ReportData->LX = STICK_MIN;
			if (ks.RIGHT) ReportData->LX = STICK_MAX;
/*
			switch (step[bufindex].button)
			{

				case UP:
					ReportData->LY = STICK_MIN;				
					break;

				case LEFT:
					ReportData->LX = STICK_MIN;				
					break;

				case DOWN:
					ReportData->LY = STICK_MAX;				
					break;

				case RIGHT:
					ReportData->LX = STICK_MAX;				
					break;

				case A:
					ReportData->Button |= SWITCH_A;
					break;

				case B:
					ReportData->Button |= SWITCH_B;
					break;

				case R:
					ReportData->Button |= SWITCH_R;
					break;

				case THROW:
					ReportData->LY = STICK_MIN;				
					ReportData->Button |= SWITCH_R;
					break;

				case TRIGGERS:
					ReportData->Button |= SWITCH_L | SWITCH_R;
					break;

				default:
					ReportData->LX = STICK_CENTER;
					ReportData->LY = STICK_CENTER;
					ReportData->RX = STICK_CENTER;
					ReportData->RY = STICK_CENTER;
					ReportData->HAT = HAT_CENTER;
					break;
			}

			duration_count++;

			if (duration_count > step[bufindex].duration)
			{
				bufindex++;
				duration_count = 0;				
			}


			if (bufindex > (int)( sizeof(step) / sizeof(step[0])) - 1)
			{

				// state = CLEANUP;

				bufindex = 7;
				duration_count = 0;

				state = BREATHE;

				ReportData->LX = STICK_CENTER;
				ReportData->LY = STICK_CENTER;
				ReportData->RX = STICK_CENTER;
				ReportData->RY = STICK_CENTER;
				ReportData->HAT = HAT_CENTER;


				// state = DONE;
//				state = BREATHE;

			} */
/*
			state = BREATHE;

			break;

		case CLEANUP:
			state = DONE;
			break;

		case DONE:
			#ifdef ALERT_WHEN_DONE
			portsval = ~portsval;
			PORTD = portsval; //flash LED(s) and sound buzzer if attached
			PORTB = portsval;
			_delay_ms(250);
			#endif
			return;
	}
*/

	// // Inking
	// if (state != SYNC_CONTROLLER && state != SYNC_POSITION)
	// 	if (pgm_read_byte(&(image_data[(xpos / 8) + (ypos * 40)])) & 1 << (xpos % 8))
	// 		ReportData->Button |= SWITCH_A;

	// Prepare to echo this report
	memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));
	//echoes = ECHOES;

}
