#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
import pandas as pd
import os

title = ["algorithmName", "referenceStringName", "memorySize", "pageFaults", "interrupts", "diskWrites"]
algorithmName = ["FIFO", "ARB", "ESC", "LRU-LFU"]
performance = ["Page faults", "Interrupts", "Disk writes"]
dataName = ["Random data", "Locality data", "Exponential random data"]
mark = ["o", "v", "s", "*", "D"]

font = {'family' : 'normal',
        'weight' : 'bold',
        'size'   : 18}

plt.rc('font', **font)

imgPath = 'img/'
isExist = os.path.exists(imgPath)
if not isExist:
    os.makedirs(imgPath)
    print("create img directory")

# 1. The plots between performance and frames each algorithm.
for i in range(4) : # of algorithm
    data = pd.read_csv(algorithmName[i] + ".csv")
    k = 0
    for j in range(0, 15, 5): # (0, # of needed row, # of memorySize)
    
        plt.plot(data[title[2]][j:j+5], data[title[3]][j:j+5], label=performance[0], linewidth=5, linestyle='--', marker=mark[0], markersize=15)
        plt.plot(data[title[2]][j:j+5], data[title[4]][j:j+5], label=performance[1], linewidth=5, linestyle=':', marker=mark[1], markersize=15)
        plt.plot(data[title[2]][j:j+5], data[title[5]][j:j+5], label=performance[2], linewidth=5, alpha=0.5, marker=mark[2], markersize=15)
        
        plt.title(algorithmName[i] + " : " + dataName[k], fontsize=36, fontweight='bold')
        plt.xlabel("The number of frames", fontsize=24, fontweight='bold')
        plt.ylabel("Values", fontsize=24, fontweight='bold')
        plt.xlim(20, 100)
        plt.ylim(0)
        plt.legend()
        plt.grid(True)
        
        fig = plt.gcf()
        fig.set_size_inches(16, 12)
        k += 1
        fig.savefig("img/" + algorithmName[i] + str(k) + '.jpg', dpi=100)
        plt.clf()

# 2. The plots between performance and frames each data.
FIFO = pd.read_csv(algorithmName[0] + ".csv")
ARB = pd.read_csv(algorithmName[1] + ".csv")
ESC = pd.read_csv(algorithmName[2] + ".csv")
LRULFU = pd.read_csv(algorithmName[3] + ".csv")


for i in range(3, 6, 1) : 
    for j in range(0, 15, 5) :
        plt.plot(FIFO[title[2]][j:j+5],    FIFO[title[i]][j:j+5], label=algorithmName[0], linewidth=5, linestyle='--', marker=mark[0], markersize=15)
        plt.plot(ARB[title[2]][j:j+5],     ARB[title[i]][j:j+5], label=algorithmName[1], linewidth=5, linestyle=':', marker=mark[1], markersize=15)
        plt.plot(ESC[title[2]][j:j+5],     ESC[title[i]][j:j+5], label=algorithmName[2], linewidth=5, linestyle='-', marker=mark[2], markersize=15, alpha=0.5)
        plt.plot(LRULFU[title[2]][j:j+5],  LRULFU[title[i]][j:j+5], label=algorithmName[3], linewidth=5, linestyle=':', marker=mark[3], markersize=15, alpha=0.5)
        
        plt.title(dataName[int(j/5)], fontsize=36, fontweight='bold')
        plt.xlabel("The number of frames", fontsize=24, fontweight='bold')
        plt.ylabel("The number of " + performance[i-3].lower(), fontsize=24, fontweight='bold')
        plt.xlim(20, 100)
        #plt.ylim(0)
        plt.legend()
        plt.grid(True)
        
        fig = plt.gcf()
        fig.set_size_inches(16, 12)
        fig.savefig("img/" + dataName[int(j/5)] + str(i-1) + '.jpg', dpi=100)
        plt.clf()
        # print(str(int(j/5+1)) + " " +str(i-1))