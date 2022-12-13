#ifndef SAMPLECHANGER_H
#define SAMPLECHANGER_H
 
#include <string>

#include "asynPortDriver.h"
#include "converter.h"

class sampleChanger : public asynPortDriver 
{
    /** 
     * @class sampleChanger
     * @brief This class implements the sample changer.
     * The sample changer is contstructed using galils that can be used to change the sample.
     * returns the current sample position.
     */

// These are the methods we override from asynPortDriver
public:
    /** 
     * Constructor for the sample changer class.
     * Calls constructor for the asynPortDriver base class.
     */
    sampleChanger(const char* portName, const char* fileName, int dims);
    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
    virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int* eomReason);
                 
private:
    /** 
     * Values used for pasynUser->reason, and indexes into the parameter library.
     * These are used by writeInt32, writeFloat64, writeString and writeOctet.\
     */
    std::string m_fileName;
    std::string m_selectedSlot;
    int32_t m_dims;
    std::map <int32_t, std::string> current_errors;

    int P_recalc; // string
    int P_errors; // string
    int P_set_slot; // string
    int P_get_slot; // string
    int P_slot_from_pos; // string
    int P_set_posn; // string
    int P_get_available_slots; // string
    int P_get_available_in_selected_slot; // string
#define FIRST_MSP_PARAM P_recalc
#define LAST_MSP_PARAM P_get_available_in_selected_slot    
};

#define NUM_MSP_PARAMS (&LAST_MSP_PARAM - &FIRST_MSP_PARAM + 1)

#define P_recalcString        "RECALC"
#define P_errorsString    "ERRORS"
#define P_set_slotString    "SET_SLOT"
#define P_get_slotString    "GET_SLOT"
#define P_slot_from_posString    "SLOT_FROM_POS"
#define P_set_posnString    "SET_POSN"
#define P_get_available_slotsString    "AVAILABLE_SLOTS"
#define P_get_available_in_selected_slotsString    "AVAILABLE_IN_SLOT"


#endif /* SAMPLECHANGER_H */
