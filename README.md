# Departure-List-Sequencing-PlugIn-for-EuroScope
Help arranging sequences of DEL, GND and TWR in EuroScope.


## Features

This plugin can manage sequences during 3 phases of ground activities: delivery, push & taxi, take-off. In order to help identify the status of a flight and sort with the departure list, respectively they are named **CLRN**, **PUSH**, **TKOF**. Each status includes 2 sub-status, which is *clrd* for cleared and *stby* for stand-by. These two status can be distinguished not only by the number of the sequence at the rear of the string but also the color of the text itself. For *clrd* it shows **--- (e.g. TKOF---)** in <font color=#81e4e2>light blue</font> by default, while for *stby* it shows **the number of its sequences (e.g. CLRN-08)** in the color of your list settings by default. The plugin accepts a maximum of 99 flights on each phases which I believe is enough for most circumstances. To prevent possible confusion when a flight reconnects to VATSIM, this plugin will automatically remember the sequence at the moment they disconnect and insert them to where they shoud be after reconnecting.
Various configurations can be made manually and save to ES config files through the command line methods below, providing flexiblity for users with different demands.


## How to use it

1. Load the plugin via the Plug-In setting of Euroscope. 
2. Modify departure list. Add the tag type called ***Ground Sequence*** and add ***GND SEQ Popup List*** in the mouse button function.
3. To start sequecing a flight, just open the popup list and click. On each state the plugin will give respective actions to choose.
    Notice:
    + Sequence number may be inconsistent due to disconnections or getting airbourne of flights. It will correct itself within seconds (customizable, 5 by default).
    + If the ground speed of a flight is higher than 80 knots, the flight will be deleted from sequences. This GS limit is also customizable.
4. Use command lines illustrated below to modify the plugin. When exiting ES, it will prompt you to save these settings or not.


## Command lines

All command lines should begin with ***.dls (followed by a space)***. Options and parameters as follows.

<style>
table th:first-of-type {
	width: 200px;
}
</style>

|Command|Function|
|-|-|
|remove all|Removes all sequences, including both online and offline flights. <br>If you start a new session else where without exitting ES, this command is recommended to clean up all records.|
|remove offline|Removes all offline flights.|
|color inactive rrr,ggg,bbb|Sets inactive tag color. r,g,b are integers between 0 and 255.|
|color cleared rrr,ggg,bbb|Sets cleared tag color. r,g,b are integers between 0 and 255.|
|color reset|Resets tag colors to default.|
|interval x|Sets refresh interval to x seconds.|
|interval reset|Resets refresh interval to default (5 seconds).|
|speed x|Sets maximum speed to x(x>30) knots. <br>If an aircraft's ground speed is high than x, it will be removed automatically from the sequence.|
|speed reset|Reset maximum speed to default (80 knots).|

