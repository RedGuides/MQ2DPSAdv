# MQ2DPSAdv

This plugin shows the DPS output of you and people around you in a custom DPS window.

## Documentation

Usage, commands, settings and the `${DPSAdv}` TLO are documented on [RedGuides](https://www.redguides.com/docs/projects/mq2dpsadv/). Questions go in the [support thread](https://www.redguides.com/community/threads/mq2dpsadv.66823/). Changes are listed in the [changelog](CHANGELOG.md).

## Building

Clone this repository into the `plugins` folder of a MacroQuest source checkout and build it with the MacroQuest solution in Visual Studio. The window layout (`resources/uifiles/default/MQUI_DPSAdvWnd.xml`) is copied with the plugin and must match the plugin version; an outdated copy in your EQ or custom UI folder shows an "Incorrect UI File" message.
