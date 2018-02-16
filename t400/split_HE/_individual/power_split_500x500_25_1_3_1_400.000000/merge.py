import os
import numpy as np

keys = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24]

comp = {key: [] for key in keys}

indir = os.path.dirname(os.path.realpath(__file__))

for root, dirs, filenames in os.walk(indir):
    for fname in filenames:
        if fname != __file__ and fname != "result.csv":
            f = open(os.path.join(root, fname), 'r')
            for i, line in enumerate(f):
                if i >= 1:
                    l = line.replace('\n','')
                    l = l.split(',')
                    comp[int(l[0])].append(float(l[3]))
            f.close()


result = {k: np.mean(np.array(v)) for k,v in comp.iteritems()}

fresult = open(os.path.join(os.path.dirname(os.path.realpath(__file__)),"result.csv"),'w')
fresult.write('node_id,lifetime\n')
for k,v in result.iteritems():
    fresult.write(str(k))
    fresult.write(',')
    fresult.write(str(v))
    fresult.write('\n')
fresult.close()
