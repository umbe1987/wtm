#!/bin/bash

# use DEVKITSMS_PATH from environment, or use default of ../lib/devkitsms
DEVKITSMS_PATH="${DEVKITSMS_PATH:-../lib}"

# ensure the target folder exists
mkdir -p ./build
cd ./build || exit
python $DEVKITSMS_PATH/assets2banks.py ../assets
sdcc -c -mz80 -I$DEVKITSMS_PATH bank2.c --debug
sdcc -c -mz80 -I$DEVKITSMS_PATH ../main.c --debug
sdcc -o wtm.ihx -mz80 --data-loc 0xC000 --no-std-crt0 $DEVKITSMS_PATH/crt0_sms.rel main.rel SMSlib.lib PSGlib.lib bank2.rel --debug
ihx2sms wtm.ihx wtm.sms

