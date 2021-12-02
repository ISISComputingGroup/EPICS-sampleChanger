#include <errlog.h>
#include <shareLib.h>

#include "converter.h"

#define ALL_POSITIONS_NAME "_ALL"

converter::converter(int i=2)
{
    /**
     * Constructor
     * @param i Number of dimensions
     */
    m_dims = i;
    loadDefRackDefs("RACKDEFS");
} 

// alternative constructor, only for testing
converter::converter(int i, std::map<std::string, std::map<std::string, samplePosn> > racks, std::map<std::string, slotData> slots)
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

void converter::loadDefRackDefs(const char* env_fname) 
{
    /** 
     * This function loads the rack definitions from the environment variable
     * @param env_fname environment variable name for file name
     * @return void
     */
    const char *fname = getenv(env_fname);
    if ( fname==NULL ) {
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
        errlogPrintf("sampleChanger: Unable to open rack defs file \"%s\"\n", fname);
        return;
    }

    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlHandle hRoot(0);

    pElem=hDoc.FirstChildElement().Element();
    if (!pElem) {
        errlogPrintf("sampleChanger: Unable to parse rack defs file \"%s\"\n", fname);
        return;
    }

    // save this for later
    hRoot=TiXmlHandle(pElem);

    loadRackDefs(hRoot);
    loadSlotDefs(hRoot);
}

// Extract the definitions of the rack types from the xml
void converter::loadRackDefs(TiXmlHandle &hRoot)
{
    /** 
     * This function loads the rack definitions from the xml
     * @param hRoot root of the xml
     * @return void
     */
    m_racks.clear();
    
    for( TiXmlElement* pElem=hRoot.FirstChild("racks").FirstChild("rack").Element(); pElem; pElem=pElem->NextSiblingElement())
    {
        std::map<std::string, samplePosn> posns;
        std::string rackName = pElem->Attribute("name");
        for ( TiXmlElement *pRack = pElem->FirstChildElement("position") ; pRack ; pRack=pRack->NextSiblingElement() ) {
            samplePosn posn;
            const char *attrib = pRack->Attribute("name");
            if ( attrib!=NULL ) {
                posn.name = attrib;
            }
            else {
                errlogPrintf("sampleChanger: rack has no name attribute \"%s\"\n", rackName.c_str());
            }

            if ( pRack->QueryDoubleAttribute("x", &posn.x)!=TIXML_SUCCESS ) {
                errlogPrintf("sampleChanger: unable to read x attribute \"%s\" \"%s\"\n", rackName.c_str(), posn.name.c_str());
            }
            if ( m_dims>1 && pRack->QueryDoubleAttribute("y", &posn.y)!=TIXML_SUCCESS ) {
                errlogPrintf("sampleChanger: unable to read y attribute \"%s\" \"%s\"\n", rackName.c_str(), posn.name.c_str());
            }
            posns[posn.name] = posn;
        }
        m_racks[rackName] = posns;
    }
}

// Extract the definitions of the slots from the xml
void converter::loadSlotDefs(TiXmlHandle &hRoot)
{
    /** 
     * This function loads the slot definitions from the xml
     * @param hRoot root of the xml
     * @return void
     */
    //printf("Loading slot defs\n");
    m_slots.clear();
    
    for( TiXmlElement* pElem=hRoot.FirstChild("slots").FirstChild("slot").Element(); pElem; pElem=pElem->NextSiblingElement())
    {
        
        slotData slot;
        std::string slotName = pElem->Attribute("name");
        slot.name = slotName;
        
        if ( pElem->QueryDoubleAttribute("x", &slot.x)!=TIXML_SUCCESS ) {
            errlogPrintf("sampleChanger: unable to read slot x attribute \"%s\"\n", slotName.c_str());
        }
        if ( m_dims>1 && pElem->QueryDoubleAttribute("y", &slot.y)!=TIXML_SUCCESS ) {
            errlogPrintf("sampleChanger: unable to read slot y attribute \"%s\"\n", slotName.c_str());
        }
        
        m_slots[slotName] = slot;
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
        errlogPrintf("sampleChanger: Unable to open slot details file \"%s\"\n", fname);
        return;
    }

    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlHandle hRoot(0);

    pElem=hDoc.FirstChildElement().Element();
    if (!pElem) {
        errlogPrintf("sampleChanger: Unable to parse slot details file \"%s\"\n", fname);
        return;
    }

    // save this for later
    hRoot=TiXmlHandle(pElem);

    loadSlotDetails(hRoot);
}

// Extract slot details from the xml
std::map<std::string, slotData> converter::loadSlotDetails(TiXmlHandle &hRoot)
{
    /** 
     * This function loads the slot details from the xml
     * @param hRoot root of the xml
     * @return m_slots map of slot data
     */
    for( TiXmlElement* pElem=hRoot.FirstChild("slot").Element(); pElem; pElem=pElem->NextSiblingElement())
    {
        std::string slotName = pElem->Attribute("name");
        
        std::map<std::string, slotData>::iterator iter = m_slots.find(slotName);
        if ( iter==m_slots.end() ) {
            errlogPrintf("sampleChanger: Unknown slot '%s' in slot details\n", slotName.c_str());
        }
        else {
            iter->second.rackType = pElem->Attribute("rack_type");

            if (pElem->QueryStringAttribute("sample_suffix", &(iter->second.sampleSuffix)) == TIXML_NO_ATTRIBUTE){
                iter->second.sampleSuffix = slotName;
            }

            if ( pElem->QueryDoubleAttribute("xoff", &(iter->second.xoff))!=TIXML_SUCCESS ) {
                errlogPrintf("sampleChanger: unable to read slot xoff attribute \"%s\"\n", slotName.c_str());
            }
            if ( m_dims>1 && pElem->QueryDoubleAttribute("yoff", &(iter->second.yoff))!=TIXML_SUCCESS ) {
                errlogPrintf("sampleChanger: unable to read slot yoff attribute \"%s\"\n", slotName.c_str());
            }
        }
    }
    return m_slots;
}

int converter::createLookup() 
{
    /** 
     * This function creates the lookup file from the slot details 
     * @return 0 on success, -1 on error
     */
    const char *fnameIn = getenv("SLOT_DETAILS_FILE");
    if ( fnameIn==NULL ) {
        errlogPrintf("Environment variable SLOT_DETAILS_FILE not set\n");
        return 1;
    }
    loadSlotDetails(fnameIn);

    const char *fnameOut = getenv("SAMPLE_LKUP_FILE");
    if ( fnameOut==NULL ) {
        errlogPrintf("Environment variable SAMPLE_LKUP_FILE not set\n");
        return 1;
    }
    FILE *fpOut = fopen(fnameOut, "w");
    if ( fpOut==NULL ) {
        errlogPrintf("Unable to open %s\n", fnameOut);
        return 1;
    }
    
    int success = createLookup(fpOut);
    
    fclose(fpOut);
    
    return success;
}

bool converter::checkSlotExists(std::string slotName) {
    /** 
     * This function checks if a slot exists
     * @param slotName name of the slot
     * @return true if the slot exists, false otherwise
     */
    try {
        m_positions_for_each_slot.at(slotName);
        return true;
    }
    catch (const std::out_of_range &e) {
        return false;
    }
}

std::string converter::get_available_slots() 
{
    /** 
     * This function returns the available slots
     * @return string of available slots
     */
    std::string res;
    for ( std::map<std::string, slotData>::iterator it=m_slots.begin() ; it!=m_slots.end() ; it++ ) {
        res += it->first;
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
        positions = m_positions_for_each_slot.at(slot);
    } catch (const std::out_of_range &e) {
        errlogPrintf("Slot '%s' unknown, returning all positions\n", slot.c_str());
        slot = ALL_POSITIONS_NAME;
    }
    for (std::list<std::string>::iterator it = positions.begin(); it != positions.end(); it++) {
        res += *it + " ";
    }
    res += "END";
    return res;
}

std::string converter::get_slot_for_position(std::string position) {
    /** 
     * This function returns the slot for a position in the list of positions for each slot
     * @param position name of the position
     * @return string of the slot
     */
    return m_slot_for_each_position.at(position);
}

int converter::createLookup(FILE *fpOut) 
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

    for ( std::map<std::string, slotData>::iterator it=m_slots.begin() ; it!=m_slots.end() ; it++ ) {
        // Check if the slot is in the list of slots
        
        slotData &slot = it->second;
        std::map<std::string, std::map<std::string, samplePosn> >::iterator iter = m_racks.find(slot.rackType);
        if ( iter==m_racks.end() ) {
            // This is not a known rack type
            errlogPrintf("sampleChanger: Unknown rack type '%s' of slot %s\n", slot.rackType.c_str(), slot.name.c_str());
            return 1;
        }
        else {
            for ( std::map<std::string, samplePosn>::iterator it2 = iter->second.begin() ; it2!=iter->second.end() ; it2++ ) {
                std::string full_position_name = it2->second.name + slot.sampleSuffix;
                m_positions_for_each_slot[ALL_POSITIONS_NAME].push_back(full_position_name);
                m_positions_for_each_slot[slot.name].push_back(full_position_name);
                m_slot_for_each_position[full_position_name] = slot.name;

                if ( m_dims==1 ) {
                    fprintf(fpOut, "%s %f\n", full_position_name.c_str(), it2->second.x+slot.x+slot.xoff);                    
                }
                else {
                    fprintf(fpOut, "%s %f %f\n", full_position_name.c_str(), it2->second.x+slot.x+slot.xoff, it2->second.y+slot.y+slot.yoff);
                }
            }
        }
        motionsetpoint_defs_written = 1;
    }
    
    if (!motionsetpoint_defs_written) {
        // No slots defined
        errlogPrintf("sampleChanger: no data written\n");
        return 1;
    }
    
    return 0;
}
