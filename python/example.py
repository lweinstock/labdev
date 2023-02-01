#!/usr/bin/env python3

import pylabdev as pld
from time import sleep

# Setup axis
ipx = pld.ip_address("192.168.2.100", pld.xenax_xvi.dflt_port)
x_axis = pld.xenax_xvi(ipx)
x_axis.power_on()

# Setup laser distance sensor
iplds = pld.ip_address("192.168.2.250", pld.om70_l.dflt_port)
lds = pld.om70_l(iplds)

# Setup dispenser
ser = pld.serial_config("/dev/ttyUSB0", 38400)
disp = pld.ml_808gx(ser)
disp.select_channel(1)
disp.timed_mode()
p, dur, on, off = disp.get_channel_params()
print(f'{p}, {dur}, {on}, {off}')

# All axes have to be referenced
if not x_axis.is_referenced():
    x_axis.reference_axis()

# Configure axis
x_axis.set_speed(25000)     # ~25000um/s

# Blocking movement of single axis
x_axis.goto_position(170000)

disp.dispense()

# Non-blocking movement
x_axis.move_position(140000)
while not x_axis.in_position():
    cur_pos = x_axis.get_position()
    force = x_axis.get_motor_current()
    distance = lds.get_distance()
    print(f'x = {cur_pos}\td = {distance}\ti = {force}')
    sleep(0.1)

disp.dispense()

print("Done!")
x_axis.power_off()