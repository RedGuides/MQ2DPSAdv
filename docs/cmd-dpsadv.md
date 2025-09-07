---
tags:
  - command
---

# /dpsadv

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/dpsadv [show | colors | reload | save | listsize | copy | limit] [setting]
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Adjust settings or view information on the MQ2DPSAdv plugin.
<!--cmd-desc-end-->

## Options

| Option | Description |
|--------|-------------|
| `show` | Shows the MQ2DPSAdv Window. |
| `colors` | Shows the Raid Class Colors Window. |
| `reload` | Reloads your settings from your MQ2DPSAdv.ini file. |
| `save` | Save current settings to your MQ2DPSAdv.ini file. |
| `listsize` | Displays the number of mobs in the DPS list. |
| `copy` | Copies the MQ2DPSAdv Window location. |
| `debug` | Toggles the on/off debug messages. |
| `mydebug` | Toggles on/off myDPS debug messages. |
| `mystart` | Starts the myDPS capture process. |
| `mystop` | Stops the myDPS capture process. |
| `myreset` | Turns off myDPS capture and sets myDPS totals to zero. |
| `tlo` | Get a list of the DPSAdv TLO members. |
| `limit [option]` | &#91;#&#93; sets your history limit to this value. |

## UI Options

- ShowMeTop: This will put an entry at the top of the list for your character.
- -> Only < #: Causes you to show up at the top of the list only if you are below this rank.
- UseRaidColors: Activates Raid Coloring. Non-Class Colors changable in INI (Read Below).
- LiveUpdate: Will cause the DPS Listing to update as soon as damage comes in. This may be CPU Intensive.
- Show Total: Options to show Total Fight Damage at Top (Above or Below ShowMeTop) or at Bottom.
- Fight I/A: Amount of time before a fight will be marked In-Active, and not used for MaxDmg. Default 8.
- Fight T/O: Amount of time before a fight Time's Out and is marked Dead. Default 30.
- Entry T/O: Amount of time inbetween attacks that must pass to not be counted towards Time DPS Calculation. Should be at least 7 due to DoT-Only Damage being every Tic (6 Seconds). Default 8.