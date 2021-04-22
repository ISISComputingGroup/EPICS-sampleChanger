#include <errlog.h>
#include <shareLib.h>

#include "converter.h"

#define ALL_POSITIONS_NAME "_ALL"

converter::converter(int i=2)
{
    m_dims = i;
    loadDefRackDefs("RACKDEFS");
} 

// alternative constructor, only for testing
converter::converter(int i, std::map<std::string, std::map<std::string, samplePosn> > racks, std::map<std::string, slotData> slots)
{
    m_dims = i;
    m_racks = racks;
    m_slots = slots;
} 

void converter::loadDefRackDefs(const char* env_fname) 
{
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

// Create the lookup file
int converter::createLookup(const std::string &selectedRack) 
{
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
    
    int success = createLookup(fpOut, selectedRack);
    
    fclose(fpOut);
    
    return success;
}

std::string converter::get_available_slots() 
{
    std::string res;
    for ( std::map<std::string, slotData>::iterator it=m_slots.begin() ; it!=m_slots.end() ; it++ ) {
        res += it->first;
        res += " ";
    }
    res += ALL_POSITIONS_NAME;
    res += " END";
    return res;
}

// Write to the lookup file
int converter::createLookup(FILE *fpOut, const std::string &selectedRack) 
{
    errlogPrintf("sampleChanger: writing motionsetpoints lookup file for selected rack '%s'\n", selectedRack.c_str());
    int motionsetpoint_defs_written = 0;
    
    fprintf(fpOut, "# Convert sample position names to motor coordinates\n");
    fprintf(fpOut, "# WARNING: Generated file - Do not edit\n");
    fprintf(fpOut, "# Instead edit samplechanger.xml and press recalc\n");

    for ( std::map<std::string, slotData>::iterator it=m_slots.begin() ; it!=m_slots.end() ; it++ ) {
        
        if (selectedRack == ALL_POSITIONS_NAME || selectedRack == it->first) {
            slotData &slot = it->second;
            std::map<std::string, std::map<std::string, samplePosn> >::iterator iter = m_racks.find(slot.rackType);
            if ( iter==m_racks.end() ) {
                errlogPrintf("sampleChanger: Unknown rack type '%s' of slot %s\n", slot.rackType.c_str(), slot.name.c_str());
                return 1;
            }
            else {
                for ( std::map<std::string, samplePosn>::iterator it2 = iter->second.begin() ; it2!=iter->second.end() ; it2++ ) {
                    if ( m_dims==1 ) {
                        fprintf(fpOut, "%s%s %f\n", it2->second.name.c_str(), slot.sampleSuffix.c_str(), it2->second.x+slot.x+slot.xoff);                    
                    }
                    else {
                        fprintf(fpOut, "%s%s %f %f\n", it2->second.name.c_str(), slot.sampleSuffix.c_str(), it2->second.x+slot.x+slot.xoff, it2->second.y+slot.y+slot.yoff);
                    }
                }
            }
            motionsetpoint_defs_written = 1;
        }
    }
    
    if (!motionsetpoint_defs_written) {
        errlogPrintf("sampleChanger: no data written (unknown rack '%s')\n", selectedRack.c_str());
        return 1;
    }
    
    return 0;
}
