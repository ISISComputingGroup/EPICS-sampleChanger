#ifndef CONVERTER_H
#define CONVERTER_H

#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <numeric>

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

        Rack(const std::string& name, const std::vector<Position>& positions)
        {
            this->name = name;
            this->positions = positions;
        }
    };

    struct SlotPositions
    {
        std::string slotName;
        std::list<std::string> positions;

        SlotPositions(const std::string& slot, const std::list<std::string>& positions)
        {
            this->slotName = slot;
            this->positions = positions;
        }
    };

    converter(int i);
    converter(int i, const std::vector<Rack>& racks, const std::vector<Slot>& slots);
    virtual ~converter() {};
    int createLookup();
    std::vector<Slot> loadSlotDetails(TiXmlHandle& hRoot);

    std::string get_available_slots() const;
    std::string get_available_in_slot(std::string slot);
    bool checkSlotExists(const std::string& slotName) const;
    std::string get_slot_for_position(const std::string& slot) const;

    void loadRackDefs(TiXmlHandle& hRoot);
    void loadSlotDefs(TiXmlHandle& hRoot);

    const std::vector<Slot>& slots() const { return m_slots; }
    const std::vector<Rack>& racks() const { return m_racks; }
    const std::string errors() {return std::accumulate(m_errors.begin(), m_errors.end(), std::string(""));};

private:
    std::vector<std::string> m_errors;
    std::vector<Slot> m_slots;
    std::vector<Rack> m_racks;
    std::vector<SlotPositions> m_positions_for_each_slot;
    std::vector<SlotPositions>::iterator find_in_positions(const std::string& slot);
    std::vector<SlotPositions>::const_iterator find_in_positions(const std::string& slot) const;

    // this is effectively just a mapping of string to string, preserving insertion order
    std::vector<std::pair<std::string, std::string>> m_slot_for_each_position;
    std::vector<std::pair<std::string, std::string>>::iterator find_in_slots(const std::string& name);
    std::vector<std::pair<std::string, std::string>>::const_iterator find_in_slots(const std::string& name) const;

    int m_dims;

    void loadDefRackDefs(const char* env_fname);
    void loadRackDefs(const char* fname);
    void loadSlotDetails(const char* fname);
    int createLookup(FILE* fpOut);
    void printError(const char* format, ...);
};

#endif /* CONVERTER_H */