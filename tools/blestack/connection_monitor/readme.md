# Setup and Users Guide For: npi_cm_host.py

This brief readme will walk through the setup and use of the **npi_cm_host.py**
script.

### Required Python Modules

In order to utilize this script you will need a valid installation of
* [Download and install Python 2.7](https://www.python.org/download/releases/2.7/)
* Install pyserial with the following command "pip install -r requirements.txt" made the ../tools/blestack/connection_monitor/ directory.

This should be all you need to successfully run this script.

### Required TI BLE Devices

This script expect you to use cc2640R2 Launch Pads with their standard usb to
UART cable. Each launch pad should enumerate 2 comports in your windows device manager.
The one tagged with the name **User UART** will be the one you will use to communicate
with this script.

Only two types of TI devices are expected for use with this script. A BLE central
device with HCI communication. This role will be filled with the **..\ble\examples\rtos\CC2640R2_LAUNCHXL\blestack\host_test**
application found in the TI BLE SDK.

 The other device will be **..\ble\examples\rtos\CC2640R2_LAUNCHXL\blestack\micro_ble_test**. This will be the actual connection monitor which will demonstrate its ability to follow an active BLE connection and report RSSI and timestamps for 1 master and 1 slave packet from each connection interval.

 A third device will be used as a BLE slave to form a connection with the host_test central but it need not be connected or controlled from the script. This demo script expects the user to compile a device as a *simple_peripheral*. The user should flash a launch pad with this project found here **..\ble\examples\rtos\CC2640R2_LAUNCHXL\blestack\simple_peripheral**.

 ### Running the Script Manually with No Arguments

 Once we have all 3 devices you will need to make some modifications to enter their information into the python script.

 1. Update host_port with your UART comport for the **host_test** loaded launch pad. Do the same for the connection monitor device comport. And also copy in the ble address of your simple_peripheral.

 ```python
 if __name__ == '__main__':
     global peerAddrs
     version_control()
     rtn_args = parse_args()

     if(len(sys.argv) < 4):
         print '\nUsing Default Hardcoded Values: '
         print 'or enter 3 default(cm_port host_port peerAddress) arguments.'
         print 'See -h help.\n'
         #set your own hard coded com ports
         host_port = 'COM128'
         cm_port     = 'COM158'
         strPeerAddress = '0xB0912265CBE2'
 ```
 2. With your **simple_peripheral** powered on. Open up a Putty or other serial terminal with speed set to 115200 and Serial line set to the devices comport. Reseting the device by pressing the reset button on the launch pad should display the device as advertising and show you its ble address.

 a. The ble address displayed here must be manually added to our python script to all the ble central device to join it without having to scan for its advertisements.

 ```python
 def main():
   global cmInfoOutPut
   #set of peripheral devices to join to. BLE address should be fixed for a given launchpad
   #You will need to update with your own peripheral device addresses since it is unique
   #for every device.
   peerAddrs1 = [0x20,0x4d,0xaa,0x2d,0x07,0x98]
 ```
b. The ble address display in Putty will need to be entered into the above array in reverse octet order. So for the example above the display window showed **0x98072daa4d20**. That address is entered in reverse in the array above.

3. Finally after all these steps are done you should simply be able to run the script. It will first join the ble central device to the peripheral. Then it will query for the current connection parameters. Finally it will launch the connection monitor with those parameters. You should shortly see a continues stream of RSSI data from the ble master and slave devices at whatever connection interval the ble devices are operating at. The default connection interval out of the box will be 100ms.

### Running the Python Script or Executable with Command Line arguments

If you would like to run the script without editing the python code as is, you can enter the connection monitor comport, host_test comport and simple peripheral ble address all at the command line as in the following example.
```command
$ROOT_DIR$\ble\tools\blestack\connection_monitor\bin\windows>npi_cm_host.exe COM4 COM5 0xB0912265CBE2
```

Or alternatively the same command line will also work with a direct call to the python script.
```command
$ROOT_DIR$\ble\tools\blestack\connection_monitor\>npi_cm_host.py COM4 COM5 0xB0912265CBE2
```
This program is setup to have host_test connect to the simple_peripheral. Out of the box the connection interval should be 100ms. One second later the program will kick off the connection monitor and you should being to see a steady stream of master and slave packets id as part of connection '1' with an RSSI value relative to the connection monitor as well as a timestamp.

### Summary

This concludes the brief getting started readme. For further details and possible issues with the script please refer to the inline comments in the python script. This should convey the basic serial port parsing logic as well as basic setup options that may need further adjustment.
