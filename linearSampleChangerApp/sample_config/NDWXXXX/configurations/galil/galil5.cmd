## configure galil crate 5

## passed parameters
##   GCID - galil crate software index. Numbering starts at 0 - will always be 0 if there is one to one galil crate <-> galil IOC mapping  
##   GALILADDR05 - address of this galil

## see README_galil_cmd.txt for usage of commands below

#G21X3Config($(GCID),"$(GALILADDR05)",8,2100,2000) 
#GalilCreateController("Galil", "$(GALILADDR05)", 20)
GalilCreateController("Galil", "$(GALILADDR05)", -40)

#G21X3NameConfig($(GCID),"A",2,0,2,0,0,0,"",0,0,"",1,0)
#G21X3NameConfig($(GCID),"B",4,0,2,0,0,0,"",0,0,"",1,0)
#G21X3NameConfig($(GCID),"C",2,0,0,0,0,0,"",0,0,"",1,0)
#G21X3NameConfig($(GCID),"D",2,0,2,0,0,0,"",0,0,"",1,0)
#G21X3NameConfig($(GCID),"E",2,0,2,0,0,0,"",0,0,"",1,0)
#G21X3NameConfig($(GCID),"F",4,0,0,0,0,1,"",0,0,"",1,0)
#G21X3NameConfig($(GCID),"G",2,0,0,0,0,1,"",0,0,"",1,0)
#G21X3NameConfig($(GCID),"H",3,0,0,0,0,1,"",0,0,"",1,0)
GalilCreateAxis("Galil","A",0,"",1)
GalilCreateAxis("Galil","B",0,"",1)
GalilCreateAxis("Galil","C",0,"",1)
GalilCreateAxis("Galil","D",0,"",1)
GalilCreateAxis("Galil","E",0,"",1)
GalilCreateAxis("Galil","F",1,"",1)
GalilCreateAxis("Galil","G",1,"",1)
GalilCreateAxis("Galil","H",1,"",1)

#G21X3StartCard($(GCID),"$(GALIL)/db/galil_Default_Header.gmc;$(GALIL)/db/galil_Home_Home.gmc!$(GALIL)/db/galil_Home_Home.gmc!$(GALIL)/db/galil_Home_Home.gmc!$(GALIL)/db/galil_Home_Home.gmc!$(GALIL)/db/galil_Home_Home.gmc!$(GALIL)/db/galil_Home_RevLimit.gmc!$(GALIL)/db/galil_Home_RevLimit.gmc!$(GALIL)/db/galil_Home_Home.gmc;$(GALIL)/db/galil_Default_Footer.gmc",0,0)
GalilStartController("Galil","$(GALIL)/gmc/galil_Default_Header.gmc;$(GALIL)/gmc/galil_Home_Home+FIneg.gmc!$(GALIL)/gmc/galil_Home_Home+FIpos.gmc!$(GALIL)/gmc/galil_Home_Home+FIneg.gmc!$(GALIL)/gmc/galil_Home_Home+FIneg.gmc!$(GALIL)/gmc/galil_Home_Home+FIneg.gmc!$(GALIL)/gmc/galil_Home_RevLimit.gmc!$(GALIL)/gmc/galil_Home_RevLimit.gmc!$(GALIL)/gmc/galil_Home_Home.gmc;$(GALIL)/gmc/galil_Default_Footer.gmc",0,0,3)
