import os
import numpy as np

individual_row_keys = ['total_bytes_received','throughput_(bps)','no_of_received_packets','packet_delivery_ratio','mean_hop_count','mean_delay_(s)','l3_drop_packets','total_initial_energy','total_remaining_energy']

keys = [1,5,10,15,20,24]

comp = {key: "" for key in keys}

indir = os.path.dirname(os.path.realpath(__file__))

for root, dirs, filenames in os.walk(indir):
    for fname in filenames:
        if fname == 'result.csv':
            #print fname, root
            f = open(os.path.join(root, fname), 'r')
            for i, line in enumerate(f):
                if i == 1:
                    l = line.replace('\n','')
                    l = l.split(',')
                    s = ""
                    for ss in l:
                        s = s + str(ss) + ","
                    s = s[:-1]
                    comp[int(root.split('_')[5])] = s
            f.close()


fresult = open(os.path.join(os.path.dirname(os.path.realpath(__file__)),"result_concat.csv"),'w')

a="num_app_nodes,"
for ss in individual_row_keys:
    a = a + str(ss) + ","
fresult.write(a[:-1])
fresult.write('\n')

for k in keys:
    a = str(k) + ","
    fresult.write(a + comp[k])
    fresult.write('\n')

fresult.close()
