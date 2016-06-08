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

#include "sh_util.h"

uint16_t read16(const uint8_t * buffer)
{
	return ((uint16_t) buffer[0]) | ((uint16_t) (buffer[1]) << 8);
}

void write16(uint8_t * buffer, uint16_t value)
{
	buffer[0] = (uint8_t) (value);
	buffer[1] = (uint8_t) (value >> 8);
}

uint32_t read32(const uint8_t * buffer)
{
	return
		((uint32_t) buffer[0]) |
		((uint32_t) (buffer[1]) << 8) |
		((uint32_t) (buffer[2]) << 16) |
		((uint32_t) (buffer[3]) << 24);
}

uint32_t read32be(const uint8_t * buffer)
{
	return
		((uint32_t) buffer[3]) |
		((uint32_t) (buffer[2]) << 8) |
		((uint32_t) (buffer[1]) << 16) | ((uint32_t) (buffer[0]) << 24);
}

void write32(uint8_t * buffer, uint32_t value)
{
	buffer[0] = (uint8_t) (value);
	buffer[1] = (uint8_t) (value >> 8);
	buffer[2] = (uint8_t) (value >> 16);
	buffer[3] = (uint8_t) (value >> 24);
}
