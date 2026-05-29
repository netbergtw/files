#!/bin/bash

# Get latest transceiver temperature on all ports and 
# sort the highest one to update it to TEMP_OPTICS

MAX_HEX="80" # -128
FOUND=0

for i in {17..48}
do
    EEPROM_PATH="/sys/bus/i2c/devices/${i}-0050/eeprom"
    
    if [ ! -f "$EEPROM_PATH" ]; then
        continue
    fi

    if BIN=$(dd if="$EEPROM_PATH" bs=1 skip=22 count=1 status=none 2>/dev/null); then
        HEX=$(echo -n "$BIN" | od -An -t x1 | tr -d ' \n')
        [ -z "$HEX" ] && continue

        CUR_RANK=$(( (0x$HEX + 128) % 256 ))
        MAX_RANK=$(( (0x$MAX_HEX + 128) % 256 ))

        if [ "$FOUND" -eq 0 ] || [ "$CUR_RANK" -gt "$MAX_RANK" ]; then
            MAX_HEX=$HEX
            FOUND=1
        fi
    fi
done

if [ "$FOUND" -eq 1 ]; then
    ipmitool raw 0x3c 0x20 0x0 0x0 0x${MAX_HEX} 0x1
fi

