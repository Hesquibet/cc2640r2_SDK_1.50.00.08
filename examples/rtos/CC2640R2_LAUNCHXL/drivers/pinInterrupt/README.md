---
# pinInterrupt

---

## Example Summary

Application that toggles board LEDs using a Pin interrupt.

## Peripherals Exercised

* `Board_PIN_LED0` - Toggled by Board_PIN_BUTTON0
* `Board_PIN_LED1` - Toggled by Board_PIN_BUTTON1
* `Board_PIN_BUTTON0` - Toggles Board_PIN_LED0
* `Board_PIN_BUTTON1` - Toggles Board_PIN_LED1

## Resources & Jumper Settings

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in
your project directory for resources used and board-specific jumper settings.
Otherwise, you can find Board.html in the directory
&lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.


## Example Usage

Run the example.

1. `Board_PIN_LED0` is toggled by pushing `Board_PIN_BUTTON0`.

2. `Board_PIN_LED1` is toggled by pushing `Board_PIN_BUTTON1`.

* Each key's interrupt is configured for a falling edge of the pulse. A small
delay has been added to provide button de-bounce logic.

## Application Design Details

* The `buttonCallbackFxn` function is defined in the *pinInterrupt.c* file.
This function is called in the context of the pin interrupt (Hwi).

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

IAR:

*When using any SensorTag(STK) Board, the XDS110 debugger must be
selected with the 4-wire JTAG connection within your projects' debugger
configuration.

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
