#include "gtest/gtest.h"
#include "shareLib.h"
#include "converter.h"
#include "string.h"
#include <stdlib.h>

#include "converter.h"


namespace {

    TEST(ConverterTests, all_and_end_get_added_to_list_of_slots) {
        std::vector<converter::Rack> racks;
        std::vector<converter::Slot> slots;
        converter::Slot slot;
        std::string slotname("TEST");
        slot.name = slotname;

        slots.push_back(slot);

        converter* conv = new converter(2, racks, slots);
        ASSERT_EQ(conv->get_available_slots(), "TEST _ALL END");
    }

    TEST(ConverterTests, GIVEN_xml_with_no_sample_suffix_WHEN_converted_THEN_suffix_is_name) {
        std::vector<converter::Rack> racks;
        std::vector<converter::Slot> slots;

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

        converter::Slot slot;
        std::string slotname("T");
        slot.name = slotname;
        
        slots.push_back(slot);

        converter* conv = new converter(2, racks, slots);
        conv->loadSlotDetails(hRoot);
        std::string suffix = std::find_if(conv->slots().cbegin(), conv->slots().cend(), [&](converter::Slot slt) {return slt.name == "T"; })->sampleSuffix;
        ASSERT_EQ(suffix, "T");
    }

    TEST(ConverterTests, xml_rack_order_preserved_WHEN_loading_xml_files)
    {
        converter conv(2, std::vector<converter::Rack>(), std::vector<converter::Slot>());

        // the input data is not ordered alphabetically
        std::string filedata = " <definitions> \
                    <racks> \
                        <rack name=\"NarrowX10\"> \
                            <position name=\"A\" y=\"0\" x=\"-592.5\"/> \
                        </rack> \
                        <rack name=\"Banjo\"> \
                            <position name=\"1\" y=\"0\" x=\"-600.5\"/> \
                        </rack> \
                    </racks> \
                    <slots> \
                        <slot name=\"T\" x=\"0\" y=\"0\"/> \
                        <slot name=\"B\" x=\"0\" y=\"0\"/> \
                        <slot name=\"F\" x=\"0\" y=\"0\"/> \
                    </slots> \
                </definitions> ";

        std::string filedata2 = "<slots> \
                        <slot name = \"T\" rack_type = \"NarrowX10\" xoff = \"284.7\" yoff = \"-125.0\" / > \
                        <slot name = \"B\" rack_type = \"NarrowX10\" xoff = \"284.5\" yoff = \"104.7\" / > \
                        <slot name = \"F\" rack_type = \"Banjo\" xoff = \"289.5\" yoff = \"-125.0\" / > \
                    </slots>";

        TiXmlDocument doc;
        doc.Parse(filedata.c_str(), 0, TIXML_ENCODING_UTF8);

        TiXmlHandle hDoc(&doc);
        TiXmlElement* pElem;
        TiXmlHandle hRoot(0);

        pElem = hDoc.FirstChildElement().Element();
        hRoot = TiXmlHandle(pElem);

        TiXmlDocument doc2;
        doc2.Parse(filedata2.c_str(), 0, TIXML_ENCODING_UTF8);

        TiXmlHandle hDoc2(&doc2);
        TiXmlElement* pElem2;
        TiXmlHandle hRoot2(0);

        pElem2 = hDoc2.FirstChildElement().Element();
        hRoot2 = TiXmlHandle(pElem2);

        conv.loadRackDefs(hRoot);
        conv.loadSlotDefs(hRoot);
        conv.loadSlotDetails(hRoot2);

        // If order is preserved in m_racks and m_slots it will be correctly outputted to lookup file as it simply loops over them
        
        auto racks = conv.racks();
        auto slots = conv.slots();
        ASSERT_EQ(racks[0].name, "NarrowX10");
        ASSERT_EQ(racks[1].name, "Banjo");
        ASSERT_EQ(slots[0].name, "T");
        ASSERT_EQ(slots[1].name, "B");
        ASSERT_EQ(slots[2].name, "F");
    }

}
