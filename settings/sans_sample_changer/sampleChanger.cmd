$(IFIOC_GALIL_01) epicsEnvSet "RACKDEFS" "$(SAMPLECHANGER)/settings/sans_sample_changer/rack_definitions.xml"
$(IFIOC_GALIL_01) epicsEnvSet "SLOT_DETAILS_FILE" "$(SAMPLECHANGER)/settings/sans_sample_changer/samplechanger.xml"

# Don't want this to be in the repo with the other files as it gets automatically generated
$(IFIOC_GALIL_01) epicsEnvSet "SAMPLE_LKUP_FILE" "c:/instrument/var/tmp/sample.txt"

$(IFIOC_GALIL_01) sampleChangerConfigure("CURRENT","CURRENT")

$(IFIOC_GALIL_01) dbLoadRecords("$(SAMPLECHANGER)/db/sampleChanger.db","P=$(MYPVPREFIX)SAMPCHNG:,TARGET_PV1=$(MYPVPREFIX)LKUP:SAMPLE:RESET,LOOKUP=CURRENT")
