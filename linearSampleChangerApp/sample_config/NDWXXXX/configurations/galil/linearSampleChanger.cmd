# $(ICPCONFIGDIR) or $(ICPCONFIGROOT) ?
$(IFDMC08) epicsEnvSet "LIN_RACKDEFS" "$(ICPCONFIGROOT)/motionSetPoints/lin_rack_definitions.xml"
$(IFDMC08) epicsEnvSet "SLOT_DETAILS_FILE" "$(ICPCONFIGROOT)/motionSetPoints/linearsamplechanger.xml"
$(IFDMC08) epicsEnvSet "LIN_SAMPLE_LKUP_FILE" "$(ICPCONFIGROOT)/motionSetPoints/linsample.txt"

$(IFDMC08) linearSampleChangerConfigure("CURRENT","CURRENT")

$(IFDMC08) dbLoadRecords("$(LINEARSAMPLECHANGER)/db/linearSampleChanger.db","P=$(MYPVPREFIX)LSAMPCHNG:,TARGET_PV1=$(MYPVPREFIX)LKUP:LSAMPLE:RESET,LOOKUP=CURRENT")
