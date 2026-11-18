---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2dpsadv.118/"
support_link: "https://www.redguides.com/community/threads/mq2dpsadv.66823/"
repository: "https://github.com/RedGuides/MQ2DPSAdv"
config: "mq2dpsadv.ini"
authors: "WARNEN, ChatWithThisName, wired420, brainiac, Knightly, ctaylor22, drwhomphd, Redbot, Sic"
tagline: "This plugin shows the dps output of you and people around you in a custom dps window."
---

# MQ2DPSAdv

<!--desc-start-->
Shows the dps output of you and characters around you in a custom dps window.
<!--desc-end-->

## Features

- Live DPS list for the current fight: rank, name, total damage, DPS, time and % of the fight's damage.
- Pet damage is added to its owner's total (shown as `Name*`).
- Pick which fight to show: your target, the highest damage fight, or any recent fight from the history dropdown.
- Optional "show me at the top" row, total damage row and raid class colored names.
- `/dpsadv mystart` / `mystop` track your own (and your pet's) DPS for a fight, available to macros through the `${DPSAdv}` TLO.

## Commands

<a href="cmd-dpsadv/">
{% 
  include-markdown "projects/mq2dpsadv/cmd-dpsadv.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2dpsadv/cmd-dpsadv.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2dpsadv/cmd-dpsadv.md') }}

## Settings

These are the settings in `MQ2DPSAdv.ini`, in your MacroQuest `config` folder. Each character has its own `[CharacterName]` section.
Edit the file only while the plugin is unloaded; otherwise the plugin's next save overwrites your changes.

```ini
[CharacterName]
Saved=0
Top=0
Bottom=0
Left=0
Right=0
Alpha=0
FadeToAlpha=0
CListType=0
LiveUpdate=0
Show=1
ShowMeTop=0
ShowMeMin=0
ShowMeMinNum=0
UseRaidColors=0
ShowTotal=0
FightIA=8
FightTO=30
EntTO=8
UseTBMKOutputs=0
HistoryLimit=25
Column0Width=30
Column1Width=30
Column2Width=30
Column3Width=30
Column4Width=30
Column5Width=30
```

Explanations for these settings can be found on the [/dpsadv](cmd-dpsadv.md) command page.

## FAQ

!!! question "How do I show the window?!"

    - `/dpsadv show`
    - if you have just loaded this plugin for the first time you may need to reload your ui with either:
        1. `reloadui`
        2. `/loadskin default`
    - If you still can't see the window, you may need to manually edit the window location in MQ2DPSAdv.ini

!!! question "How do I save the location?"

    It saves automatically when you move or resize the window or change a setting. `/dpsadv save` saves immediately, but is normally not needed.

## Top-Level Objects

## [DPSAdv](tlo-dpsadv.md)
{% include-markdown "projects/mq2dpsadv/tlo-dpsadv.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2dpsadv/tlo-dpsadv.md') }}

## DataTypes

## [DPSAdv](datatype-dpsadv.md)
{% include-markdown "projects/mq2dpsadv/datatype-dpsadv.md" start="<!--dt-desc-start-->" end="<!--dt-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2dpsadv/datatype-dpsadv.md') }}

<h2>Members</h2>
{% include-markdown "projects/mq2dpsadv/datatype-dpsadv.md" start="<!--dt-members-start-->" end="<!--dt-members-end-->" %}
{% include-markdown "projects/mq2dpsadv/datatype-dpsadv.md" start="<!--dt-linkrefs-start-->" end="<!--dt-linkrefs-end-->" %}
