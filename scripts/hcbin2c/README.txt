The hcbin2c.py script reads a firmware file in the Hillcrest .hcbin format
and writes a .c file containing an HcBin_t object.

The resulting .c file may be compiled into a host software system,
where the HcBin_t object can be passed to the bno070_performDfu()
function to perform the Download Firmware Update process.

Usage:

python hcbin2c.py <infile> <outfile>

<infile> should be a file in .hcbin format.
<outfile> will contain the C code.

For example:

python hcbin2c.py 1000-3251_1.8.4.415.hcbin Firmware.c

This will read the file 1000-3251_1.8.4.415.hcbin, which contains the
firmware for version 1.8.4 of the BNO070, and produces the file
Firmware.c.

Firmware.c contains the definition of a variable named
bno070_firmware.  This can be compiled in to an application and used
in a statement such as this:

        printf("Starting DFU process.\n");
        rc = bno070_performDfu(0, &bno070_firmware);
        if (rc == 0) {
          printf("DFU Succeeded.\n");
        }

