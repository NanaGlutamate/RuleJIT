from pylab import mpl
from matplotlib import pyplot as plt
import numpy as np

mpl.rcParams['font.sans-serif'] = ['SimHei']

data1 = [0.000151167, 0.000142042, 0.000133556, 0.000143708, 0.000139283, 0.000193583, 0.000217762, 0.00022375, 0.000205278,
         0.000148642, 0.000147621, 0.000140042, 0.000138705, 0.000178423, 0.0001881, 0.000150583, 0.000170608, 0.000169954, 0.000144561,]
data2 = [1.26667e-05, 9.66667e-06, 9.69444e-06, 9.29167e-06, 9.36667e-06, 9.34722e-06, 9.95238e-06, 1.12396e-05, 1.08426e-05,
         9.45833e-06, 9.25e-06, 9.51389e-06, 9.45513e-06, 1.45238e-05, 1.83889e-05, 1.78594e-05, 1.88186e-05, 1.88056e-05, 1.75263e-05,]
width = 0.4

x = np.arange(1, 20)
plt.bar(x + width/2, data1, width, align='center',
        tick_label=[str(i*10) for i in x], label='解释引擎')
plt.bar(x - width/2, data2, width, align='center',
        tick_label=[str(i*10) for i in x], label='代码生成引擎')
plt.xlabel('数据量/条')
plt.ylabel('平均每Tick耗时/ms')
plt.legend()
plt.show()
