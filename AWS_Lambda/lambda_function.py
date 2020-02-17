#
# Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#

# greengrassHelloWorld.py
# Demonstrates a simple publish to a topic using Greengrass core sdk
# This lambda function will retrieve underlying platform information and send
# a hello world message along with the platform information to the topic 'hello/world'
# The function will sleep for five seconds, then repeat.  Since the function is
# long-lived it will run forever when deployed to a Greengrass core.  The handler
# will NOT be invoked in our example since the we are executing an infinite loop.

import greengrasssdk
import pexpect
from threading import Timer
import time

DEVICE = "D8:80:39:F3:75:59"

# Creating a greengrass core sdk client
client = greengrasssdk.client('iot-data')

child = pexpect.spawn("gatttool -I")
child.sendline("sec-level medium")
child.sendline("connect {0}".format(DEVICE))
child.expect("Connection successful", timeout=5)

# When deployed to a Greengrass core, this code will be executed immediately
# as a long-lived lambda function.  The code will enter the infinite while loop
# below.
# If you execute a 'test' on the Lambda Console, this test will fail by hitting the
# execution timeout of three seconds.  This is expected as this function never returns
# a result.
def AVR_BLE_DATA():
	child.sendline("char-read-hnd 0x72")
	child.expect("Characteristic value/descriptor: ", timeout=10)
	child.expect("\r\n", timeout=10)
	if child.before:
		pressureData = "Pressure: " + str(child.before.replace(" ","")).lstrip('0') + " Pa"
		client.publish(topic='BLE/data', payload=pressureData)

	child.sendline("char-read-hnd 0x75")
	child.expect("Characteristic value/descriptor: ", timeout=10)
	child.expect("\r\n", timeout=10)
	if child.before:
		temperatureData = "Temperature: " + str(child.before.replace(" ", "")[:4]).lstrip('0') + "." + str(child.before.replace(" ", "")[6:]) + " C"
		client.publish(topic='BLE/data', payload=temperatureData)
	
	child.sendline("char-read-hnd 0x78")
	child.expect("Characteristic value/descriptor: ", timeout=10)
	child.expect("\r\n", timeout=10)
	if child.before:
		humidityData = "Humidity: " + str(child.before.replace(" ", "")[:4]).lstrip('0') + "." + str(child.before.replace(" ", "")[6:]) + "%"
		client.publish(topic='BLE/data', payload=humidityData)
	Timer(5, AVR_BLE_DATA).start()

AVR_BLE_DATA()
	
# This is a dummy handler and will not be invoked
# Instead the code above will be executed in an infinite loop for our example
def function_handler(event, context):
	return