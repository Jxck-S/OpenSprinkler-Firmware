#include "hardware_detection.h"

namespace osboard {
namespace {

void detect_driver(HardwareDetection& result, I2cProbe probe_i2c) {
	if (probe_i2c(AC_DRIVER_ADDRESS)) {
		result.power_type = POWER_AC;
		result.driver_address = AC_DRIVER_ADDRESS;
	} else if (probe_i2c(DC_DRIVER_ADDRESS)) {
		result.power_type = POWER_DC;
		result.driver_address = DC_DRIVER_ADDRESS;
	} else if (probe_i2c(LATCH_DRIVER_ADDRESS)) {
		result.power_type = POWER_LATCH;
		result.driver_address = LATCH_DRIVER_ADDRESS;
	}
}

} // namespace

HardwareDetection detect_hardware(I2cProbe probe_i2c, Revision1Probe probe_revision1) {
	HardwareDetection result = {
		0,
		POWER_UNKNOWN,
		PROFILE_UNKNOWN,
		DRIVER_PCA9555,
		AC_DRIVER_ADDRESS,
		false,
		false,
	};

	if (probe_i2c(MAIN_IO_EXPANDER_ADDRESS)) {
		result.profile = PROFILE_OS_30;
		result.driver_kind = DRIVER_PCF8574;
		result.separate_main_io = true;
		detect_driver(result, probe_i2c);
		return result;
	}

	if (probe_revision1()) {
		result.revision = 1;
		result.profile = PROFILE_OS_31;
		detect_driver(result, probe_i2c);
		return result;
	}

	const bool has_revision4_eeprom = probe_i2c(EEPROM_ADDRESS + 2);
	const bool has_ch224_primary = probe_i2c(CH224_ADDRESS);
	const bool has_ch224_secondary = probe_i2c(CH224_ADDRESS + 1);
	if (has_revision4_eeprom || (has_ch224_primary && has_ch224_secondary)) {
		result.revision = 4;
		result.profile = PROFILE_OS_34;
		result.power_type = (has_ch224_primary && has_ch224_secondary) ? POWER_DC : POWER_AC;
		result.initialize_usb_pd = result.power_type == POWER_DC;
		return result;
	}

	result.revision = probe_i2c(EEPROM_ADDRESS) ? 3 : 2;
	result.profile = PROFILE_OS_32_33;
	detect_driver(result, probe_i2c);
	return result;
}

HardwareDetection detect_os4_hardware(I2cProbe probe_i2c) {
	const bool has_ch224_primary = probe_i2c(CH224_ADDRESS);
	const bool has_ch224_secondary = probe_i2c(CH224_ADDRESS + 1);
	const bool is_dc = has_ch224_primary && has_ch224_secondary;
	return {
		0,
		is_dc ? POWER_DC : POWER_AC,
		PROFILE_OS_40,
		DRIVER_PCA9555,
		MAIN_IO_EXPANDER_ADDRESS,
		false,
		is_dc,
	};
}

// Classic ESP32 + RMII PHY: no CH224, USB-PD or current sensing, relays on the
// main expander at 0x20. POWER_UNKNOWN keeps current alerts and boost/current
// options off. The expander chip type (PCA9555 or PCF8574) is detected at
// runtime by the caller; a missing expander still selects the profile so GPIO
// stations work.
HardwareDetection detect_esp32_rmii_hardware(I2cProbe probe_i2c) {
	(void)probe_i2c;
	return {
		0,
		POWER_UNKNOWN,
		PROFILE_ESP32_RMII,
		DRIVER_PCA9555,
		MAIN_IO_EXPANDER_ADDRESS,
		false,
		false,
	};
}

} // namespace osboard
