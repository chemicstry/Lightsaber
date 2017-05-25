//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

#include "stm32f10x.h"

#include "Timer/Timer.h"
#include "I2Cdev/I2C.h"
#include "I2Cdev/I2Cdev.h"
#include "MPU6050/MPU6050.h"
#include "MAX17205/MAX17205.h"
#include "LED/LED.h"
#include "BT/BT.h"
#include "fatfs/ff.h"
#include "Audio/Audio.h"
#include "Power/Power.h"
#include "Button/Button.h"

Button* btn;

MPU6050 gyro;
int32_t rot;

#define STEP 128

void LEDPWM()
{
    static double h = 0;

    if (h >= 1.0)
        h = 0;
    else
        h += 0.01;

    sLed.Set(HSL(h, 1, 0.5));
}

void BTRX(uint8_t* data, uint32_t len)
{
	trace_printf("BTRX: %u\n", len);

	switch(data[0])
	{
	case 'R':
		sLed.Set(RGB(255, 0, 0));
		break;
	case 'G':
		sLed.Set(RGB(0, 255, 0));
		break;
	case 'B':
		sLed.Set(RGB(0, 0, 255));
		break;
	default:
		sLed.Set(RGB(0, 0, 0));
	}
}

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
            	trace_printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

bool CheckI2CAddress(I2C_TypeDef* I2Cx, uint8_t slaveAddress)
{
	// Wait until I2C module is idle:
	//while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

	// Generate the start condition
	I2C_GenerateSTART(I2Cx, ENABLE);
	//
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

	//Send the address of the slave to be contacted:
	I2C_Send7bitAddress(I2Cx, slaveAddress<<1, I2C_Direction_Transmitter);

	//If this is a write operation, set I2C for transmit
	uint32_t timeout = 0xFFF;
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (--timeout == 0)
			break;
	}

	if (timeout == 0)
		return false;
	else
		return true;
}

void ReportBatteryStats()
{
	trace_printf("Voltage: %.2f V, Current: %.1f mA, AvgCurrent: %.1f mA, SOC: %.1f %\n", sBatt.GetVoltage(), sBatt.GetCurrent(), sBatt.GetAvgCurrent(), sBatt.GetSOC());
	trace_printf("Cell1: %.2f V, Cell2: %.2f V, Cell3: %.2f V, Cell4: %.2f\n", sBatt.GetCellVoltage(0), sBatt.GetCellVoltage(1), sBatt.GetCellVoltage(2), sBatt.GetCellVoltage(3));
}

uint32_t sHum;

void PlayHum()
{
	trace_printf("HUM");
	sHum = sAudio.Play("hum.wav", 1, true);
}

#define GYRO_INTERVAL 30
#define SWING_THRESHOLD 10000
#define SWING_DELAY 1000

uint32_t tSwing; // Swing timer
int sSwing; // Swing sound
int16_t ax, ay, az;
int16_t gx, gy, gz;

bool swingMutex = false;

void CheckSwing()
{
	// Get rotation
	gyro.getRotation(&gx, &gy, &gz);

	// Apply filter
	rot += (abs(gx) + abs(gy) + abs(gz) - rot)/4;

	// Change swing pitch
	sAudio.Atten(sSwing, max(0, 40000/rot - 1));
}

void PowerOff()
{
	sPower.PowerOff();
}

// program states
enum ProgramState
{
	STATE_ON,
	STATE_MODE_SELECT_PRE,
	STATE_MODE_SELECT,
	STATE_COLOR_SELECT_PRE,
	STATE_COLOR_SELECT,
	STATE_LIGHTNESS_SELECT_PRE,
	STATE_LIGHTNESS_SELECT,
	STATE_EXIT_CONFIG,
	STATE_NULL
};

uint8_t state = STATE_ON;

#define LONG_PRESS 1500

HSL color(0.5f, 1.0f, 0.005f);

// modes
enum
{
  MODE_SOLID,
  MODE_FLICKER,
  MODE_RAINBOW,
  MODE_END
};

uint8_t mode = MODE_FLICKER;

struct Config
{
	Color& color;
	uint8_t& mode;
} config{color, mode};

void LoadConfig()
{
	trace_printf("File size: %u\n", sizeof(Config));
	FIL fil;
	UINT size;

    // Open file
	FRESULT fr = f_open(&fil, "config.bin", FA_READ);

    if (fr)
    	return;

    // Read data
    f_read(&fil, (void*)&config, sizeof(Config), &size);

    // Close
    f_close(&fil);
}

void SaveConfig()
{
	FIL fil;
	UINT size;

	// Open file
	FRESULT fr = f_open(&fil, "config.bin", FA_WRITE | FA_CREATE_ALWAYS);

	if (fr)
		return;

	// Write data
	f_write(&fil, (void*)&config, sizeof(Config), &size);

	// Close
	f_close(&fil);
}

#define COLOR_SELECT_DELAY 80

void ColorSelectTick()
{
	sLed.Set(HSL(color.h, 1, color.l));

	if (color.h >= 1)
		color.h = 0;
	else
		color.h += 0.005;
}

#define LIGHTNESS_SELECT_DELAY 50

void LightnessSelectTick()
{
	static bool lightnessDirection = false;

	if (lightnessDirection)
		color.l += 0.005;
	else
		color.l -= 0.005;

	if (color.l <= 0.01) {
		color.l = 0.01;
		lightnessDirection = true;
	} else if (color.l >= 1) {
		color.l = 1;
		lightnessDirection = false;
	}

	// Only update color with mode solid. Other modes have their own update timers
	if (mode == MODE_SOLID)
		sLed.Set(color);
}

#define MODE_RAINBOW_DELAY 10

#define MODE_FLICKER_DELAY 10
#define MODE_FLICKER_CHANCE 20
#define MODE_FLICKER_ALPHA 0.96f
#define MODE_FLICKER_ALPHA_GYRO 3000.0f

void UpdateMode()
{
	static uint32_t tRainbow = NO_TIMER_AVAILABLE;
	static uint32_t tFlicker = NO_TIMER_AVAILABLE;

	Timer::ClearTimeout(tRainbow);
	tRainbow = NO_TIMER_AVAILABLE;
	Timer::ClearTimeout(tFlicker);
	tFlicker = NO_TIMER_AVAILABLE;

	switch(mode)
	{
		case MODE_SOLID:
		{
			sLed.Set(color);
			break;
		}
		case MODE_RAINBOW:
		{
			tRainbow = Timer::SetInterval([](){
				static float hue = 0;

				sLed.Set(HSL(hue, 1, color.l));

				if (hue >= 1)
					hue = 0;
				else
					hue += 0.01;
			}, MODE_RAINBOW_DELAY);

			break;
		}
		case MODE_FLICKER:
		{
			tFlicker = Timer::SetInterval([](){
				if (RandomInt(0, 100) < MODE_FLICKER_CHANCE)
					sLed.Set(HSL(color.h, color.s, color.l * MODE_FLICKER_ALPHA * min(1.0f, MODE_FLICKER_ALPHA_GYRO/double(rot))));
				else
				    sLed.Set(color);
			}, MODE_FLICKER_DELAY);

			break;
		}
	}
}

#define SETTINGS_COLOR HSL(1.0f/6.0f, 1.0f, 0.3f)

void Loop()
{
	static uint32_t tColorSelect, tLightnessSelect;

	Timer::FireCallbacks();
	//BT::getInstance().Update();
	sAudio.Update();
	btn->read();

	switch (state)
	{
		case STATE_ON:
		{
			if (btn->wasReleased())
			{
				//SaveConfig();

				sAudio.Stop(sHum);
				sAudio.Stop(sSwing);
				sAudio.Play("off.wav", 1);

				Timer::SetTimeout(PowerOff, 1500);
			}
			else if (btn->pressedFor(LONG_PRESS))
			{
				// Reset lightness for config
				color.l = 0.5;

				// Set state to null while blinking
				state = STATE_NULL;

				// Blink and change state after finished
				sLed.BlinkAsync(SETTINGS_COLOR, []() {
					UpdateMode();
					state = STATE_MODE_SELECT;
				});

				// Play sfx
				//sAudio.Play("ui/mode.wav");
			}

			break;
		}
		case STATE_MODE_SELECT_PRE:
		{
			if (btn->wasReleased())
				state = STATE_MODE_SELECT;

			break;
		}
		case STATE_MODE_SELECT:
		{
			if (btn->wasReleased())
			{
				mode = mode == MODE_END-1 ? MODE_SOLID : mode+1;
				UpdateMode();
			}
			else if (btn->pressedFor(LONG_PRESS))
			{
				// Set state to null while blinking
				if (mode != MODE_RAINBOW)
				{
					state = STATE_COLOR_SELECT_PRE;

					// Play sfx
					//sAudio.Play("ui/color.wav");
				}
				else
				{
					state = STATE_LIGHTNESS_SELECT_PRE;

					// Play sfx
					//sAudio.Play("ui/lightness.wav");
				}

				// Blink and change state after finished
				sLed.BlinkAsync(SETTINGS_COLOR, []() {
					UpdateMode();
				});
			}
			break;
			// Temporary state to reset long press
		}
		case STATE_COLOR_SELECT_PRE:
		{
			if (btn->wasReleased())
			{
				trace_printf("State: STATE_COLOR_SELECT");
				state = STATE_COLOR_SELECT;
				tColorSelect = Timer::SetInterval(ColorSelectTick, COLOR_SELECT_DELAY);
				sAudio.Play("off.wav", 1);
			}
			break;
		}
		case STATE_COLOR_SELECT:
		{
			if (btn->wasPressed()) {
				Timer::ClearTimeout(tColorSelect);
			} else if (btn->wasReleased()) {
				tColorSelect = Timer::SetInterval(ColorSelectTick, COLOR_SELECT_DELAY);
			} else if (btn->pressedFor(LONG_PRESS)) {
				// Set state to null while blinking
				state = STATE_LIGHTNESS_SELECT_PRE;

				sLed.BlinkAsync(SETTINGS_COLOR, []() {
					UpdateMode();
				});

				// Play sfx
				//sAudio.Play("ui/lightness.wav");
			}
			break;
			// Temporary state to reset long press
		}
		case STATE_LIGHTNESS_SELECT_PRE:
		{
			if (btn->wasReleased())
			{
				state = STATE_LIGHTNESS_SELECT;
				tLightnessSelect = Timer::SetInterval(LightnessSelectTick, LIGHTNESS_SELECT_DELAY);
			}
			break;
		}
		case STATE_LIGHTNESS_SELECT:
		{
			if (btn->wasPressed()) {
				Timer::ClearTimeout(tLightnessSelect);
			} else if (btn->wasReleased()) {
				tLightnessSelect = Timer::SetInterval(LightnessSelectTick, LIGHTNESS_SELECT_DELAY);
			} else if (btn->pressedFor(LONG_PRESS)) {
				// Set state to null while blinking
				state = STATE_EXIT_CONFIG;

				sLed.BlinkAsync(SETTINGS_COLOR, []() {
					UpdateMode();
				});
			}
			break;
		}
		case STATE_EXIT_CONFIG:
		{
			if (btn->wasReleased())
			{
				state = STATE_ON;
			}
			break;
		}
	}

}

int main(int argc, char* argv[])
{
    // At this stage the system clock should have already been configured at high speed.

	// Quickly latch power
	sPower.Init();

    //trace_printf("System clock: %u Hz\n", SystemCoreClock);
    Timer::Start();

    // Init leds early to prevent partial mosfet conduction
    sLed.Init();

    //trace_printf("I2C Init\n");
    I2C1_Initialize();

    // Init battery monitor
    sBatt.Init();

    // Power manager will turn off device if conidtions are not met
    sPower.CheckOnConditions();

    // Main button
    btn = new Button(RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_6, false, false, 20);

    // Initialize file system
    FATFS fs;

	int status = f_mount(&fs, "0", 1);

	if (status != FR_OK)
	{
		trace_printf("Mount failed! %u\n", status);
		sLed.Blink(RGB(255, 0, 0));
		sPower.PowerOff();
	}

	// Load config
	//LoadConfig();

	// Initialize gyro
	gyro.initialize();

	// Init audio
	sAudio.Init();

	// Play turn on sound
	sAudio.Play("on.wav", 2);

	// Start hum after delay
	int t = Timer::SetTimeout(PlayHum, 500);
	trace_printf("Hum: %u\n", t);

	// Start swing detection
	tSwing = Timer::SetInterval(CheckSwing, GYRO_INTERVAL);
	trace_printf("Swing: %u\n", tSwing);

	// Start swing sound
	sSwing = sAudio.Play("high.wav", 12, true);

	// Light on!
	UpdateMode();

	// Reset button
	btn->read();
	btn->read();

	// Statistics
	Timer::SetInterval(ReportBatteryStats, 1000);

    while(1)
    {
    	Loop();
    }

    return 0;
}

/*
 	 trace_printf("I2C Init\n");
    //led.SetVal(255, 255, 255);

    I2C1_Initialize();

    for (int i = 0; i < 0x7F; ++i)
    {
    	if (CheckI2CAddress(I2C1, i))
    		trace_printf("Addr: %u\n", i);
    }

    gyro.initialize();
    BT::getInstance().Init();
    BT::getInstance().SetRXCB(BTRX);
    BT::getInstance().SetCMDMode(false);

    trace_printf("Waiting\n");

    trace_printf("Testcon: %u\n", gyro.getDeviceID());

    sLed.Init();
    //Timer::SetInterval(Test, 10);
    //Timer::SetInterval(LEDPWM, 10);
    //Timer::SetInterval(BTTEST, 1000);
    //Timer::SetInterval(BTTEST2, 1200);

    MAX17205 max;
    uint16_t devName = max.ReadReg(0x21);
    trace_printf("DevName: %u\nName: %u\nRev: %u\n", devName, devName & 0xF, devName >> 4);

    float temp = max.GetInternalTemp();
    trace_printf("Temp: %.1f\n", temp);

    float vol = max.GetBatteryVoltage();
    trace_printf("Voltage: %.3f\n", vol);

    FATFS fs;
	FIL fil;
	char line[82];
	FRESULT fr;

	int status = f_mount(&fs, "0", 1);

	if (status != FR_OK)
	{
		trace_printf("Mount failed! %u\n", status);
		return 0;
	}

	// Open file
	fr = f_open(&fil, "hello.txt", FA_READ);
	if (fr) return (int)fr;

	// Read all lines
	while (f_gets(line, sizeof(line), &fil))
		trace_printf(line);

	// Close file
	f_close(&fil);

	//char path[] = "/";
	//scan_files(path);

	sAudio.Init();
	sAudio.Play("2on32k.wav", 2);
	//sAudio.Play("bee32k.wav");
	Timer::SetTimeout(HUM, 1000);
	Timer::SetInterval(SWING, 5000);
 */

