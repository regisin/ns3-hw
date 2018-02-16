import os
import numpy as np

keys = ['total_bytes_received','throughput_(bps)','no_of_received_packets','packet_delivery_ratio','mean_hop_count','mean_delay_(s)','l3_drop_packets','total_initial_energy','total_remaining_energy','average_lifetime']

comp = {key: [] for key in keys}

indir = os.path.dirname(os.path.realpath(__file__))

for root, dirs, filenames in os.walk(indir):
    for fname in filenames:
        if fname != __file__ and fname != 'result.csv':
            f = open(os.path.join(root, fname), 'r')
            for i, line in enumerate(f):
                if i == 1:
                    l = line.replace('\n','')
                    l = l.split(',')
                    if not 'nan' in l and not '-nan' in l:
                        for hcount in xrange(len(keys)):
                            comp[keys[hcount]].append(float(l[hcount]))
            f.close()


result = {k: np.mean(np.array(v)) for k,v in comp.iteritems()}


fresult = open(os.path.join(os.path.dirname(os.path.realpath(__file__)),"result.csv"),'w')

for i,k in enumerate(keys):
    fresult.write(str(k))
    if i != len(keys)-1:
        fresult.write(',')
fresult.write('\n')

for i,k in enumerate(keys):
    fresult.write(str(result[k]))
    if i != len(keys)-1:
        fresult.write(',')

fresult.close()
