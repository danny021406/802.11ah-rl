#!/usr/bin/env python
# coding: utf-8

def file_management():
    
    os.system("rm info.txt")  
    os.system("rm OptimalRawGroup/RawConfig-finding.txt")

    os.system("touch info.txt")  
    os.system("touch OptimalRawGroup/RawConfig-finding.txt")


def write_file(reward, file):
    fp = open(file,'a')
    string = reward
    fp.write((string))
    fp.close()


def case_config(action):
    if(action == 0):
        return 3, 50
    elif(action == 1):
        return 8, 50
    elif(action == 2):
        return 13, 50
    elif(action == 3):
        return 3, 200
    elif(action == 4):
        return 8, 200
    elif(action == 5):
        return 13, 200
    elif(action == 6):
        return 3, 1000
    elif(action == 7):
        return 8, 1000
    elif(action == 8):
        return 13, 1000

def init_interval(s_size, traffic_interval, file_name):
    
    os.system("rm freq.txt")  
    os.system("touch freq.txt")  
    state = []
    for i in range(Nsta):
        rnd = random.uniform(4000, 6000)
        state.append(rnd)
        fp = open('freq.txt','a')
        string = str(int(rnd))+ '\n'
        fp.write((string))
        fp.close()
    
    os.system('cp freq.txt ' + file_name)


def collect_data(step_round, step, Nsta, s_size, traffic_interval, file_name):
    action_count = 0
    file_management()
    action_space_size = 9
    for i in range(action_space_size):
            
        group, duration = case_config(i) 
        
        step = int(Nsta / group)
        
        line = "1\n" + str(group)
        
        for j in range(group):
            first = j * step + 1
            second = (j+1) * step
            
            if(j == group-1):
                line += "\n0 1 1 " + str(duration) + " 2 0 "
                line += (str(first) + " " + str(Nsta))
            else:
                line += "\n0 1 1 " + str(duration) + " 2 0 "
                line += (str(first) + " " + str(second))

        fp = open('OptimalRawGroup/RawConfig-finding.txt','wb')
        fp.write(bytes(line.encode()))
        fp.close()

        os.system(" ./waf --run \"test --simulationTime=5 --payloadSize=256 --RAWConfigFile=OptimalRawGroup/RawConfig-finding.txt \"")  

        f = open("info.txt")
        line = f.readline()
        seperate_lines = line.split()
        reward = 100 - float(seperate_lines[9])
            
        write_file(line, file_name)
                



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

Nsta = 80
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
        
        

