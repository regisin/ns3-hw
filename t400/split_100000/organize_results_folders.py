import os
import glob
from shutil import copyfile

routing = "split" # split,olsr,etc

indir = os.path.dirname(os.path.realpath(__file__))

num_app = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24]

for n in num_app:
    try:
        os.mkdir(os.path.join(indir,routing + '_500x500_25_1_' + str(n) + '_1_400.000000'))
    except OSError:
        print "Dir already exists"
    copyfile(os.path.join(indir,'merge_o.py'),os.path.join(os.path.join(indir,routing + '_500x500_25_1_' + str(n) + '_1_400.000000'),'merge.py'))
    try:
        os.mkdir(os.path.join(indir,'power_' + routing + '_500x500_25_1_' + str(n) + '_1_400.000000'))
    except OSError:
        print "Dir already exists"
    copyfile(os.path.join(indir,'merge_p.py'),os.path.join(os.path.join(indir,'power_' + routing + '_500x500_25_1_' + str(n) + '_1_400.000000'),'merge.py'))


for n in num_app:
    files = glob.glob(routing + '_500x500_25_1_' + str(n) + '_1_400.000000*.csv')
    for f in files:
        os.rename(os.path.join(indir,f), os.path.join(os.path.join(indir,routing + '_500x500_25_1_' + str(n) + '_1_400.000000'),f))

    files = glob.glob('power_' + routing + '_500x500_25_1_' + str(n) + '_1_400.000000*.csv')
    for f in files:
        os.rename(os.path.join(indir,f), os.path.join(os.path.join(indir,'power_' + routing + '_500x500_25_1_' + str(n) + '_1_400.000000'),f))
