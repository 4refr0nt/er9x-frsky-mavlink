[English](https://code.google.com/p/er9x-frsky-mavlink/) [Russian](https://code.google.com/p/er9x-frsky-mavlink/wiki/Russian)
### About ###

This project contain two firmware: Transmiter firmware (based on [er9x](https://code.google.com/p/er9x/)) and firmware for Arduino Pro Mini (based on [APM-Mavlink-to-FrSky for Taranis](https://github.com/vizual54/APM-Mavlink-to-FrSky)) for transmitting telemetry data from APM via Mavlink protocol to Turnigy 9X(R) display over [FrSky modules](http://www.frsky-rc.com/product/category.php?cate_id=14) supported original FrSky telemetry protocol without 3DR Radio or XBee.

Ardupilot -> Arduino Pro Mini -> FrSky receiver -> er9x-based transmitter
![http://110.imagebam.com/download/366VpSAnf9JgHMUPLp2C1Q/32797/327961161/mavlink-frsky.jpg](http://110.imagebam.com/download/366VpSAnf9JgHMUPLp2C1Q/32797/327961161/mavlink-frsky.jpg)
![http://110.imagebam.com/download/-kdNbIi7uPIv0qLdzbkiIA/32797/327961796/displays.jpg](http://110.imagebam.com/download/-kdNbIi7uPIv0qLdzbkiIA/32797/327961796/displays.jpg)

### Required hardware ###

[Arduino Pro Mini](http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=26869&aff=233755) 5V version recommended, but 3.3V version working well.

[USB->TTL 5V convertor](http://www.ebay.com/sch/i.html?_odkw=usb+ttl&_osacat=0&_from=R40&_trksid=p2045573.m570.l1313.TR12.TRC2.A0.H0.Xusb+ttl+5v&_nkw=usb+ttl+5v&_sacat=0) based on FTDI, PL2303 or any other chip. Required DTR pin for flashing Arduino. (With USB cable)

[USBasp AVR programmer](http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=27990&aff=233755) for flashing Turnigy 9XR

[10 pin to 6 pin adapter for USBasp](http://www.ebay.com/sch/i.html?_odkw=usbasp+programmer&_osacat=0&_from=R40&_trksid=p2045573.m570.l1313.TR0.TRC0.H0.Xusbasp+adapter+6pin&_nkw=usbasp+adapter+6pin&_sacat=0)

[Telemetry cable for APM](http://www.ebay.com/sch/i.html?_odkw=telemetry+cable+for+apm&_osacat=0&_from=R40&_trksid=p2045573.m570.l1313.TR0.TRC0.H0.Xtelemetry+cable+apm&_nkw=telemetry+cable+apm&_sacat=0)

### Tested configuration ###
[Turnigy 9XR Transmitter Mode 2](http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=31544&aff=233755) (or any other based on Turnigy 9X) with FrSky hardware MOD

You can using any tutorial for making this mod [1](http://er9x.googlecode.com/svn/trunk/doc/FrSky%20Telemetry%20%20details.pdf) [2](http://code.google.com/p/gruvin9x/wiki/FrskyInterfacing) [3](http://er9x.googlecode.com/svn/trunk/doc/TelemetryMods.pdf) [4](http://www.flickr.com/photos/erezraviv/5830896454/in/photostream) [5](http://er9x.googlecode.com/svn/trunk/doc/FRSKYTelemetry.pdf)

[FrSky DJT and D8R-IIplus modules](http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=14355&aff=233755) (or any other with FrSky telemetry support)

[Ardupilot compatible Flight Controller v2.52](http://www.hobbyking.com/hobbyking/store/uh_viewitem.asp?idproduct=37328&aff=233755) (or any other controller with [APM firmware](http://ardupilot.com/))

### Required software ###
[eePe](https://code.google.com/p/eepe/) for flashing you radio

[XLoader](http://russemotto.com/xloader/) (or any other Arduino HEX uploader) OR
[Arduino IDE](http://arduino.cc/en/Main/Software) for flashing Arduino from source code.

For using Arduino IDE you must import [libraries](https://code.google.com/p/er9x-frsky-mavlink/source/browse/#svn/trunk/source/mavlink-driver/libraries)

### Downloads latest binaries ###
https://code.google.com/p/er9x-frsky-mavlink/source/browse/#svn/trunk/bin


[![](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=7EERYFP7D2K7Q&lc=US&item_name=FrSky%2dMavlink%20converter%20for%20er9x&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)