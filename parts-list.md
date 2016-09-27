
#Parts:

[GSM SIM Card: $9 ($6 per device per month + 1 cent a MB)]: https://www.adafruit.com/products/2505
[LiPo Battery for Cell Modem: $7.95]: https://www.adafruit.com/products/1578
[Antenna: $2.95]: https://www.adafruit.com/product/1991
[Cell Modem (No GPS): $39.95]: https://www.adafruit.com/products/2468
[Cell Modem (GPS): $49.95]: https://www.adafruit.com/products/2636
[LCD Display: $19.95]: https://www.adafruit.com/products/772
[Vibrating Motor: $1.95]: http://www.digikey.com/product-detail/en/1201/1528-1177-ND/5353637?WT.mc_id=IQ_7595_G_pla5353637&wt.srch=1&wt.medium=cpc&&gclid=CJDc6c7yq88CFQJZhgod4WQCMQ
[Arduino Uno: $24.95]: https://www.adafruit.com/products/50

Total No GPS: 106.70
Total GPS: 116.70

Assuming we built our own PCBs we could probably get this price down to $10-20 for a unit.

Estimating data usage:

IP Header Size: ~20 bytes
HTTP Header Size: ~200 bytes
HTTP body size: ~100 bytes

GET Request from the unit to the server: ~220 bytes
Response from server to unit: ~320 bytes
POST request from the unit to the server: ~220 bytes
POST acknowledge from the server to the unit: ~250 bytes (assuming that the ack from the server is something small)
 

#Random Forum Posts that Could be Helpfuler
[POST AT Command]: http://stackoverflow.com/questions/33346425/sim800-at-command-post-data-to-server
[SIM800 AT Commands]: https://cdn-shop.adafruit.com/datasheets/sim800_series_ip_application_note_v1.00.pdf
[Arduino Library for FONA 800]: https://github.com/adafruit/Adafruit_FONA/blob/master/Adafruit_FONA.cpp
[Location Tracker]: http://www.instructables.com/id/How-to-make-a-Mobile-Cellular-Location-Logger-with/?ALLSTEPS
[How to use Arduino Ethernet Shield]: https://www.arduino.cc/en/Tutorial/WebClient
[Data Sheet for FONA 800]: https://learn.adafruit.com/adafruit-fona-mini-gsm-gprs-cellular-phone-module/downloads
[LCD Shield Wiring]: https://learn.adafruit.com/assets/1439
[Arduino CMake]: https://github.com/queezythegreat/arduino-cmake#arduino-sketches
[Sample Arduino Project]: https://github.com/ladislas/Bare-Arduino-Project
[Cell+GPS Arduino]: https://learn.adafruit.com/adafruit-fona-808-cellular-plus-gps-shield-for-arduino
[Ting Rates]: https://ting.com/rates?ab=1


