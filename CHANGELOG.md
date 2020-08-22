08/22/2020   1.5.00 (Knightly)
Fix naming collision where a non-PC having the same name as a PC would crash
Add HistoryLimit setting to the ini which allows you to specify how many mobs for which to keep a DPS history.
  This should help in those times where running the plugin for long periods of time eventually slows down or
  crashes.  The default is 25.
Add /dps limit command to set the history limit for that character
Hopefully fixes an issue where empty sections would get pushed to the ini

08/22/2020   1.4.00 (Chatwiththisname)
Fix bugs in time calculations so that "First" damage can no longer occur after "Last" damage

03/03/2020   1.4.00 (Ctaylor22)
Added DPS Meter logic for self and pet. Added a TLO so you can access the information.
This update does not require the DPSADV UI window to be open. Added MyStart, MyStop, MyReset commands.
New TLO DPSAdv and members: MyDamage, PetDamage, TotalDamage, MyDPS, PetDPS, TotalDPS, TimeElapsed, MyStatus.
added several commands including help.

10/20/19  1.3.00 (Chatwiththisname)
Added comma delimited outputs, IE: 361,987,590 instead of 361987590 for the total dmg output.
created two new functions. ReverseString(PCHAR szLine) and PutCommas(PCHAR szLine) to manipulate the strings.
These should only be used just prior to output to the Listbox. This is now done by default.

Added new checkbox in the settings menu. Use T B M K Numbers. Which allows for 2.9 mil, or 3.5 k, or 1.5 bil etc.
outputs instead of 2,905,376 3,527 or 1,587,439,068 outputs to the DPS list.
Updated the XML file for the above change.

Changed where Anon was happening. It was happening during the entity creation instead of just anonymizing
the string being written to the listbox for the name. I've changed, so now you can "/caption anon on" mid
combat and it won't create a new entry in the listbox. 

Unknown  1.2.07 (Chatwiththisname)
Add new column %, to show percentage of damage per entry (ShowTotal is an exception)
Update .xml file

Unknown  1.2.06 (Chatwiththisname)
Add new column Time to show the length of time the entry has been engaged with the target.
Update .xml file

2/27/19  1.2.05 (Chatwiththisname)
Added various checks throughout to avoid crashes
Added Debug statements throughout while fixing crashes and left them for later reference if needed.
Added "/dpsadv debug" toggle to toggle on and off the debug while in game.
Added "Smashes" and "Smash" to the hits (Berserker pet did this, added to both as options just in case others can smash).

1/19/19  1.2.05 (Chatwiththisname)
Tried to categorize all Chat colors in comments
Added Thorn and Fire based DS's
Added Pet Non-Melee damage.

1/6/19  1.2.04 (Chatwiththisname)
Added 2 new colors for Others DoTs and Others Non-Melee
Changed the color of your DoTs
Fixed DoTs and Non-Melee Damage for self and others
Added "shoot" for YourHits and "shoots" for OtherHits to parse.

7/18/09  1.2.03 (Warnen)
Added handling for Your Pet filter.

7/13/09 
Source made public

3/13/09  1.2.02 (Warnen)
Fixed a bug that caused the plugin to keep parsing after closing the window and before zoning.
Fixed a crash related to charmed mobs.
New Colors: EntHover, EntHighlight.
/dpsadv copy - Untested - Will copy ini settings from a different char name.

2/27/09  1.2.01 (Warnen)
Fixed a bug with settings not loading/assigning properly for first time.
Fixed/Improved code for zoning and removing pointers that may have caused decreased performance or crash.
Fixed a bug causing MaxDmg to include InActive if there were Active mobs.

2/19/09  1.2.00 (Warnen)
SpawnStruct Implimentation; A Spawn is assigned to each Fight and Entity/Entry to pull information if found.
Settings Page: Various settings available.
Pet Support: First checks name for `s pet, then checks Master ID in Spawn. Entrys with pets are marked with a .
Pets no longer show as Fights.
Coloring Support: Option to use Raid Coloring for PC Classes. Other colors setable in INI. See Main Post.
Mercenary Support: Mercenary's will not show up as a Fight. When displayed as Entry, they are indicated by [M] and Utilize Coloring.
Option to Show yourself at the Top of the list.
Option to show Total Damage at Top (Above/Below ShowMeTop), or Bottom of list.
Several Code Changes / Improvements.
This update includes UI XML Changes. Make sure you replace the old UI File.

2/13/09  1.1.04b (Warnen)
Updated for new zip.

2/12/09  1.1.04 (Warnen)
Updated for patch.

2/04/09  1.1.03 (Warnen)
Fixed some Performance Code being skipped.
Added /dpsadv show command to re-show the window.
The window will save its open/closed status in INI File now.
Version now showed in the Window Title.

2/03/09  1.1.02 (Warnen)
Crash bug fix.

2/03/09  1.1.01 (Warnen)
Fixed fight list combo box updating in a loop.

2/03/09  1.1 (Warnen)
Release