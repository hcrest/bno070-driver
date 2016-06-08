# HcBin file conversion utility
#
# Copyright 2015-16 Hillcrest Laboratories, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License and 
# any applicable agreements you may have with Hillcrest Laboratories, Inc.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

header = """
/*
 * Copyright 2015-16 Hillcrest Laboratories, Inc.
 *
 * All rights reserved.
 *
 */

#ifndef BNO070_FIRMWARE_H
#define BNO070_FIRMWARE_H

#include "HcBin.h"

extern const HcBin_t bno070_firmware;

#endif
"""

part1 = """
/*
 * Copyright 2015-16 Hillcrest Laboratories, Inc.
 *
 * All rights reserved.
 *
 */

#include "HcBin.h"

#include <string.h>

#define ARRAY_LEN(a) ((sizeof(a))/(sizeof(a[0])))

/* Forward declarations of private functions */
static int hcbin_open(void);
static int hcbin_close(void);
static const char * hcbin_getMeta(const char * key);
static uint32_t hcbin_getAppLen(void);
static uint32_t hcbin_getPacketLen(void);
static int hcbin_getAppData(uint8_t *packet, uint32_t offet, uint32_t len);

/* hcbin object to be used by DFU code */
const HcBin_t bno070_firmware = {
	hcbin_open,
	hcbin_close,
	hcbin_getMeta,
	hcbin_getAppLen,
	hcbin_getPacketLen,
	hcbin_getAppData
};

/* ------------------------------------------------------------------------ */
/* Private data */

struct HcbinMetadata {
	const char * key;
	const char * value;
};
"""

part2 = """
/* ------------------------------------------------------------------------ */
/* Private functions */

static int hcbin_open(void)
{
	/* Nothing to do */
	return 0;
}

static int hcbin_close(void)
{
	/* Nothing to do */
	return 0;
}

static const char * hcbin_getMeta(const char * key)
{
	for (int i = 0; i < ARRAY_LEN(hcbinMetadata); i++) {
		if (strcmp(key, hcbinMetadata[i].key) == 0) {
			/* Found key, return value */
			return hcbinMetadata[i].value;
		}
	}

	/* Not found */
	return 0;
}

static uint32_t hcbin_getAppLen(void)
{
	return ARRAY_LEN(hcbinFirmware);
}

static uint32_t hcbin_getPacketLen(void)
{
	/* This implementation doesn't have a preferred packet len */
	return 0;
}

static int hcbin_getAppData(uint8_t *packet, uint32_t offset, uint32_t len)
{
	int index = offset;
	int copied = 0;

        if ((offset+len) > ARRAY_LEN(hcbinFirmware)) {
                /* requested data beyond the end */
                return -1;
        }

	while ((copied < len) && (index < ARRAY_LEN(hcbinFirmware))) {
		packet[copied++] = hcbinFirmware[index++];
	}

	return 0;
}
"""

class HcBinGen:
    MAX_COL = 16

    def __init__(self):
        self.hcbin = None

    def setData(self, hcbin):
        self.hcbin = hcbin

    def write(self, c_filename, h_filename):
        f = open(h_filename, "w")
        f.write(header)
        f.close()
        
        f = open(c_filename, "w")
        f.write(part1)
        self.writeMetadata(f)
        self.writeFirmware(f)
        f.write(part2)
        f.close()


    def writeMetadata(self, f):
        f.write("static const struct HcbinMetadata hcbinMetadata[] = {\n")
        for entry in self.hcbin.getMetadata():
            f.write('    {"%s", "%s"},\n' % (entry[0], entry[1]))
        f.write("};\n\n")

    def writeFirmware(self, f):
        f.write("static const uint8_t hcbinFirmware[] = {\n")
        col = 0
        for x in self.hcbin.getFirmware():
            if col == 0:
                # indent
                f.write("    ");
            f.write("0x%02x," % (x,))

            # space or newline following
            col += 1
            if (col == HcBinGen.MAX_COL):
                f.write("\n")
                col = 0
            else:
                f.write(" ")

        # Add final newline, if necessary
        if (col != 0):
            f.write("\n")

        f.write("};\n\n")


