#!/usr/bin/env python
# coding: utf-8

def file_management():
    
    os.system("rm info.txt")  
    os.system("rm OptimalRawGroup/RawConfig-finding.txt")

    os.system("touch info.txt")  
    os.system("touch OptimalRawGroup/RawConfig-finding.txt")


def write_file(reward, file):
    fp = open(file,'a')
    string = str(float(reward)) + '\n'
    fp.write((string))
    fp.close()


# In[2]:


def init_interval(s_size, traffic_interval, file_name):
    
    os.system("rm freq.txt")  
    os.system("touch freq.txt")  
    state = []
    for i in range(Nsta):
        rnd = random.uniform(1, traffic_interval)
        state.append(rnd)
        fp = open('freq.txt','a')
        string = str(int(rnd))+ '\n'
        fp.write((string))
        fp.close()
    
    os.system('cp freq.txt ' + file_name)


def collect_data(step_round, step, Nsta, s_size, traffic_interval, file_name):
    action_count = 0
    file_management()
    for i in range(step_round):
        for j in range(step_round):
            
            if(j<=i): 
                continue
            first_slide = (i+1) * step
            second_slide = (j+1) * step
            if(second_slide >= Nsta):
                continue
               
            line = "1\n3\n0 1 1 209 2 0 "
            line += (str(1) + " " + str(first_slide) + "\n")
            line += "0 1 1 209 2 0 "
            line += (str(first_slide+1) + " " + str(second_slide) + "\n")
            line += "0 1 1 209 2 0 "
            line += (str(second_slide+1) + " " + str(Nsta) + "\n")

            fp = open('OptimalRawGroup/RawConfig-finding.txt','wb')
            fp.write(bytes(line.encode()))
            fp.close()

            os.system(" ./waf --run \"test --simulationTime=5 --payloadSize=256 --RAWConfigFile=OptimalRawGroup/RawConfig-finding.txt \"")  

            f = open("info.txt")
            line = f.readline()
            seperate_lines = line.split()
            reward = 100 - float(seperate_lines[9])
            
            write_file(reward, file_name)
                



import gym
import tensorflow as tf
# import tensorflow.contrib.slim as slim
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from tensorflow import keras
import matplotlib as mpl
import random
import os


times = 2000

Nsta = 125
step = 10
traffic_interval = 10000
step_round = int(Nsta / step)


s_size = Nsta
a_size = 0


for i in range(step_round):
    for j in range(step_round):
        if(j<=i): 
            continue
        first_slide = (i+1) * step
        second_slide = (j+1) * step
        if(second_slide >= Nsta):
            continue
        a_size = a_size + 1

os.system("rm training_info.txt")
os.system("touch training_info.txt")  

time_history = []
rew_history = []

for time in range(times):
    
    state_file_name = 'training_data/state_' + str(time) + '.txt'
    os.system('touch ' + state_file_name)
    init_interval(s_size, traffic_interval, state_file_name)

    # Step
    reward_file_name = 'training_data/reward_' + str(time) + '.txt'
    os.system('touch ' + reward_file_name)
    collect_data(step_round, step, Nsta, s_size, traffic_interval, reward_file_name)
        
        

