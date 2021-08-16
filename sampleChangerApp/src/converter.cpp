#include <errlog.h>
#include <shareLib.h>

#include "converter.h"

#define ALL_POSITIONS_NAME "_ALL"

converter::converter(int i = 2)
{
    m_dims = i;
    loadDefRackDefs("RACKDEFS");
}

// alternative constructor, only for testing
converter::converter(int i, const std::vector<Rack>& racks, const std::vector<Slot>& slots)
{
    m_dims = i;
    v_racks = racks;
    v_slots = slots;
}

std::vector<converter::SlotPositions>::iterator converter::find_in_positions(const std::string& slot)
{
    std::vector<SlotPositions>::iterator it =
        std::find_if(v_positions_for_each_slot.begin(), v_positions_for_each_slot.end(),
            [&](SlotPositions slpos) {return slpos.slotName == slot; });
    return it;
}

// const version of above
std::vector<converter::SlotPositions>::const_iterator converter::find_in_positions(const std::string& slot) const
{
    std::vector<SlotPositions>::const_iterator it =
        std::find_if(v_positions_for_each_slot.cbegin(), v_positions_for_each_slot.cend(),
            [&](SlotPositions slpos) {return slpos.slotName == slot; });
    return it;
}

std::vector<std::pair<std::string, std::string>>::iterator converter::find_in_slots(const std::string& name)
{
    std::vector<std::pair<std::string, std::string>>::iterator slot_it =
        std::find_if(v_slot_for_each_position.begin(), v_slot_for_each_position.end(),
            [&](std::pair<std::string, std::string> pair) {return pair.first == name; });
    return slot_it;
}

// const version of above
std::vector<std::pair<std::string, std::string>>::const_iterator converter::find_in_slots(const std::string& name) const
{
    std::vector<std::pair<std::string, std::string>>::const_iterator slot_it =
        std::find_if(v_slot_for_each_position.cbegin(), v_slot_for_each_position.cend(),
            [&](std::pair<std::string, std::string> pair) {return pair.first == name; });
    return slot_it;
}

void converter::loadDefRackDefs(const char* env_fname)
{
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
    TiXmlDocument doc(fname);
    if (!doc.LoadFile()) {
        errlogPrintf("sampleChanger: Unable to open rack defs file \"%s\"\n", fname);
        return;
    }

    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlHandle hRoot(0);

    pElem = hDoc.FirstChildElement().Element();
    if (!pElem) {
        errlogPrintf("sampleChanger: Unable to parse rack defs file \"%s\"\n", fname);
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
    v_racks.clear();

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
                errlogPrintf("sampleChanger: rack has no name attribute \"%s\"\n", rackName.c_str());
            }

            if (pRack->QueryDoubleAttribute("x", &posn.x) != TIXML_SUCCESS) {
                errlogPrintf("sampleChanger: unable to read x attribute \"%s\" \"%s\"\n", rackName.c_str(), posn.name.c_str());
            }
            if (m_dims > 1 && pRack->QueryDoubleAttribute("y", &posn.y) != TIXML_SUCCESS) {
                errlogPrintf("sampleChanger: unable to read y attribute \"%s\" \"%s\"\n", rackName.c_str(), posn.name.c_str());
            }
            posns.push_back(posn);
        }
        v_racks.push_back(Rack(rackName, posns));
    }
}

// Extract the definitions of the slots from the xml
void converter::loadSlotDefs(TiXmlHandle& hRoot)
{
    v_slots.clear();

    for (TiXmlElement* pElem = hRoot.FirstChild("slots").FirstChild("slot").Element(); pElem; pElem = pElem->NextSiblingElement())
    {

        Slot slt;
        std::string slotName = pElem->Attribute("name");
        slt.name = slotName;

        if (pElem->QueryDoubleAttribute("x", &slt.x) != TIXML_SUCCESS) {
            errlogPrintf("sampleChanger: unable to read slot x attribute \"%s\"\n", slotName.c_str());
        }
        if (m_dims > 1 && pElem->QueryDoubleAttribute("y", &slt.y) != TIXML_SUCCESS) {
            errlogPrintf("sampleChanger: unable to read slot y attribute \"%s\"\n", slotName.c_str());
        }

        v_slots.push_back(slt);
    }
}

// Load the slot details - ie the current setup in samplechanger.xml
void converter::loadSlotDetails(const char* fname)
{
    TiXmlDocument doc(fname);
    if (!doc.LoadFile()) {
        errlogPrintf("sampleChanger: Unable to open slot details file \"%s\"\n", fname);
        return;
    }

    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlHandle hRoot(0);

    pElem = hDoc.FirstChildElement().Element();
    if (!pElem) {
        errlogPrintf("sampleChanger: Unable to parse slot details file \"%s\"\n", fname);
        return;
    }

    // save this for later
    hRoot = TiXmlHandle(pElem);

    loadSlotDetails(hRoot);
}

// Extract slot details from the xml
std::vector<converter::Slot> converter::loadSlotDetails(TiXmlHandle& hRoot)
{
    for (TiXmlElement* pElem = hRoot.FirstChild("slot").Element(); pElem; pElem = pElem->NextSiblingElement())
    {
        std::string slotName = pElem->Attribute("name");

        std::vector<Slot>::iterator iter = std::find_if(v_slots.begin(), v_slots.end(), [&](Slot slt) { return slt.name == slotName; });
        if (iter == v_slots.end()) {
            errlogPrintf("sampleChanger: Unknown slot '%s' in slot details\n", slotName.c_str());
        }
        else {
            iter->rackType = pElem->Attribute("rack_type");

            if (pElem->QueryStringAttribute("sample_suffix", &(iter->sampleSuffix)) == TIXML_NO_ATTRIBUTE) {
                iter->sampleSuffix = slotName;
            }

            if (pElem->QueryDoubleAttribute("xoff", &(iter->xoff)) != TIXML_SUCCESS) {
                errlogPrintf("sampleChanger: unable to read slot xoff attribute \"%s\"\n", slotName.c_str());
            }
            if (m_dims > 1 && pElem->QueryDoubleAttribute("yoff", &(iter->yoff)) != TIXML_SUCCESS) {
                errlogPrintf("sampleChanger: unable to read slot yoff attribute \"%s\"\n", slotName.c_str());
            }
        }
    }
    return v_slots;
}

// Create the lookup file
int converter::createLookup()
{
    const char* fnameIn = getenv("SLOT_DETAILS_FILE");
    if (fnameIn == NULL) {
        errlogPrintf("Environment variable SLOT_DETAILS_FILE not set\n");
        return 1;
    }
    loadSlotDetails(fnameIn);

    const char* fnameOut = getenv("SAMPLE_LKUP_FILE");
    if (fnameOut == NULL) {
        errlogPrintf("Environment variable SAMPLE_LKUP_FILE not set\n");
        return 1;
    }
    FILE* fpOut = fopen(fnameOut, "w");
    if (fpOut == NULL) {
        errlogPrintf("Unable to open %s\n", fnameOut);
        return 1;
    }

    int success = createLookup(fpOut);

    fclose(fpOut);

    return success;
}

bool converter::checkSlotExists(const std::string& slotName) const {
    try {
        std::vector<SlotPositions>::const_iterator it = find_in_positions(slotName);
        if (it == v_positions_for_each_slot.cend())
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
    std::string res;
    for (std::vector<Slot>::const_iterator it = v_slots.cbegin(); it != v_slots.cend(); it++) {
        res += it->name;
        res += " ";
    }
    res += ALL_POSITIONS_NAME;
    res += " END";
    return res;
}

std::string converter::get_available_in_slot(std::string slot)
{
    std::string res;
    std::list<std::string> positions;
    try {
        std::vector<SlotPositions>::const_iterator it = find_in_positions(slot);
        if (it == v_positions_for_each_slot.cend())
        {
            throw std::out_of_range("not available in slot");
        }
        positions = it->positions;
    }
    catch (const std::out_of_range& e) {
        errlogPrintf("Slot '%s' unknown, returning all positions\n", slot.c_str());
        slot = ALL_POSITIONS_NAME;
    }
    for (std::list<std::string>::const_iterator it = positions.cbegin(); it != positions.cend(); it++) {
        res += *it + " ";
    }
    res += "END";
    return res;
}

std::string converter::get_slot_for_position(const std::string& position) const
{
    if (find_in_slots(position) == v_slot_for_each_position.cend())
    {
        throw std::out_of_range("\nSlot not found at given position");
    }
    return find_in_slots(position)->second;
}

// Write to the lookup file
int converter::createLookup(FILE* fpOut)
{
    errlogPrintf("sampleChanger: writing motionsetpoints lookup file\n");
    int motionsetpoint_defs_written = 0;
    v_positions_for_each_slot.clear();
    v_slot_for_each_position.clear();

    fprintf(fpOut, "# Convert sample position names to motor coordinates\n");
    fprintf(fpOut, "# WARNING: Generated file - Do not edit\n");
    fprintf(fpOut, "# Instead edit samplechanger.xml and press recalc\n");

    for (std::vector<Slot>::const_iterator it = v_slots.cbegin(); it != v_slots.cend(); it++) {

        const Slot& slot = *it;
        std::vector<Rack>::const_iterator iter = std::find_if(v_racks.cbegin(), v_racks.cend(),
            [&](Rack r) {return r.name == slot.rackType; });
        if (iter == v_racks.cend()) {
            errlogPrintf("sampleChanger: Unknown rack type '%s' of slot %s\n", slot.rackType.c_str(), slot.name.c_str());
            return 1;
        }
        else {
            for (std::vector<Position>::const_iterator it2 = iter->positions.cbegin(); it2 != iter->positions.cend(); it2++) {
                std::string full_position_name = it2->name + slot.sampleSuffix;

                std::vector<SlotPositions>::const_iterator pos_it = find_in_positions(ALL_POSITIONS_NAME);
                if (pos_it == v_positions_for_each_slot.cend())
                {
                    v_positions_for_each_slot.push_back(SlotPositions(ALL_POSITIONS_NAME, std::list<std::string>()));
                }
                find_in_positions(ALL_POSITIONS_NAME)->positions.push_back(full_position_name);

                std::vector<SlotPositions>::const_iterator pos_it2 = find_in_positions(slot.name);
                if (pos_it2 == v_positions_for_each_slot.end())
                {
                    v_positions_for_each_slot.push_back(SlotPositions(slot.name, std::list<std::string>()));
                }
                find_in_positions(slot.name)->positions.push_back(full_position_name);

                std::vector<std::pair<std::string, std::string>>::iterator slot_it = find_in_slots(full_position_name);
                if (slot_it == v_slot_for_each_position.end())
                {
                    v_slot_for_each_position.push_back(std::make_pair(full_position_name, slot.name));
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
        errlogPrintf("sampleChanger: no data written\n");
        return 1;
    }

    return 0;
}
