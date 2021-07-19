#ifndef CONVERTER_H
#define CONVERTER_H

#include <map>
#include <list>
#include <string>

#define TIXML_USE_STL 
#include "tinyxml.h"
#include <vector>

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
    converter(int i, std::vector<std::pair<std::string, std::vector<std::pair<std::string, samplePosn>>>> racks, std::vector<std::pair<std::string, slotData>> slots);
    virtual ~converter() {};
    int createLookup();
    std::vector<std::pair<std::string, slotData>> loadSlotDetails(TiXmlHandle& hRoot);

    std::string get_available_slots();
    std::string get_available_in_slot(std::string slot);
    bool checkSlotExists(std::string slotName);
    std::string get_slot_for_position(std::string slot);

    void loadRackDefs(TiXmlHandle& hRoot);
    void loadSlotDefs(TiXmlHandle& hRoot);

    std::vector<std::pair<std::string, slotData>> v_slots;
    std::vector<std::pair<std::string, std::vector<std::pair<std::string, samplePosn>>>> v_racks;

private:
    // vectors of pairs are used here rather than maps to preserve insertion order

    // slot : list(positions)
    std::vector<std::pair<std::string, std::list<std::string>>> v_positions_for_each_slot;

    // position : slot
    std::vector<std::pair<std::string, std::string>> v_slot_for_each_position;

    int m_dims;

    void loadDefRackDefs(const char* env_fname);
    void loadRackDefs(const char* fname);
    void loadSlotDetails(const char* fname);
    int createLookup(FILE* fpOut);
};

#endif /* CONVERTER_H */