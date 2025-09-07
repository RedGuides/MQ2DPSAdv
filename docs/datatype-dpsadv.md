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
### {{ renderMember(type='int64', name='MyDamage') }}

:   My total damage

### {{ renderMember(type='int64', name='PetDamage') }}

:   My pet's damage.

### {{ renderMember(type='int64', name='TotalDamage') }}

:   The combined value of mine and my pet's damage.

### {{ renderMember(type='float', name='MyDPS') }}

:   My "Damage Per Second"

### {{ renderMember(type='float', name='PetDPS') }}

:   My pet's "Damage Per Second"

### {{ renderMember(type='float', name='TotalDPS') }}

:   Combined "Damage Per Second" from mine and my pet's damage.

### {{ renderMember(type='int', name='TimeElapsed') }}

:   The time elapsed of a fight.

### {{ renderMember(type='int', name='MyStatus') }}

:   Returns your "MyActive" status 0 for off, 1 for on.

### {{ renderMember(type='int', name='MyPetID') }}

:   Returns your pet's ID.

<!--dt-members-end-->

<!--dt-linkrefs-start-->
[float]: ../macroquest/reference/data-types/datatype-float.md
[int]: ../macroquest/reference/data-types/datatype-int.md
[int64]: ../macroquest/reference/data-types/datatype-int64.md
<!--dt-linkrefs-end-->
