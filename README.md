# PhantESPThing
Connecting the Sparkfun ESP8266 Thing to Phant (soldering on optional TMP102 and TSL2561)

To use, put in your phant public and private key for a feed with the following three keys; "name," "temp," and "lux." Optionally, un-comment the line to send battery voltage as well and add the "batt" key. Solder a wire between the reset via in the middle of the board and the XPD pin. On the first run, the ESP8266 thing will go into soft AP mode; connect to it and it will request credentials to get on your network. Once it successfully authenticates, it will continuously update via Phant!