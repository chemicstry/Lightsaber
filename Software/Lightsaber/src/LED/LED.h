#ifndef _LED_H_
#define _LED_H_

#include "stm32f10x.h"
#include "RGBConverter.h"

#include <functional>

#define LED_TIMER TIM1
#define LED_TIMER_PERIPH RCC_APB2Periph_TIM1
#define LED_PORT GPIOA
#define LED_PORT_PERIPH RCC_APB2Periph_GPIOA
#define LED_R_PIN GPIO_Pin_8
#define LED_G_PIN GPIO_Pin_9
#define LED_B_PIN GPIO_Pin_10

typedef std::function<void()> BlinkCB;

class Color
{
public:
	virtual ~Color() {};
	virtual void GetRGB(uint8_t rgb[]) const = 0;
	virtual Color* Copy() const = 0;
};

class RGB : public Color
{
public:
	RGB(uint8_t _r = 0, uint8_t _g = 0, uint8_t _b = 0): r(_r), g(_g), b(_b) {}

	void GetRGB(uint8_t rgb[]) const
	{
		rgb[0] = r;
		rgb[1] = g;
		rgb[2] = b;
	}

	Color* Copy() const
	{
		return new RGB(*this);
	}

	uint8_t r, g, b;
};

class HSL : public Color
{
public:
	HSL(double _h = 0.0f, double _s = 0.0f, double _l = 0.0f): h(_h), s(_s), l(_l) {}

	void GetRGB(uint8_t rgb[]) const
	{
		RGBConverter::hslToRgb(h, s, l, rgb);
	}

	Color* Copy() const
	{
		return new HSL(*this);
	}

	double h, s, l;
};

class LED
{
public:
    void Init();
    void Set(const Color& color, bool force = false);
    void Set(const uint8_t rgb[]);

    void Blink(const Color& color, uint32_t count = 3, uint32_t delay = 200);
    void BlinkAsync(const Color& color, BlinkCB cb = nullptr, uint32_t count = 4, uint32_t delay = 100);

private:
    uint8_t _rgb[3];

    struct BlinkAsyncParams
    {
    	Color* 		color;
    	uint32_t 	count;
    	BlinkCB		cb;
    	uint32_t 	timer;
    } _blinkParams;
};

extern LED sLed;

#endif
