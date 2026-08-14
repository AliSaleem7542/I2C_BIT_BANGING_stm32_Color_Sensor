cd Color_sensor_bit_banging

BUILD:

cmake --preset Debug

cmake --build --preset Debug

Flash:

openocd -f openocd/mainboard.cfg -c "program build/Debug/Color_sensor_bit_banging.elf verify reset exit"
