/* SH-1 MCU Driver - library for communicating with BNO070
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

#ifndef SENSORHUB_H
#define SENSORHUB_H

#include <stdint.h>
#include <stdbool.h>
#include "sh_types.h"

#define SH1_DRIVER_VERSION "1.1.1"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a session with the SensorHub.
 *
 * This function should be called before any others in the API.  It returns
 * a void * which is used a reference to the SensorHub in all other API calls.
 * 
 * @param  unit Which SensorHub to open if the system supports multiple units.
 * @return      Reference to the SensorHub or NULL on failure.
 */
void * sh_init(unsigned unit);

/**
 * @brief Read the current configuration of a sensor.
 *
 * Configuration includes the reporting rate, whether the events should 
 * wake the host processor, etc.
 *
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param      sensorId Which sensor to operate on.
 * @param[out] config   Sensor configuration returned through this.
 * @return              SH_STATUS_SUCCESS or failure code.
 */
int sh_getSensorConfig(void *sh,
                       sh_SensorId_t sensorId,
                       sh_SensorConfig_t *config);

/**
 * @brief Set the current configuration of a sensor.
 *
 * Enable, Disable and set rate of a sensor.  Also sets other operational
 * parameters of the sensor such as wake-on-event.
 *
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param      sensorId Which sensor to operate on.
 * @param      config   The new configuration for the sensor.
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_setSensorConfig(void *sh,
                       sh_SensorId_t sensorId,
                       sh_SensorConfig_t *config);

/**
 * @brief Checks for a sensor event.
 * 
 * Returns true if a sensor event is ready to read.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @return              true if data ready.
 */
bool sh_eventReady(void *sh);

/**
 * @brief Read a sensor event from the SensorHub.
 * 
 * Reads a sensor event if one is available.  If no event is ready,
 * returns SH_STATUS_NO_DATA.  Sensor events represent the output of
 * the SensorHub based on the device's motion and/or state.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param[out] pEvent   The event that was read.
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_getEvent(void *sh, sh_SensorEvent_t *pEvent);

/**
 * @brief Read a sensor event from the SensorHub, blocks with timeout.
 * 
 * Reads a sensor event.  If no event is ready, blocks until an event
 * arrives or the timeout expires.  Will return without blocking if a
 * timeout of zero is provided.  A timeout of SH_WAIT_FOREVER will
 * block indefinitely.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param      timeout_ms  Max time to wait. [ms]  SH_WAIT_FOREVER will block indefinitely
 * @param[out] pEvent   Storage for the event that is read.
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_getEventTO(void *sh, uint16_t timeout_ms, sh_SensorEvent_t *pEvent);

/**
 * @brief Get Metadata related to a particular sensor.
 * 
 * The Metadata describes the sensor's range of operating rates, 
 * manufacturer identification, etc.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param      sensorId Which sensor to operate on.
 * @param[out] pData    Contains the requested metadata on return.
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_getMetadata(void *sh,
                   sh_SensorId_t sensorId,
                   sh_SensorMetadata_t *pData);

/**
 * @brief Read a Non-Volatile configuration record (FRS record).
 * 
 * A variety of FRS records are defined to store calibration and other
 * important data associated with the SensorHub.  This function can
 * retrieve one of those records.
 * 
 * @param         sh           The SensorHub reference obtained via sh_init().
 * @param         recordId     Which record to access.
 * @param[out]    pData        Storage location to hold the record on return.
 * @param[in,out] dataLenWords The size of pData (in 32-bit words).  
 *                             On return this is the number of words retrieved.
 * @return        SH_STATUS_SUCCESS or some failure code.
 */
int sh_getFrs(void *sh,
	      uint16_t recordId,
	      uint32_t *pData, uint16_t *dataLenWords);

/**
 * @brief Write/Update a Non-Volatile configuration record (FRS record).
 * 
 * A variety of FRS records are defined to store calibration and other
 * important data associated with the SensorHub.  This function will
 * store or update one of those records.
 *
 * On a system with flash-based FRS, this data is permanently stored in the
 * SensorHub.  Devices with RAM-based FRS storage will not keep this data
 * so key FRS records need to be re-written at startup using this function.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param      recordId Which record to access.
 * @param      pData    Pointer to data to be stored.
 * @param      dataLenWords Size of data to be stored.  (32-bit words).
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_setFrs(void *sh,
	      uint16_t recordId,
	      uint32_t *pData, uint16_t dataLenWords);

/**
 * @brief Get Product Ids.
 * 
 * Product Id structures identify which Hillcrest products comprise the
 * SensorHub and which revision levels those components are at.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param[out] prodIds  Array of product ids returned.
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_getProdIds(void *sh,
                  sh_ProductId_t prodId[SH_NUM_PRODUCT_IDS]);

/**
 * @brief Read the SensorHub error queue.
 * 
 * The SensorHub maintains an internal error event queue.  This API call retrieves 
 * the error information for diagnostic purposes.
 * 
 * @param         sh        The SensorHub reference obtained via sh_init().
 * @param         severity  All errors at this severity and higher are returned.  
 *                          Pass 0 to read all errors.
 * @param[out]    pErrors   Storage for returned errors.
 * @param[in,out] numErrors Size of pErrors array.  Number of errors retrieved on return.
 * @return        SH_STATUS_SUCCESS or some failure code.
 */
int sh_getErrors(void *sh,
                 uint8_t severity,
                 sh_ErrorRecord_t *pErrors, uint16_t *numErrors);

/**
 * @brief Read the SensorHub internal counters associated with a sensor.
 * 
 * For each sensor, there are four counters tracking the sensor's performance.  This 
 * API call can retrieve those values.  The four counters associated with each sensor are offered, 
 * accepted, on, and attempted.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param      sensorId Which sensor to operate on.
 * @param[out] pCounts  The counter values retrieved.
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_getCounts(void *sh,
                 sh_SensorId_t sensorId,
                 sh_Counts_t *pCounts);

/**
 * @brief Clear counters associated with a sensor.
 *
 * Clears the four counters measuring a sensor's performance.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param      sensorId Which sensor to operate on.
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_clearCounts(void *sh,
                   sh_SensorId_t sensorId);

/**
 * @brief Establish the current orientation as the orientation reference frame.
 * 
 * Orientation values are relative to some reference frame.  This operation
 * establishes the device's current orientation as the reference for later
 * readings.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param      axes     A bitmap specifying which axes should be affected.
 * @param      basis    Which of the available rotation vectors defines the
 *                      new reference frame.
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_tareNow(void *sh,
               uint8_t axes,          // bitmap of sh_TareAxis_t
               sh_TareBasis_t basis); 

/**
 * @brief Reset the orientation reference frame.
 * 
 * The reference frame for rotation values will revert to its power-up setting.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_tareClear(void *sh);

/**
 * @brief Save the orientation reference frame in Non-Volatile storage (FRS record).
 * 
 * The currently established rotation vector reference frame will be saved so it
 * remains in effect after reinitializing or power cycling the device.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_persistTare(void *sh);

/**
 * @brief Set the orientation reference frame explicitly.
 * 
 * The reference orientation is set via a provided orientation quaternion.
 * 
 * @param      sh          The SensorHub reference obtained via sh_init().
 * @param      orientation The new reference frame expressed as a quaternion.
 * @return     SH_STATUS_SUCCESS or some failure code.
 */
int sh_setReorientation(void *sh,
                        sh_Quaternion_t *orientation);

/**
 * @brief Reinitialize the SensorHub.
 * 
 * Reset the sensorhub as if it were just powered on.  For devices with
 * RAM-based FRS, it is necessary to write the required FRS records after
 * boot-up, then perform this operation to establish the SensorHub in
 * a properly calibrated running state. 
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_reinitialize(void *sh);

/**
 * @brief Immediately save dynamic calibration data to non-volatile storage.
 * 
 * Dynamic calibration data is produced periodically as the SensorHub operates
 * and is automatically stored periodically in non-volatile RAM.  This operation 
 * causes the data to be stored immediately.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_dcdSaveNow(void *sh);

// Flags for sensors field of sh_calConfig
#define SH_CAL_ACCEL (0x01)
#define SH_CAL_GYRO  (0x02)
#define SH_CAL_MAG   (0x04)

/**
 * @brief Modify dynamic calibration behavior.
 * 
 * Enable/disable particular sensors from storing their dynamic calibration parameters
 * in non-volatile storage.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param      sensors  Bit fields indicating which sensors should have their dynamic
 *                      calibration values stored in non-volatile memory.
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_calConfig(void *sh,
                 uint8_t sensors);

/**
 * @brief Perform operations related to Rotation Vector synchronization.
 * 
 * The production of rotation vector events can be synchronized with the outside
 * world in several ways.  The SYNC_NOW option synchronizes RV production with
 * this command.  SYNC_ENABLE and SYNC_DISABLE enable/disable RV production with
 * an external signal.
 * 
 * @param      sh       The SensorHub reference obtained via sh_init().
 * @param      rvSyncOp Which RV synchronization option to use.
 * @return              SH_STATUS_SUCCESS or some failure code.
 */
int sh_rvSync(void *sh,
              sh_RvSyncOp_t rvSyncOp);

#ifdef __cplusplus
}   // end of extern "C"
#endif

#endif
