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

#include <string.h>

#include "SensorHubHid.h"
#include "SensorHubDev.h"
#include "sh_util.h"

// #define DEBUG_PRINTS
#ifdef DEBUG_PRINTS
#include <stdio.h>
#endif

#define SH_DESC_V1_LEN 30
#define SH_DESC_V1_BCD 0x0100

#define RESET_WAIT_MS (200)

// --- Private Types -----------------------------------------------------------

// SensorHub HID Registers
enum {
	SH_REGISTER_HID_DESCRIPTOR    = 1,
	SH_REGISTER_REPORT_DESCRIPTOR = 2,
	SH_REGISTER_INPUT             = 3,
	SH_REGISTER_OUTPUT            = 4,
	SH_REGISTER_COMMAND           = 5,
	SH_REGISTER_DATA              = 6,
};

enum hid_ReportType_e {
	HID_REPORT_TYPE_INPUT = 0x10,
	HID_REPORT_TYPE_OUTPUT = 0x20,
	HID_REPORT_TYPE_FEATURE = 0x30
};
typedef uint8_t sensorhub_ReportType_t;

enum hid_ReportOpcode_e {
	HID_RESET_OPCODE = 0x01,
	HID_GET_REPORT_OPCODE = 0x02,
	HID_SET_REPORT_OPCODE = 0x03,
	HID_GET_IDLE_OPCODE = 0x04,
	HID_SET_IDLE_OPCODE = 0x05,
	HID_GET_PROTOCOL_OPCODE = 0x06,
	HID_SET_PROTOCOL_OPCODE = 0x07,
	HID_SET_POWER_OPCODE = 0x08
};

typedef struct hid_descriptor_s {
	uint16_t wHIDDescLength;
	uint16_t bcdVersion;
	uint16_t wReportDescriptorLength;
	uint16_t wReportDescriptorRegister;
	uint16_t wInputRegister;
	uint16_t wMaxInputLength;
	uint16_t wOutputRegister;
	uint16_t wMaxOutputLength;
	uint16_t wCommandRegister;
	uint16_t wDataRegister;
	uint16_t wVendorID;
	uint16_t wProductID;
	uint16_t wVersionID;
	uint8_t reserved[4];
} __packed hid_descriptor_t;

typedef struct Hid_s {
	unsigned unit;
	void * dev;  // Pointer to platform-specific stuff
} Hid_t;

// --- Forward Declarations ----------------------------------------------------

static sh_Status_t shhid_setReport(void * hid,
                                   uint8_t reportType,
                                   uint8_t reportId,
                                   uint8_t *payload, uint16_t payloadLen);

static sh_Status_t shhid_getReport(void * hid,
                                   uint8_t reportType,
                                   uint8_t reportId,
                                   uint8_t *payload, uint16_t *payloadLen);

// --- Private data ------------------------------------------------------------

// SensorHub_t objects to be returned via shhid_init
Hid_t hid[MAX_SH_UNITS];

// --- Public API --------------------------------------------------------------
  
void * shhid_init(int unit, void * dev)
{
	sh_HidReport_t inReport;
	uint16_t bufLen = sizeof(inReport);
  
	// Validate unit
	if ((unit < 0) || (unit >= MAX_SH_UNITS)) {
		// no such unit
		return 0;
	}

	// Allocate a HID structure for this unit.
	Hid_t * pHid = &hid[unit];
	pHid->unit = unit;
	pHid->dev = dev;

	// Reset the device layer
	shdev_reset(pHid->dev);

	// HID over I2C specifies that there should be a message of all zeros
	// to be read after reset.  Read it and discard it.
	shhid_in(pHid, &inReport, &bufLen, RESET_WAIT_MS, 0);

	return pHid;
}

// Performs HID over i2c OUT,
// On entry,
//   report[0] should hold report id.
//   reportLen should store the length of the report including report id.
sh_Status_t shhid_out(void * hid, void *report, uint16_t reportLen)
{
	Hid_t * pHid = (Hid_t *)hid;
	uint8_t buffer[SHHID_MAX_REPORT_LEN+4];

	write16(&buffer[0], SH_REGISTER_OUTPUT);
	write16(&buffer[2], reportLen + 2);
	memcpy(&buffer[4], report, reportLen);

	return shdev_i2c(pHid->dev, buffer, reportLen+4, 0, 0);
}

// Check INTN, then i2c read to get IN report
// On entry,
//   *reportLen should be length of buffer pointed to by report
// On exit,
//   report[0] will hold report Id
//   subsequent bytes of report[] will hold report data
//   *reportLen will hold length of report (including report id)
sh_Status_t shhid_in(void * hid, sh_HidReport_t *report, uint16_t *reportLen, uint16_t wait_ms, uint32_t *timestamp)
{
	Hid_t * pHid = (Hid_t *)hid;
	sh_Status_t rc = SH_STATUS_SUCCESS;
	uint8_t buffer[SHHID_MAX_REPORT_LEN+2];
  
	// Check INTN
	bool ready = (shdev_waitIntn(pHid->dev, wait_ms) == false);

	if (!ready) {
		// No data available
#ifdef DEBUG_PRINTS
		printf("shhid_in: not ready");
#endif
		*reportLen = 0;
		return SH_STATUS_NO_DATA;
	}
	else {
		if (timestamp != NULL) {
			// Grab timestamp
			*timestamp = shdev_getTimestamp_us(pHid->dev);
		}
		
		// Read from I2C
		rc = shdev_i2c(pHid->dev, NULL, 0, buffer, sizeof(buffer));

		if (rc != SH_STATUS_SUCCESS) 
                        return rc;

		// Set returned report length
		*reportLen = read16(buffer);
    
		if ((*reportLen < 2) || (*reportLen > SHHID_MAX_INPUT_REPORT_LEN+2)) {
			// Invalid length
#ifdef DEBUG_PRINTS
			printf("shhid_in: invalid len: %d", *reportLen);
#endif
			return SH_STATUS_ERROR_I2C_IO;
		}

		// Set returned report (don't include len field.)
		memcpy((void *)report, buffer+2, *reportLen-2);
		*reportLen -= 2;
	}

#ifdef DEBUG_PRINTS
	printf("%-16s [0x%02x]:", "shhid_in", report->reportId);
	for (int i = 0; i < *reportLen-1; i++) {
		printf(" %02x", report->body[i]);
	}
	printf("\n");
#endif

	return rc;
}

// Performs HID over i2c SET_REPORT for OUT report, reportId should be in report[0]
sh_Status_t shhid_setOutReport(void * hid, void *report, uint16_t reportLen)
{
	sh_HidReport_t * r = (sh_HidReport_t *)report;
	return shhid_setReport(hid, HID_REPORT_TYPE_OUTPUT, r->reportId, r->body, reportLen-1);
}

// Performs HID over i2c SET_REPORT for FEATURE report, reportId should be in report[0]
sh_Status_t shhid_setFeatureReport(void * hid, void *report, uint16_t reportLen)
{
	sh_HidReport_t * r = (sh_HidReport_t *)report;
	return shhid_setReport(hid, HID_REPORT_TYPE_FEATURE, r->reportId, r->body, reportLen-1);
}

// Performs HID over i2c GET_REPORT for IN report, reportId should be in report[0]
sh_Status_t shhid_getInReport(void * hid, sh_HidReport_t *report, uint16_t *reportLen)
{
	uint16_t len = *reportLen - 1;
	sh_Status_t rc = shhid_getReport(hid, HID_REPORT_TYPE_INPUT, report->reportId, report->body, &len);

	if (rc != SH_STATUS_SUCCESS) return rc;

	*reportLen = len + 1;

	return rc;
}

// Performs HID over i2c GET_REPORT for IN report, reportId should be in report[0]
sh_Status_t shhid_getFeatureReport(void * hid, sh_HidReport_t *report, uint16_t *reportLen)
{
	uint16_t len = *reportLen - 1;
	sh_Status_t rc = shhid_getReport(hid, HID_REPORT_TYPE_FEATURE, report->reportId, report->body, &len);

	if (rc != SH_STATUS_SUCCESS) return rc;

	*reportLen = len + 1;

	return rc;
}

// --- Private methods ---------------------------------------------------------

static sh_Status_t shhid_setReport(void * hid, uint8_t reportType, uint8_t reportId,
                                   uint8_t *payload, uint16_t payloadLen)
{
	uint8_t cmd[SHHID_MAX_REPORT_LEN+8];
	int ix;
	Hid_t * pHid = (Hid_t *)hid;

#ifdef DEBUG_PRINTS
	printf("%-16s [0x%02x]:", "shhid_setReport", reportId);
	for (int n = 0; n < payloadLen; n++) {
		printf(" %02x", payload[n]);
	}
	printf("\n");
#endif

	cmd[0] = SH_REGISTER_COMMAND;
	cmd[1] = 0;

	if (reportId < 0x0F) {
		cmd[2] = reportType | reportId;
		cmd[3] = HID_SET_REPORT_OPCODE;
		ix = 4;
	}
	else {
		cmd[2] = reportType | 0x0F;
		cmd[3] = HID_SET_REPORT_OPCODE;
		cmd[4] = reportId;
		ix = 5;
	}

	cmd[ix++] = SH_REGISTER_DATA;
	cmd[ix++] = 0;
	cmd[ix++] = payloadLen + 2;
	cmd[ix++] = 0;

	memcpy(&cmd[ix], payload, payloadLen);
	ix += payloadLen;

	return shdev_i2c(pHid->dev, cmd, ix, NULL, 0);
}

static sh_Status_t shhid_getReport(void * hid, uint8_t reportType, uint8_t reportId,
                                   uint8_t *payload, uint16_t *payloadLen)
{
	uint8_t cmd[7];
	int ix;
	Hid_t * pHid = (Hid_t *)hid;
	sh_Status_t status;
	uint8_t buffer[SHHID_MAX_REPORT_LEN+2];
	int copylen;

	cmd[0] = SH_REGISTER_COMMAND;
	cmd[1] = 0;

	if (reportId < 0x0F) {
		cmd[2] = reportType | reportId;
		cmd[3] = HID_GET_REPORT_OPCODE;
		ix = 4;
	}
	else {
		cmd[2] = reportType | 0x0F;
		cmd[3] = HID_GET_REPORT_OPCODE;
		cmd[4] = reportId;
		ix = 5;
	}

	cmd[ix++] = SH_REGISTER_DATA;
	cmd[ix++] = 0;

	status = shdev_i2c(pHid->dev, cmd, ix, buffer, sizeof(buffer));

	if (status < 0) return status;

	copylen = read16(buffer)-2;
	if (copylen > *payloadLen) copylen = *payloadLen;
	memcpy(payload, buffer+2, copylen);
	*payloadLen = copylen;
  
#ifdef DEBUG_PRINTS
	printf("%-16s [0x%02x]:", "shhid_getReport", reportId);
	for (uint16_t n = 0; n < *payloadLen; n++) {
		printf(" %02x", payload[n]);
	}
	printf("\n");
#endif
  
	return status;
}
