# CACRS-on-STM32F4简介
基于 STM32 平台和数字滤波算法，设计了实时监测使用者心率、体温和运动数据的无线运动数据采集系统。该系统以屏幕和按键进行交互，采
用24bits模拟前端芯片ADS1292R设计心电信号采集电路，利用FIR带通滤波器去除基线漂移和肌电信号干扰，获得稳定可靠的心电信号，通过阈值分
割法测量 R 波周期，计算出精确的心率。并对得到的数据进行监督式学习算法进行对心电图信号的标签分类，再根据数据集的心电图数据进行对比分析，
得到较为精准的疾病信息或者心肌状态。

采用24bits模拟前端芯片ADS1292R设计心电信号采集电路，利用FIR带通滤波器去除基线漂移和心肌电信号干扰，获得稳定可靠的心电信号，通过阈值分割法测量R波周期，计算出精确的心率。 

## 硬件
STM32F429/407, ADS1292R，MPU6050，LCD屏幕，Max-bit（K210版）
<br><br><br>



## 软件
基于C语言的hal库构建，使用STM32CubeMX初始化操作
![](https://github.com//liji3597/CACRS-on-STM32F4/raw/main/planoverview/1.png)   
<br><br><br>


## 心电图示例
![](https://github.com//liji3597/CACRS-on-STM32F4/raw/main/planoverview/2.png)
