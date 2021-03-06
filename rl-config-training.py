def file_management():
    
    os.system("rm info.txt")  
    os.system("rm OptimalRawGroup/RawConfig-finding.txt")

    os.system("touch info.txt")  
    os.system("touch OptimalRawGroup/RawConfig-finding.txt")
def init_interval(s_size, traffic_interval):
    
    os.system("rm freq.txt")  
    os.system("touch freq.txt")  
    state = []
    for i in range(Nsta):
        rnd = random.uniform(0, traffic_interval)
        state.append(rnd)
        fp = open('freq.txt','a')
        string = str(int(rnd))+ '\n'
        fp.write((string))
        fp.close()

    state = np.reshape(state, [1, s_size])
    
    return state
def env_step(action, step_round, step, Nsta, s_size, traffic_interval):
    action_count = 0
    file_management()
    for i in range(step_round):
        for j in range(step_round):
            if(action_count == action):
                first_slide = (i+1) * step
                second_slide = (j+1) * step

                line = "1\n3\n0 1 1 209 2 0 "
                line += (str(1) + " " + str(first_slide) + "\n")
                line += "0 1 1 209 2 0 "
                line += (str(first_slide+1) + " " + str(second_slide) + "\n")
                line += "0 1 1 209 2 0 "
                line += (str(second_slide+1) + " " + str(Nsta) + "\n")
        #         print (line)

                fp = open('OptimalRawGroup/RawConfig-finding.txt','wb')
                fp.write(bytes(line.encode()))
                fp.close()

                os.system(" ./waf --run \"test --simulationTime=30 --payloadSize=256 --RAWConfigFile=OptimalRawGroup/RawConfig-finding.txt \"")  

                f = open("info.txt")
                line = f.readline()
                seperate_lines = line.split()
                reward = 100 - float(seperate_lines[9])
                
                state = init_interval(s_size, traffic_interval)
                
                return state, reward, 0, 1
            action_count = action_count + 1
                
                
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


total_episodes = 30
max_env_steps = 1

epsilon = 1.0               # exploration rate
epsilon_min = 0.01
epsilon_decay = 0.999

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

        
model = keras.Sequential()
model.add(keras.layers.Dense(s_size, input_shape=(s_size,), activation='relu'))
model.add(keras.layers.Dense(a_size, activation='softmax'))
model.compile(optimizer=tf.keras.optimizers.Adam(0.001),
              loss='categorical_crossentropy',
              metrics=['accuracy'])


time_history = []
rew_history = []

for e in range(total_episodes):
    
    state = init_interval(s_size, traffic_interval)
    
    rewardsum = 0
    
    for time in range(max_env_steps):

        # Choose action
        if np.random.rand(1) < epsilon:
            action = np.random.randint(a_size)
        else:
            action = np.argmax(model.predict(state)[0])

        # Step
        next_state, reward, done, _ = env_step(action, step_round, step, Nsta, s_size, traffic_interval)
        
        
        
        
        
        

        if done:
            print("episode: {}/{}, time: {}, rew: {}, eps: {:.2}"
                  .format(e, total_episodes, time, rewardsum, epsilon))
            break

        next_state = np.reshape(next_state, [1, s_size])

        # Train
        target = reward
        if not done:
            target = (reward + 0.95 * np.amax(model.predict(next_state)[0]))

        target_f = model.predict(state)
        target_f[0][action] = target
        model.fit(state, target_f, epochs=1, verbose=0)

        state = next_state
        rewardsum += reward
        print(reward)
        if epsilon > epsilon_min: epsilon *= epsilon_decay
        
    time_history.append(time)
    rew_history.append(rewardsum)

#for n in range(2 ** s_size):
#    state = [n >> i & 1 for i in range(0, 2)]
#    state = np.reshape(state, [1, s_size])
#    print("state " + str(state) 
#        + " -> prediction " + str(model.predict(state)[0])
#        )

#print(model.get_config())
#print(model.to_json())
#print(model.get_weights())

print("Plot Learning Performance")
mpl.rcdefaults()
mpl.rcParams.update({'font.size': 16})

fig, ax = plt.subplots(figsize=(10,4))
plt.grid(True, linestyle='--')
plt.title('Learning Performance')
plt.plot(range(len(time_history)), time_history, label='Steps', marker="^", linestyle=":")#, color='red')
plt.plot(range(len(rew_history)), rew_history, label='Reward', marker="", linestyle="-")#, color='k')
plt.xlabel('Episode')
plt.ylabel('Time')
plt.legend(prop={'size': 12})

plt.savefig('learning.pdf', bbox_inches='tight')
plt.show()