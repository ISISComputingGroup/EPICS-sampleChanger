TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

### NOTE: there should only be one build.mak for a given IOC family and this should be located in the ###-IOC-01 directory

#=============================
# Build the IOC application sampleChanger
# We actually use $(APPNAME) below so this file can be included by multiple IOCs

LIBRARY_IOC = $(APPNAME)
# sampleChanger.dbd will be created and installed
DBD += $(APPNAME).dbd

# Add all the support libraries needed by this IOC
## ISIS standard libraries ##
$(APPNAME)_LIBS += devIocStats 
$(APPNAME)_LIBS += pvdump $(MYSQLLIB) easySQLite sqlite 
$(APPNAME)_LIBS += caPutLog
$(APPNAME)_LIBS += icpconfig pugixml
$(APPNAME)_LIBS += autosave
$(APPNAME)_LIBS += utilities
$(APPNAME)_LIBS += asubFunctions asyn
$(APPNAME)_LIBS += seq pv
$(APPNAME)_LIBS += TinyXML
## Add other libraries here ##

$(APPNAME)_SRCS += converter.cpp sampleChanger.cpp

# Add support from base/src/vxWorks if needed
#$(APPNAME)_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
$(APPNAME)_LIBS += $(EPICS_BASE_IOC_LIBS)

ifeq ($(findstring 10.0,$(VCVERSION)),)
    # googleTest Runner
    GTESTPROD_HOST+= runner
    runner_LIBS += $(EPICS_BASE_IOC_LIBS) TinyXML
    runner_SRCS += converter.cpp converterTests.cpp
endif

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
# -include $(GTEST)/cfg/compat.RULES_BUILD


ifdef T_A
install: dllPath.bat

dllPath.bat:
	$(CONVERTRELEASE) -a $(T_A) -T $(IOCS_APPL_TOP) $@
endif
