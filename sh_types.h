/* * SH-1 MCU Driver - library for communicating with BNO070
*
* Copyright 2015-16 Hillcrest Laboratories, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License and 
* any applicable agreements you may have with Hillcrest Laboratories, Inc.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/** 
 * @file sh_types.h
 * @author David Wheeler
 * @date 22 May 2015
 * @brief Type definitions for Hillcrest SH-1 API.
 *
 * Struct and type definitions supporting the Hillcrest SH-1 SensorHub API.
 * 
 */


#ifndef SH_TYPES_H
#define SH_TYPES_H

#include <stdint.h>
#include <stdbool.h>


/* Special timeout values */
/** Never time out. */
#define SH_WAIT_FOREVER (0xFFFF)
/** Never block. */
#define SH_NO_WAIT (0)

/** Convert float to 16Q16 fixed point */
#define TO_16Q16(f) ((int16_t)(f * (1 << 16)))
#define TO_16Q15(f) ((int16_t)(f * (1 << 15)))
#define TO_16Q14(f) ((int16_t)(f * (1 << 14)))
#define TO_16Q13(f) ((int16_t)(f * (1 << 13)))
#define TO_16Q12(f) ((int16_t)(f * (1 << 12)))
#define TO_16Q11(f) ((int16_t)(f * (1 << 11)))
#define TO_16Q10(f) ((int16_t)(f * (1 << 10)))
#define TO_16Q9(f) ((int16_t)(f * (1 << 9)))
#define TO_16Q8(f) ((int16_t)(f * (1 << 8)))
#define TO_16Q7(f) ((int16_t)(f * (1 << 7)))
#define TO_16Q6(f) ((int16_t)(f * (1 << 6)))
#define TO_16Q5(f) ((int16_t)(f * (1 << 5)))
#define TO_16Q4(f) ((int16_t)(f * (1 << 4)))
#define TO_16Q3(f) ((int16_t)(f * (1 << 3)))
#define TO_16Q2(f) ((int16_t)(f * (1 << 2)))
#define TO_16Q1(f) ((int16_t)(f * (1 << 1)))

/** Convert 16Q16 fixed point to floating point */
#define FROM_16Q16(n) ((float)n / (float)(1 << 16))
#define FROM_16Q15(n) ((float)n / (float)(1 << 15))
#define FROM_16Q14(n) ((float)n / (float)(1 << 14))
#define FROM_16Q13(n) ((float)n / (float)(1 << 13))
#define FROM_16Q12(n) ((float)n / (float)(1 << 12))
#define FROM_16Q11(n) ((float)n / (float)(1 << 11))
#define FROM_16Q10(n) ((float)n / (float)(1 << 10))
#define FROM_16Q9(n) ((float)n / (float)(1 << 9))
#define FROM_16Q8(n) ((float)n / (float)(1 << 8))
#define FROM_16Q7(n) ((float)n / (float)(1 << 7))
#define FROM_16Q6(n) ((float)n / (float)(1 << 6))
#define FROM_16Q5(n) ((float)n / (float)(1 << 5))
#define FROM_16Q4(n) ((float)n / (float)(1 << 4))
#define FROM_16Q3(n) ((float)n / (float)(1 << 3))
#define FROM_16Q2(n) ((float)n / (float)(1 << 2))
#define FROM_16Q1(n) ((float)n / (float)(1 << 1))

/**
 * @brief Return codes for most SensorHub API calls.
 */
enum sh_Status_e {

	/** Success */
	SH_STATUS_SUCCESS = 0,

	/** General Error */
	SH_STATUS_ERROR = -1,

	/** Bad parameter to an API call */
	SH_STATUS_BAD_PARAM = -2,

	/** Error response in message from SH-1 device */
	SH_STATUS_SH_ERR = -3,

	/** Got report from SH that was invalid or couldn't be processed */
	SH_STATUS_BAD_REPORT = -4,
	
	/** Errors (<0) */
	SH_STATUS_ERROR_I2C_IO = -5,

	/** Attempt to read IN report when none available */
	SH_STATUS_NO_DATA = -6,
	
	/** received an out of order FRS read response */
	SH_STATUS_FRS_READ_BAD_OFFSET = -100,      

	/** received an FRS read response with a bad length field */
	SH_STATUS_FRS_READ_BAD_LENGTH = -101,      

	/** FRS read response: bad type field */
	SH_STATUS_FRS_READ_BAD_TYPE = -102,
	
	/** FRS read response: Unrecognized FRS type code. */
	SH_STATUS_FRS_READ_UNRECOGNIZED_FRS = -103,
	
	/** FRS read response: Protocol error, read busy. */
	SH_STATUS_FRS_READ_BUSY = -104,
	
	/** FRS read response: Device error. */
	SH_STATUS_FRS_READ_DEVICE_ERROR = -105,
	
	/** FRS read response: Unknown error. */
	SH_STATUS_FRS_READ_UNKNOWN_ERROR = -106,
	
	/** FRS read response: Empty FRS record. */
	SH_STATUS_FRS_READ_EMPTY = -107,
	
	/** FRS read response: Offset out of range. */
	SH_STATUS_FRS_READ_OFFSET_OUT_OF_RANGE = -108,

	/** FRS Read produced more responses than expected. */
	SH_STATUS_FRS_READ_UNEXPECTED_LENGTH = -109,
	
	/** FRS write error: busy. */
	SH_STATUS_FRS_WRITE_BUSY =           -200,
	
	/** FRS write error: Bad FRS record id. */
	SH_STATUS_FRS_WRITE_BAD_TYPE =       -201,
	
	/** FRS write error: Bad length. */
	SH_STATUS_FRS_WRITE_BAD_LENGTH =     -202,
	
	/** FRS write error: Device error. */
	SH_STATUS_FRS_WRITE_DEVICE_ERROR =   -203,
	
	/** FRS write error: Bad status. */
	SH_STATUS_FRS_WRITE_BAD_STATUS =     -204,
	
	/** FRS write error: Bad mode. */
	SH_STATUS_FRS_WRITE_BAD_MODE =       -205,
	
	/** FRS write error: Write failed. */
	SH_STATUS_FRS_WRITE_FAILED =         -206,
	
	/** FRS write error: Write to read-only record. */
	SH_STATUS_FRS_WRITE_READ_ONLY =      -207,
	
	/** FRS write error: Invalid record data. */
	SH_STATUS_FRS_WRITE_INVALID_RECORD = -208,
	
	/** FRS write error: FRS record truncated. */
	SH_STATUS_FRS_WRITE_NOT_ENOUGH =     -209,

	/** Invalid firmware passed to DFU */
	SH_STATUS_INVALID_HCBIN = -400,

	/** NACK occurred during DFU process */
	SH_STATUS_NACK = -401,
};
typedef enum sh_Status_e sh_Status_t;

/**
 * @brief List of sensor types supported by the hub
 *
 * See the SH-1 Reference Manual for more information on each type.
 */
enum sh_SensorId_e {
	SH_RAW_ACCELEROMETER = 0x14,
	SH_ACCELEROMETER = 0x01,
	SH_LINEAR_ACCELERATION = 0x04,
	SH_GRAVITY = 0x06,
	SH_RAW_GYROSCOPE = 0x15,
	SH_GYROSCOPE_CALIBRATED = 0x02,
	SH_GYROSCOPE_UNCALIBRATED = 0x07,
	SH_RAW_MAGNETOMETER = 0x16,
	SH_MAGNETIC_FIELD_CALIBRATED = 0x03,
	SH_MAGNETIC_FIELD_UNCALIBRATED = 0x0f,
	SH_ROTATION_VECTOR = 0x05,
	SH_GAME_ROTATION_VECTOR = 0x08,
	SH_GEOMAGNETIC_ROTATION_VECTOR = 0x09,
	SH_PRESSURE = 0x0a,
	SH_AMBIENT_LIGHT = 0x0b,
	SH_HUMIDITY = 0x0c,
	SH_PROXIMITY = 0x0d,
	SH_TEMPERATURE = 0x0e,
	SH_SAR = 0x17,
	SH_TAP_DETECTOR = 0x10,
	SH_STEP_DETECTOR = 0x18,
	SH_STEP_COUNTER = 0x11,
	SH_SIGNIFICANT_MOTION = 0x12,
	SH_ACTIVITY_CLASSIFICATION = 0x13,
	SH_SHAKE_DETECTOR = 0x19,
	SH_FLIP_DETECTOR = 0x1a,
	SH_PICKUP_DETECTOR = 0x1b,
	SH_STABILITY_DETECTOR = 0x1c,
	SH_PERSONAL_ACTIVITY_CLASSIFIER = 0x1e,
	SH_SLEEP_DETECTOR = 0x1f,
	
	SH_MAX_SENSOR_ID = 0x1f,  // CAUTION: Always update this to reflect any added sensor ids
};
typedef uint8_t sh_SensorId_t;

/**
 * @brief Raw Accelerometer
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_RawAccelerometer {
	/* Units are ADC counts */
	int16_t x;  /**< @brief [ADC counts] */
	int16_t y;  /**< @brief [ADC counts] */
	int16_t z;  /**< @brief [ADC counts] */

	/* Microseconds */
	uint32_t timestamp;  /**< @brief [uS] */
} sh_RawAccelerometer_t;

/**
 * @brief Accelerometer
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_Accelerometer {
	int16_t x_16Q8;  /**< @brief [m/s^2], 16Q8 fixed point format. */
	int16_t y_16Q8;  /**< @brief [m/s^2], 16Q8 fixed point format. */
	int16_t z_16Q8;  /**< @brief [m/s^2], 16Q8 fixed point format. */
} sh_Accelerometer_t;

/**
 * @brief Raw gyroscope
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_RawGyroscope {
	/* Units are ADC counts */
	int16_t x;  /**< @brief [ADC Counts] */
	int16_t y;  /**< @brief [ADC Counts] */
	int16_t z;  /**< @brief [ADC Counts] */
	int16_t temperature;  /**< @brief [ADC Counts] */

	/* Microseconds */
	uint32_t timestamp;  /**< @brief [uS] */
} sh_RawGyroscope_t;

/**
 * @brief Gyroscope
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_Gyroscope {
	/* Units are rad/s */
	int16_t x_16Q9;  /**< @brief [rad/s] 16Q9 Fixed point format */
	int16_t y_16Q9;  /**< @brief [rad/s] 16Q9 Fixed point format */
	int16_t z_16Q9;  /**< @brief [rad/s] 16Q9 Fixed point format */
} sh_Gyroscope_t;

/**
 * @brief Uncalibrated gyroscope
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_GyroscopeUncalibrated {
	/* Units are rad/s */
	int16_t x_16Q9;  /**< @brief [rad/s] 16Q9 Fixed point format */
	int16_t y_16Q9;  /**< @brief [rad/s] 16Q9 Fixed point format */
	int16_t z_16Q9;  /**< @brief [rad/s] 16Q9 Fixed point format */
	int16_t biasx_16Q9;  /**< @brief [rad/s] 16Q9 Fixed point format */
	int16_t biasy_16Q9;  /**< @brief [rad/s] 16Q9 Fixed point format */
	int16_t biasz_16Q9;  /**< @brief [rad/s] 16Q9 Fixed point format */
} sh_GyroscopeUncalibrated_t;

/**
 * @brief Raw Magnetometer
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_RawMagnetometer {
	/* Units are ADC counts */
	int16_t x;  /**< @brief [ADC Counts] */
	int16_t y;  /**< @brief [ADC Counts] */
	int16_t z;  /**< @brief [ADC Counts] */

	/* Microseconds */
	uint32_t timestamp;  /**< @brief [uS] */
} sh_RawMagnetometer_t;

/**
 * @brief Magnetic field
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_MagneticField {
	/* Units are uTesla */
	int16_t x_16Q4;  /**< @brief [uTesla] 16Q4 fixed point format */
	int16_t y_16Q4;  /**< @brief [uTesla] 16Q4 fixed point format */
	int16_t z_16Q4;  /**< @brief [uTesla] 16Q4 fixed point format */
} sh_MagneticField_t;

/**
 * @brief Uncalibrated magnetic field
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_MagneticFieldUncalibrated {
	/* Units are uTesla */
	int16_t x_16Q5;  /**< @brief [uTesla] 16Q5 fixed point format */
	int16_t y_16Q5;  /**< @brief [uTesla] 16Q5 fixed point format */
	int16_t z_16Q5;  /**< @brief [uTesla] 16Q5 fixed point format */
	int16_t biasx_16Q5;  /**< @brief [uTesla] 16Q5 fixed point format */
	int16_t biasy_16Q5;  /**< @brief [uTesla] 16Q5 fixed point format */
	int16_t biasz_16Q5;  /**< @brief [uTesla] 16Q5 fixed point format */
} sh_MagneticFieldUncalibrated_t;

/**
 * @brief Step counter
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_StepCounter {
	uint32_t detectLatency;  /**< @brief [uS] */
	uint16_t steps;  /**< @brief [steps] */
	uint16_t reserved;
} sh_StepCounter_t;

/**
 * @brief Rotation Vector with Accuracy
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_RotationVectorWAcc {
	int16_t i_16Q14;  /**< @brief Quaternion component i, 16Q4 fixed point notation */
	int16_t j_16Q14;  /**< @brief Quaternion component j, 16Q4 fixed point notation */
	int16_t k_16Q14;  /**< @brief Quaternion component k, 16Q4 fixed point notation */
	int16_t real_16Q14;  /**< @brief Quaternion component, real, 16Q4 fixed point notation */
	int16_t accuracy_16Q12;  /**< @brief Accuracy estimate, 16Q12 fixed point notation */
} sh_RotationVectorWAcc_t;

/**
 * @brief Rotation Vector
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_RotationVector {
	int16_t i_16Q14;  /**< @brief Quaternion component i, 16Q4 fixed point notation */
	int16_t j_16Q14;  /**< @brief Quaternion component j, 16Q4 fixed point notation */
	int16_t k_16Q14;  /**< @brief Quaternion component k, 16Q4 fixed point notation */
	int16_t real_16Q14;  /**< @brief Quaternion component real, 16Q4 fixed point notation */
} sh_RotationVector_t;

/**
 * @brief Sensor Event
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_SensorEvent {
	/** Which sensor produced this event. */
	sh_SensorId_t sensor;

	/** @brief 8-bit unsigned integer used to track reports.
	 *
	 * The sequence number increments once for each report sent.  Gaps
	 * in the sequence numbers indicate missing or dropped reports.
	 */
	uint8_t sequenceNumber;

	/** @brief 64-bit microsecond timestamp
	 * 
	 */
	uint64_t time_us;

	/* Bits 1:0 - indicate the status of a sensor.
	 *   0 - Unreliable
	 *   1 - Accuracy low
	 *   2 - Accuracy medium
	 *   3 - Accuracy high
	 * Bits 4:2 - delay exponent
	 * Bits 7:5 - reserved
	 */
	uint8_t status; /**< @brief bits 7-5: reserved, 4-2: exponent delay, 1-0: Accuracy */

	/* 8-bit significand representing the delay from when a
	 * sensor sample is taken until that sample is reported to the
	 * host. The delay is scaled by the delay exponent. The total
	 * delay is given by delay * (2 ^ delay exponent). The units
	 * are microseconds. Delays greater than 32640 us are reported
	 * as 32640 us.
	 */
	uint8_t delay; /**< @brief [uS] value is delay * 2^exponent (see status) */

	/** @brief Sensor Data
	 *
	 * Use the structure based on the value of the sensor
	 * field.
	 */
	union {
		sh_RawAccelerometer_t rawAccelerometer;
		sh_Accelerometer_t accelerometer; 
		sh_Accelerometer_t linearAcceleration; 
		sh_Accelerometer_t gravity; 
		sh_RawGyroscope_t rawGyroscope; 
		sh_Gyroscope_t gyroscope; 
		sh_GyroscopeUncalibrated_t gyroscopeUncal; 
		sh_RawMagnetometer_t rawMagnetometer; 
		sh_MagneticField_t magneticField; 
		sh_MagneticFieldUncalibrated_t magneticFieldUncal; 
		sh_RotationVectorWAcc_t rotationVector; 
		sh_RotationVector_t gameRotationVector; 
		sh_RotationVectorWAcc_t geoMagRotationVector; 
		sh_StepCounter_t stepCounter; 

		/* Internal fields */
		uint16_t field16[6];        /* Many reports are 3 to 6 16-bit ints */
		uint32_t field32[3];        /* Some reports just contain 32-bit ints */
	} un;
} sh_SensorEvent_t;

/**
 * @brief Product Id value
 *
 * See the SH-1 Reference Manual for more detail.
 */
#define SH_NUM_PRODUCT_IDS (4)
typedef struct sh_ProductId_s {
	uint8_t resetCause;
	uint8_t swVersionMajor;
	uint8_t swVersionMinor;
	uint32_t swPartNumber;
	uint32_t swBuildNumber;
	uint16_t swVersionPatch;
} sh_ProductId_t;

/**
 * @brief Sensor Configuration settings
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_SensorConfig {
	/* Change sensitivity enabled */
	bool changeSensitivityEnabled;  /**< @brief Enable reports on change */

	/* Change sensitivity - true if relative; false if absolute */
	bool changeSensitivityRelative;  /**< @brief Change reports relative (vs absolute) */

	/* Wake-up enabled */
	bool wakeupEnabled;  /**< @brief Wake host on event */

	/* 16-bit signed fixed point integer representing the value a
	 * sensor output must exceed in order to trigger another input
	 * report. A setting of 0 causes all reports to be sent.
	 */
	uint16_t changeSensitivity;  /**< @brief Report-on-change threshold */

	/* Interval in microseconds between asynchronous input reports. */
	uint32_t reportInterval_us;  /**< @brief [uS] Report interval */

	/* Reserved field, not used. */
	uint32_t reserved1;  /**< @brief Reserved.  Set as zero. */

	/* Meaning is sensor specific */
	uint32_t sensorSpecific;  /**< @brief See SH-1 Reference Manual for details. */
} sh_SensorConfig_t;

/**
 * @brief SensorHub Error Record
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_ErrorRecord {
	uint8_t severity;   /**< @brief Error severity, 0: most severe. */
	uint8_t sequence;   /**< @brief Sequence number (by severity) */
	uint8_t source;     /**< @brief 1-MotionEngine, 2-MotionHub, 3-SensorHub, 4-Chip  */
	uint8_t error;      /**< @brief See SH-1 Reference Manual */
	uint8_t module;     /**< @brief See SH-1 Reference Manual */
	uint8_t code;       /**< @brief See SH-1 Reference Manual */
} sh_ErrorRecord_t;
  
/**
 * @brief SensorHub Counter Record
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_Counts {
	uint32_t offered;   /**< @brief [events] */
	uint32_t accepted;  /**< @brief [events] */
	uint32_t on;        /**< @brief [events] */
	uint32_t attempted; /**< @brief [events] */
} sh_Counts_t;

/**
 * @brief Bit Fields for specifying tare axes.
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef enum sh_TareAxis {
	SH_TARE_X = 1,  /**< @brief sh_tareNow() axes bit field */
	SH_TARE_Y = 2,  /**< @brief sh_tareNow() axes bit field */
	SH_TARE_Z = 4,  /**< @brief sh_tareNow() axes bit field */
} sh_TareAxis_t;

/**
 * @brief Values for specifying tare basis
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef enum sh_TareBasis {
	SH_TARE_BASIS_ROTATION_VECTOR = 0,             /**< @brief Use Rotation Vector */
	SH_TARE_BASIS_GAMING_ROTATION_VECTOR = 1,      /**< @brief Use Game Rotation Vector */
	SH_TARE_BASIS_GEOMAGNETIC_ROTATION_VECTOR = 2, /**< @brief Use Geomagnetic R.V. */
} sh_TareBasis_t;

/**
 * @brief Rotation Vector Sync Operations
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef enum sh_RvSyncOp {
	SYNC_NOW = 0,         /**< @brief Sync RV generation with this command */
	EXT_SYNC_ENABLE = 1,  /**< @brief Enable RV sync using external signal */
	EXT_SYNC_DISABLE = 2  /**< @brief Disable RV sync from external signal */
} sh_RvSyncOp_t;

/**
 * @brief Quaternion (double precision floating point representation.)
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_Quaternion {
	double x;
	double y;
	double z;
	double w;
} sh_Quaternion_t;

/**
 * @brief Sensor Metadata Record
 *
 * See the SH-1 Reference Manual for more detail.
 */
typedef struct sh_SensorMetadata {
	uint8_t meVersion;   /**< @brief Motion Engine Version */
	uint8_t mhVersion;  /**< @brief Motion Hub Version */
	uint8_t shVersion;  /**< @brief SensorHub Version */
	uint32_t range;  /**< @brief Same units as sensor reports */
	uint32_t resolution;  /**< @brief Same units as sensor reports */
	uint16_t revision;  /**< @brief Metadata record format revision */
	uint16_t power_mA;    /**< @brief [mA] Fixed point 16Q10 format */
	uint32_t minPeriod_uS;  /**< @brief [uS] */
	uint32_t fifoReserved;  /**< @brief (Unused) */
	uint32_t fifoMax;  /**< @brief (Unused) */
	uint32_t batchBufferBytes;  /**< @brief (Unused) */
	uint16_t qPoint1;     /**< @brief q point for sensor values */
	uint16_t qPoint2;     /**< @brief q point for accuracy or bias fields */
	uint32_t vendorIdLen; /**< @brief [bytes] */
	char vendorId[48];  /**< @brief Vendor name and part number */
	uint32_t sensorSpecificLen;  /**< @brief [bytes] */
	uint8_t sensorSpecific[48];  /**< @brief See SH-1 Reference Manual */
} sh_SensorMetadata_t;

#endif
