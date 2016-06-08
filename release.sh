#!/bin/bash

# Run doxygen to produce documementation
doxygen

# Copy README file into sh1 dir so user can find it.
cp README ../

# Now go above sh1 and combine everything into a zip file
cd ../..
zip -r -9 -@ sh1.zip sh1/README sh1/sh1-example-nucleo sh1/sh1-mcu-driver/docs sh1/sh1-mcu-driver/scripts -x "sh1/sh1-example-nucleo/.git/*" "*.gitignore" < sh1/sh1-mcu-driver/release-files
