growboard project
Adafruit Feather Wi-Fi

Author Jacob A. Psimos
------------------------------------------------------------------

- Real Time Clock + SD
- SHT4X hygrometer
- Wi-Fi client with HTTP server supporting basic RESTful API requests

------------------------------------------------------------------

Greenroom companion board that controls relays and takes air quality measurements.
Configuration for the Wi-Fi is in a text file that is suppose to exist on the SD card.
I've included a template config file that should be placed on an SD card formatted with FAT32.
For the sake of accuracy, I've added support for an I2C RTC that allows time keeping and 
file system datetime functionality. Moreover, there is a HTTP server that has a basic HTML
template that I've included for your reading pleasure. Dive into the javascript and C++ code
to see how RESTful GET requests are processed through the client -> middleware.

The Wi-Fi middleware does not automatically re-connect if it looses connection. It *should* be
easy to work in such a feature.

------------------------------------------------------------------

Your Arduino IDE must be configured to use C++17.

Know that in the United States, Wi-Fi channels 12-14 are generally prohibited
and the Atmel BSP *should* be modified to mask out those channels. If you are 
interested in making your BSP compliant, you can contact me at jpsimos@gmail.com
and I will be happy to share the diffs for the Wi-Fi library that disables those channels
or the diffs to enable C++17 for the Feather board.

Happy hacking.
