import os
import numpy as np

indir = os.path.dirname(os.path.realpath(__file__))

for a in os.listdir(indir):
    try:
        os.rmdir(a)
    except OSError as ex:
        print "directory not empty"



keys = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24]

comp = {key: {k: 0.0 for k in xrange(1,25)} for key in keys}

for root, dirs, filenames in os.walk(indir):
    for fname in filenames:
        if fname == 'result.csv':
            f = open(os.path.join(root, fname), 'r')
            for i, line in enumerate(f):
                if i >= 1:
                    l = line.replace('\n','')
                    l = l.split(',')
                    comp[int(l[0])][int(root.split('_')[6])] = float(l[1])
            f.close()



#result = {k: np.mean(np.array(v)) for k,v in comp.iteritems()}

fresult = open(os.path.join(os.path.dirname(os.path.realpath(__file__)),"result_concat.csv"),'w')

a="Node_id\/--num_app_nodes->,"
for ss in xrange(1,25):
    a = a + str(ss) + ","
fresult.write(a[:-1])
fresult.write('\n')

for k,v in comp.iteritems():
    s = str(k) + ","
    for k1,v1 in comp[k].iteritems():
        s = s + str(v1) + ","
    fresult.write(s[:-1])
    fresult.write('\n')
fresult.close()
