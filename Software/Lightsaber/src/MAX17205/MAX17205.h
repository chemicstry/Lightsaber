#ifndef _MAX17205_H_
#define _MAX17205_H_

#include "../I2Cdev/I2Cdev.h"

#define MAX17205_ADDR_LOW 0x36
#define MAX17205_ADDR_HIGH 0x0B

#define RSENSE 0.01f
#define RCOEF 1.0f/RSENSE

enum MAX17205_Registers
{
	REG_RepCap				= 0x005, // Filtered capacity
	REG_RepSOC				= 0x006, // Filtered SOC
	REG_Current				= 0x00A, // Real time current
	REG_AvgCurrent			= 0x00B, // Filtered current
	REG_DevName				= 0x021, // Device Name
	REG_Config2				= 0x0BB,
	REG_PackCfg				= 0x0BD,
	REG_AvgCell4			= 0x0D1,
	REG_AvgCell3			= 0x0D2,
	REG_AvgCell2			= 0x0D3,
	REG_AvgCell1			= 0x0D4,
	REG_Cell4				= 0x0D5,
	REG_Cell3				= 0x0D6,
	REG_Cell2				= 0x0D7,
	REG_Cell1				= 0x0D8,
	REG_Batt				= 0x0DA, // Battery voltage
	REG_IntTemp				= 0x135, // Internal temperature
	REG_AvgIntTemp			= 0x138, // Internal temperature
	REG_nFilterCfg			= 0x19D, // Filter config
	REG_nPackCfg			= 0x1B5, // Cell pack config
	REG_nRSense				= 0x1CF, // Current sense resistor value
};

struct FilterCfg
{
	uint8_t CURR	: 4;
	uint8_t VOLT	: 3;
	uint8_t MIX		: 4;
	uint8_t TEMP	: 3;
	uint8_t D14		: 1;
	uint8_t D15		: 1;
} __attribute__((packed));

struct PackCfg
{
	uint8_t NCELLS	: 4;
	bool	D4		: 1;
	uint8_t	BALCFG	: 3;
	bool 	CxEn	: 1;
	bool	BtEn	: 1;
	bool 	ChEn	: 1;
	bool	TdEn	: 1;
	bool	A1En	: 1;
	bool 	A2En	: 1;
	bool	D14		: 1;
	bool	FGT		: 1;
} __attribute__((packed));

class MAX17205
{
public:
    void Init();
    void FuelGaugeReset();

    float GetInternalTemp();
    float GetVoltage();
    float GetCellVoltage(uint8_t cell);
    float GetCap(); // mAh
    float GetSOC(); // %
    float GetCurrent(); // mA
    float GetAvgCurrent(); // mA

//private:
    uint16_t ReadReg(uint16_t reg);
    void WriteReg(uint16_t reg, uint16_t val);
};

extern MAX17205 sBatt;

#endif
