$(IFIOC_GALIL_01) epicsEnvSet "LOOKUPFILE" "$(SAMPLECHANGER)/settings/sans_sample_changer/sample.txt"
$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILE","LOOKUPFILE")
$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPoints.db","P=$(MYPVPREFIX)LKUP:SAMPLE:,TARGET_PV1=$(MYPVPREFIX)MOT:SAMPLECHANGER:X:MTR,TARGET_RBV1=$(MYPVPREFIX)MOT:SAMPLECHANGER:X:MTR.RBV,TARGET_DONE=$(MYPVPREFIX)MOT:SAMPLECHANGER:X:MTR.DMOV,TARGET_PV2=$(MYPVPREFIX)MOT:SAMPLECHANGER:Y:MTR,TARGET_RBV2=$(MYPVPREFIX)MOT:SAMPLECHANGER:Y:MTR.RBV,TARGET_DONE2=$(MYPVPREFIX)MOT:SAMPLECHANGER:Y:MTR.DMOV,TOL=1,LOOKUP=LOOKUPFILE")
