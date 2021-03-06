{
 "cells": [
  {
   "cell_type": "code",
   "execution_count": 1,
   "metadata": {},
   "outputs": [],
   "source": [
    "def file_management():\n",
    "    \n",
    "    os.system(\"rm info.txt\")  \n",
    "    os.system(\"rm OptimalRawGroup/RawConfig-finding.txt\")\n",
    "\n",
    "    os.system(\"touch info.txt\")  \n",
    "    os.system(\"touch OptimalRawGroup/RawConfig-finding.txt\")\n"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 2,
   "metadata": {},
   "outputs": [],
   "source": [
    "def init_interval(s_size, traffic_interval):\n",
    "    \n",
    "    os.system(\"rm freq.txt\")  \n",
    "    os.system(\"touch freq.txt\")  \n",
    "    state = []\n",
    "    for i in range(Nsta):\n",
    "        rnd = random.uniform(0, traffic_interval)\n",
    "        state.append(rnd)\n",
    "        fp = open('freq.txt','a')\n",
    "        string = str(int(rnd))+ '\\n'\n",
    "        fp.write((string))\n",
    "        fp.close()\n",
    "\n",
    "    state = np.reshape(state, [1, s_size])\n",
    "    \n",
    "    return state"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": 3,
   "metadata": {},
   "outputs": [],
   "source": [
    "def env_step(action, step_round, step, Nsta, s_size, traffic_interval):\n",
    "    action_count = 0\n",
    "    file_management()\n",
    "    for i in range(step_round):\n",
    "        for j in range(step_round):\n",
    "            if(action_count == action):\n",
    "                first_slide = (i+1) * step\n",
    "                second_slide = (j+1) * step\n",
    "\n",
    "                line = \"1\\n3\\n0 1 1 209 2 0 \"\n",
    "                line += (str(1) + \" \" + str(first_slide) + \"\\n\")\n",
    "                line += \"0 1 1 209 2 0 \"\n",
    "                line += (str(first_slide+1) + \" \" + str(second_slide) + \"\\n\")\n",
    "                line += \"0 1 1 209 2 0 \"\n",
    "                line += (str(second_slide+1) + \" \" + str(Nsta) + \"\\n\")\n",
    "        #         print (line)\n",
    "\n",
    "                fp = open('OptimalRawGroup/RawConfig-finding.txt','wb')\n",
    "                fp.write(bytes(line.encode()))\n",
    "                fp.close()\n",
    "\n",
    "                os.system(\" ./waf --run \\\"test --simulationTime=30 --payloadSize=256 --RAWConfigFile=OptimalRawGroup/RawConfig-finding.txt \\\"\")  \n",
    "\n",
    "                f = open(\"info.txt\")\n",
    "                line = f.readline()\n",
    "                seperate_lines = line.split()\n",
    "                reward = 100 - float(seperate_lines[9])\n",
    "                \n",
    "                state = init_interval(s_size, traffic_interval)\n",
    "                \n",
    "                return state, reward, 0, 1\n",
    "            action_count = action_count + 1\n",
    "                \n",
    "                "
   ]
  },
  {
   "cell_type": "code",
   "execution_count": null,
   "metadata": {},
   "outputs": [
    {
     "name": "stdout",
     "output_type": "stream",
     "text": [
      "12.471999999999994\n",
      "1.867999999999995\n",
      "15.873000000000005\n",
      "13.0261\n",
      "4.5411\n",
      "14.936899999999994\n",
      "9.559200000000004\n"
     ]
    }
   ],
   "source": [
    "import gym\n",
    "import tensorflow as tf\n",
    "# import tensorflow.contrib.slim as slim\n",
    "import numpy as np\n",
    "import matplotlib as mpl\n",
    "import matplotlib.pyplot as plt\n",
    "from tensorflow import keras\n",
    "import matplotlib as mpl\n",
    "import random\n",
    "import os\n",
    "\n",
    "\n",
    "total_episodes = 30\n",
    "max_env_steps = 1\n",
    "\n",
    "epsilon = 1.0               # exploration rate\n",
    "epsilon_min = 0.01\n",
    "epsilon_decay = 0.999\n",
    "\n",
    "Nsta = 125\n",
    "step = 10\n",
    "traffic_interval = 10000\n",
    "step_round = int(Nsta / step)\n",
    "\n",
    "\n",
    "s_size = Nsta\n",
    "a_size = 0\n",
    "\n",
    "\n",
    "for i in range(step_round):\n",
    "    for j in range(step_round):\n",
    "        if(j<=i): \n",
    "            continue\n",
    "        first_slide = (i+1) * step\n",
    "        second_slide = (j+1) * step\n",
    "        if(second_slide >= Nsta):\n",
    "            continue\n",
    "        a_size = a_size + 1\n",
    "\n",
    "        \n",
    "model = keras.Sequential()\n",
    "model.add(keras.layers.Dense(s_size, input_shape=(s_size,), activation='relu'))\n",
    "model.add(keras.layers.Dense(a_size, activation='softmax'))\n",
    "model.compile(optimizer=tf.keras.optimizers.Adam(0.001),\n",
    "              loss='categorical_crossentropy',\n",
    "              metrics=['accuracy'])\n",
    "\n",
    "\n",
    "time_history = []\n",
    "rew_history = []\n",
    "\n",
    "for e in range(total_episodes):\n",
    "    \n",
    "    state = init_interval(s_size, traffic_interval)\n",
    "    \n",
    "    rewardsum = 0\n",
    "    \n",
    "    for time in range(max_env_steps):\n",
    "\n",
    "        # Choose action\n",
    "        if np.random.rand(1) < epsilon:\n",
    "            action = np.random.randint(a_size)\n",
    "        else:\n",
    "            action = np.argmax(model.predict(state)[0])\n",
    "\n",
    "        # Step\n",
    "        next_state, reward, done, _ = env_step(action, step_round, step, Nsta, s_size, traffic_interval)\n",
    "        \n",
    "        \n",
    "        \n",
    "        \n",
    "        \n",
    "        \n",
    "\n",
    "        if done:\n",
    "            print(\"episode: {}/{}, time: {}, rew: {}, eps: {:.2}\"\n",
    "                  .format(e, total_episodes, time, rewardsum, epsilon))\n",
    "            break\n",
    "\n",
    "        next_state = np.reshape(next_state, [1, s_size])\n",
    "\n",
    "        # Train\n",
    "        target = reward\n",
    "        if not done:\n",
    "            target = (reward + 0.95 * np.amax(model.predict(next_state)[0]))\n",
    "\n",
    "        target_f = model.predict(state)\n",
    "        target_f[0][action] = target\n",
    "        model.fit(state, target_f, epochs=1, verbose=0)\n",
    "\n",
    "        state = next_state\n",
    "        rewardsum += reward\n",
    "        print(reward)\n",
    "        if epsilon > epsilon_min: epsilon *= epsilon_decay\n",
    "        \n",
    "    time_history.append(time)\n",
    "    rew_history.append(rewardsum)\n",
    "\n",
    "#for n in range(2 ** s_size):\n",
    "#    state = [n >> i & 1 for i in range(0, 2)]\n",
    "#    state = np.reshape(state, [1, s_size])\n",
    "#    print(\"state \" + str(state) \n",
    "#        + \" -> prediction \" + str(model.predict(state)[0])\n",
    "#        )\n",
    "\n",
    "#print(model.get_config())\n",
    "#print(model.to_json())\n",
    "#print(model.get_weights())\n",
    "\n",
    "print(\"Plot Learning Performance\")\n",
    "mpl.rcdefaults()\n",
    "mpl.rcParams.update({'font.size': 16})\n",
    "\n",
    "fig, ax = plt.subplots(figsize=(10,4))\n",
    "plt.grid(True, linestyle='--')\n",
    "plt.title('Learning Performance')\n",
    "plt.plot(range(len(time_history)), time_history, label='Steps', marker=\"^\", linestyle=\":\")#, color='red')\n",
    "plt.plot(range(len(rew_history)), rew_history, label='Reward', marker=\"\", linestyle=\"-\")#, color='k')\n",
    "plt.xlabel('Episode')\n",
    "plt.ylabel('Time')\n",
    "plt.legend(prop={'size': 12})\n",
    "\n",
    "plt.savefig('learning.pdf', bbox_inches='tight')\n",
    "plt.show()"
   ]
  },
  {
   "cell_type": "code",
   "execution_count": null,
   "metadata": {},
   "outputs": [],
   "source": []
  }
 ],
 "metadata": {
  "kernelspec": {
   "display_name": "Python 3",
   "language": "python",
   "name": "python3"
  },
  "language_info": {
   "codemirror_mode": {
    "name": "ipython",
    "version": 3
   },
   "file_extension": ".py",
   "mimetype": "text/x-python",
   "name": "python",
   "nbconvert_exporter": "python",
   "pygments_lexer": "ipython3",
   "version": "3.6.9"
  }
 },
 "nbformat": 4,
 "nbformat_minor": 4
}
