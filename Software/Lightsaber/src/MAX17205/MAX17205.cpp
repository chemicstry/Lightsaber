#include "MAX17205.h"

#include "diag/trace.h"

void MAX17205::Init()
{
	// Initialize pack config
	PackCfg pcfg;
	pcfg.NCELLS = 3; // 3S configuration
	pcfg.D4 = 0;
	pcfg.BALCFG = 3; // 40mV diff
	pcfg.CxEn = false; // Do not use Cellx for measurement
	pcfg.BtEn = false; // Do not use vbatt for measurement
	pcfg.ChEn = true; // Enable cell channels
	pcfg.TdEn = true; // Enable internal temp measurement
	pcfg.A1En = false; // Disable external temp1
	pcfg.A2En = false; // Disable external temp2
	pcfg.D14 = 0;
	pcfg.FGT = false; // Use internal temp for fuel gauge

	// Cast to 2 byte register
	uint16_t reg;
	memcpy(&reg, &pcfg, sizeof(PackCfg));

	// Read current value
	uint16_t val = sBatt.ReadReg(REG_PackCfg);

	// If values do no match write and reset fuel gauge
	if (reg != val)
	{
		trace_printf("Wrong fuel gauge settings detected!\nDetected: %#04X, New: %#04X\nInitializing...\n", val, reg);

		// Write both registers
		sBatt.WriteReg(REG_PackCfg, reg);
		sBatt.WriteReg(REG_nPackCfg, reg);

		// Reset fuel gauge to load new settings
		FuelGaugeReset();
	}
}

void MAX17205::FuelGaugeReset()
{
	WriteReg(REG_Config2, 0x0001);
}

float MAX17205::GetInternalTemp()
{
	return (float(ReadReg(REG_IntTemp))*0.1f)-273.0f;
}

float MAX17205::GetVoltage()
{
	return float(ReadReg(REG_Batt))*0.00125f;
}

float MAX17205::GetCellVoltage(uint8_t cell)
{
	if (cell > 4)
		return 0.0f;

	return float(ReadReg(REG_Cell1-cell))*0.000078125f;
}

float MAX17205::GetCap()
{
	return float(ReadReg(REG_RepCap))*0.005f*RCOEF;
}

float MAX17205::GetSOC()
{
	return float(ReadReg(REG_RepSOC))/256.0f;
}

float MAX17205::GetCurrent()
{
	return float(int16_t(ReadReg(REG_Current)))*0.0015625f*RCOEF;
}

float MAX17205::GetAvgCurrent()
{
	return float(int16_t(ReadReg(REG_AvgCurrent)))*0.0015625f*RCOEF;
}

uint16_t MAX17205::ReadReg(uint16_t reg)
{
	uint16_t val;

	if (reg > 0xFF)
		I2Cdev::readWord(MAX17205_ADDR_HIGH, reg & 0xFF, &val);
	else
		I2Cdev::readWord(MAX17205_ADDR_LOW, reg, &val);

	return val;
}

void MAX17205::WriteReg(uint16_t reg, uint16_t val)
{
	if (reg > 0xFF)
		I2Cdev::writeWord(MAX17205_ADDR_HIGH, reg & 0xFF, val);
	else
		I2Cdev::writeWord(MAX17205_ADDR_LOW, reg, val);
}

MAX17205 sBatt;
