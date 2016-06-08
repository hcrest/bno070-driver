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


#ifndef HCBIN_H
#define HCBIN_H

#include <stdint.h>

// Abstract Data Type for HcBin objects.
// HcBin objects represent Hillcrest Binary files.  This interface
// definition is intended to represent such files in a way that
// supports compression and/or streaming data via a serial interface.

typedef struct HcBin_s {
	int (*open)(void);
	int (*close)(void);
	const char * (*getMeta)(const char * key);
	uint32_t (*getAppLen)(void);
	uint32_t (*getPacketLen)(void);
	int (*getAppData)(uint8_t *packet, uint32_t offset, uint32_t len);
} HcBin_t;

#endif
