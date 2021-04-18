#ifndef SAMPLECHANGER_H
#define SAMPLECHANGER_H
 
#include <string>

#include "asynPortDriver.h"
#include "converter.h"

class sampleChanger : public asynPortDriver 
{
public:
    sampleChanger(const char* portName, const char* fileName, int dims);
    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
    virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int* eomReason);
                 
private:
    std::string m_fileName;
    std::string m_selectedSlot;
    int m_dims;

    int P_recalc; // string
    int P_set_slot; // string
    int P_get_slot; // string
    int P_get_available_slots; // string
    int P_get_available_in_selected_slot; // string
#define FIRST_MSP_PARAM P_recalc
#define LAST_MSP_PARAM P_get_available_in_selected_slot    
};

#define NUM_MSP_PARAMS (&LAST_MSP_PARAM - &FIRST_MSP_PARAM + 1)

#define P_recalcString        "RECALC"
#define P_set_slotString    "SET_SLOT"
#define P_get_slotString    "GET_SLOT"
#define P_get_available_slotsString    "AVAILABLE_SLOTS"
#define P_get_available_in_selected_slotsString    "AVAILABLE_IN_SLOT"

#endif /* SAMPLECHANGER_H */
