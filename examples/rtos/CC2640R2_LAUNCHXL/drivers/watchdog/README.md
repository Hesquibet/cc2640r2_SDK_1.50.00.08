## Example Summary

This application demonstrates how to use the Watchdog driver to call a
function on expiration of the watchdog timer.

## Peripherals Exercised

* `Board_PIN_LED0`      - Indicator LED
* `Board_PIN_BUTTON0`   - Used to control the application
* `Board_WATCHDOG0` - Timer to trigger the watchdog interrupt service routine

## Resources & Jumper Settings

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in
your project directory for resources used and board-specific jumper settings.
Otherwise, you can find Board.html in the directory
&lt;SDK_INSTALL_DIR&gt;/source/ti/boards/&lt;BOARD&gt;.


## Example Usage

* The application's `mainThread()` opens a Watchdog driver object.
When serviceFlag is set, the thread will continously call `Watchdog_clear()`.
If a button is pressed, serviceFlag is cleared, and the watchdog timer
will expire and call `watchdogCallback()`.

## Application Design Details

* The application turns OFF `Board_PIN_LED0` during startup. The
serviceFlag is true by default, so `Watchdog_clear()` is called continously
in the loop at the end of mainThread.

* When `Board_PIN_BUTTON0` is pressed the serviceFlag is cleared, so that
the mainThread stops continously calling Watchdog_clear(). When the
watchdog expires, watchdogCallback() is triggered, and the function sets a
watchdogExpired flag.  Back in the continous loop in mainThread, when this
expired flag is detected `Board_PIN_LED0` is turned ON, and the
thread sleeps for 5 seconds.  When mainThread resumes, flags are reset to
resume calls to Watchdog_clear(), and `Board_PIN_LED0` is turned OFF again,
until the next button press and watchdog expiration.

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
