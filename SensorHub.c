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
#include <stdio.h>

#include "SensorHub.h"
#include "SensorHubHid.h"
#include "SensorHubDev.h"
#include "sh_util.h"

// Max length of an FRS record, words. (actually SH-1 limit is 68, but we're building in headroom.)
#define MAX_FRS_WORDS (72)

#define SH_TIMEOUT_MS (10)

// --- Private Data Types -------------------------------------------------

typedef struct sh_SensorHub_s {
	unsigned unit;
	void * hid;  // Pointer to hid layer
	void * dev;  // Pointer to platform-specific stuff
	uint8_t commandSeq;
} sh_SensorHub_t;

enum sh_MetadataRecordId {
	SH_META_RAW_ACCELEROMETER            = 0xE301,
	SH_META_ACCELEROMETER                = 0xE302,
	SH_META_LINEAR_ACCELERATION          = 0xE303,
	SH_META_GRAVITY                      = 0xE304,
	SH_META_RAW_GYROSCOPE                = 0xE305,
	SH_META_GYROSCOPE_CALIBRATED         = 0xE306,
	SH_META_GYROSCOPE_UNCALIBRATED       = 0xE307,
	SH_META_RAW_MAGNETOMETER             = 0xE308,
	SH_META_MAGNETIC_FIELD_CALIBRATED    = 0xE309,
	SH_META_MAGNETIC_FIELD_UNCALIBRATED  = 0xE30A,
	SH_META_ROTATION_VECTOR              = 0xE30B,
	SH_META_GAME_ROTATION_VECTOR         = 0xE30C,
	SH_META_GEOMAGNETIC_ROTATION_VECTOR  = 0xE30D,
	SH_META_PRESSURE                     = 0xE30E,
	SH_META_AMBIENT_LIGHT                = 0xE30F,
	SH_META_HUMIDITY                     = 0xE310,
	SH_META_PROXIMITY                    = 0xE311,
	SH_META_TEMPERATURE                  = 0xE312,
	SH_META_TAP_DETECTOR                 = 0xE313,
	SH_META_STEP_DETECTOR                = 0xE314,
	SH_META_STEP_COUNTER                 = 0xE315,
	SH_META_SIGNIFICANT_MOTION           = 0xE316,
	SH_META_ACTIVITY_CLASSIFICATION      = 0xE317,
	SH_META_SHAKE_DETECTOR               = 0xE318,
	SH_META_FLIP_DETECTOR                = 0xE319,
	SH_META_PICKUP_DETECTOR              = 0xE31A,
	SH_META_STABILITY_DETECTOR           = 0xE31B,
	SH_META_PERSONAL_ACTIVITY_CLASSIFIER = 0xE31C,
	SH_META_SLEEP_DETECTOR               = 0xE31D,
};
typedef enum sh_MetadataRecordId sh_MetadataRecordId_t;

// --- Forward Declarations -----------------------------------------------

static int decodeEvent(sh_SensorEvent_t *event, sh_HidReport_t *report, uint16_t reportLen, uint32_t timestamp);

// --- Private Data -------------------------------------------------------

// sh_SensorHub_t objects to be returned via shdev_probe
sh_SensorHub_t device[MAX_SH_UNITS];

// --- Public API ---------------------------------------------------------

// sh_init
void * sh_init(unsigned unit)
{
	// Validate unit
	if (unit >= MAX_SH_UNITS) {
		// no such unit
		return 0;
	}
  
	// "Allocate" a SensorHub for this unit
	sh_SensorHub_t *sh = 0;
	sh = &device[unit];
  
	// Connect with the device-specific portion of the driver
	sh->dev = shdev_init(unit);
  
	// Connect with the HID layer
	sh->hid = shhid_init(unit, sh->dev);
  
	return sh;
}

// sh_getSensorConfig
int sh_getSensorConfig(void *sh, sh_SensorId_t sensorId, sh_SensorConfig_t *config)
{
	sh_SensorHub_t *pHub = (sh_SensorHub_t *)sh;
	sh_HidReport_t report;
	uint16_t reportLen = sizeof(report);
	int status;
  
	report.reportId = sensorId;
	status = shhid_getFeatureReport(pHub->hid, &report, &reportLen);
	if (status < 0) {
		return status;
	}
	if ((reportLen != sizeof(sh_SensorConfigFeatureReport_t)) ||
	    (report.reportId != sensorId)) {
		return SH_STATUS_BAD_REPORT;
	}

	sh_SensorConfigFeatureReport_t *featReport = (sh_SensorConfigFeatureReport_t *)&report;

	// unpack report into config
	config->changeSensitivityRelative =
		((featReport->flags & SH_CHANGE_SENSITIVITY_RELATIVE) != 0);
	config->changeSensitivityEnabled =
		((featReport->flags & SH_CHANGE_SENSITIVITY_ENABLED) != 0);
	config->wakeupEnabled =
		((featReport->flags & SH_WAKEUP_ENABLED) != 0);

	config->changeSensitivity = featReport->changeSensitivity;
	config->reportInterval_us = featReport->reportInterval_uS;
	config->reserved1 = featReport->reserved1;

	return 0;
}

// sh_setSensorConfig
int sh_setSensorConfig(void *sh, sh_SensorId_t sensorId, sh_SensorConfig_t *config)
{
	sh_SensorHub_t *pHub = (sh_SensorHub_t *)sh;
	sh_SensorConfigFeatureReport_t report;

	report.reportId = sensorId;
	report.flags = 
		(config->changeSensitivityRelative ? SH_CHANGE_SENSITIVITY_RELATIVE : 0x0) |
		(config->changeSensitivityEnabled ? SH_CHANGE_SENSITIVITY_ENABLED : 0x0) |
		(config->wakeupEnabled ? SH_WAKEUP_ENABLED : 0x0);
	report.changeSensitivity = config->changeSensitivity;
	report.reportInterval_uS = config->reportInterval_us;
	report.reserved1 = config->reserved1;
	report.sensorSpecific = config->sensorSpecific;

	return shhid_setFeatureReport(pHub->hid, (uint8_t *)&report, sizeof(report));
}

// sh_eventReady
bool sh_eventReady(void *sh)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	
	bool state = shdev_getIntn(pSensorHub->dev);

	// event is ready if intn is low
	return (state == false);
}

// sh_getEvent
int sh_getEvent(void *sh, sh_SensorEvent_t *pEvent)
{
	return sh_getEventTO(sh, 0, pEvent);
}

// sh_getEventTO
int sh_getEventTO(void *sh, uint16_t timeout_ms, sh_SensorEvent_t *pEvent)
{
	int rc = SH_STATUS_SUCCESS;
	sh_HidReport_t inReport;
	uint16_t reportLen = sizeof(inReport);
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	uint32_t timestamp;

	rc = shhid_in(pSensorHub->hid, &inReport, &reportLen, timeout_ms, &timestamp);

	if (rc != SH_STATUS_SUCCESS) {
	  return rc;
	}

	rc = decodeEvent(pEvent, &inReport, reportLen, timestamp);
  
	return rc;
}

// sh_getMetadata
int sh_getMetadata(void *sh, sh_SensorId_t sensorId, sh_SensorMetadata_t *pData)
{
	const static struct {
		sh_SensorId_t sensorId;
		sh_MetadataRecordId_t recordId;
	} sensorToRecordMap[] = {
		{ SH_RAW_ACCELEROMETER,            SH_META_RAW_ACCELEROMETER },
		{ SH_ACCELEROMETER,                SH_META_ACCELEROMETER },
		{ SH_LINEAR_ACCELERATION,          SH_META_LINEAR_ACCELERATION },
		{ SH_GRAVITY,                      SH_META_GRAVITY },
		{ SH_RAW_GYROSCOPE,                SH_META_RAW_GYROSCOPE },
		{ SH_GYROSCOPE_CALIBRATED,         SH_META_GYROSCOPE_CALIBRATED },
		{ SH_GYROSCOPE_UNCALIBRATED,       SH_META_GYROSCOPE_UNCALIBRATED },
		{ SH_RAW_MAGNETOMETER,             SH_META_RAW_MAGNETOMETER },
		{ SH_MAGNETIC_FIELD_CALIBRATED,    SH_META_MAGNETIC_FIELD_CALIBRATED },
		{ SH_MAGNETIC_FIELD_UNCALIBRATED,  SH_META_MAGNETIC_FIELD_UNCALIBRATED },
		{ SH_ROTATION_VECTOR,              SH_META_ROTATION_VECTOR },
		{ SH_GAME_ROTATION_VECTOR,         SH_META_GAME_ROTATION_VECTOR },
		{ SH_GEOMAGNETIC_ROTATION_VECTOR,  SH_META_GEOMAGNETIC_ROTATION_VECTOR },
		{ SH_PRESSURE,                     SH_META_PRESSURE },
		{ SH_AMBIENT_LIGHT,                SH_META_AMBIENT_LIGHT },
		{ SH_HUMIDITY,                     SH_META_HUMIDITY },
		{ SH_PROXIMITY,                    SH_META_PROXIMITY },
		{ SH_TEMPERATURE,                  SH_META_TEMPERATURE },
		{ SH_TAP_DETECTOR,                 SH_META_TAP_DETECTOR },
		{ SH_STEP_DETECTOR,                SH_META_STEP_DETECTOR },
		{ SH_STEP_COUNTER,                 SH_META_STEP_COUNTER },
		{ SH_SIGNIFICANT_MOTION,           SH_META_SIGNIFICANT_MOTION },
		{ SH_ACTIVITY_CLASSIFICATION,      SH_META_ACTIVITY_CLASSIFICATION },
		{ SH_SHAKE_DETECTOR,               SH_META_SHAKE_DETECTOR },
		{ SH_FLIP_DETECTOR,                SH_META_FLIP_DETECTOR },
		{ SH_PICKUP_DETECTOR,              SH_META_PICKUP_DETECTOR },
		{ SH_STABILITY_DETECTOR,           SH_META_STABILITY_DETECTOR },
		{ SH_PERSONAL_ACTIVITY_CLASSIFIER, SH_META_PERSONAL_ACTIVITY_CLASSIFIER },
		{ SH_SLEEP_DETECTOR,               SH_META_SLEEP_DETECTOR },
	};

	uint32_t frsData[MAX_FRS_WORDS];
	uint16_t frsDataLen;

	// pData must not be NULL!
	if (pData == 0) return SH_STATUS_BAD_PARAM;
  
	// Convert sensorId to metadata recordId
	int i;
	for (i = 0; i < ARRAY_LEN(sensorToRecordMap); i++) {
		if (sensorToRecordMap[i].sensorId == sensorId) {
			break;
		}
	}
	if (i >= ARRAY_LEN(sensorToRecordMap)) {
		// no match was found
		return SH_STATUS_BAD_PARAM;
	}
	uint16_t recordId = sensorToRecordMap[i].recordId;
  
	// Fetch the metadata
	frsDataLen = ARRAY_LEN(frsData);
	int rc = sh_getFrs(sh, recordId, frsData, &frsDataLen);
	if (rc != 0) {
		return rc;
	}
  
	// Populate the sensorMetadata structure with results
	pData->meVersion        = (frsData[0] >> 0) & 0xFF;
	pData->mhVersion        = (frsData[0] >> 8) & 0xFF;
	pData->shVersion        = (frsData[0] >> 16) & 0xFF;
	pData->range            = frsData[1];
	pData->resolution       = frsData[2];
	pData->power_mA         = (frsData[3] >> 0) & 0xFFFF;    // 16.10 forma = Xt
	pData->revision         = (frsData[3] >> 16) & 0xFFFF;
	pData->minPeriod_uS     = frsData[4];
	pData->fifoMax          = (frsData[5] >> 0) & 0xFFFF;
	pData->fifoReserved     = (frsData[5] >> 16) & 0xFFFF;
	pData->batchBufferBytes = (frsData[6] >> 0) & 0xFFFF;;
	pData->vendorIdLen      = (frsData[6] >> 16) & 0xFFFF;
	strcpy(pData->vendorId, ""); // init with empty string in case vendorIdLen == 0

	if (pData->vendorIdLen > ARRAY_LEN(pData->vendorId)) {
		return SH_STATUS_BAD_PARAM;
	}
	if (pData->revision == 0) {
		memcpy(pData->vendorId, (uint8_t *)&frsData[7], pData->vendorIdLen);
	}
	else if (pData->revision == 1) {
		pData->qPoint1        = (frsData[7] >> 0) & 0xFFFF;
		pData->qPoint2        = (frsData[7] >> 16) & 0xFFFF;
		memcpy(pData->vendorId, (uint8_t *)&frsData[8], pData->vendorIdLen);
	}
	else if (pData->revision == 2) {
		pData->qPoint1        = (frsData[7] >> 0) & 0xFFFF;
		pData->qPoint2        = (frsData[7] >> 16) & 0xFFFF;
		pData->sensorSpecificLen = (frsData[8] >> 0) & 0xFFFF;
		if (pData->sensorSpecificLen > ARRAY_LEN(pData->sensorSpecific)) {
			return SH_STATUS_BAD_PARAM;
		}
		memcpy(pData->sensorSpecific, (uint8_t *)&frsData[9], pData->sensorSpecificLen);
		int vendorIdOffset = 9 + ((pData->sensorSpecificLen+3)/4); // 9 + one word for every 4 bytes of SS data
		memcpy(pData->vendorId, (uint8_t *)&frsData[vendorIdOffset],
		       pData->vendorIdLen);
	}
	else {
		// Unrecognized revision!
	}

	return SH_STATUS_SUCCESS;
}

// sh_getFrs
int sh_getFrs(void *sh, uint16_t recordId, uint32_t *pData, uint16_t *dataLenWords)
{
	int rc = SH_STATUS_SUCCESS;
	sh_SensorHub_t * pSensorHub = (sh_SensorHub_t *)sh;
	sh_FrsReadReq_t outReport;
	sh_HidReport_t inReport;
	sh_FrsReadResp_t * readResp;
	uint16_t reportLen;
	bool done = false;
	uint8_t words;
	uint8_t offset;
	uint16_t lastCopied = 0;
	uint8_t status;
	uint16_t readLenWords;
	uint16_t n = 0;

	// store incoming dataLenWords, set outgoing to zero in case we return w/o setting data.
	readLenWords = *dataLenWords;
	*dataLenWords = 0;
  
	// Issue FRS read request
	outReport.reportId = SH_FRS_READ_REQUEST;
	outReport.reserved = 0;
	outReport.offset = 0;
	outReport.recordId = recordId;
	outReport.readLenWords = readLenWords;
  
	shhid_setOutReport(pSensorHub->hid, &outReport, sizeof(outReport));

	// Collect FRS read responses
	while (!done) {
		// Get the next input report
		reportLen = sizeof(inReport);

		rc = shhid_in(pSensorHub->hid, &inReport, &reportLen, SH_TIMEOUT_MS, 0);

		// if poll fails, we fail
		if (rc != SH_STATUS_SUCCESS) return rc;

		// Ignore anything but FRS Read responses for the requested recordId
		if (reportLen != sizeof(sh_FrsReadResp_t)) continue;

		if (inReport.reportId != SH_FRS_READ_RESPONSE) continue;
		
		readResp = (sh_FrsReadResp_t *)&inReport;
		if (readResp->recordId != recordId) continue;

		// Check status
		status = readResp->words_status & 0x0F;
    
		if (status == SH_FRS_READ_UNRECOGNIZED) return SH_STATUS_FRS_READ_UNRECOGNIZED_FRS;
		if (status == SH_FRS_READ_BUSY) return SH_STATUS_FRS_READ_BUSY;
		if (status == SH_FRS_READ_OUT_OF_RANGE) return SH_STATUS_FRS_READ_OFFSET_OUT_OF_RANGE;
		if (status == SH_FRS_READ_DEVICE_ERROR) return SH_STATUS_FRS_READ_DEVICE_ERROR;

		if (status == SH_FRS_READ_EMPTY) {
			*dataLenWords = 0;
			return SH_STATUS_SUCCESS;
		}
		
		// Store this portion of FRS record
		words = (readResp->words_status >> 4) & 0x0F;
		offset = readResp->offset;

		for (n = 0; n < words; n++) {
			if (offset+n >= readLenWords) {
				rc = SH_STATUS_FRS_READ_UNEXPECTED_LENGTH;
			}
			else {
				lastCopied = offset+n;
				pData[lastCopied] = readResp->dataWord[n];
			}
		}

		// Check for done condition
		if ((status == SH_FRS_READ_RECORD_COMPLETED) ||
		    (status == SH_FRS_READ_BLOCK_COMPLETED) ||
		    (status == SH_FRS_READ_BOTH_COMPLETED)) {
			done = true;
		}
	}

	// set dataLenWords to last offset copied + 1 for return
	*dataLenWords = lastCopied+1;
  
	return rc;
}

// sh_setFrs
int sh_setFrs(void *sh, uint16_t recordId, uint32_t *pData, uint16_t dataLen)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_FrsWriteReq_t writeReq;
	sh_FrsWriteDataReq_t writeDataReq;
	sh_HidReport_t inReport;
	sh_FrsWriteResp_t *writeResp;
	uint16_t len;
	uint16_t toWrite = dataLen;
	uint16_t offset = 0;
	int rc;
	uint16_t status;
  
	// Issue FRS write request
	writeReq.reportId = SH_FRS_WRITE_REQUEST;
	writeReq.reserved = 0;
	writeReq.dataLen = dataLen;
	writeReq.recordId = recordId;
	shhid_setOutReport(pSensorHub->hid, &writeReq, sizeof(writeReq));

	while (true) {
		// Get Write Response
		len = sizeof(inReport);
		rc = shhid_in(pSensorHub->hid, &inReport, &len, SH_TIMEOUT_MS, 0);
		if (rc != 0) {
			return rc;
		}

		// If this isn't a write response, ignore it
		if (inReport.reportId != SH_FRS_WRITE_RESPONSE) {
			continue;
		}
		if (len != sizeof(sh_FrsWriteResp_t)) {
			continue;
		}

		writeResp = (sh_FrsWriteResp_t *)&inReport;
		status = writeResp->status;
    
		// Check for errors
		if (status == SH_FRS_WRITE_UNRECOGNIZED)
			return SH_STATUS_FRS_WRITE_BAD_TYPE;
		if (status == SH_FRS_WRITE_BUSY)
			return SH_STATUS_FRS_WRITE_BUSY;
		if (status == SH_FRS_WRITE_FAILED)
			return SH_STATUS_FRS_WRITE_FAILED;
		if (status == SH_FRS_WRITE_BAD_MODE)
			return SH_STATUS_FRS_WRITE_BAD_MODE;
		if (status == SH_FRS_WRITE_BAD_LEN)
			return SH_STATUS_FRS_WRITE_BAD_LENGTH;
		if (status == SH_FRS_WRITE_INVALID)
			return SH_STATUS_FRS_WRITE_INVALID_RECORD;
		if (status == SH_FRS_WRITE_DEVICE_ERR)
			return SH_STATUS_FRS_WRITE_DEVICE_ERROR;
		if (status == SH_FRS_WRITE_READ_ONLY)
			return SH_STATUS_FRS_WRITE_READ_ONLY;

		// Check for successful completion condition
		if ((status == SH_FRS_WRITE_COMPLETED) &&
		    (toWrite == 0)) {
			return SH_STATUS_SUCCESS;
		}

		// Check for SH ended before we were ready
		if (status == SH_FRS_WRITE_COMPLETED)
			return SH_STATUS_FRS_WRITE_NOT_ENOUGH;

		if (toWrite > 0) {
			// Write more data
			writeDataReq.reportId = SH_FRS_WRITE_DATA_REQUEST;
			writeDataReq.reserved = 0;
			writeDataReq.wordOffset = offset;

			// set data[0] field
			writeDataReq.data[0] = pData[offset++];
			toWrite -= 1;
			if (toWrite > 0) {
				// set data[1], too
				writeDataReq.data[1] = pData[offset++];
				toWrite -= 1;
			}

			// Issue FRS write data request
			rc = shhid_setOutReport(pSensorHub->hid, &writeDataReq, sizeof(writeDataReq));
			if (rc != 0) {
				return rc;
			}
		}
	}

	// should never get here.  return is from inside the loop
}

// sh_getProdIds
int sh_getProdIds(void * sh, sh_ProductId_t prodId[SH_NUM_PRODUCT_IDS])
{
	int rc = SH_STATUS_SUCCESS;
	sh_ProdIdReq_t prodIdReq;
	sh_HidReport_t inReport;
	uint16_t reportLen;
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
        int prodIds = 0;

	// Send Product ID Request
	prodIdReq.reportId = SH_PRODUCT_ID_REQUEST;
	prodIdReq.reserved = 0;
	rc = shhid_setOutReport(pSensorHub->hid, &prodIdReq, sizeof(prodIdReq));
	if (rc != SH_STATUS_SUCCESS)
		goto exit;

	// Read SH_NUM_PRODUCT_IDS Product ID Responses
	while (prodIds < SH_NUM_PRODUCT_IDS) {
		reportLen = sizeof(inReport);
		rc = shhid_in(pSensorHub->hid, &inReport, &reportLen, SH_TIMEOUT_MS, 0);

		if (rc != SH_STATUS_SUCCESS) 
			goto exit;

		if ((reportLen == sizeof(sh_ProdIdResp_t)) &&
		    (inReport.reportId == SH_PRODUCT_ID_RESPONSE)) {
			// Got a product Id Response
			sh_ProdIdResp_t *prodIdResp = (sh_ProdIdResp_t *)&inReport;
			
			prodId[prodIds].resetCause = prodIdResp->resetCause;
			prodId[prodIds].swVersionMajor = prodIdResp->swVerMajor;
			prodId[prodIds].swVersionMinor = prodIdResp->swVerMinor;
			prodId[prodIds].swPartNumber = prodIdResp->swPartNo;
			prodId[prodIds].swBuildNumber = prodIdResp->swBuildNo;
			prodId[prodIds].swVersionPatch = prodIdResp->swVerPatch;
			prodIds++;
		}
	}

exit:
	// return status
	return rc;
}

// sh_getErrors
int sh_getErrors(void *sh, uint8_t severity, sh_ErrorRecord_t *pErrors, uint16_t *numErrors)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_GetErrsReq_t request;
	sh_HidReport_t inReport;
	int rc;
	uint16_t reportLen;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	// get command sequence number for this command
	uint8_t thisSeq = pSensorHub->commandSeq++;

	// format a request to get counts
	request.reportId = SH_COMMAND_REQUEST;
	request.sequence = thisSeq;
	request.command = SH_CR_REPORT_ERRORS;
	request.severity = severity;

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));
	if (rc != 0) return rc;

	int replies = 0;
	bool finished = false;
	while (!finished) {
		// get reply
		reportLen = sizeof(inReport);
		rc = shhid_in(pSensorHub->hid, &inReport, &reportLen, SH_TIMEOUT_MS, 0);
		if (rc != 0) return rc;

		// ignore if not the right reportId
		if ((reportLen != sizeof(sh_CommandResp_t)) ||
		    (inReport.reportId != SH_COMMAND_RESPONSE)) {
			continue;
		}

		// ignore if not the response for the command we just sent
		sh_CommandResp_t * cmdResp = (sh_CommandResp_t *)&inReport;
		if ((cmdResp->command != SH_CR_REPORT_ERRORS) ||
		    (cmdResp->cmdSeq != thisSeq)) {
			continue;
		}

		sh_GetErrsResp_t * errResp = (sh_GetErrsResp_t *)&inReport;

		// detect end of response sequence
                // Version 1.2.5 uses severity == 255 to denote no errors
                // Version 1.8.x uses source == 255.
		if ((errResp->source == 255) || (errResp->severity == 255)){
			finished = true;
		}
		else {
			// store content if we still have room
			if (replies < *numErrors) {
				pErrors[replies].severity = errResp->severity;
				pErrors[replies].sequence = errResp->errSeq;
				pErrors[replies].source   = errResp->source;
				pErrors[replies].error    = errResp->error;
				pErrors[replies].module   = errResp->module;
				pErrors[replies].code     = errResp->code;

				// increment replies seen
				replies++;
			}
		}
	}

	*numErrors = replies;

	return 0;
}

// sh_getCounts
int sh_getCounts(void *sh, sh_SensorId_t sensorId, sh_Counts_t *pCounts)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_CountsReq_t request;
	sh_HidReport_t response;
	
	int rc;
	uint16_t reportLen;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	uint8_t thisSeq = pSensorHub->commandSeq++;

	// format a request to get counts
	request.reportId   = SH_COMMAND_REQUEST;
	request.sequence   = thisSeq;
	request.command    = SH_CR_COUNTS;
	request.subCommand = SH_CR_COUNTS_GET;
	request.sensorId   = sensorId;

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));
	if (rc != 0) return rc;

	int replies = 0;
	while (replies < 2) {
		// get reply
		reportLen = sizeof(response);
		rc = shhid_in(pSensorHub->hid, &response, &reportLen, SH_TIMEOUT_MS, 0);
		if (rc != 0) return rc;

		// ignore if not the right message
		if ((reportLen != sizeof(sh_CommandResp_t)) ||
		    (response.reportId != SH_COMMAND_RESPONSE)) {
			continue;
		}

		sh_CommandResp_t *cmdResp = (sh_CommandResp_t *)&response;

		// ignore if not the response to this command
		if ((cmdResp->command != SH_CR_COUNTS) ||
		    (cmdResp->cmdSeq != thisSeq)) {
			continue;
		}

		sh_GetCountsResp_t * counts = (sh_GetCountsResp_t *)&response;

		// Check for bad status
		if (counts->status != 1) return SH_STATUS_SH_ERR;

		// store content, based on response seq num
		if (counts->respSeq == 0) {
			pCounts->offered = counts->value[0];
			pCounts->accepted = counts->value[1];
		}
		else if (counts->respSeq == 1) {
			pCounts->on = counts->value[0];
			pCounts->attempted = counts->value[1];
		}

		// increment replies seen
		replies++;
	}

	return 0;
}

// sh_clearCounts
int sh_clearCounts(void *sh, sh_SensorId_t sensorId)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_CountsReq_t request;
	int rc;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	uint8_t thisSeq = pSensorHub->commandSeq++;

	// format a request to clear counts
	request.reportId   = SH_COMMAND_REQUEST;
	request.sequence   = thisSeq;
	request.command    = SH_CR_COUNTS;
	request.subCommand = SH_CR_COUNTS_CLEAR;
	request.sensorId   = sensorId;

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));
	if (rc != 0) return rc;

	return 0;
}

// sh_setTare
int sh_tareNow(void *sh, uint8_t axes, sh_TareBasis_t basis)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_TareNowReq_t request;
	int rc;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	uint8_t thisSeq = pSensorHub->commandSeq++;

	// format the tare now command
	request.reportId = SH_COMMAND_REQUEST;
	request.sequence = thisSeq;
	request.command = SH_CR_TARE;
	request.subcommand = SH_CR_TARE_NOW;
	request.axes = axes;
	request.basis = basis;

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));
	if (rc != 0) return rc;

	return 0;
}

// sh_tareClear
int sh_tareClear(void *sh)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_SetReorientationReq_t request;
	int rc;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	uint8_t thisSeq = pSensorHub->commandSeq++;

	// format the tare set-orientation command
	request.reportId = SH_COMMAND_REQUEST;
	request.sequence = thisSeq;
	request.command = SH_CR_TARE;
	request.subcommand = SH_CR_TARE_SET_ORIENT;
	request.x = 0;
	request.y = 0;
	request.z = 0;
	request.w = 0;

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));
	if (rc != 0) return rc;

	return 0;
}

// sh_persistTare
int sh_persistTare(void *sh)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_PersistTareReq_t request;
	int rc;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	uint8_t thisSeq = pSensorHub->commandSeq++;

	// format the tare persist-tare command
	request.reportId = SH_COMMAND_REQUEST;
	request.sequence = thisSeq;
	request.command = SH_CR_TARE;
	request.subcommand = SH_CR_TARE_PERSIST;

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));
	if (rc != 0) return rc;

	return 0;
}

// sh_setReorientation
int sh_setReorientation(void *sh, sh_Quaternion_t *orientation)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_SetReorientationReq_t request;
	int rc;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	uint8_t thisSeq = pSensorHub->commandSeq++;

	// format the tare set-orientation command
	request.reportId = SH_COMMAND_REQUEST;
	request.sequence = thisSeq;
	request.command = SH_CR_TARE;
	request.subcommand = SH_CR_TARE_SET_ORIENT;
	request.x = TO_16Q14(orientation->x);
	request.y = TO_16Q14(orientation->y);
	request.z = TO_16Q14(orientation->z);
	request.w = TO_16Q14(orientation->w);

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));
	if (rc != 0) return rc;

	return 0;
}

// sh_reinitialize
int sh_reinitialize(void *sh)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_ReinitializeReq_t request;
	int rc;
  
	uint8_t thisSeq = pSensorHub->commandSeq++;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	// format a request to get counts
	request.reportId = SH_COMMAND_REQUEST;
	request.sequence = thisSeq;
	request.command = SH_CR_INITIALIZE;
	request.subsystem = SH_CR_INITIALIZE_SENSORHUB;

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));
	if (rc != 0) return rc;

	return 0;
}

// sh_dcdSaveNow
int sh_dcdSaveNow(void *sh)
{
	// command : SH_CR_SAVE_DCD
	// response : single message, [5] contains status, 0 on success

	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_DcdSaveNowReq_t request;
	sh_HidReport_t response;
	int rc;
	uint16_t reportLen;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	uint8_t thisSeq = pSensorHub->commandSeq++;

	// format a request to get counts
	request.reportId = SH_COMMAND_REQUEST;
	request.sequence = thisSeq;
	request.command = SH_CR_SAVE_DCD;

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));
	if (rc != 0) return rc;

	int replies = 0;
	while (replies < 1) {
		// get reply
		reportLen = sizeof(response);
		rc = shhid_in(pSensorHub->hid, &response, &reportLen, SH_TIMEOUT_MS, 0);
		if (rc != 0) return rc;

		// ignore if not the right message
		if ((reportLen != sizeof(sh_CommandResp_t)) ||
		    (response.reportId != SH_COMMAND_RESPONSE)) {
			continue;
		}

		sh_CommandResp_t *cmdResp = (sh_CommandResp_t *)&response;

		// ignore if not the response to this command
		if ((cmdResp->command != SH_CR_SAVE_DCD) ||
		    (cmdResp->cmdSeq != thisSeq)) {
			continue;
		}

		sh_DcdSaveNowResp_t * dcdSaveResp = (sh_DcdSaveNowResp_t *)&response;
		
		// copy status to rc for return
		rc = dcdSaveResp->status;

		replies++;
	}

	return rc;
}

// sh_calConfig
int sh_calConfig(void *sh, uint8_t sensors)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_CalConfigReq_t request;
	sh_HidReport_t response;
	int rc;
	uint16_t reportLen;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	uint8_t thisSeq = pSensorHub->commandSeq++;

	// format a request to get counts
	request.reportId = SH_COMMAND_REQUEST;
	request.sequence = thisSeq;
	request.command = SH_CR_CAL_CONFIG;
	request.accel = (sensors & SH_CAL_ACCEL) ? 1 : 0;
	request.gyro =  (sensors & SH_CAL_GYRO)  ? 1 : 0;
	request.mag =   (sensors & SH_CAL_MAG)   ? 1 : 0;

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));
	if (rc != 0) return rc;

	int replies = 0;
	while (replies < 1) {
		// get reply
		reportLen = sizeof(response);
		rc = shhid_in(pSensorHub->hid, &response, &reportLen, SH_TIMEOUT_MS, 0);
		if (rc != 0) return rc;
		
		// ignore if not the right message
		if ((reportLen != sizeof(sh_CommandResp_t)) ||
		    (response.reportId != SH_COMMAND_RESPONSE)) {
			continue;
		}

		sh_CommandResp_t *cmdResp = (sh_CommandResp_t *)&response;

		// ignore if not the response to this command
		if ((cmdResp->command != SH_CR_CAL_CONFIG) ||
		    (cmdResp->cmdSeq != thisSeq)) {
			continue;
		}

		sh_CalConfigResp_t * calCfgResp = (sh_CalConfigResp_t *)&response;

		// copy status to rc for return
		rc = calCfgResp->status;

		replies++;
	}

	return rc;
}

// sh_rvSync
int sh_rvSync(void *sh, sh_RvSyncOp_t rvSyncOp)
{
	sh_SensorHub_t *pSensorHub = (sh_SensorHub_t *)sh;
	sh_RvSyncReq_t request;
	int rc;

	// zero the report before sending
	memset(&request, 0, sizeof(request));

	uint8_t thisSeq = pSensorHub->commandSeq++;

	// format a request to get counts
	request.reportId = SH_COMMAND_REQUEST;
	request.sequence = thisSeq;
	request.command = SH_CR_RV_SYNC;
	request.operation = rvSyncOp;

	// send the request
	rc = shhid_setOutReport(pSensorHub->hid, &request, sizeof(request));

	return rc;
}


// --- Private utility functions --------------------------------------------------------------

static int decodeEvent(sh_SensorEvent_t *event, sh_HidReport_t *report, uint16_t length, uint32_t timestamp)
{
	sh_SensorEventReport_t *r = (sh_SensorEventReport_t *)report;
	static uint64_t time_us = 0;
	static uint32_t last_timestamp = 0;
	int32_t delta_t;
	uint32_t delay;
	
	if (length > SHHID_MAX_INPUT_REPORT_LEN) {
		return SH_STATUS_BAD_PARAM;
	}

	// Only sensor events (reportId <= 0x7F) are decoded by this function
	if (report->reportId >= 0x80) {
		return SH_STATUS_BAD_REPORT;
	}

	// Compute delay
	delay = r->delay * (1 << ((r->status >> 2) & 0x07));

	// timestamp processing
	delta_t = timestamp - last_timestamp;
	last_timestamp = timestamp;
	time_us += delta_t;
	event->time_us = time_us - delay;
	
	// Common fields
	event->sensor = r->reportId;
	event->sequenceNumber = r->sequenceNumber;
	event->status = r->status;
	event->delay = r->delay;

	// Do sensor-specific stuff
	switch (event->sensor) {
		/* Reports that are 1 16-bit integer */
	case SH_HUMIDITY:
	case SH_PROXIMITY:
	case SH_TEMPERATURE:
	case SH_SIGNIFICANT_MOTION:
	case SH_SHAKE_DETECTOR:
	case SH_FLIP_DETECTOR:
	case SH_PICKUP_DETECTOR:
	case SH_STABILITY_DETECTOR:
		if (length < 6)
			return SH_STATUS_BAD_REPORT;

		event->un.field16[0] = read16(&r->data[0]);
		break;

		/* Reports that are 1 32-bit integer */
	case SH_PRESSURE:
	case SH_AMBIENT_LIGHT:
	case SH_STEP_DETECTOR:
		if (length < 8)
			return SH_STATUS_BAD_REPORT;

		event->un.field32[0] = read32(&r->data[0]);
		break;

		/* 4 16-bit integers and a 32-bit timestamp */
	case SH_RAW_ACCELEROMETER:
	case SH_RAW_GYROSCOPE:
	case SH_RAW_MAGNETOMETER:
		if (length < 16)
			return SH_STATUS_BAD_REPORT;

		event->un.field16[0] = read16(&r->data[0]);
		event->un.field16[1] = read16(&r->data[2]);
		event->un.field16[2] = read16(&r->data[4]);
		event->un.field16[3] = read16(&r->data[6]);
		event->un.field32[2] = read32(&r->data[8]);
		break;

		/* Reports that are 3 16-bit integers */
	case SH_ACCELEROMETER:
	case SH_LINEAR_ACCELERATION:
	case SH_GRAVITY:
	case SH_GYROSCOPE_CALIBRATED:
	case SH_MAGNETIC_FIELD_CALIBRATED:
		if (length < 10)
			return SH_STATUS_BAD_REPORT;

		event->un.field16[0] = read16(&r->data[0]);
		event->un.field16[1] = read16(&r->data[2]);
		event->un.field16[2] = read16(&r->data[4]);
		break;

		/* Reports that are 4 16-bit integers */
	case SH_GAME_ROTATION_VECTOR:
		if (length < 12)
			return SH_STATUS_BAD_REPORT;

		event->un.field16[0] = read16(&r->data[0]);
		event->un.field16[1] = read16(&r->data[2]);
		event->un.field16[2] = read16(&r->data[4]);
		event->un.field16[3] = read16(&r->data[6]);
		break;

		/* Reports that are 5 16-bit integers */
	case SH_ROTATION_VECTOR:
	case SH_GEOMAGNETIC_ROTATION_VECTOR:
		if (length < 14)
			return SH_STATUS_BAD_REPORT;

		event->un.field16[0] = read16(&r->data[0]);
		event->un.field16[1] = read16(&r->data[2]);
		event->un.field16[2] = read16(&r->data[4]);
		event->un.field16[3] = read16(&r->data[6]);
		event->un.field16[4] = read16(&r->data[8]);
		break;

		/* Reports that are 6 16-bit integers */
	case SH_GYROSCOPE_UNCALIBRATED:
	case SH_MAGNETIC_FIELD_UNCALIBRATED:
		if (length < 16)
			return SH_STATUS_BAD_REPORT;

		event->un.field16[0] = read16(&r->data[0]);
		event->un.field16[1] = read16(&r->data[2]);
		event->un.field16[2] = read16(&r->data[4]);
		event->un.field16[3] = read16(&r->data[6]);
		event->un.field16[4] = read16(&r->data[8]);
		event->un.field16[5] = read16(&r->data[10]);
		break;

	case SH_STEP_COUNTER:
		if (length < 12)
			return SH_STATUS_BAD_REPORT;

		event->un.stepCounter.detectLatency = read32(&r->data[0]);
		event->un.stepCounter.steps = read16(&r->data[4]);
		event->un.stepCounter.reserved = read16(&r->data[6]);
		break;

		/* TBD */
	case SH_SAR:
	case SH_TAP_DETECTOR:
	case SH_ACTIVITY_CLASSIFICATION:
	default:
		return SH_STATUS_BAD_REPORT;
	}
  
	return SH_STATUS_SUCCESS;
}

