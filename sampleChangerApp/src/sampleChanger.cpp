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
    createParam(P_errorsString, asynParamOctet, &P_errors);
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
    int function = pasynUser->reason;
    const char* functionName = "writeOctet";
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
    getParamName(function, &paramName);

    char error[100];
    current_errors[function] = "";

    if (function == P_recalc) 
    {
        converter c(m_dims);
        c.createLookup();

        current_errors[function] = c.errors();
        
        *nActual = strlen(value);
    }
    else if (function == P_set_slot)
    {
        std::string newRack = std::string(value);

        if (newRack == "") {
            current_errors[function] = "Sample changer not yet selected - you will be unable to select any positions until the sample changer is set.\nUse sample changer \"_ALL\" to enable the use of positions from all sample changers.";
        }
        else {
            converter c(m_dims);
            if (c.createLookup() == 0 && c.checkSlotExists(newRack)) {
                asynPrint(pasynUser, ASYN_TRACE_ERROR, "%s:%s: setting slot=%s \n", driverName, functionName, newRack.c_str());
                m_selectedSlot = newRack;
            }
            else {
                sprintf(error, "%s:%s: setting slot=%s not possible (does not exist). Keeping old rack (%s)\n", driverName, functionName, newRack.c_str(), m_selectedSlot.c_str());
                asynPrint(pasynUser, ASYN_TRACE_ERROR, error);
                current_errors[function] = error;
                status = asynError;
            }

            *nActual = strnlen(value, maxChars);
        }
    }
    else if (function == P_set_posn)
    {
        converter c(m_dims);
        c.createLookup();

        std::string posn = std::string(value);

        try {
            std::string slot = c.get_slot_for_position(posn);
            current_errors[function] = c.errors();
            setStringParam(P_slot_from_pos, slot);
        }
        catch (const std::out_of_range &e) {
            sprintf(error, "%s:%s: position not recognised: %s\n", driverName, functionName, posn.c_str());
            asynPrint(pasynUser, ASYN_TRACE_ERROR, error);
            current_errors[function] = error;
            status = asynError;
        }

        *nActual = strlen(value);
    }
    else
    {
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, value=%s, error=%s", 
                  driverName, functionName, status, function, paramName, value, "unknown parameter");
        status = asynError;
        *nActual = 0;
    }

    std::map<int, std::string>::iterator it;
    for (it = current_errors.begin(); it != current_errors.end(); ++it) {
        if (!it->second.empty()) {
            setStringParam(P_errors, it->second);
            break;
        }
        setStringParam(P_errors, "");
    }

    callParamCallbacks();
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s, value=%s\n", 
              driverName, functionName, function, paramName, value);
    return status;
}

asynStatus sampleChanger::readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int* eomReason)
{
    int function = pasynUser->reason;
    const char* functionName = "readOctet";
    asynStatus status = asynSuccess;
    
    if (function == P_get_slot) 
    {
        strncpy(value, m_selectedSlot.c_str(), maxChars);
        *nActual = std::min(m_selectedSlot.length(), maxChars);
    }
    else if (function == P_get_available_slots) 
    {
        converter c(m_dims);
        std::string result = c.get_available_slots();
        strncpy(value, result.c_str(), maxChars);
        *nActual = std::min(result.length(), maxChars);
    }
    else if (function = P_get_available_in_selected_slot)
    {
        converter c(m_dims);
        c.createLookup();
        std::string result = c.get_available_in_slot(m_selectedSlot);
        strncpy(value, result.c_str(), maxChars);
        *nActual = std::min(result.length(), maxChars);
    }
    else
    {
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

int sampleChangerConfigure(const char *portName, const char* fileName, int dims=2)
{
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

// EPICS iocsh shell commands 

static const iocshArg initArg0 = { "portName", iocshArgString};            ///< The name of the asyn driver port we will create
static const iocshArg initArg1 = { "fileName", iocshArgString};            ///< The name of the lookup file
static const iocshArg initArg2 = { "nDims", iocshArgInt};                ///< Number of dimensions of the sample changer

static const iocshArg * const initArgs[] = { &initArg0, &initArg1, & initArg2 };

static const iocshFuncDef initFuncDef = {"sampleChangerConfigure", sizeof(initArgs) / sizeof(iocshArg*), initArgs};

static void initCallFunc(const iocshArgBuf *args)
{
    sampleChangerConfigure(args[0].sval, args[1].sval, args[2].ival);
}

static void sampleChangerRegister(void)
{
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(sampleChangerRegister);

}

