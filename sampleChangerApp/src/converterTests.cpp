#include "gtest/gtest.h"
#include "shareLib.h"
#include "converter.h"
#include <stdlib.h>


namespace {
    
    TEST(ConverterTests, all_and_end_get_added_to_list_of_slots) {
        std::map<std::string, std::map<std::string, samplePosn> > racks;
        std::map<std::string, slotData> slots;
        
        slotData slot;
        std::string slotname("TEST");
        slot.name = slotname;
        
        slots[slotname] = slot;
        
        converter* conv = new converter(2, racks, slots);
        ASSERT_EQ(conv->get_available_slots(), "TEST _ALL END");
    }
}
