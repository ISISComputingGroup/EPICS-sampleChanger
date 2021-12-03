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
    /** 
     * A converter class to convert the xml file to a map of strings.
     * The map is used to store the data of the xml file.
     * Returns a map of strings.
     */

public:
    struct Position
    {
        /** 
         * A struct to store the position of the sample.
         * The position is stored as a list of strings.
         * @param x The x position of the sample.
         * @param y The y position of the sample.
         * @return A list of strings.
         */
        std::string name;
        double x;
        double y;
    };

    struct Slot
    {
        /** 
         * A struct to store the slot of the sample.
         * The slot is stored as a list of strings.
         * @param name The name of the slot.
         * @param sampleSuffix The suffix of the sample.
         * @param x The x position of the sample.
         * @param y The y position of the sample.
         * @param rackType The type of the rack.
         * @param xoff The x offset of the sample.
         * @param yoff The y offset of the sample.
         * @return A list of strings.
         */
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
        /** 
         * A struct to store the rack of the sample and its position.
         * The rack is stored as a list of strings.
         * @param name The name of the rack.
         * @param position The position of the rack.
         */
        std::string name;
        std::vector<Position> positions;

        Rack(const std::string& name, const std::vector<Position>& positions)
        {
            /** 
             * A constructor to create a rack.
             * @param name The name of the rack.
             * @param position The position of the rack.
             */
            this->name = name;
            this->positions = positions;
        }
    };

    struct SlotPositions
    {
        /** 
         * A struct to store the slot of the sample and its position.
         * The slot is stored as a list of strings.
         * @param slotName The name of the slot.
         * @param positions The position of the slot.
         */
     
        std::string slotName;
        std::list<std::string> positions;

        SlotPositions(const std::string& slot, const std::list<std::string>& positions)
        {
            /** 
             * Constructor of the struct SlotPositions.
             * The slot is stored as a list of strings.
             * @param slot The name of the slot.
             * @param positions The position of the slot.
             */
            this->slotName = slot;
            this->positions = positions;
        }
    };

    converter(int i);
    converter(int i, const std::vector<Rack>& racks, const std::vector<Slot>& slots);
    virtual ~converter() {};
    int createLookup();
    std::vector<Slot> loadSlotDetails(TiXmlHandle& hRoot);

    std::string get_available_slots() const; // Returns a list of available slots
    std::string get_available_in_slot(std::string slot); // returns a list of positions in the slot
    bool checkSlotExists(const std::string& slotName) const; // returns true if the slot exists
    std::string get_slot_for_position(const std::string& slot) const; // returns the slot name for a given position

    // The following functions are used to convert between the slot and position names
    void loadRackDefs(TiXmlHandle& hRoot);
    void loadSlotDefs(TiXmlHandle& hRoot);

    // The following is a map of rack types to a map of positions to samplePosn
    const std::vector<Slot>& slots() const { return m_slots; } // create a const version of the slots
    const std::vector<Rack>& racks() const { return m_racks; } // create a const version of the racks
    const std::string errors() {return std::accumulate(m_errors.begin(), m_errors.end(), std::string(""));}; // create a const version of the errors

private:
    /** 
     * A private member function to load the racks and slots.
     */
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

    void loadDefRackDefs(const char* env_fname); // loads the rack definitions from the environment file
    void loadRackDefs(const char* fname); 
    void loadSlotDetails(const char* fname);
    int createLookup(FILE* fpOut);
    void printError(const char* format, ...);
};

#endif /* CONVERTER_H */