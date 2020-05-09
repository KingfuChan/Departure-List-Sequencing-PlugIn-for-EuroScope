# Departure-List-Sequencing-PlugIn-for-EuroScope
Help arranging sequences of DEL, GND and TWR in EuroScope

使用说明：
加载插件后需要手动设置Departure List内的功能，加入插件提供的TAG，设置左右键功能。
本插件提供的地面状态描述显示格式为：XXXX-XX
其中前四个字母包括CLRN, PUSH, TKOF，分别对应Clearance delivery, Push and start, Take off。
（这样做的目的是C P T三个字母有次序可以直接在Departure List内排序）
每一地面状态分为standby以及cleared两种，standby后两位为数字，表明次序（可以手动更改），cleared后两位为--
另外对standby及cleared有颜色区分（建议不要将默认颜色设为灰色或天蓝色）
本插件有两条命令行指令：
.dls remove all         移除所有机组排序。如果管制员掉线重连后出现次序错乱，可以使用此命令重置所有排序。
.dls remove offline     从排序中去除已断线机组