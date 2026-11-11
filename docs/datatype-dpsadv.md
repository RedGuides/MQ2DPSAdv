---
tags:
  - datatype
---
# `DPSAdv`

<!--dt-desc-start-->
Returns information about status, time, your DPS and your pet's DPS.
<!--dt-desc-end-->

## Members
<!--dt-members-start-->
/// member | int64 MyDamage
My total damage
///

/// member | int64 PetDamage
My pet's damage.
///

/// member | int64 TotalDamage
The combined value of mine and my pet's damage.
///

/// member | float MyDPS
My "Damage Per Second"
///

/// member | float PetDPS
My pet's "Damage Per Second"
///

/// member | float TotalDPS
Combined "Damage Per Second" from mine and my pet's damage.
///

/// member | int TimeElapsed
The time elapsed of a fight.
///

/// member | int MyStatus
Returns your "MyActive" status 0 for off, 1 for on.
///

/// member | int MyPetID
Returns your pet's ID.
///

<!--dt-members-end-->

<!--dt-linkrefs-start-->
[float]: ../macroquest/reference/data-types/datatype-float.md
[int]: ../macroquest/reference/data-types/datatype-int.md
[int64]: ../macroquest/reference/data-types/datatype-int64.md
<!--dt-linkrefs-end-->
