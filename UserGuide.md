# Hillcrest SH-1 SensorHub Driver for MCU Applications

## Introduction

The SH-1 MCU Driver is a library that provides high-level access to
the Hillcrest SH-1 Sensorhub.  This library provides functions that
initialize, configure and manage the SensorHub device.  And, of
course, the library supports reading sensor data produced by the
SensorHub.
Before the SH-1 library can work, however, the system developer needs
to supply certain functions that the library needs to access the i2c
bus and GPIO signals of the target.

This document describes how to incorporate the SensorHub driver into
an application.  It covers setting up a project to include the
library, providing target-specific functions needed by this library
and using the library API from the application's code.

----------------------------------------
## SH-1 SensorHub API

The main API of the SH-1 library is contained in the file SensorHub.h.
The operations defined here fall into 4 main categories: Initializing
the device, Configuring sensors, Reading sensors and Managing the
device.

### API Conventions

The SensorHub API follows a simple naming convention for consistency.
There are also conventions on how parameters are passed into the API
and values are returned from it.  The following sections address
these, briefly.

#### Naming Conventions

The following naming conventions are used to make code easier to read.

All SensorHub API functions and types are prefixed with sh_.

Function names use camel case with the initial letter in lower case.  (e.g. sh_setSensorConfig()).

Type names use camel case but with an initial letter in upper case.  They also end with _t.  (e.g. sh_SensorConfig_t).

#### SensorHub references.

When the sh_init() function is called, it returns a (void *) value that
refers to the SensorHub.  The application should treat this as an
opaque pointer, and should not de-reference it to access the data it
points to.

This pointer is passed in to later API calls as the first parameter.
Through this value, the SensorHub library keeps track of the private
data associated with a SensorHub.

This pointer should never be zero when passed to the API functions.
Nor should it be any value other than what was returned from
sh_init().

#### Return Values

All SensorHub API functions (except sh_init()) return a status code
indicating whether an operation was successful.  0 indicates success.
Any negative value indicates an error.  The enumeration sh_Status_e
lists all the values used.

In cases where an API function needs to return some other data, there
is generally a pointer parameter allowing the caller to provide memory
where the API can store data on return.

#### Memory Allocation

The SensorHub library does not perform any memory allocation
operations.  All memory used internally by the library is statically
allocated at compile time.  

Several API functions require the caller to provide empty buffers to
hold new data.  The application is responsible for allocating this
memory and freeing it later.  The library will not access these
buffers between API calls.

### API Functions

Each function of the SH-1 API is described briefly below.  For complete details
for a particular function, follow the links to the reference material.

#### Device Initialization

* sh_init()

The sh_init() function should be the first API call made by the application.
It resets the sensorhub device and establishes communications with it.

#### Configuring Sensors

* sh_setSensorConfig()
* sh_getSensorConfig()

The sh_setSensorConfig() function is used to enable and disable sensors.
It sets the desired event rate and a few other attributes that control data
production.

The sh_getSensorConfig() function retrieves the actual configuration
of a particular sensor.  The configuration as read can differ from the
configuration that was written.  For example, if a device only
supports a limited set of data rates, the actual rate may differ from
the requested rate.

#### Reading Sensors

* sh_getEvent()

The sh_getEvent() function will read the next available sensor event,
if any.  Each event contains one sample from one sensor.  The data is
returned via the sh_SensorEvent_t structure, which provides fields to
identify which sensor is represented and what the specific data is.
Sensor values often have multiple components representing vectors,
quaternions, accuracy, etc.

#### Managing the SensorHub

  * sh_getMetadata()
  * sh_getFrs()
  * sh_setFrs()
  * sh_getProdIds()
  * sh_getCounts()
  * sh_clearCounts()
  * sh_tareNow()
  * sh_tareClear()
  * sh_persistTare()
  * sh_setReorientation()
  * sh_reinitialize()
  * sh_dcdSaveNow()
  * sh_calConfig()
  * sh_rvSync()

A variety of utility functions provide control over many facets of the
SensorHub's operation.  Some of these functions read and write FRS
records (Non-volatile data, usually stored in Flash memory on the
device.)  Others provide access to version information, internal
counters, etc.  The tare operations modify the reference frame used
for reporting rotation vectors.

----------------------------------------
## SHDEV Interface: Hardware Adaptation Layer

The SH-1 Library needs to perform some low level operations to communicate with the SensorHub device.  Since these operations are target-specific, they must be provided by the system designer.  The SH-1 library defines a low-level API, shdev, that specifies the necessary functions.  These can be found in the file SensorHubDev.h.  The functions in this API provide for device initialization, i2c communications and control of the digital i/o signals of the SensorHub.

### Device Initialization

* shdev_init()

This function is called from sh_init() during system initialization.  It should allocate whatever internal resources are needed within the device interface.  It returns a void * that will be used in the SH-1 library as a handle to the low level device.  Each of the other shdev API calls takes this pointer as the first parameter.  (This is analogous to a file id or file pointer).

### I2C Communications

* shdev_i2c()

This function provides i2c read and write capability to the SH-1
Library.  The parameters to this function describe a transmit buffer
and a receive buffer.  Either one both may be set to a non-zero size,
providing three different communications scenarios.  If only the
transmit buffer is valid, the function should perform an i2c write to
the SensorHub.  If only the receive buffer is valid, the function
should perform a read.  If both are valid, the function needs to
perform the write operation first and then the read with an i2c
repeated start condition.  The repeated start is necessary for proper
communication with the SH-1 device.

### Digital I/O Control

* shdev_reset()
* shdev_reset_dfu()
* shdev_waitIntn()

These functions all read or write certain digital i/o signals.  The
shdev_reset() and shdev_reset_dfu() functions assert the reset line to
the device, then deassert it with the BOOT signal either high (for a
normal reset) or low (for a reset to DFU.)  

The shdev_waitIntn() function polls the INTN signal from the SH-1
device.  It may return immediately or wait until the signal reaches a
desired state, depending on how it is called.

----------------------------------------
## Example Project

To further illustrate how the SH-1 API can be used, an example project
has been developed.  The example runs on an STM32F401 Nucleo board with
a BNO070 Development board providing the SensorHub functionality.

### Requirements

To build run this example application, the following hardware and
software components are required:
  * Hillcrest BNO070 Development Board
  * STM32F401 Nucleo Board
  * IAR Embedded Workbench for ARM

### Project Structure

The Example project can be opened by double-clicking on the
Project.eww file in the sh1/sh1-example-nucleo/EWARM folder.

This project has been set up to target the STM32F401 processor of the
Nucleo evaluation board.  It uses BSP support from the ST Embedded Software, the
STM32Cube HAL library and FreeRTOS.

Hillcrest folder contains 
  * sensor_app.c with the example application code.
  * sh_bno_stm32f401.c with hardware access functions for SH-1 (the "shdev" routines).
  * Firmware.c with DFU functions for SH-1.
  * console.c with VCOM USB interface on Nucleo boards.

The FreeRTOS folder under Middlewares contains the OS for this example app.

The Driver folder contains hardware access functions supplied by ST in their STM32Cube library.

The Src folder contains the main.c file as well as some startup and platform configuration files for the STM32F4xx processor.

Most of the code in this project, for example the startup code, HAL
and FreeRTOS support, would be found in any typical embedded systems
application.  The Hillcrest\sensor_app.c file and Hillcrest\sh_bno_stm32f401.c
files are the ones with code specific to the SH-1 SensorHub.  So we
should examine them in more detail.

### SensorHub Task

The sensor_app.c file contains the SensorHub functionality in this example.  It consists of a single task, sensorTask, that initializes the SensorHub then services it.

The following code initializes the SensorHub.  It calls sh_init, then
uses the sensorHub reference to read the product ids and start the
flow of sensor events.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~c
void sensorTask(void)
{
	// Initialize stuff
	...
	void *pSensorHub = 0;
      
	// Get reference to sensorhub (unit 0)
	pSensorHub = sh_init(0);
  
	// Read out product id
	reportProdIds(pSensorHub);
    
	// Enable reports from Rotation Vector.
	startReports(pSensorHub);

	...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The reportProdIds function uses the sh_getProdIds() API function to
read out the version information from the SensorHub.  It then prints
that to the console so the user can see it.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~c
void reportProdIds(void *pSensorHub)
{
	sh_ProductId_t prodId[SH_NUM_PRODUCT_IDS];

	int status = sh_getProdIds(pSensorHub, prodId);
	if (status < 0) {
		printf("Error.  Could not get product ids.\n");
	}
	else {
		for (int n = 0; n < SH_NUM_PRODUCT_IDS; n++) {
			printf("Part %d : Version %d.%d.%d Build %d\n",
			       prodId[n].swPartNumber,
			       prodId[n].swVersionMajor, prodId[n].swVersionMinor, 
			       prodId[n].swVersionPatch, prodId[n].swBuildNumber);
		}
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then the startReports function runs to start the flow of sensor
events.  This function uses sh_setSensorConfig to apply a new
configuration to the SH_ROTATION_VECTOR sensor.  The configuration has
the report interval set to 10000 microseconds for a rate of 100 Hz.
Change sensitivity is disabled -- we want to see all the reports not
just those that represent a change.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~c
void startReports(void *pSensorHub)
{
	sh_SensorConfig_t config;
	int status;

	config.changeSensitivityEnabled = false;
	config.wakeupEnabled = false;
	config.changeSensitivityRelative = false;
	config.changeSensitivity = 0;
	config.reportInterval_us = 10000;  // microseconds (100Hz)
	config.reserved1 = 0;

	status = sh_setSensorConfig(pSensorHub, SH_ROTATION_VECTOR, &config);
        if (status != SH_STATUS_SUCCESS) {
          printf("Error while enabling RotationVector sensor: %d\n", status);
        }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once the sensor is enabled, the sensor task enters a loop to read the
generated events.  The example code prints ROTATION VECTOR output report to
serial port to show the user some of the data being produced.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~c
	while (1) {
		// Get an event from the sensorhub
		if (sh_eventReady(pSensorHub)) {
			rc = sh_getEvent(pSensorHub, &event);
			if (rc == SH_STATUS_SUCCESS) {
				reports++;   
			    printEvent(&event);
		        }
            }
	}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


### SensorHub HAL (shdev interface)

The hardware access function supporting the SH-1 API are implemented in sh_bno_stm32f401.c
 

shdev_init() is the system initialization function.  It performs the following actions:
  * Create I2C mutex and semaphore
  * Validity checks the unit number passed in.
  * Initializes the GPIO pins and I2C peripheral to be used with the BNO070 chip.
  * Sets the dfuMode flag for this device to false.
  * Returns a pointer to the device state.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~c
void * shdev_init(int unit)
{
	if (!shdev_first_init_done) {
		shdev_first_init_done = true;

		// Create i2c mutex and semaphore
		bno_i2cOperationDone = xSemaphoreCreateBinary();
		bno_i2cMutex = xSemaphoreCreateMutex();

		for (int n = 0; n < MAX_SH_UNITS; n++) {
			bno_dev[n].intnSem = xSemaphoreCreateBinary();
		}
	}

	// Validate unit
	if ((unit < 0) || (unit >= MAX_SH_UNITS)) {
		// no such unit
		return 0;
	}
  
	bno_t *pDev = &bno_dev[unit];

    pDev->dfuMode = false;
 	pDev->intnState = pDev->getIntN();
	
 	return pDev;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

shdev_reset() performs the reset handshake on the BNO070 and sets the dfuMode flag to false.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~c
sh_Status_t shdev_reset(void * dev)
{
	bno_t *pDev = (bno_t *)dev;
        
        pDev->dfuMode = false;
    
	// Assert reset
	pDev->setRstN(false);
    
	// Boot into sensorhub, not bootloader
	pDev->setBootN(true);

	pDev->intnState = true;
	xSemaphoreGive(pDev->intnSem);
    
	// Wait 10ms
	vTaskDelay(10); 
    
	// Take BNO out of reset
	pDev->setRstN(true);

	return SH_STATUS_SUCCESS;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Similarly, shdev_reset_dfu() performs the reset handshake with BOOTN low so that the BNO enters the bootloader.  The dfuMode flag is set to true in this case so future operations know to use the bootloader's i2c address.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~c
sh_Status_t shdev_reset_dfu(void * dev)
{
	bno_t *pDev = (bno_t *)dev;
    
        pDev->dfuMode = true;
        
	// Assert reset
	pDev->setRstN(false);
    
	// Boot into bootlaoder, not sensorhub application
	pDev->setBootN(false);

	pDev->intnState = true;
	xSemaphoreGive(pDev->intnSem);
    
	// Wait 10ms
	vTaskDelay(10); 
    
	// Take BNO out of reset
	pDev->setRstN(true);

	// Wait until bootloader is ready.
	vTaskDelay(200); 
    
	return SH_STATUS_SUCCESS;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

shdev_i2c() is implemented using the STM32Cube HAL functions, HAL_I2C_Master_Transmit_IT and HAL_I2C_Master_Receive_IT.  In order to perform the required write/read operation with repeated start, used HAL_I2C_Master_Sequential_Transmit_IT and HAL_I2C_Master_Sequential_Receive_IT, based on ST's example code:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~c
sh_Status_t shdev_i2c(void *pDev,
                      const uint8_t *pSend, unsigned sendLen,
                      uint8_t *pReceive, unsigned receiveLen)
{
	int rc;
	bno_t * pBno = (bno_t *)pDev;
	uint16_t i2cAddr = 0;
        
	if ((sendLen == 0) && (receiveLen == 0)) {
		// Nothing to send, skip the whole thing
		return SH_STATUS_SUCCESS;
	}
	
	/* Determine which I2C address to use, based on unit and DFU mode */
	if (pBno->unit == 0) {
		if (!pBno->dfuMode) {
			i2cAddr = BNO_I2C_0 << 1;
		}
		else {
			i2cAddr = BNO_DFU_I2C_0 << 1;
		}
	}
	else {
		if (!pBno->dfuMode) {
			i2cAddr = BNO_I2C_1 << 1;
		}
		else {
			i2cAddr = BNO_DFU_I2C_1 << 1;
		}
	}
  
	// Acquire i2c mutex
	xSemaphoreTake(bno_i2cMutex, portMAX_DELAY);
	bno_i2cStatus = SH_STATUS_SUCCESS;
	
	if ((sendLen != 0) && (receiveLen != 0)) {
		// Perform write, then read with repeated start
		// Acquire i2c mutex
		rc = HAL_I2C_Master_Sequential_Transmit_IT(hi2c, i2cAddr,
		                                           (uint8_t *)pSend, sendLen,
		                                           I2C_FIRST_FRAME);
		
		// Transfer portion started, wait until it finishes.
		xSemaphoreTake(bno_i2cOperationDone, portMAX_DELAY);

		// Finish with receive portion.
		rc = HAL_I2C_Master_Sequential_Receive_IT(hi2c, i2cAddr,
		                                          pReceive, receiveLen,
			                                      I2C_LAST_FRAME);
	}
	else if (sendLen != 0) {
		// Perform write only
		rc = HAL_I2C_Master_Transmit_IT(hi2c, i2cAddr,
		                             (uint8_t *)pSend, sendLen);
	}
	else {
		// Perform read only
		rc = HAL_I2C_Master_Receive_IT(hi2c, i2cAddr,
		                            pReceive, receiveLen);
	}

	if (rc == HAL_OK) {
		// Operation started,
		
		// Change intn to deasserted state
		pBno->intnState = true;
		xSemaphoreGive(pBno->intnSem);
		
		// wait until operation finishes.
		xSemaphoreTake(bno_i2cOperationDone, portMAX_DELAY);

		// Use i2c operation status now for rc
		rc = bno_i2cStatus;
	}
		
	// Release i2c mutex
	xSemaphoreGive(bno_i2cMutex);
		
	if (rc != HAL_OK) {
		return SH_STATUS_ERROR_I2C_IO;
	}
  
	return SH_STATUS_SUCCESS;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Finally, we have the function shdev_waitIntn().  A binary Semaphore is used to service data from BNO070. Interrupt callback function will perform xTaskGetTickCountFromISR(), and shdev_waitIntn() will process the interrupt.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~c
bool shdev_waitIntn(void *dev, bool desired, uint16_t wait_ms, uint32_t *timestamp)
{
	bno_t *pDev = (bno_t *)dev;
	bool actual = pDev->intnState;

	// Convert wait_ms to proper FreeRTOS timeout value
	TickType_t semWait = (wait_ms == SH_WAIT_FOREVER) ? portMAX_DELAY : wait_ms * portTICK_PERIOD_MS;

	while ((actual != desired) && (wait_ms != 0)) {
		// Wait for intn to change state, then check again
		
		xSemaphoreTake(pDev->intnSem, semWait);
		actual = pDev->intnState;
	}

	if (timestamp) {
		*timestamp = intn0_timestamp * 1000;
	}

	return actual;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



