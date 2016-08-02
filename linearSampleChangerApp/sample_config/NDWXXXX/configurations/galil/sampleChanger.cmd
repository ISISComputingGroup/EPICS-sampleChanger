# $(ICPCONFIGDIR) or $(ICPCONFIGROOT) ?
$(IFDMC05) epicsEnvSet "RACKDEFS" "$(ICPCONFIGROOT)/motionSetPoints/rack_definitions.xml"
$(IFDMC05) epicsEnvSet "SLOT_DETAILS_FILE" "$(ICPCONFIGROOT)/motionSetPoints/samplechanger.xml"
$(IFDMC05) epicsEnvSet "SAMPLE_LKUP_FILE" "$(ICPCONFIGROOT)/motionSetPoints/sample.txt"

$(IFDMC05) sampleChangerConfigure("CURRENT","CURRENT")

$(IFDMC05) dbLoadRecords("$(SAMPLECHANGER)/db/sampleChanger.db","P=$(MYPVPREFIX)SAMPCHNG:,TARGET_PV1=$(MYPVPREFIX)LKUP:SAMPLE:RESET,LOOKUP=CURRENT")
