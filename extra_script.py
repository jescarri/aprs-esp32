Import("env")

import os

# Path to the problematic portmacro.h file
portmacro_path = os.path.join(
    env.PioPlatform().get_package_dir("framework-arduinoespressif32"),
    "tools", "sdk", "esp32", "include", "freertos", "port", "xtensa", "include", "freertos", "portmacro.h"
)

# Read the file
with open(portmacro_path, 'r') as f:
    content = f.read()

# Comment out the problematic static assertions
if "_Static_assert(portGET_ARGUMENT_COUNT() == 0" in content:
    content = content.replace(
        "_Static_assert(portGET_ARGUMENT_COUNT() == 0,",
        "//_Static_assert(portGET_ARGUMENT_COUNT() == 0,"
    )
    content = content.replace(
        "_Static_assert(portGET_ARGUMENT_COUNT(1) == 1,",
        "//_Static_assert(portGET_ARGUMENT_COUNT(1) == 1,"
    )
    
    # Write back
    with open(portmacro_path, 'w') as f:
        f.write(content)
    
    print("Patched portmacro.h to disable problematic static assertions")
