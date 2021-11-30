#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <exception>
#include <iostream>
#include <algorithm>

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <iocsh.h>

#include "sampleChanger.h"

#include <macLib.h>
#include <epicsGuard.h>

#include <epicsExport.h>

static const char *driverName = "sampleChanger";

// Create a new instance of the sampleChanger class
sampleChanger::sampleChanger(const char *portName, const char* fileName, int dims=2)
   : asynPortDriver(portName, 
                    0, /* maxAddr */ 
                    NUM_MSP_PARAMS, /* num parameters */
                    asynInt32Mask | asynFloat64Mask | asynOctetMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask | asynOctetMask,  /* Interrupt mask */
                    ASYN_CANBLOCK, /* asynFlags.  This driver can block but it is not multi-device */
                    1, /* Autoconnect */
                    0,
                    0), m_fileName(fileName), m_selectedSlot("")
{
    createParam(P_recalcString, asynParamOctet, &P_recalc);  
    createParam(P_set_slotString, asynParamOctet, &P_set_slot);  
    createParam(P_get_slotString, asynParamOctet, &P_get_slot);  
    createParam(P_slot_from_posString, asynParamOctet, &P_slot_from_pos);
    createParam(P_set_posnString, asynParamOctet, &P_set_posn);
    createParam(P_get_available_slotsString, asynParamOctet, &P_get_available_slots);  
    createParam(P_get_available_in_selected_slotsString, asynParamOctet, &P_get_available_in_selected_slot);
    
    // set dims. Default should be 2. Only 1 and 2 are currently supported
    m_dims = dims;
    if ( m_dims!=1 ) m_dims = 2;
}

asynStatus sampleChanger::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual)
{
    /**
    * This function is called when asyn clients call pasynOctet->write().
    * For all parameters it sets the value in the parameter library and calls any registered callbacks..
    * @param[in] pasynUser pasynUser structure that encodes the reason and address.
    * @param[in] value Address of the string to write.
    * @param[in] maxChars Maximum number of characters to write.
    * @param[out] nActual Number of characters actually written.
    * @return asynSuccess if the value was successfully written.
    */

    int function = pasynUser->reason; // Function to call
    const char* functionName = "writeOctet"; 
    asynStatus status = asynSuccess; // Return status
    const char *paramName = NULL;
    getParamName(function, &paramName); // Get the name of the parameter

    if (function == P_recalc) 
    {
        converter c(m_dims);
        c.createLookup();
        
        *nActual = strlen(value); // return the number of characters actually written
    }
    else if (function == P_set_slot)
    {
        // set the selected slot
        std::string newRack = std::string(value);
        
        converter c(m_dims);
        // check if the slot is valid
        if (c.createLookup() == 0 && c.checkSlotExists(newRack)) {
            asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s:%s: setting slot=%s \n", driverName, functionName, newRack.c_str());
            m_selectedSlot = newRack;
        } else {
            asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s:%s: setting slot=%s not possible (does not exist). Keeping old rack (%s)\n", driverName, functionName, newRack.c_str(), m_selectedSlot.c_str());
            status = asynError;
        }
        
        *nActual = strnlen(value, maxChars); // return the number of characters actually written
    }
    else if (function == P_set_posn)
    {
        converter c(m_dims);
        c.createLookup();

        std::string posn = std::string(value);

        // check if posn is valid
        try {
            std::string slot = c.get_slot_for_position(posn);
            setStringParam(P_slot_from_pos, slot); // set the slot
        }
        catch (const std::out_of_range &e) {
            // position not found
            asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s:%s: position not recognised: %s\n", driverName, functionName, posn.c_str());
            status = asynError;
        }

        *nActual = strlen(value); // return the number of characters written
    }
    else
    {
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, value=%s, error=%s", 
                  driverName, functionName, status, function, paramName, value, "unknown parameter");
        status = asynError;
        *nActual = 0;
    }
    callParamCallbacks(); // call the callbacks to update the values
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s, value=%s\n", 
              driverName, functionName, function, paramName, value);
    return status;
}

asynStatus sampleChanger::readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int* eomReason)
{
    /**
     * This function is called when asyn clients call pasynOctet->read().
     * It reads the value of the parameter and returns it to the client.
     * @param[in] pasynUser pasynUser structure that encodes the reason and address.
     * @param[out] value Address of the string to read.
     * @param[in] maxChars Maximum number of characters to read.
     * @param[out] nActual Number of characters actually read.
     * @param[out] eomReason End-of-string reason.
     * @return asynSuccess if the value was successfully read.
     * @return asynError if the value could not be read.
     * @return asynTimeout if the value was not yet available.
     */

    int function = pasynUser->reason;
    const char* functionName = "readOctet";
    asynStatus status = asynSuccess;
    
    if (function == P_get_slot) 
    {
        // get the selected slot
        strncpy(value, m_selectedSlot.c_str(), maxChars);
        *nActual = std::min(m_selectedSlot.length(), maxChars);
    }
    else if (function == P_get_available_slots) 
    {
        // get the available slots
        converter c(m_dims);
        std::string result = c.get_available_slots();
        strncpy(value, result.c_str(), maxChars);
        *nActual = std::min(result.length(), maxChars);
    }
    else if (function = P_get_available_in_selected_slot)
    {
        // get the available positions in the selected slot
        converter c(m_dims);
        c.createLookup();
        std::string result = c.get_available_in_slot(m_selectedSlot);
        strncpy(value, result.c_str(), maxChars);
        *nActual = std::min(result.length(), maxChars);
    }
    else
    {
        // unknown parameter
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, value=%s, error=%s", 
                  driverName, functionName, status, function, value, "unknown parameter");
        status = asynError;
        *nActual = 0;
    }
    callParamCallbacks();
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, value=%s\n", 
              driverName, functionName, function, value);
    return status;
}


extern "C" {
    // This is the function that is called by the IOC shell to create a new instance of the driver

int sampleChangerConfigure(const char *portName, const char* fileName, int dims=2)
{
    /** 
     * Creates a new instance of the sampleChanger driver.
     * @param[in] portName The name of the asyn port driver to be created.
     * @param[in] fileName The name of the file containing the lookup table.
     * @param[in] dims The number of dimensions of the sample.
     * @return 0 if successful, otherwise -1.
     */

    try
    {
        new sampleChanger(portName, fileName, dims);
        return(asynSuccess);
    }
    catch(const std::exception& ex)
    {
        std::cerr << "sampleChanger failed: " << ex.what() << std::endl;
        return(asynError);
    }
}

// EPICS iocsh shell commands to call the asynSetup and asynDisconnect functions.
static const iocshArg initArg0 = { "portName", iocshArgString};            ///< The name of the asyn driver port we will create
static const iocshArg initArg1 = { "fileName", iocshArgString};            ///< The name of the lookup file
static const iocshArg initArg2 = { "nDims", iocshArgInt};                ///< Number of dimensions of the sample changer

static const iocshArg * const initArgs[] = { &initArg0, &initArg1, & initArg2 }; 

static const iocshFuncDef initFuncDef = {"sampleChangerConfigure", sizeof(initArgs) / sizeof(iocshArg*), initArgs};

static void initCallFunc(const iocshArgBuf *args)
{
    /**
     * This function is called from the iocsh shell.
     * It creates a new instance of the sampleChanger driver.
     * @param[in] args The arguments passed to the shell.
     * @return void
     */
    sampleChangerConfigure(args[0].sval, args[1].sval, args[2].ival);
}

static void sampleChangerRegister(void)
{
    /** 
     * This function registers the sampleChanger driver with the IOC shell.
     * @return void
     */
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(sampleChangerRegister);

}

