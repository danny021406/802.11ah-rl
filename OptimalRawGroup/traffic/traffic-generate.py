import numpy as np

traffic = 0.01
staNumber = 132
a = np.random.random(staNumber)
a /= a.sum()
a *= traffic
su = 0

lines = []
for i in range(a.size):
    string = str(i) + " " + str(a[i]) + "\n"
    su += a[i]
    lines.append(string.encode())

print(su)
fp = open('data-' + str(a.size) + '-' + str(traffic) + '.txt','wb')

fp.writelines(lines)


fp.close()