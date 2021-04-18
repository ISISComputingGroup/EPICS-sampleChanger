#ifndef CONVERTER_H
#define CONVERTER_H
 
#include <map>
#include <list>
#include <string>

#define TIXML_USE_STL 
#include "tinyxml.h"

struct samplePosn
{
    std::string name;
    double x;
    double y;
};

struct slotData
{
    std::string name;
    std::string sampleSuffix;
    double x;
    double y;
    std::string rackType;
    double xoff;
    double yoff;
};

class converter
{
public:
    converter(int i);
    converter(int i, std::map<std::string, std::map<std::string, samplePosn> > racks, std::map<std::string, slotData> slots);
    virtual ~converter() {};
    int createLookup();
    std::map<std::string, slotData> loadSlotDetails(TiXmlHandle &hRoot);
    
    std::string get_available_slots();
    std::string get_available_in_slot(std::string slot);
    bool checkSlotExists(std::string slotName);
   

private:
    // rack : (position : position_data)
    std::map<std::string, std::map<std::string, samplePosn> > m_racks;
    
    // slot : slot_data
    std::map<std::string, slotData> m_slots;

    // slot : list(positions)
    std::map<std::string, std::list<std::string>> m_positions_for_each_slot;

    int m_dims;

    void loadDefRackDefs(const char* env_fname);
    void loadRackDefs(const char* fname);
    void loadRackDefs(TiXmlHandle &hRoot);
    void loadSlotDefs(TiXmlHandle &hRoot);
    void loadSlotDetails(const char* fname);
    int createLookup(FILE *fpOut);
};

#endif /* CONVERTER_H */