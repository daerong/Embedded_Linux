#!/bin/sh
tftp -r fpga_fnd_driver.ko -g 192.168.1.70
tftp -r fpga_led_driver.ko -g 192.168.1.70
tftp -r fpga_push_switch_driver.ko -g 192.168.1.70
tftp -r fpga_dot_driver.ko -g 192.168.1.70
tftp -r fpga_text_lcd_driver.ko -g 192.168.1.70
tftp -r fpga_step_motor_driver.ko -g 192.168.1.70
tftp -r fpga_buzzer_driver.ko -g 192.168.1.70
tftp -r fpga_dip_switch_driver.ko -g 192.168.1.70
tftp -r sr04_driver.ko -g 192.168.1.70