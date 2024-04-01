import numpy as np
from matplotlib import pyplot as plt
from pylab import mpl

plt.rcParams['font.sans-serif'] = ['SimHei']

"""car1v1"""
datac1 = [0.1872,0.15712,0.150293,0.15328,0.160096,]
datac2 = [0.01368,0.01232,0.0128,0.0125,0.0124,]

"""air2v2"""
dataa1 = [0.271326,0.245251,0.247909,0.244892,0.244982,]
dataa2 = [0.0113799,0.0113799,0.0114695,0.0115367,0.011595,]
width = 0.4

x = np.arange(1, 6)

ax1 = plt.subplot(1, 2, 2)
ax1.set_ybound(0, 0.00045)
ax1.bar(x + width/2, datac1, width, align='center', #edgecolor='black', color='white', hatch='//',
        label='解释引擎')
ax1.bar(x - width/2, datac2, width, align='center', #edgecolor='black', color='white', hatch='xx',
        label='代码生成引擎')
ax1.set_xticks(x)
ax1.set_xticklabels([str(i*1250) for i in x])
ax1.set_xlabel('数据量/条')
ax1.set_ylabel('平均每Tick耗时/ms')
ax1.legend()
ax1_ = ax1.twinx()
ax1_.plot(x, [i / j for i, j in zip(datac1, datac2)], 'o-', color='black')
ax1_.set_ylabel('加速比')
ax1.set_title('陆战一对一测试场景')

ax1 = plt.subplot(1, 2, 1)
ax1.bar(x + width/2, dataa1, width, align='center', #edgecolor='black', color='white', hatch='//',
        label='解释引擎')
ax1.bar(x - width/2, dataa2, width, align='center', #edgecolor='black', color='white', hatch='xx',
        label='代码生成引擎')
ax1.set_xticks(x)
ax1.set_xticklabels([str(i*1250) for i in x])
ax1.set_xlabel('数据量/条')
ax1.set_ylabel('平均每Tick耗时/ms')
ax1.legend()
ax1_ = ax1.twinx()
ax1_.plot(x, [i / j for i, j in zip(dataa1, dataa2)], 'o-', color='black')
ax1_.set_ylabel('加速比')
plt.title('空战二对二测试场景')

plt.show()
