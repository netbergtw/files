#!/bin/bash

for variable in $(seq 1 100)
do
./nb_fpga QSFP_w 32 0xe2 0x33
sleep 0.001
./nb_fpga QSFP_w 32 0xe3 0x22
sleep 0.001
./nb_fpga QSFP_r 32 0xe2
sleep 0.001
./nb_fpga QSFP_r 32 0xe3

done
