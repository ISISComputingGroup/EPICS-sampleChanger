import genie
import time
import os

# Simple class to emulate epics.PV
class PV:
	"""
	A class to emulate epics.PV for the sample changer using the genie library
	"""
	def __init__(self, pv):
		self.pv = pv
	def get(self):
		return genie.get_pv(self.pv)
	def put(self, value):
		genie.set_pv(self.pv, value)

# Move to a named position and check that we go where we expect
def move_to(posn, coord1, coord2):
	"""
	A method to move to a named position and check that we go where we expect
	@param posn: The position to move to
	@param coord1: The first coordinate
	@param coord2: The second coordinate
	@return: True if the move was successful, False otherwise
	"""
	posn_sp.put(posn)
	time.sleep(1)
	t1 = coord1t.get()
	t2 = coord2t.get()
	if abs(t1 - coord1)>0.01 or abs(t2 - coord2)>0.01:
		print 'Error: Expected (' + str(coord1) + ',' + str(coord2) + ') but got (' + str(t1) + ',' + str(t2) + ')'
		os.exit(0)
	print 'Target = (' + str(t1) + ',' + str(t2) + ')'
	while positioned.get()==0:
		print 'Current = (' + str(coord1c.get()) + ',' + str(coord2c.get()) + ')'
		time.sleep(5)
	c1 = coord1t.get()
	c2 = coord2t.get()
	if abs(c1 - coord1)>0.01 or abs(c2 - coord2)>0.01:
		print 'Error: Expected (' + str(coord1) + ',' + str(coord2) + ') but reached (' + str(c1) + ',' + str(c2) + ')'
		os.exit(0)

sim = True

# Make sure the motors will drive
if sim:
	motorY_spmg = PV(os.environ['MYPVPREFIX'] + 'MOT:STACK:Y:MTR.SPMG')
	motorY_spmg.put(3)
	motorZ_spmg = PV(os.environ['MYPVPREFIX'] + 'MOT:STACK:ZLO:MTR.SPMG')
	motorZ_spmg.put(3)

# Create the PVs
posn_sp = PV(os.environ['MYPVPREFIX'] + 'LKUP:SAMPLE:POSN:SP')
positioned = PV(os.environ['MYPVPREFIX'] + 'LKUP:SAMPLE:POSITIONED')
coord1t = PV(os.environ['MYPVPREFIX'] + 'LKUP:SAMPLE:COORD1')
coord2t = PV(os.environ['MYPVPREFIX'] + 'LKUP:SAMPLE:COORD2')
coord1c = PV(os.environ['MYPVPREFIX'] + 'LKUP:SAMPLE:COORD1:RBV')
coord2c = PV(os.environ['MYPVPREFIX'] + 'LKUP:SAMPLE:COORD2:RBV')
recalc = PV(os.environ['MYPVPREFIX'] + 'SAMPCHNG:RECALC')

# Set the offsets to zero
file = open('C:\\Instrument\\Settings\\config\\NDW1298\\configurations\\motionSetPoints\\samplechanger.xml','w')
file.write('<?xml version="1.0"?>\n')
file.write('<slots>\n')
file.write('	<slot name="Top_Left" rack_type="Banjo 1mm" xoff="0.0" yoff="0.0"/>\n')
file.write('	<slot name="Top_Right" rack_type="Rectangular" xoff="0.0" yoff="0.0"/>\n')
file.write('	<slot name="Bottom_Left" rack_type="Double Stopper" xoff="0.0" yoff="0.0"/>\n')
file.write('	<slot name="Bottom_Right" rack_type="Banjo 5mm" xoff="0.0" yoff="0.0"/>\n')
file.write('</slots>\n')
file.close()

# Create and load the new lookup
recalc.put(1)
time.sleep(1)

# Drive to a position
move_to('Bottom_Left_1',10,1)

# Drive to another position
move_to('Top_Left_2',9,15)

# Now with some offsets
file = open('C:\\Instrument\\Settings\\config\\NDW1298\\configurations\\motionSetPoints\\samplechanger.xml','w')
file.write('<?xml version="1.0"?>\n')
file.write('<slots>\n')
file.write('	<slot name="Top_Left" rack_type="Banjo 1mm" xoff="0.0" yoff="0.0"/>\n')
file.write('	<slot name="Top_Right" rack_type="Rectangular" xoff="0.0" yoff="0.0"/>\n')
file.write('	<slot name="Bottom_Left" rack_type="Double Stopper" xoff="1.0" yoff="2.0"/>\n')
file.write('	<slot name="Bottom_Right" rack_type="Banjo 5mm" xoff="0.0" yoff="0.0"/>\n')
file.write('</slots>\n')
file.close()

recalc.put(1)
time.sleep(1)

# Drive to the original position, but expect offsets this time
move_to('Bottom_Left_1',11,3)
