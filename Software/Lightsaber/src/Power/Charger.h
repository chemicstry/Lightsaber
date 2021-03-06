#ifndef _CHARGER_H_
#define _CHARGER_H_

#include "stm32f10x.h"

#define CHARGER_BOOST_FREQ 250000 // 250 kHz

#define CHARGER_CURRENT_SHUNT 0.05f // 0.05 ohm

class Charger
{
public:
	void Init();
	float GetCurrent();
	float GetVoltage();

private:
	struct ADCMeasurements
	{
		uint16_t I;
		uint16_t U;
	} ADC;
};

extern Charger sCharger;

#endif
