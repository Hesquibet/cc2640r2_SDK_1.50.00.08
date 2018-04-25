---
# nvs

---

## Example Summary

Example to read and write data into non-volatile storage using the NVS driver.

##Peripherals Exercised

* `Board_UART0` - Used for console output.

## Resources & Jumper Settings

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in
 your project directory for resources used and board-specific jumper settings.
 Otherwise, you can find Board.html in the directory
 &lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.

## Example Usage

* Example output is generated through use of Display driver APIs. Refer to the
Display driver documentation found in the  SimpleLink MCU SDK User's Guide.

* Open a serial session (e.g. [`PuTTY`](http://www.putty.org/ "PuTTY's
 Homepage"), etc.) to the appropriate COM port.
 *The COM port can be determined via Device Manager in Windows or via
 `ls /dev/tty*` in Linux.

The connection will have the following settings:
```
    Baud-rate:     115200
    Data bits:          8
    Stop bits:          1
    Parity:          None
    Flow Control:    None
```

* Run the example.

* The example checks if the signature, "SimpleLink SDK Non-Volatile Storage
 (NVS) Example" is saved in flash.

 * If the signature is present in flash, the signature is output to the UART
 and then the flash region is erased. Otherwise, the application will write the
 signature to flash so that it will be detected after the next reset.

 * When prompted, reset the device. This will cause the application to
 start over.

The following is example output.
```
    Region Base Address: 0x1B000
    Sector Size: 0x1000
    Region Size: 0x4000

    Writing signature to flash...
    Reset the device.
    ==================================================
```
After device reset:
```
    Region Base Address: 0x1B000
    Sector Size: 0x1000
    Region Size: 0x4000

    SimpleLink SDK Non-Volatile Storage (NVS) Example.
    Erasing flash...
    Reset the device.
    ==================================================
```

## Application Design Details

* The application utilizes a single thread to demonstrate using the
 non-volatile storage.

* A block of memory the size of `signature` bytes is read from flash and
copied into `buffer`. The contents are then compared to `signature`. If equal,
the application displays the signature as stored in flash and erases the
flash. Otherwise, `signature` is written to flash.

* At the end of execution, the application prompts the user to reset the
device. Upon a reset, the processor will restart. The application will
automatically start over.

* `NVS_getAttrs()` will populate a `NVS_Attrs` struct with properties specific
to a `NVS_Handle` such as region base address, size and sector size.
```C
NVS_getAttrs(nvsHandle, &regionAttrs);
```

* This application demonstrates how to use the `NVS_read()`, `NVS_write()` and
`NVS_erase()` APIs with appropriate offsets.

* The `NVS_WRITE_ERASE` flag is used with the `NVS_write()` API to ensure the
flash region is erased prior to performing a write. This flag will erase
`NVS_Attrs.sectorSize` bytes of memory.

TI-RTOS:

* When building in Code Composer Studio, the kernel configuration project will
be imported along with the example. The kernel configuration project is
referenced by the example, so it will be built first. The "release" kernel
configuration is the default project used. It has many debug features disabled.
These feature include assert checking, logging and runtime stack checks. For a
detailed difference between the "release" and "debug" kernel configurations and
how to switch between them, please refer to the SimpleLink MCU SDK User's
Guide. The "release" and "debug" kernel configuration projects can be found
under &lt;SDK_INSTALL_DIR&gt;/kernel/tirtos/builds/&lt;BOARD&gt;/(release|debug)/(ccs|gcc).

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
