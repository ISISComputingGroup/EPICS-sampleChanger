# $(ICPCONFIGDIR) or $(ICPCONFIGROOT) ?
$(IFDMC05) epicsEnvSet "RACKDEFS" "$(ICPCONFIGROOT)/motionSetPoints/rack_definitions.xml"
$(IFDMC05) epicsEnvSet "SLOT_DETAILS_FILE" "$(ICPCONFIGROOT)/motionSetPoints/samplechanger.xml"
$(IFDMC05) epicsEnvSet "SAMPLE_LKUP_FILE" "$(ICPCONFIGROOT)/motionSetPoints/sample.txt"

$(IFDMC05) sampleChangerConfigure("CURRENT","CURRENT",2)

$(IFDMC05) dbLoadRecords("$(SAMPLECHANGER)/db/sampleChanger.db","P=$(MYPVPREFIX)SAMPCHNG:,MOT_SET_POINT=$(MYPVPREFIX)LKUP:SAMPLE:,LOOKUP=CURRENT")

$(IFDMC08) epicsEnvSet "RACKDEFS" "$(ICPCONFIGROOT)/motionSetPoints/lin_rack_definitions.xml"
$(IFDMC08) epicsEnvSet "SLOT_DETAILS_FILE" "$(ICPCONFIGROOT)/motionSetPoints/linearsamplechanger.xml"
$(IFDMC08) epicsEnvSet "SAMPLE_LKUP_FILE" "$(ICPCONFIGROOT)/motionSetPoints/linsample.txt"

$(IFDMC08) sampleChangerConfigure("CURRENT_LIN","CURRENT_LIN",1)

$(IFDMC08) dbLoadRecords("$(SAMPLECHANGER)/db/sampleChanger.db","P=$(MYPVPREFIX)LSAMPCHNG:,MOT_SET_POINT=$(MYPVPREFIX)LKUP:LSAMPLE:,LOOKUP=CURRENT_LIN")
