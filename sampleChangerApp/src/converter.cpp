#include <errlog.h>
#include <epicsString.h>
#include <shareLib.h>

#include "converter.h"

#define ALL_POSITIONS_NAME "_ALL"

converter::converter(int i = 2)
{
    /**
     * Constructor
     * @param i Number of dimensions
     */
    m_dims = i;
    loadDefRackDefs("RACKDEFS");
}

// alternative constructor, only for testing
converter::converter(int i, const std::vector<Rack>& racks, const std::vector<Slot>& slots)
{
    /** 
     * This constructor is only for testing.
     * @param i number of dimensions
     * @param racks map of rack names to rack definitions
     * @param slots map of slot names to slot data
     */
    m_dims = i;
    m_racks = racks;
    m_slots = slots;
}

void converter::printError(const char* format, ...) {
    char buff[256];
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(buff, sizeof(buff), format, argptr);
    va_end(argptr);
    errlogPrintf(buff);
    m_errors.push_back(buff);
}

std::vector<converter::SlotPositions>::iterator converter::find_in_positions(const std::string& slot)
{
    std::vector<SlotPositions>::iterator it =
        std::find_if(m_positions_for_each_slot.begin(), m_positions_for_each_slot.end(),
            [&](SlotPositions slpos) {return epicsStrCaseCmp(slpos.slotName.c_str(), slot.c_str()) == 0; });
    return it;
}

// const version of above
std::vector<converter::SlotPositions>::const_iterator converter::find_in_positions(const std::string& slot) const
{
    std::vector<SlotPositions>::const_iterator it =
        std::find_if(m_positions_for_each_slot.cbegin(), m_positions_for_each_slot.cend(),
            [&](SlotPositions slpos) {return epicsStrCaseCmp(slpos.slotName.c_str(), slot.c_str()) == 0; });
    return it;
}

std::vector<std::pair<std::string, std::string>>::iterator converter::find_in_slots(const std::string& name)
{
    std::vector<std::pair<std::string, std::string>>::iterator slot_it =
        std::find_if(m_slot_for_each_position.begin(), m_slot_for_each_position.end(),
            [&](std::pair<std::string, std::string> pair) {return epicsStrCaseCmp(pair.first.c_str(), name.c_str()) == 0; });
    return slot_it;
}

// const version of above
std::vector<std::pair<std::string, std::string>>::const_iterator converter::find_in_slots(const std::string& name) const
{
    std::vector<std::pair<std::string, std::string>>::const_iterator slot_it =
        std::find_if(m_slot_for_each_position.cbegin(), m_slot_for_each_position.cend(),
            [&](std::pair<std::string, std::string> pair) {return epicsStrCaseCmp(pair.first.c_str(), name.c_str()) == 0; });
    return slot_it;
}

void converter::loadDefRackDefs(const char* env_fname)
{
    /** 
     * This function loads the rack definitions from the environment variable
     * @param env_fname environment variable name for file name
     * @return void
     */
    const char* fname = getenv(env_fname);
    if (fname == NULL) {

        errlogPrintf("sampleChanger: Environment variable \"%s\" not set\n", env_fname);
        return;
    }
    loadRackDefs(fname);
}

// Read rack_definitions.xml
void converter::loadRackDefs(const char* fname)
{
    /** 
     * This function loads the rack definitions from the file
     * @param fname file name
     * @return void
     */
    TiXmlDocument doc(fname);
    if (!doc.LoadFile()) {
        printError("sampleChanger: Unable to open rack defs file \"%s\". Error on line %i: %s\n\n", fname, doc.ErrorRow(), doc.ErrorDesc());
        return;
    }

    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlHandle hRoot(0);

    pElem = hDoc.FirstChildElement().Element();
    if (!pElem) {
        printError("sampleChanger: Unable to parse rack defs file \"%s\"\n", fname);
        return;
    }

    // save this for later
    hRoot = TiXmlHandle(pElem);

    loadRackDefs(hRoot);
    loadSlotDefs(hRoot);
}

// Extract the definitions of the rack types from the xml
void converter::loadRackDefs(TiXmlHandle& hRoot)
{
    /** 
     * This function loads the rack definitions from the xml
     * @param hRoot root of the xml
     * @return void
     */
    m_racks.clear();

    for (TiXmlElement* pElem = hRoot.FirstChild("racks").FirstChild("rack").Element(); pElem; pElem = pElem->NextSiblingElement())
    {
        std::vector<Position> posns;
        std::string rackName = pElem->Attribute("name");
        for (TiXmlElement* pRack = pElem->FirstChildElement("position"); pRack; pRack = pRack->NextSiblingElement()) {
            Position posn;
            const char* attrib = pRack->Attribute("name");
            if (attrib != NULL) {
                posn.name = attrib;
            }
            else {
                printError("sampleChanger: rack has no name attribute \"%s\"\n", rackName.c_str());
            }

            if (pRack->QueryDoubleAttribute("x", &posn.x) != TIXML_SUCCESS) {
                printError("sampleChanger: unable to read x attribute \"%s\" \"%s\"\n", rackName.c_str(), posn.name.c_str());
            }
            if (m_dims > 1 && pRack->QueryDoubleAttribute("y", &posn.y) != TIXML_SUCCESS) {
                printError("sampleChanger: unable to read y attribute \"%s\" \"%s\"\n", rackName.c_str(), posn.name.c_str());
            }
            posns.push_back(posn);
        }
        m_racks.push_back(Rack(rackName, posns));
    }
}

// Extract the definitions of the slots from the xml
void converter::loadSlotDefs(TiXmlHandle& hRoot)
{
    /** 
     * This function loads the slot definitions from the xml
     * @param hRoot root of the xml
     * @return void
     */
    //printf("Loading slot defs\n");

    m_slots.clear();

    for (TiXmlElement* pElem = hRoot.FirstChild("slots").FirstChild("slot").Element(); pElem; pElem = pElem->NextSiblingElement())
    {

        Slot slot;
        std::string slotName = pElem->Attribute("name");
        slot.name = slotName;

        if (pElem->QueryDoubleAttribute("x", &slot.x) != TIXML_SUCCESS) {
            printError("sampleChanger: unable to read slot x attribute \"%s\"\n", slotName.c_str());
        }
        if (m_dims > 1 && pElem->QueryDoubleAttribute("y", &slot.y) != TIXML_SUCCESS) {
            printError("sampleChanger: unable to read slot y attribute \"%s\"\n", slotName.c_str());
        }

        m_slots.push_back(slot);
    }
}

// Load the slot details - ie the current setup in samplechanger.xml
void converter::loadSlotDetails(const char* fname)
{
    /** 
     * This function loads the slot details from the file
     * @param fname file name
     * @return void
     */
    TiXmlDocument doc(fname);
    if (!doc.LoadFile()) {
        printError("sampleChanger: Unable to open slot details file \"%s\"\n", fname);
        return;
    }

    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlHandle hRoot(0);

    pElem = hDoc.FirstChildElement().Element();
    if (!pElem) {
        printError("sampleChanger: Unable to parse slot details file \"%s\"\n", fname);
        return;
    }

    // save this for later
    hRoot = TiXmlHandle(pElem);

    loadSlotDetails(hRoot);
}

// Extract slot details from the xml
std::vector<converter::Slot> converter::loadSlotDetails(TiXmlHandle& hRoot)
{
    /** 
     * This function loads the slot details from the xml
     * @param hRoot root of the xml
     * @return m_slots map of slot data
     */
  
    for (TiXmlElement* pElem = hRoot.FirstChild("slot").Element(); pElem; pElem = pElem->NextSiblingElement())
    {
        std::string slotName = pElem->Attribute("name");

        std::vector<Slot>::iterator iter = std::find_if(m_slots.begin(), m_slots.end(), [&](Slot slt) { return epicsStrCaseCmp(slt.name.c_str(), slotName.c_str()) == 0; });
        if (iter == m_slots.end()) {
            printError("sampleChanger: Unknown slot '%s' in slot details\n", slotName.c_str());
        }
        else {
            iter->rackType = pElem->Attribute("rack_type");

            if (pElem->QueryStringAttribute("sample_suffix", &(iter->sampleSuffix)) == TIXML_NO_ATTRIBUTE) {
                iter->sampleSuffix = slotName;
            }

            if (pElem->QueryDoubleAttribute("xoff", &(iter->xoff)) != TIXML_SUCCESS) {
                printError("sampleChanger: unable to read slot xoff attribute \"%s\"\n", slotName.c_str());
            }
            if (m_dims > 1 && pElem->QueryDoubleAttribute("yoff", &(iter->yoff)) != TIXML_SUCCESS) {
                printError("sampleChanger: unable to read slot yoff attribute \"%s\"\n", slotName.c_str());
            }
        }
    }
    return m_slots;
}

// Create the lookup file
int converter::createLookup()
{
   /** 
     * This function creates the lookup file from the slot details 
     * @return 0 on success, -1 on error
     */
    const char* fnameIn = getenv("SLOT_DETAILS_FILE");
    if (fnameIn == NULL) {
        printError("Environment variable SLOT_DETAILS_FILE not set\n");
        return 1;
    }
    loadSlotDetails(fnameIn);

    const char* fnameOut = getenv("SAMPLE_LKUP_FILE");
    if (fnameOut == NULL) {
        printError("Environment variable SAMPLE_LKUP_FILE not set\n");
        return 1;
    }
    FILE* fpOut = fopen(fnameOut, "w");
    if (fpOut == NULL) {
        printError("Unable to open %s\n", fnameOut);
        return 1;
    }

    int success = createLookup(fpOut);

    fclose(fpOut);

    return success;
}

bool converter::checkSlotExists(const std::string& slotName) const {
    /** 
     * This function checks if a slot exists
     * @param slotName name of the slot
     * @return true if the slot exists, false otherwise
     */
    try {
        std::vector<SlotPositions>::const_iterator it = find_in_positions(slotName);
        if (it == m_positions_for_each_slot.cend())
        {
            throw std::out_of_range("slot doesnt exist");
        }
        return true;
    }
    catch (const std::out_of_range& e) {
        return false;
    }
}

std::string converter::get_available_slots() const
{
    /** 
     * This function returns the available slots
     * @return string of available slots
     */
    std::string res;
    for (std::vector<Slot>::const_iterator it = m_slots.cbegin(); it != m_slots.cend(); it++) {
        res += it->name;
        res += " ";
    }
    res += ALL_POSITIONS_NAME;
    res += " END";
    return res;
}

std::string converter::get_available_in_slot(std::string slot)
{
    /** 
     * This function returns the available positions in a slot
     * @param slot name of the slot
     * @return string of available positions in the slot
     */
    std::string res;
    std::list<std::string> positions;
    try {
        std::vector<SlotPositions>::const_iterator it = find_in_positions(slot);
        if (it == m_positions_for_each_slot.cend())
        {
            throw std::out_of_range("not available in slot");
        }
        positions = it->positions;
    }
    catch (const std::out_of_range& e) {
        printError("Slot '%s' unknown\n", slot.c_str());
    }
    for (std::list<std::string>::const_iterator it = positions.cbegin(); it != positions.cend(); it++) {
        res += *it + " ";
    }
    res += "END";
    return res;
}

std::string converter::get_slot_for_position(const std::string& position) const
{
      /** 
     * This function returns the slot for a position in the list of positions for each slot
     * @param position name of the position
     * @return string of the slot
     */
    if (find_in_slots(position) == m_slot_for_each_position.cend())
    {
        throw std::out_of_range("\nSlot not found at given position");
    }
    return find_in_slots(position)->second;
}

// Write to the lookup file
int converter::createLookup(FILE* fpOut)
{
    /** 
     * This function creates the lookup file from the slot details
     * @param fpOut file pointer
     * @return 0 on success, 1 on error
     */
    errlogPrintf("sampleChanger: writing motionsetpoints lookup file\n");
    int motionsetpoint_defs_written = 0;
    m_positions_for_each_slot.clear();
    m_slot_for_each_position.clear();

    fprintf(fpOut, "# Convert sample position names to motor coordinates\n");
    fprintf(fpOut, "# WARNING: Generated file - Do not edit\n");
    fprintf(fpOut, "# Instead edit samplechanger.xml and press recalc\n");

    for (std::vector<Slot>::const_iterator it = m_slots.cbegin(); it != m_slots.cend(); it++) {
        // Check if the slot is in the list of slots
        const Slot& slot = *it;
        std::vector<Rack>::const_iterator iter = std::find_if(m_racks.cbegin(), m_racks.cend(),
            [&](Rack r) {return epicsStrCaseCmp(r.name.c_str(), slot.rackType.c_str()) == 0; });
        if (iter == m_racks.cend()) {
            // This is not a known rack type
            printError("sampleChanger: Unknown rack type '%s' of slot %s\n", slot.rackType.c_str(), slot.name.c_str());
            return 1;
        }
        else {
            for (std::vector<Position>::const_iterator it2 = iter->positions.cbegin(); it2 != iter->positions.cend(); it2++) {
                std::string full_position_name = it2->name + slot.sampleSuffix;

                std::vector<SlotPositions>::const_iterator pos_it = find_in_positions(ALL_POSITIONS_NAME);
                if (pos_it == m_positions_for_each_slot.cend())
                {
                    m_positions_for_each_slot.push_back(SlotPositions(ALL_POSITIONS_NAME, std::list<std::string>()));
                }
                find_in_positions(ALL_POSITIONS_NAME)->positions.push_back(full_position_name);

                std::vector<SlotPositions>::const_iterator pos_it2 = find_in_positions(slot.name);
                if (pos_it2 == m_positions_for_each_slot.end())
                {
                    m_positions_for_each_slot.push_back(SlotPositions(slot.name, std::list<std::string>()));
                }
                find_in_positions(slot.name)->positions.push_back(full_position_name);

                std::vector<std::pair<std::string, std::string>>::iterator slot_it = find_in_slots(full_position_name);
                if (slot_it == m_slot_for_each_position.end())
                {
                    m_slot_for_each_position.push_back(std::make_pair(full_position_name, slot.name));
                }
                else
                {
                    slot_it->second = slot.name;
                }

                // print logs
                if (m_dims == 1) {
                    fprintf(fpOut, "%s %f\n", full_position_name.c_str(), it2->x + slot.x + slot.xoff);
                }
                else {
                    fprintf(fpOut, "%s %f %f\n", full_position_name.c_str(), it2->x + slot.x + slot.xoff, it2->y + slot.y + slot.yoff);
                }
            }
        }
        motionsetpoint_defs_written = 1;
    }


    if (!motionsetpoint_defs_written) {
        // No slots defined
        printError("sampleChanger: no data written\n");
        return 1;
    }

    return 0;
}
