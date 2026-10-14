#pragma once

// Board configuration for classic ESP32 + RMII Ethernet PHY builds (OS_ETH_RMII).
// Every value can be overridden with a -D build flag; platformio.ini has presets.
//
// The RMII data pins are fixed in ESP32 silicon and cannot be remapped:
// TXD0=19, TX_EN=21, TXD1=22, RXD0=25, RXD1=26, CRS_DV=27, plus the 50 MHz
// reference clock on GPIO0 (input or output), GPIO16 or GPIO17 (output).

#if defined(OS_ETH_RMII)

#include <ETH.h>

#ifndef OS_ETH_PHY_TYPE
#define OS_ETH_PHY_TYPE  ETH_PHY_LAN8720
#endif
#ifndef OS_ETH_PHY_ADDR
#define OS_ETH_PHY_ADDR  1
#endif
#ifndef OS_ETH_PHY_MDC
#define OS_ETH_PHY_MDC   23
#endif
#ifndef OS_ETH_PHY_MDIO
#define OS_ETH_PHY_MDIO  18
#endif
#ifndef OS_ETH_PHY_POWER
#define OS_ETH_PHY_POWER -1 // GPIO enabling the PHY/oscillator, or -1 if none
#endif
#ifndef OS_ETH_CLK_MODE
#define OS_ETH_CLK_MODE  ETH_CLOCK_GPIO0_IN
#endif

#ifndef OS_I2C_SDA
#define OS_I2C_SDA       33
#endif
#ifndef OS_I2C_SCL
#define OS_I2C_SCL       32
#endif

// GPIOs offered in the UI for "GPIO" station type (direct relay wiring).
// Passed as a bare comma list because shells brace-expand {a,b} in build flags.
#ifndef OS_GPIO_FREE_PINS
#define OS_GPIO_FREE_PINS 4, 5, 14, 15
#endif
#define OS_GPIO_FREE_LIST {OS_GPIO_FREE_PINS}

namespace osboard {
namespace rmii {

constexpr int clock_pin() {
	return (OS_ETH_CLK_MODE == ETH_CLOCK_GPIO16_OUT) ? 16 :
	       (OS_ETH_CLK_MODE == ETH_CLOCK_GPIO17_OUT) ? 17 : 0;
}

// True if the pin cannot be used as an output on an RMII board.
constexpr bool is_reserved_output(int pin) {
	return pin < 0 || pin > 33 ||          // 34-39 are input-only
		pin == 1 || pin == 3 ||              // UART0 (flashing, logs)
		(pin >= 6 && pin <= 11) ||           // internal SPI flash
		pin == 12 ||                         // MTDI strap: high at boot selects 1.8V flash
		pin == 19 || pin == 21 || pin == 22 || // RMII TX
		pin == 25 || pin == 26 || pin == 27 || // RMII RX
		pin == clock_pin() ||
		pin == OS_ETH_PHY_MDC || pin == OS_ETH_PHY_MDIO ||
		pin == OS_ETH_PHY_POWER;
}

constexpr unsigned char free_list[] = OS_GPIO_FREE_LIST;

constexpr bool free_list_valid(unsigned i = 0) {
	return i >= sizeof(free_list) ||
		(!is_reserved_output(free_list[i]) &&
		 free_list[i] != OS_I2C_SDA && free_list[i] != OS_I2C_SCL &&
		 free_list_valid(i + 1));
}

static_assert(!is_reserved_output(OS_I2C_SDA), "OS_I2C_SDA conflicts with RMII, flash, UART or strapping pins");
static_assert(!is_reserved_output(OS_I2C_SCL), "OS_I2C_SCL conflicts with RMII, flash, UART or strapping pins");
static_assert(OS_I2C_SDA != OS_I2C_SCL, "OS_I2C_SDA and OS_I2C_SCL must differ");
static_assert(free_list_valid(), "OS_GPIO_FREE_LIST contains a reserved or I2C pin");

} // namespace rmii
} // namespace osboard

#endif // OS_ETH_RMII
