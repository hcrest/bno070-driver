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
 * @file SensorHubDev.h 
 * @author David Wheeler
 * @date 22 May 2015
 * @brief API Definition for Hillcrest SH-1 Device support.
 *
 * The SensorHubDev API provides platform-specific functions needed by 
 * the SensorHub device driver.  The system integrator is required to provide
 * these functions so the SensorHub device driver can perform platform-specific
 * operations.
 */

#ifndef SENSORHUB_DEV_H
#define SENSORHUB_DEV_H

#include "SensorHub.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Defaults to supporting one sensorhub.
#ifndef MAX_SH_UNITS
#define MAX_SH_UNITS (1)
#endif

/**
 * Initialize (open) a sensorhub device.
 * 
 * The platform-specific code should perform all device initialization required
 * to establish proper SH-1 operation and communication.  It can return an
 * opaque pointer that will be passed to other device API calls by the client
 * code.
 *
 * @param  unit    Which of several Sensor Hubs should be initialized.
 * @return         Opaque pointer to use as a reference to this device.
 */
void * shdev_init(int unit);

/**
 * Reset the sensorhub.
 *
 * This function should assert the RESET line of the sensorhub then release it,
 * being careful to set the BOOTN GPIO line to cause the device to boot into
 * application mode (as opposed to DFU mode.)
 *
 * @param  pDev    The device reference obtained via shdev_init().
 * @return         SH_STATUS_SUCCESS or error code.
 */
sh_Status_t shdev_reset(void * pDev);

/**
 * Reset the sensorhub into DFU mode.
 *
 * This function should assert the RESET line of the sensorhub then release it,
 * being careful to clear the BOOTN GPIO line to cause the device to boot into
 * DFU mode (not application mode.)
 *
 * @param  pDev    The device reference obtained via shdev_init().
 * @return         SH_STATUS_SUCCESS or error code.
 */
sh_Status_t shdev_reset_dfu(void * pDev);

/**
 * Perform an I2C transaction with the device.
 *
 * Performs an i2c read, write or both (with repeated START) on a particular
 * device.  The sendLen and receiveLen params determine whether the operation
 * should be a read, write or read/write.
 * 
 * @param       pDev       The device reference obtained via shdev_init().
 * @param       pSend      Pointer to data to send (or NULL)
 * @param       sendLen    Number of bytes to send (or 0 for write operation)
 * @param[out]  pReceive   Pointer to buffer receiving data (or NULL)
 * @param       receiveLen Size of pReceive buffer (0 for read operations)
 * @return      SH_STATUS_SUCCESS or error code.
 */
sh_Status_t shdev_i2c(void *pDev,
                      const uint8_t *pSend, unsigned sendLen,
                      uint8_t *pReceive, unsigned receiveLen);

/**
 * Read the current state of INTN signal.
 *
 * @param  pDev    The device reference obtained via shdev_init().
 * @return         Actual state of INTN signal.
 */
bool shdev_getIntn(void *pDev);
	
/**
 * Block until SensorHub's INTN line is asserted.
 * (May return after a timeout, even if INTN is not in desired state.)
 *
 * If the target cannot provide timing support, this function should 
 * query INTN and return immediately if wait_ms is 0.  If wait_ms is
 * non-zero, it should busy-wait until INTN is asserted.
 *
 * @param  pDev    The device reference obtained via shdev_init().
 * @param  wait_ms Time to wait for desired state, ms.  (0 = no wait, SH_WAIT_FOREVER = no timeout)
 * @return         Actual state of INTN.  (false if interrupt was asserted.)
 */
bool shdev_waitIntn(void *pDev, uint16_t wait_ms);

/**
 * Get timestamp associated with last time Sensor Hub asserted interrupt.
 *
 * @param  pDev    The device reference obtained via shdev_init().
 * @param  timestamp The timestamp taken in ISR will be returned via this ptr.
 * @return         The timestamp (in units of microseconds) of last interrupt assertion.
 */
uint32_t shdev_getTimestamp_us(void *pDev);
	
#ifdef __cplusplus
}    // end of extern "C"
#endif

#endif
