rm ../../bin/er9x-64.hex
make clean
make EXT=FRSKY TEMPLATES=NO HELI=NO
mv er9x.hex ../../bin/er9x-64.hex
