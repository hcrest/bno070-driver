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

#ifndef SENSORHUB_HID_H
#define SENSORHUB_HID_H

#include <stdint.h>
#include "sh_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// HID Report lengths
#define SHHID_MAX_INPUT_REPORT_LEN 16
#define SHHID_MAX_OUTPUT_REPORT_LEN 16
#define SHHID_MAX_REPORT_LEN 16

#ifdef ARDUINO
  // On Arduino, don't do packed structures
  #define __packed 
#endif


// A generic structure to overlay over any HID report
typedef struct sh_HidReport_s {
	uint8_t reportId;
	uint8_t body[SHHID_MAX_REPORT_LEN-1];
} __packed sh_HidReport_t;

// Overlay structure for sensor event reports
typedef struct sh_SensorEventReport_s {
	uint8_t reportId;       // equals sensor id.  Always < 0x80
	uint8_t sequenceNumber;
	uint8_t status;
	uint8_t delay;
	uint8_t data[SHHID_MAX_REPORT_LEN-4];
} __packed sh_SensorEventReport_t;

// Flags used in changeSensitivity
#define SH_CHANGE_SENSITIVITY_RELATIVE (0x01)
#define SH_CHANGE_SENSITIVITY_ENABLED (0x02)
#define SH_WAKEUP_ENABLED (0x04)

typedef struct sh_SensorConfigFeatureReport_s {
	uint8_t reportId;      // equals sensor id.  Always < 0x80
	uint8_t flags;
	uint16_t changeSensitivity;
	uint32_t reportInterval_uS;
	uint32_t reserved1;
	uint32_t sensorSpecific;
} __packed sh_SensorConfigFeatureReport_t;

typedef uint8_t sensorhub_ReportId_t;

// Product Id Request
#define SH_PRODUCT_ID_REQUEST (0x80)
typedef struct sh_ProdIdReq_s {
	uint8_t reportId;
	uint8_t reserved;
} __packed sh_ProdIdReq_t;

// Product Id Response
#define SH_PRODUCT_ID_RESPONSE (0x81)
typedef struct sh_ProdIdResp_s {
	uint8_t reportId;
	uint8_t resetCause;
	uint8_t swVerMajor;
	uint8_t swVerMinor;
	uint32_t swPartNo;
	uint32_t swBuildNo;
	uint16_t swVerPatch;
	uint16_t reserved;
} __packed sh_ProdIdResp_t;

// FRS Write Request
#define SH_FRS_WRITE_REQUEST (0x82)
typedef struct sh_FrsWriteReq_s {
	uint8_t reportId;
	uint8_t reserved;
	uint16_t dataLen;
	uint16_t recordId;
} __packed sh_FrsWriteReq_t;

// FRS WriteData Request
#define SH_FRS_WRITE_DATA_REQUEST (0x83)
typedef struct sh_FrsWriteDataReq_s {
	uint8_t reportId;
	uint8_t reserved;
	uint16_t wordOffset;
	uint32_t data[2];
} __packed sh_FrsWriteDataReq_t;

// Values of sh_FrsWriteResp status field
#define SH_FRS_WRITE_OK (0)
#define SH_FRS_WRITE_UNRECOGNIZED (1)
#define SH_FRS_WRITE_BUSY (2)
#define SH_FRS_WRITE_COMPLETED (3)
#define SH_FRS_WRITE_READY (4)
#define SH_FRS_WRITE_FAILED (5) 
#define SH_FRS_WRITE_BAD_MODE (6)
#define SH_FRS_WRITE_BAD_LEN (7)
#define SH_FRS_WRITE_VALID (8)
#define SH_FRS_WRITE_INVALID (9)
#define SH_FRS_WRITE_DEVICE_ERR (10)
#define SH_FRS_WRITE_READ_ONLY (11)

// FRS Write Response
#define SH_FRS_WRITE_RESPONSE (0x84)
typedef struct sh_FrsWriteResp_s {
	uint8_t reportId;
	uint8_t status;
	uint16_t wordOffset;
} __packed sh_FrsWriteResp_t;

#define SH_FRS_READ_REQUEST (0x85)
typedef struct sh_FrsReadReq_s {
	uint8_t reportId;
	uint8_t reserved;
	uint16_t offset;
	uint16_t recordId;
	uint16_t readLenWords;
} __packed sh_FrsReadReq_t;

// Values of sh_FrsReadResp_t status field
#define SH_FRS_READ_NO_ERROR (0)
#define SH_FRS_READ_UNRECOGNIZED (1)
#define SH_FRS_READ_BUSY (2)
#define SH_FRS_READ_RECORD_COMPLETED (3)
#define SH_FRS_READ_OUT_OF_RANGE (4)
#define SH_FRS_READ_EMPTY (5)
#define SH_FRS_READ_BLOCK_COMPLETED (6)
#define SH_FRS_READ_BOTH_COMPLETED (7)
#define SH_FRS_READ_DEVICE_ERROR (8)

#define SH_FRS_READ_RESPONSE (0x86)
typedef struct sh_FrsReadResp_s {
	uint8_t reportId;
	uint8_t words_status;  // words in 0xF0, status in 0x0F
	uint16_t offset;
	uint32_t dataWord[2];
	uint16_t recordId;
	uint16_t reserved;
} __packed sh_FrsReadResp_t;

// Command/Response command and subcommand codes
#define SH_CR_REPORT_ERRORS        (0x01)      // Command
#define SH_CR_COUNTS               (0x02)      // Command
#define SH_CR_COUNTS_GET             (0x00)    //   subcommand
#define SH_CR_COUNTS_CLEAR           (0x01)    //   subcommand
#define SH_CR_TARE                 (0x03)      // Command
#define SH_CR_TARE_NOW               (0x00)    //   subcommand
#define SH_CR_TARE_PERSIST           (0x01)    //   subcommand
#define SH_CR_TARE_SET_ORIENT        (0x02)    //   subcommand
#define SH_CR_INITIALIZE           (0x04)      // Command
#define SH_CR_INITIALIZE_NOP         (0x00)    //   subsystem
#define SH_CR_INITIALIZE_SENSORHUB   (0x01)    //   subsystem
#define SH_CR_FRS_CHANGE           (0x05)      // (Response only)
#define SH_CR_SAVE_DCD             (0x06)      // Command
#define SH_CR_CAL_CONFIG           (0x07)      // Command
#define SH_CR_RV_SYNC              (0x08)      // Command

// Command Request (General form)
#define SH_COMMAND_REQUEST (0x87)
typedef struct sh_CommandReq_s {
	uint8_t reportId;  // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;   // SH_CR_...
	uint8_t p[9];
} __packed sh_CommandReq_t;

// Command Response (General form)
#define SH_COMMAND_RESPONSE (0x88)
typedef struct sh_CommandResp_s {
	uint8_t reportId;  // SH_COMMAND_RESPONSE
	uint8_t sequence;
	uint8_t command;   // SH_CR_...
	uint8_t cmdSeq;
	uint8_t respSeq;
	uint8_t r[11];
} __packed sh_CommandResp_t;

// Command Request (for REPORT_ERRORS)
typedef struct sh_GetErrsReq_s {
	uint8_t reportId;  // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;   // SH_CR_REPORT_ERRORS
	uint8_t severity;
	uint8_t unused[8];
} __packed sh_GetErrsReq_t;

// Command Response (REPORT_ERRORS response)
typedef struct sh_GetErrsResp_s {
	uint8_t reportId;  // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;   // SH_CR_REPORT_ERRORS
	uint8_t cmdSeq;
	uint8_t respSeq;
	uint8_t severity;
	uint8_t errSeq;
	uint8_t source;
	uint8_t error;
	uint8_t module;
	uint8_t code;
	uint8_t reserved[5];
} __packed sh_GetErrsResp_t;

// Command Request (COUNTS_GET)
typedef struct sh_CountsReq_s {
	uint8_t reportId;     // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;      // SH_CR_COUNTS
	uint8_t subCommand;   // SH_CR_COUNTS_GET or _CLEAR
	uint8_t sensorId;
	uint8_t reserved[7];
} __packed sh_CountsReq_t;

typedef struct sh_GetCountsResp_s {
	uint8_t reportId;  // SH_COMMAND_RESPONSE
	uint8_t sequence;
	uint8_t command;   // SH_CR_COUNTS
	uint8_t cmdSeq;
	uint8_t respSeq;
	uint8_t sensorId;
	uint8_t status;
	uint8_t reserved;
	uint32_t value[2];
} __packed sh_GetCountsResp_t;

typedef struct sh_TareNowReq_s {
	uint8_t reportId;    // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;     // SH_CR_TARE
	uint8_t subcommand;  // SH_CR_TARE_NOW
	uint8_t axes;
	uint8_t basis;
	uint8_t reserved[6];
} __packed sh_TareNowReq_t;

typedef struct sh_PersistTareReq_s {
	uint8_t reportId;    // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;     // SH_CR_TARE
	uint8_t subcommand;  // SH_CR_TARE_PERSIST
	uint8_t reserved[8];
} __packed sh_PersistTareReq_t;

typedef struct sh_SetReorientationReq_s {
	uint8_t reportId;    // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;     // SH_CR_TARE
	uint8_t subcommand;  // SH_CR_TARE_SET_ORIENT
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t w;
} __packed sh_SetReorientationReq_t;

typedef struct sh_ReinitializeReq_s {
	uint8_t reportId;     // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;      // SH_CR_INITIALIZE
	uint8_t subsystem;
	uint8_t reserved[8];
} __packed sh_ReinitializeReq_t;

typedef struct sh_InitResp_s {
	uint8_t reportId;     // SH_COMMAND_RESPONSE
	uint8_t sequence;
	uint8_t command;      //SH_CR_INITIALIZE
	uint8_t cmdSeq;
	uint8_t respSeq;
	uint8_t status;
	uint8_t subsystem;
	uint8_t reserved[9];
} __packed sh_InitResp_t;

typedef struct sh_FrsChangeNotif_s {
	uint8_t reportId;     // SH_COMMAND_RESPONSE
	uint8_t sequence;
	uint8_t command;      // SH_CR_FRS_CHANGE
	uint8_t cmdSeq;
	uint8_t respSeq;
	uint8_t reserved;
	uint16_t frsType;
	uint8_t reserved2[5];
} __packed sh_FrsChangeNotif_t;

typedef struct sh_DcdSaveNowReq_s {
	uint8_t reportId;     // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;      // SH_CR_SAVE_DCD
	uint8_t reserved[9];
} __packed sh_DcdSaveNowReq_t;

typedef struct sh_DcdSaveNowResp_s {
	uint8_t reportId;     // SH_COMMAND_RESPONSE
	uint8_t sequence;
	uint8_t command;      // SH_CR_SAVE_DCD
	uint8_t cmdSeq;
	uint8_t respSeq;
	uint8_t status;
	uint8_t reserved[10];
} __packed sh_DcdSaveNowResp_t;

typedef struct sh_CalConfigReq_s {
	uint8_t reportId;     // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;      // SH_CR_CAL_CONFIG
	uint8_t accel;
	uint8_t gyro;
	uint8_t mag;
	uint8_t reserved[6];
} __packed sh_CalConfigReq_t;

typedef struct sh_CalConfigResp_s {
	uint8_t reportId;     // SH_COMMAND_RESPONSE
	uint8_t sequence;
	uint8_t command;      // SH_CR_CAL_CONFIG
	uint8_t cmdSeq;
	uint8_t respSeq;
	uint8_t status;
	uint8_t reserved[12];
} __packed sh_CalConfigResp_t;


typedef struct sh_RvSyncReq_s {
	uint8_t reportId;     // SH_COMMAND_REQUEST
	uint8_t sequence;
	uint8_t command;      // SH_CR_RV_SYNC
	uint8_t operation;
	uint8_t reserved[8];
} __packed sh_RvSyncReq_t;

// --- Public Methods --------------------------------------------------

// HID Communications layer for Sensor Hub SH-1

void * shhid_init(int unit, void * dev);

// Performs HID over i2c OUT, reportId should be in report[0]
sh_Status_t shhid_out(void * hid, void *report, uint16_t reportLen);

// Performs HID over i2c IN, reportId will be returned in report[0]
sh_Status_t shhid_in(void * hid, sh_HidReport_t *report, uint16_t *reportLen,
                     uint16_t wait_ms, uint32_t *timestamp);

// Performs HID over i2c SET_REPORT for OUT report,
sh_Status_t shhid_setOutReport(void * hid, void *report, uint16_t reportLen);

// Performs HID over i2c SET_REPORT for FEATURE report,
sh_Status_t shhid_setFeatureReport(void * hid, void *report, uint16_t reportLen);

// Performs HID over i2c GET_REPORT for IN report,
// report->reportId field should be set to requested report id on entry.
sh_Status_t shhid_getInReport(void * hid, sh_HidReport_t *report, uint16_t *reportLen);

// Performs HID over i2c GET_REPORT for FEATURE report,
// report->reportId field should be set to requested report id on entry.
sh_Status_t shhid_getFeatureReport(void * hid, sh_HidReport_t *report, uint16_t *reportLen);

#ifdef __cplusplus
}    // end of extern "C"
#endif

#endif
