import os
import glob
import shutil
from shutil import copyfile

indir = os.path.dirname(os.path.realpath(__file__))

routing = "split"

appnodes = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24]

for n in appnodes:
    os.chdir(os.path.join(indir, routing + "_500x500_25_1_" + str(n) + "_1_400.000000"))
    os.system("python merge.py")
    os.chdir(os.path.join(indir,"power_" + routing + "_500x500_25_1_" + str(n) + "_1_400.000000"))
    os.system("python merge.py")


os.chdir(indir)
try:
    os.mkdir(os.path.join(indir,"_individual"))
except OSError:
    print "Dir already exists"

try:
    os.mkdir(os.path.join(indir,"_overall"))
except OSError:
    print "Dir already exists"

files = glob.glob("power_*")
for f in files:
    shutil.move(os.path.join(indir,f), os.path.join(indir,"_individual",f))
copyfile(os.path.join(indir,'merge_merge_p.py'),os.path.join(indir,'_individual','merge_merge_p.py'))

files = glob.glob(routing+"_*")
for f in files:
    shutil.move(os.path.join(indir,f), os.path.join(indir,"_overall",f))
copyfile(os.path.join(indir,'merge_merge_o.py'),os.path.join(indir,'_overall','merge_merge_o.py'))

os.chdir(os.path.join(indir,"_individual"))
os.system("python merge_merge_p.py")

os.chdir(os.path.join(indir,"_overall"))
os.system("python merge_merge_o.py")
