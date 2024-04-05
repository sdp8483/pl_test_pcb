#!/usr/bin/python

Import("env") # type: ignore

import os

try:
    import bincopy
except ImportError:
    env.Execute("$PYTHONEXE -m pip install bincopy") # type: ignore
    import bincopy

def after_hex_build(source, target, env):
    print("\nFormatting firmware.hex for standalone programmer")

    # get output file name
    unit_name = ''  # empty incase UNIT_NAME is not found
    fw_version = '' # empty incase FW_VERSION is not found
    for item in env['CPPDEFINES']:
        if item[0].strip() == 'UNIT_NAME':
            unit_name = item[1].strip().replace("\\\"", '')
        if item[0].strip() == 'FW_VERSION':
            fw_version = item[1].strip().replace("\\\"", '')

    input_hex_file = str(target[0])
    output_hex_file = str(target[0]).replace(".hex", "_{}_{}.hex".format(unit_name, fw_version))

    # remove old file
    if os.path.exists(output_hex_file):
        os.remove(output_hex_file)

    binfile = bincopy.BinFile(input_hex_file)   # open hex file
    binfile.fill(value=b'\xff', max_words=16)   # fill empty space between segments

    # pad until max_address is a multiple of 16
    while(binfile.maximum_address % 16):
        binfile.add_binary(b'\xff', address=binfile.maximum_address)
    
    with open(output_hex_file, 'w') as f:
        f.write(binfile.as_ihex(number_of_data_bytes=16, address_length_bits=16))

    print("Formated hex file: ", output_hex_file)

# script
env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", after_hex_build) # type: ignore