Utilizing this library requires a few implementations from the user.
The general format for receiving ASYNC ASCII packets is to generate a write command that
will be sent to the sensors, then await to receive a response from the sensor that contains
the desired information. Then call the implmented parsing function that will parse the packet
for you.

**To try this example, follow these steps:** 

1. Change to the top-level directory (i.e. `OBC-firmware/`)

2. Run `make -f adcs/VN_100_driver/Makefile` to build
    - To delete the build files, run `make -f adcs/VN_100_driver/Makefile clean`
3. Open a serial terminal
    - For Windows: Try [Putty](https://www.putty.org/)
    - For Linux: Try [this](https://www.cyberciti.biz/faq/find-out-linux-serial-ports-with-setserial/)
4. Find the COM port identified as `XDS Class Application/User UART`
5. Set the serial port settings:
    - COM Port: What you found above
    - Baud Rate: 9600
    - Data Bits: 8
    - Stop Bits: 2
    - Parity: None
6. Start the serial terminal
7. Open Uniflash and flash `acds/VN_100_driver/build/vector_nav_example.out` onto the LaunchPad
8. View User LED A blinking on the board and `Hello World!` being printed to the serial terminal