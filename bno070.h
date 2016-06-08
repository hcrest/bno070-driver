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

#ifndef DFUBNO070_H
#define DFUBNO070_H

#include "SensorHubDev.h"
#include "HcBin.h"

#ifdef cplusplus
extern "C" {
#endif

/**
 * @brief Perform Download Firmware Update operation on BNO070.
 * 
 * Resets the BNO070 and performs the DFU process sending the firmware represented
 * by the hcbin parameter to the device.
 *
 * @param unit      Which BNO070 device to operate on.
 * @param hcbin     An object representing the firmware to be downloaded.
 * @return           SH_STATUS_SUCCESS or an error code.
 */	
int bno070_performDfu(int unit, const HcBin_t *hcbin);

#ifdef cplusplus
};   // end of extern "C"
#endif

// end of include guard
#endif
