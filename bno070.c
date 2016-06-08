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

#include "bno070.h"
#include "HcBin.h"
#include "sh_types.h"

#include <string.h>

#define max(a, b) (((a) < (b)) ? (b) : (a))
#define min(a, b) (((a) < (b)) ? (a) : (b))

#define MAX_PACKET_LEN (64)  // actual packets will have 2 byte CRC in addition to up to 64 bytes data

static void write32be(uint8_t *buf, uint32_t value);
static int dfu_send(void * dev, uint8_t *packet, uint8_t len);
  
int bno070_performDfu(int unit, const HcBin_t *hcbin)
{
	uint8_t packet[MAX_PACKET_LEN + 2];   // max data len + 2 byte CRC
	uint32_t app_len = 0;
	uint32_t packet_len = MAX_PACKET_LEN;
	uint32_t offset = 0;
	int rc = 0;

        void * dev = shdev_init(unit);
        if (dev == 0) {
          // error!
          while (1);
        }
    
	// Prepare the HcBin object for reading
	hcbin->open();
	
	// Validity checks on HCBIN file
        const char * fwFormat = hcbin->getMeta("FW-Format");
	if ((fwFormat == 0) || (strcmp(fwFormat, "BNO_V1") != 0)) {
		// This firmware isn't for BNO070
		return SH_STATUS_INVALID_HCBIN;
	}

	// Get lengths
	app_len = hcbin->getAppLen();
	packet_len = hcbin->getPacketLen();
	if ((packet_len == 0) || (packet_len > MAX_PACKET_LEN)) {
		packet_len = MAX_PACKET_LEN;
	}

	// Reset MCU into DFU mode
	rc = shdev_reset_dfu(dev);
	if (rc != SH_STATUS_SUCCESS) { 
		return rc;
	}

	// Send size of application code
        write32be(packet, app_len);
	rc = dfu_send(dev, packet, 4);
	if (rc < 0) goto error;
	
	// Send packet size
        packet[0] = packet_len;
	rc = dfu_send(dev, packet, 1);
	if (rc < 0) goto error;
	
	// Send data in packets of <packet-size>
	for (offset = 0; offset < app_len; offset += packet_len) {
		uint32_t toSend = min(app_len - offset, packet_len);
		rc = hcbin->getAppData(packet, offset, toSend);
		if (rc < 0) goto error;

		rc = dfu_send(dev, packet, toSend);
		if (rc < 0) goto error;
	}

	// We are done with the hcbin object
	hcbin->close();

	// BNO should watchdog reset, wait for INTN to be asserted
	while (shdev_waitIntn(dev, SH_WAIT_FOREVER) != false);

	return rc;

 error:
	// DFU process failed.
	hcbin->close();
	return rc;
}

static void write32be(uint8_t *buf, uint32_t value)
{
	*buf++ = (value >> 24) & 0xFF;
	*buf++ = (value >> 16) & 0xFF;
	*buf++ = (value >> 8) & 0xFF;
	*buf++ = (value >> 0) & 0xFF;
}

static void append_crc(uint8_t *packet, uint8_t len)
{
  uint16_t crc;
  uint16_t x;

  // compute CRC of packet
  crc = 0xFFFF;
  for (int n = 0; n < len; n++) {
    x = (uint16_t)(packet[n]) << 8;
    for (int i = 0; i < 8; i++) {
      if ((crc ^ x) & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      }
      else {
        crc = crc << 1;
      }
      x <<= 1;
    }
  }

  // Append the CRC to packet
  packet[len] = (crc >> 8) & 0xFF;
  packet[len+1] = crc & 0xFF;
}

static int dfu_send(void * dev, uint8_t *packet, uint8_t len)
{
  uint8_t ack_resp;
  int rc = SH_STATUS_SUCCESS;
  
  // Append CRC to packet, then send to device.
  append_crc(packet, len);

  // Send the packet to the device
  rc = shdev_i2c(dev, packet, len+2, 0, 0);
  if (rc != SH_STATUS_SUCCESS) {
    rc = SH_STATUS_ERROR_I2C_IO;
    goto error;
  }

  // Get ack.
  rc = shdev_i2c(dev, 0, 0, &ack_resp, 1);
  if (rc != SH_STATUS_SUCCESS) {
    rc = SH_STATUS_ERROR_I2C_IO;
    goto error;
  }
  if (ack_resp != 's') {
    // Got NACK
    rc = SH_STATUS_NACK;
    goto error;
  }

  return SH_STATUS_SUCCESS;

 error:
  return rc;
}
