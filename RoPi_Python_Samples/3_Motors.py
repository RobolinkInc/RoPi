import RoPi_SerialCom as ropi
#this program will show you how you
#how to use the motors and set the speed.
import time

print("speed 80%")
ropi.speed(80)
ropi.moveForwards()
time.sleep(2)

print("speed 50%")
ropi.speed(50)
ropi.moveForwards()
time.sleep(2)

print("speed 30%")
ropi.speed(30)
ropi.moveForwards()
time.sleep(2)

ropi.moveStop()
