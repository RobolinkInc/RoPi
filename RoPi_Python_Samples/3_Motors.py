#this program will show you how you
#how to use the motors and set the speed.
import time
import RoPi as rp

ropi = rp.RoPi()

print("speed 80%")
ropi.speed(80)
ropi.moveForward()
time.sleep(2)

print("speed 50%")
ropi.speed(50)
ropi.moveForward()
time.sleep(2)

print("speed 30%")
ropi.speed(30)
ropi.moveForward()
time.sleep(2)

ropi.moveStop()
