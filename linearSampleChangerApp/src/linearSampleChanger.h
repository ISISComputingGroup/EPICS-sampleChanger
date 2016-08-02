#ifndef LINEARSAMPLECHANGER_H
#define LINEARSAMPLECHANGER_H
 
#include <string>

#include "asynPortDriver.h"
#include "../../sampleChangerApp/src/converter.h"

class linearSampleChanger : public asynPortDriver 
{
public:
    linearSampleChanger(const char* portName, const char* fileName);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
	virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
                 
private:
	double m_outval;
    std::string m_fileName;

	int P_recalc; // string
	int P_outval; // string
#define FIRST_MSP_PARAM P_recalc
#define LAST_MSP_PARAM P_outval    
};

#define NUM_MSP_PARAMS (&LAST_MSP_PARAM - &FIRST_MSP_PARAM + 1)

#define P_recalcString		"RECALC"
#define P_outvalString	"OUTVAL"

#endif /* LINEARSAMPLECHANGER_H */
