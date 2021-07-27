#ifndef CONVERTER_H
#define CONVERTER_H

#include <map>
#include <list>
#include <string>

#define TIXML_USE_STL 
#include "tinyxml.h"
#include <vector>

class converter
{
public:
    struct Position
    {
        std::string name;
        double x;
        double y;
    };

    struct Slot
    {
        std::string name;
        std::string sampleSuffix;
        double x;
        double y;
        std::string rackType;
        double xoff;
        double yoff;
    };

    struct Rack
    {
        std::string name;
        std::vector<Position> positions;

        Rack(std::string name, std::vector<Position> positions)
        {
            this->name = name;
            this->positions = positions;
        }
    };

    struct SlotPositions
    {
        std::string slotName;
        std::list<std::string> positions;

        SlotPositions(std::string slot, std::list<std::string> positions)
        {
            this->slotName = slot;
            this->positions = positions;
        }
    };

    converter(int i);
    converter(int i, std::vector<Rack> racks, std::vector<Slot> slots);
    virtual ~converter() {};
    int createLookup();
    std::vector<Slot> loadSlotDetails(TiXmlHandle& hRoot);

    std::string get_available_slots();
    std::string get_available_in_slot(std::string slot);
    bool checkSlotExists(std::string slotName);
    std::string get_slot_for_position(std::string slot);

    void loadRackDefs(TiXmlHandle& hRoot);
    void loadSlotDefs(TiXmlHandle& hRoot);

    std::vector<Slot> v_slots;
    std::vector<Rack> v_racks;

private:

    std::vector<SlotPositions> v_positions_for_each_slot;
    std::vector<SlotPositions>::iterator find_in_positions(std::string slot);

    // this is effectively just a mapping of string to string, preserving insertion order
    std::vector<std::pair<std::string, std::string>> v_slot_for_each_position;
    std::vector<std::pair<std::string, std::string>>::iterator find_in_slots(std::string name);

    int m_dims;

    void loadDefRackDefs(const char* env_fname);
    void loadRackDefs(const char* fname);
    void loadSlotDetails(const char* fname);
    int createLookup(FILE* fpOut);
};

#endif /* CONVERTER_H */