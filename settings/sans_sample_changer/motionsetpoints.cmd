# Don't want this to be in the repo with the other files as it gets automatically generated
$(IFIOC_GALIL_01) epicsEnvSet "LOOKUPFILE" "$(ICPVARDIR)/tmp/sample.txt"
$(IFIOC_GALIL_01) motionSetPointsConfigure("LOOKUPFILE","LOOKUPFILE", 2)
$(IFIOC_GALIL_01) dbLoadRecords("$(MOTIONSETPOINTS)/db/motionSetPointsDoubleAxis.db","P=$(MYPVPREFIX)LKUP:SAMPLE:,AXIS0=$(MYPVPREFIX)MOT:SC:X,AXIS1=$(MYPVPREFIX)MOT:SC:Y,TOL=1,LOOKUP=LOOKUPFILE")
