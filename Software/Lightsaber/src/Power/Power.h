#ifndef _POWER_H_
#define _POWER_H_

#define BATT_EMPTY_SOC 5.0f

#define POWER_LATCH_PERIPH RCC_APB2Periph_GPIOA
#define POWER_LATCH_PORT GPIOA
#define POWER_LATCH_PIN GPIO_Pin_7

class Power
{
public:
	Power();

	void Init();
	void InitWatchdog();

	void PowerOff();

	bool CheckOnConditions();
};

extern Power sPower;

#endif
