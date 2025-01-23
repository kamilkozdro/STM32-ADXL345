
#include "stm32l1xx_hal.h"

#define ADXL_REG_DEVID					0x00	// Device ID
#define ADXL_REG_THRESH_TAP				0x1D	// Tap threshold
#define ADXL_REG_OFSX					0x1E	// X-axis offset
#define ADXL_REG_OFSY					0x1F	// Y-axis offset
#define ADXL_REG_OFSZ					0x20	// Z-axis offset
#define ADXL_REG_DUR					0x21	// Tap duration
#define ADXL_REG_Latent					0x22	// Tap latency
#define ADXL_REG_Window					0x23	// Tap window
#define ADXL_REG_THRESH_ACT				0x24	// Activity threshold
#define ADXL_REG_THRESH_INACT			0x25	// Inactivity threshold
#define ADXL_REG_TIME_INACT				0x26	// Inactivity time
#define ADXL_REG_ACT_INACT_CTL			0x27	// Axis enable control for activity and inactivity detection
#define ADXL_REG_THRESH_FF				0x28	// Free-fall threshold
#define ADXL_REG_TIME_FF				0x29	// Free-fall time
#define ADXL_REG_TAP_AXES				0x2A	// Axis control for single tap/double tap
#define ADXL_REG_ACT_TAP_STATUS			0x2B	// Source of single tap/double tap
#define ADXL_REG_BW_RATE				0x2C	// Data rate and power mode control
#define ADXL_REG_POWER_CTL				0x2D	// Power-saving features control
#define ADXL_REG_INT_ENABLE				0x2E	// Interrupt enable control
#define ADXL_REG_INT_MAP				0x2F	// Interrupt mapping control
#define ADXL_REG_INT_SOURCE				0x30	// Source of interrupts
#define ADXL_REG_DATA_FORMAT			0x31	// Data format control
#define ADXL_REG_DATAX0					0x32	// X-Axis Data 0
#define ADXL_REG_DATAX1					0x33	// X-Axis Data 1
#define ADXL_REG_DATAY0					0x34	// Y-Axis Data 0
#define ADXL_REG_DATAY1					0x35	// Y-Axis Data 1
#define ADXL_REG_DATAZ0					0x36	// Z-Axis Data 0
#define ADXL_REG_DATAZ1					0x37	// Z-Axis Data 1
#define ADXL_REG_FIFO_CTL				0x38	// FIFO control
#define ADXL_REG_FIFO_STATUS			0x39	// FIFO status

#define ADXL_BW_RATE_LOW_POWER_BIT		4		// 0 - Normal operation; 1 - Reduced power operation
#define ADXL_BW_RATE_100_VAL			0x0A	// Default device bandwidth and output rate - 100 Hz

#define ADXL_POWER_CTL_LINK_BIT			5
#define ADXL_POWER_CTL_AUTO_SLEEP_BIT	4
#define ADXL_POWER_CTL_MEASURE_BIT		3		// Standby mode - 0; Measurement mode - 1
#define ADXL_POWER_CTL_SLEEP_BIT		2
#define ADXL_POWER_CTL_WAKEUP_8_VALUE	0		// 0 - 8Hz; 1 - 4Hz; 2 - 2Hz; 3 - 1Hz

#define ADXL_INT_ENABLE_DATA_READY_BIT	7
#define ADXL_INT_ENABLE_SINGLE_TAP_BIT	6
#define ADXL_INT_ENABLE_DOUBLE_TAP_BIT	5
#define ADXL_INT_ENABLE_ACTIVITY_BIT	4
#define ADXL_INT_ENABLE_INACTIVITY_BIT	3
#define ADXL_INT_ENABLE_FREE_FALL_BIT	2
#define ADXL_INT_ENABLE_WATERMARK_BIT	1
#define ADXL_INT_ENABLE_OVERRUN_BIT		0

#define ADXL_INT_MAP_DATA_READY_BIT		7
#define ADXL_INT_MAP_SINGLE_TAP_BIT		6
#define ADXL_INT_MAP_DOUBLE_TAP_BIT		5
#define ADXL_INT_MAP_ACTIVITY_BIT		4
#define ADXL_INT_MAP_INACTIVITY_BIT		3
#define ADXL_INT_MAP_FREE_FALL_BIT		2
#define ADXL_INT_MAP_WATERMARK_BIT		1
#define ADXL_INT_MAP_OVERRUN_BIT		0

#define ADXL_INT_SOURCE_DATA_READY_BIT	7
#define ADXL_INT_SOURCE_SINGLE_TAP_BIT	6
#define ADXL_INT_SOURCE_DOUBLE_TAP_BIT	5
#define ADXL_INT_SOURCE_ACTIVITY_BIT	4
#define ADXL_INT_SOURCE_INACTIVITY_BIT	3
#define ADXL_INT_SOURCE_FREE_FALL_BIT	2
#define ADXL_INT_SOURCE_WATERMARK_BIT	1
#define ADXL_INT_SOURCE_OVERRUN_BIT		0

#define ADXL_DATA_FORMAT_SELF_TEST_BIT	7
#define ADXL_DATA_FORMAT_SPI_BIT		6	//	0 - 4-wire SPI mode; 1 - 3-wire SPI mode
#define ADXL_DATA_FORMAT_INT_INVERT_BIT	5	// 0 - Sets interrupts to active high; 1 - Interrupts active low
#define ADXL_DATA_FORMAT_FULL_RES_BIT	3	// 0 - 10-bit mode, RANGE bits set max G range; 1 - resol. increases with G range to maintain 4 mG/LSB
#define ADXL_DATA_FORMAT_JUSTIFY_BIT	2	// 0 - Right-justified mode with sign ext; 1 - Left-justified mode (MSB)
#define ADXL_DATA_FORMAT_RANGE_2_VAL	0	// Range +/- 2G
#define ADXL_DATA_FORMAT_RANGE_4_VAL	1	// Range +/- 4G
#define ADXL_DATA_FORMAT_RANGE_8_VAL	2	// Range +/- 8G
#define ADXL_DATA_FORMAT_RANGE_16_VAL	3	// Range +/- 16G



