## Read_lidar

import numpy as np
import matplotlib.pyplot as plt
import math

import paho.mqtt.client as mqtt
import time
import re


def on_message(client, userdata, message):
	(angle, dist) = [int(x) for x in str(message.payload.decode("utf-8")).split(',')]
	print("New point: " ,str(angle), " ", str(dist))
	
	if(dist != 0):
		x.append(dist * math.cos(math.radians(angle)))
		y.append(dist * math.sin(math.radians(angle)))	
	
	

mqttBroker = "10.0.0.136"

#plt.axis([-2000, 2000, -2000, 2000])

plt.ion()
fig, ax = plt.subplots()
x, y = [],[]
sc = ax.plot(x,y)

plt.draw()

client = mqtt.Client("Debug")
client.connect(mqttBroker)

client.loop_start()

client.subscribe("scan_results")
client.on_message=on_message 

while True:
	#sc.set_offsets(np.c_[x,y])
	plt.plot(y, x, color='blue')
	fig.canvas.draw_idle()
	plt.pause(0.1)
	
client.loop_stop()