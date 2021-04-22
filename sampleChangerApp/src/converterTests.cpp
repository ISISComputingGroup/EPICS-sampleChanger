#include "gtest/gtest.h"
#include "shareLib.h"
#include "converter.h"
#include "string.h"
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

    TEST(ConverterTests, GIVEN_xml_with_no_sample_suffix_WHEN_converted_THEN_suffix_is_name) {
        std::map<std::string, std::map<std::string, samplePosn> > racks;
        std::map<std::string, slotData> slots;

        std::string filedata = "<slots> \
                        <slot name = \"T\" rack_type = \"NarrowX10\" xoff = \"284.7\" yoff = \"-125.0\" / > \
                    </slots>";
        
        TiXmlDocument doc;
        doc.Parse(filedata.c_str(), 0, TIXML_ENCODING_UTF8);

        TiXmlHandle hDoc(&doc);
        TiXmlElement* pElem;
        TiXmlHandle hRoot(0);

        pElem = hDoc.FirstChildElement().Element();
        hRoot = TiXmlHandle(pElem);

        slotData slot;
        std::string slotname("T");
        slot.name = slotname;

        slots[slotname] = slot;

        converter* conv = new converter(2, racks, slots);
        slots = conv->loadSlotDetails(hRoot);
        ASSERT_EQ(slots["T"].sampleSuffix, "T");
    }
}
