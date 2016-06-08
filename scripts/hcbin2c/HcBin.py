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

from __future__ import print_function

import struct
import re
import zlib
import array

class HcBin:
    ID = 0x6572d028
    FF_VER = 4
    INIT_CRC = 0
    
    def __init__(self):
        self.stuff = None
        self.metadata = []
        self.firmware = []
        self.crc32 = HcBin.INIT_CRC

    def read(self, infile):
        self.filename = infile

        self.crc32 = HcBin.INIT_CRC
        self.f = open(infile, "rb")

        # Read and check header fields
        self.id = self.read32be()
        if (self.id != HcBin.ID):
            print("Invalid Id field in %s.  Expected 0x%08x, got 0x%08x." % (infile, HcBin.ID, self.id))
            self.f.close()
            return
        
        self.sz = self.read32be()
        self.ff_ver = self.read32be()
        if (self.ff_ver != HcBin.FF_VER):
            print("Unknown file format version.  Expected 0x%08x, got 0x%08x." % (HcBin.FF_VER, ff_ver))
            self.f.close()
            return
            
        self.payload_offset = self.read32be()
        print("Payload offset: %d" % (self.payload_offset,))

        # Read metadata
        self.readMetadata()

        # Read firmware
        self.readFirmware()

        # Read CRC32 and check it.
        computed_crc32 = self.crc32 & 0xFFFFFFFF
        stored_crc32 = self.read32be()
        if (stored_crc32 != computed_crc32):
            # TODO-DW : flag error
            print("CRC error.")
        
        self.f.close()

    def getMetadata(self):
        return self.metadata

    def getFirmware(self):
        return self.firmware

    def updateCrc(self, s):
        self.crc32 = zlib.crc32(s, self.crc32)

    def read32be(self):
        # Read 4 bytes, then unpack as big endian into unsigned int
        data = self.f.read(4)

        x = struct.unpack('>I', data)[0]

        # Update running CRC32
        self.updateCrc(data)
        
        return x

    def readMetadata(self):
        toRead = self.payload_offset - self.f.tell()
        s = self.f.read(toRead)
        
        # Update running CRC32
        self.updateCrc(s)

        # Split section into lines
        metaRe = re.compile("([^:]*): ([^W]*)")
        lines = re.split("[\r\n\0]+", s)
        for line in lines:
            m = metaRe.match(line)
            if m:
                self.metadata.append( (m.group(1), m.group(2)) )

        print("Metadata:", self.metadata)
        
    def readFirmware(self):
        # Payload size: total size - payload offset - 4 bytes for footer (CRC)
        toRead = self.sz - self.payload_offset - 4
        s = self.f.read(toRead)

        # Update running CRC32
        self.updateCrc(s)

        # Store the firmware
        self.firmware = array.array('B', s)
        return
    
