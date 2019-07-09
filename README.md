# mpf4bbram
## NOTE: minimally tested!
Use the battery-backed RAM and registers in STM32F4xx

### Usage

#### Initialization
One-time initialization in boot.py or main.pyswitch power supply to battery-backed SRAM.
```
import mpf4bbram
mpf4bbram.init()
```
#### Use in application script
Enable power interface clock, then enable BBRAM interface clock
and finally, enable access to RTC domain by setting the DBP bit
(Disable Backup Protection).
```
mpf4bbram.enable()
mpf4bbram.write32_bbram (index, data)
```
index is the 32-bit offset into the  BKPSRAM space (0x4002 4000 - 0x4002 4FFF).
data is a 32-bit data

#### Disable accidental writes
Can enable again when required.
```
mpf4bbram.disable()
```


#### Kill BKPSRAM power
This removes battery power from the backup SRAM and if main power
is removed, will erase the data stored in backup SRAM.
```
mpf4bbram.kill()
```
#### Compiling MicroPython
```
make  BOARD=BLACK_F407VE  USER_C_MODULES=. CFLAGS_EXTRA="-DBARE_M=1 -DMODULE_MPF4BBRAM_ENABLED=1"
```
