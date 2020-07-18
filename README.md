# Departure-List-Sequencing-PlugIn-for-EuroScope
Help arranging sequences of DEL, GND and TWR in EuroScope.


## Features

This plugin can manage sequences during 3 phases of ground activities: delivery, push & taxi, take-off. In order to help identify the status of a flight and sort with the departure list, respectively they are named **CLRN**, **PUSH**, **TKOF**. Each status includes 2 sub-status, which is *clrd* for cleared and *stby* for stand-by. These two status can be distinguished not only by the number of the sequence at the rear of the string but also the color of the text itself (needs customization, by default the color are the same with other items in the list). For *clrd* it shows **___ (e.g. TKOF___)**, while for *stby* it shows **the number of its sequences (e.g. CLRN_08)**. The plugin accepts a maximum of 99 flights for each phases which I believe is enough for most circumstances. To prevent possible confusion when a flight reconnects to VATSIM, this plugin will automatically remember the sequence at the moment they disconnect and insert them to where they shoud be after reconnecting.
Various configurations including color modifications can be made manually and be saved into ES config files through the command line methods below, providing flexiblity for users with different demands.


## How to use it

1. Download the .dll file in Release folder and load the plugin into Euroscope. 
2. Modify departure list. Add the tag type called ***Ground Sequence*** and add ***GND SEQ Popup List*** to the mouse button function.
3. To start sequecing a flight, just open the popup list and click. On each state the plugin will give respective actions to choose.
    Notice:
    + If a flight is not yet in the sequence, nothing will show up in its tag.
    + Sequence number may be inconsistent due to disconnections or aircrafts getting airbourne. It will correct itself within seconds (this refresh interval is customizable, 5 by default).
    + If the ground speed of a flight is higher than 80 knots, the flight will be deleted from sequences. This GS limit is also customizable.
4. Use command lines illustrated below to modify the plugin. When exiting ES, it will prompt you to save these settings or not.


## Command lines

All command lines should begin with ***.dls (followed by a space)***. Options and parameters as follows.

|Command|Function|
|-|-|
|remove all|Removes all sequences, including both online and offline flights. <br>If you start a new session else where without exitting ES, this command is recommended to clean up all records.|
|remove offline|Removes all offline flights.|
|color standby rrr,ggg,bbb|Sets standby tag color. r,g,b are integers between 0 and 255. If the format doesn't match, it deletes the color.|
|color cleared rrr,ggg,bbb|Sets cleared tag color. r,g,b are integers between 0 and 255. If the format doesn't match, it deletes the color.|
|color reset|Deletes all color customizations.|
|interval x|Sets refresh interval to x seconds.|
|interval reset|Resets refresh interval to default (5 seconds).|
|speed x|Sets maximum speed to x(x>30) knots. <br>If an aircraft's ground speed is high than x, it will be removed automatically from the sequence.|
|speed reset|Reset maximum speed to default (80 knots).|

