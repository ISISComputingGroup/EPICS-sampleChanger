#ifndef CONVERTER_H
#define CONVERTER_H
 
#include <unordered_map>
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
    int createLookup(const std::string &selectedRack);
    std::unordered_map<std::string, slotData> loadSlotDetails(TiXmlHandle &hRoot);
    
    std::string get_available_slots();
   

private:
    std::unordered_map<std::string, std::unordered_map<std::string, samplePosn> > m_racks;
    
    std::unordered_map<std::string, slotData> m_slots;

    int m_dims;

    void loadDefRackDefs(const char* env_fname);
    void loadRackDefs(const char* fname);
    void loadRackDefs(TiXmlHandle &hRoot);
    void loadSlotDefs(TiXmlHandle &hRoot);
    void loadSlotDetails(const char* fname);
    int createLookup(FILE *fpOut, const std::string &selectedRack);
};

#endif /* CONVERTER_H */