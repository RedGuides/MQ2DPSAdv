// DPS ADV CREATED BY WARNEN 2008-2009
// MQ2DPSAdv.cpp

#include "../MQ2Plugin.h"
using namespace std;
PreSetup("MQ2DPSAdv");
#include <vector>
#include "MQ2DPSAdv.h"

#define DPSVERSION "1.2.06"
//#define DPSDEV

/*
 ### UPDATE THIS IF YOU MAKE A CHANGE ###
 == 2/27/19 == 1.2.05 (Chatwiththisname)
 Added various checks throughout to avoid crashes
 Added Debug statements throughout while fixing crashes and left them for later reference if needed.
 Added "/dpsadv debug" toggle to toggle on and off the debug while in game.
 Added "Smashes" and "Smash" to the hits (Berserker pet did this, added to both as options just in case others can smash).

 == 1/19/19 == 1.2.05 (Chatwiththisname)
 Tried to categorize all Chat colors in comments
 Added Thorn and Fire based DS's
 Added Pet Non-Melee damage.

 == 1/6/19 == 1.2.04 (Chatwiththisname)
Added 2 new colors for Others DoTs and Others Non-Melee
Changed the color of your DoTs
Fixed DoTs and Non-Melee Damage for self and others
Added "shoot" for YourHits and "shoots" for OtherHits to parse.

== 7/18/09 == 1.2.03 (Warnen)
* Added handling for Your Pet filter.

== 7/13/09 ==
* Source made public

== 3/13/09 == 1.2.02 (Warnen)
* Fixed a bug that caused the plugin to keep parsing after closing the window and before zoning.
* Fixed a crash related to charmed mobs.
* New Colors: EntHover, EntHighlight.
* /dpsadv copy - Untested - Will copy ini settings from a different char name.

== 2/27/09 == 1.2.01 (Warnen)
* Fixed a bug with settings not loading/assigning properly for first time.
* Fixed/Improved code for zoning and removing pointers that may have caused decreased performance or crash.
* Fixed a bug causing MaxDmg to include InActive if there were Active mobs.

== 2/19/09 == 1.2.00 (Warnen)
* SpawnStruct Implimentation; A Spawn is assigned to each Fight and Entity/Entry to pull information if found.
* Settings Page: Various settings available.
* Pet Support: First checks name for `s pet, then checks Master ID in Spawn. Entrys with pets are marked with a *.
* Pets no longer show as Fights.
* Coloring Support: Option to use Raid Coloring for PC Classes. Other colors setable in INI. See Main Post.
* Mercenary Support: Mercenary's will not show up as a Fight. When displayed as Entry, they are indicated by [M] and Utilize Coloring.
* Option to Show yourself at the Top of the list.
* Option to show Total Damage at Top (Above/Below ShowMeTop), or Bottom of list.
* Several Code Changes / Improvements.
This update includes UI XML Changes. Make sure you replace the old UI File.

== 2/13/09 == 1.1.04b (Warnen)
* Updated for new zip.

== 2/12/09 == 1.1.04 (Warnen)
* Updated for patch.

== 2/04/09 == 1.1.03 (Warnen)
* Fixed some Performance Code being skipped.
* Added /dpsadv show command to re-show the window.
* The window will save its open/closed status in INI File now.
* Version now showed in the Window Title.

== 2/03/09 == 1.1.02 (Warnen)
* Crash bug fix.

== 2/03/09 == 1.1.01 (Warnen)
* Fixed fight list combo box updating in a loop.

== 2/03/09 == 1.1 (Warnen)
* Release

*/

// ############################### DPSEntry Start ############################################

DPSMob::DPSEntry::DPSEntry() {
   Init();
}

DPSMob::DPSEntry::DPSEntry(char EntName[64], DPSMob *pParent) {
   Init();
   Parent = pParent;
   strcpy_s(Name, EntName);
   GetSpawn();
}

void DPSMob::DPSEntry::Init() {
   Parent = 0;
   SpawnType = -1;
   Mercenary = false;
   Class = 0;
   Spawn = 0;
   Master = 0;
   DoSort = false;
   Pets = false;
   CheckPetName = false;
   UsingPetName = false;
   strcpy_s(Name, "");
   Damage.Total = 0;
   Damage.First = 0;
   Damage.Last = 0;
   Damage.AddTime = 0;
}

void DPSMob::DPSEntry::GetSpawn() {
   PSPAWNINFO pSpawn=(PSPAWNINFO)pSpawnList;
   while(pSpawn) {
      if (!_stricmp(pSpawn->DisplayedName, Name)) {
         if (pSpawn->Type != SPAWN_CORPSE) {
            Spawn = pSpawn;
            SpawnType = Spawn->Type;
            Class = Spawn->mActorClient.Class;
            Mercenary = pSpawn->Mercenary;
            return;
         }
      }
      pSpawn = pSpawn->pNext;
   }
   if (Debug && !Spawn) WriteChatf("\arError: Could not find Ent Spawn (%s)", Name);
}

bool DPSMob::DPSEntry::CheckMaster() {
   if (DoSort) return Master ? true : false;
   if (UsingPetName) return true;
   if (!CheckPetName) {
      CheckPetName = true;
      if(strstr(Name, "`s pet")) {
         UsingPetName = true;
		 CHAR szMaster[MAX_STRING] = { 0 };
         strcpy_s(szMaster, Name);
		 if (char *pDest = strstr(szMaster, "`s pet")) {
			 pDest[0] = '\0';
		 }
         Master = Parent->GetEntry(szMaster);
         return true;
      }
   }
   if (Master && (!Spawn || Spawn->MasterID <= 0 || !Master->Spawn || Master->Spawn->SpawnID != Spawn->MasterID)) Master = 0;
   if (Master) return true;
   else if (Spawn && Spawn->MasterID > 0) {
      PSPAWNINFO NewMaster = (PSPAWNINFO)GetSpawnByID(Spawn->MasterID);
      if (NewMaster) {
         Master = Parent->GetEntry(NewMaster->DisplayedName);
         return true;
      }
   }
   return false;
}

void DPSMob::DPSEntry::AddDamage(int aDamage) {
   if (CheckMaster()) {
      Master->Pets = true;
      Master->AddDamage(aDamage);
      DoSort = true;
      return;
   }
   DoSort = true;
   if (Damage.First && (time(NULL) - Damage.Last >= EntTO)) {
      Damage.AddTime += (int)(Damage.Last - Damage.First) + 1;
      Damage.First = 0;
   }
   Damage.Total += aDamage;
   Damage.Last = time(NULL);
   if (!Damage.First) Damage.First = time(NULL);
   Parent->AddDamage(aDamage);
}

int DPSMob::DPSEntry::GetDPS() {
   return (int)(Damage.Total / (((Damage.Last - Damage.First) + 1) + Damage.AddTime));
}

void DPSMob::DPSEntry::Sort() {
   DoSort = false;
   if (Master && Master->DoSort) Master->Sort();
   if (Damage.Total == 0) return;
//   DPSWnd->ReSort = true;
   int i = 0, x = -1;
   bool Inserted = false;
   for (i = 0; i < (int)Parent->EntList.size(); i++) {
      if (Parent->EntList[i] == this) {
         Inserted = true;
         if (x == -1) break;
         Parent->EntList.erase(Parent->EntList.begin()+i);
         Parent->EntList.insert(Parent->EntList.begin()+x, this);
         break;
      } else if (x == -1 && Parent->EntList[i]->Damage.Total < Damage.Total) x = i;
   }
   if (!Inserted)
      if (x != -1) Parent->EntList.insert(Parent->EntList.begin()+x, this);
      else Parent->EntList.push_back(this);
   if (LiveUpdate && Parent == CurListMob && !CurListMob->Dead) DPSWnd->DrawList();
};

// ############################### DPSMob START ############################################

DPSMob::DPSMob() {
   Init();
}

DPSMob::DPSMob(PCHAR MobName, size_t MobLen) {
   Init();
   strcpy_s(Name, MobName);
   GetSpawn();
   if (!_stricmp(Name, "`s pet"))
	   PetName = true;
}

void DPSMob::Init() {
   strcpy_s(Name, "");
   Damage.Total = 0;
   Damage.First = 0;
   Damage.Last = 0;
   LastEntry = 0;
   SpawnType = -1;
   Mercenary = false;
   Spawn = 0;
   Active = false;
   Dead = false;
   PetName = false;
}

void DPSMob::GetSpawn() {
   PSPAWNINFO pSpawn=(PSPAWNINFO)pSpawnList;
   while(pSpawn) {
      if (!_stricmp(pSpawn->DisplayedName, Name)) {
         SpawnType = pSpawn->Type;
         Mercenary = pSpawn->Mercenary;
         Spawn = pSpawn;
         if (SpawnType != SPAWN_CORPSE) {
            Dead = false;
            return;
         } else Dead = true;
      }
      pSpawn = pSpawn->pNext;
   }
   if (Debug && !Spawn) WriteChatf("\arError: Could not find Mob Spawn (%s)", Name);
}

bool DPSMob::IsPet() {
   return PetName || (Spawn && Spawn->MasterID > 0);
}

void DPSMob::AddDamage(int aDamage) {
   Damage.Total += aDamage;
   Damage.Last = time(NULL);
   if (!Damage.First) Damage.First = time(NULL);
   if (!Active) {
      Active = true;
      sprintf_s(Tag, "[A] ");
      if (!IsPet() && !Mercenary) DPSWnd->DrawCombo();
   }
}

DPSMob::DPSEntry *DPSMob::GetEntry(char EntName[64], bool Create) {
   if (LastEntry && !strcmp(LastEntry->Name, EntName)) return LastEntry;
   else {
      if (LastEntry && LastEntry->DoSort) LastEntry->Sort();
      for (int i = 0; i < (int)EntList.size(); i++) {
         if (!strcmp(EntList[i]->Name, EntName)) {
            LastEntry = EntList[i];
            return LastEntry;
         }
      }
   }
   if (Create) {
      LastEntry = new DPSEntry(EntName, this);
      EntList.push_back(LastEntry);
      return LastEntry;
   }
   return 0;
}

// ############################### CDPSAdvWnd START ############################################

CDPSAdvWnd::CDPSAdvWnd():CCustomWnd("DPSAdvWnd") {
   int CheckUI = false;
   if (!(Tabs = (CTabWnd*)GetChildItem("DPS_Tabs"))) CheckUI = true;
   if (!(LTopList = (CListWnd*)GetChildItem("DPS_TopList"))) CheckUI = true;
   if (!(CMobList = (CComboWnd*)GetChildItem("DPS_MobList"))) CheckUI = true;
   if (!(CShowMeTop = (CCheckBoxWnd*)GetChildItem("DPS_ShowMeTopBox"))) CheckUI = true;
   if (!(CShowMeMin = (CCheckBoxWnd*)GetChildItem("DPS_ShowMeMinBox"))) CheckUI = true;
   if (!(TShowMeMin = (CTextEntryWnd*)GetChildItem("DPS_ShowMeMinInput"))) CheckUI = true;
   if (!(CUseRaidColors = (CCheckBoxWnd*)GetChildItem("DPS_UseRaidColorsBox"))) CheckUI = true;
   if (!(CLiveUpdate = (CCheckBoxWnd*)GetChildItem("DPS_LiveUpdateBox"))) CheckUI = true;
   if (!(TFightIA = (CTextEntryWnd*)GetChildItem("DPS_FightIAInput"))) CheckUI = true;
   if (!(TFightTO = (CTextEntryWnd*)GetChildItem("DPS_FightTOInput"))) CheckUI = true;
   if (!(TEntTO = (CTextEntryWnd*)GetChildItem("DPS_EntTOInput"))) CheckUI = true;
   if (!(CShowTotal = (CComboWnd*)GetChildItem("DPS_ShowTotal"))) CheckUI = true;
   //if (!(LFightList = (CListWnd*)GetChildItem("DPS_FightList"))) CheckUI = true;
   this->BGColor = 0xFF000000;
   Tabs->BGColor = 0xFF000000;
   LTopList->BGColor = 0xFF000000;
   CShowMeTop->BGColor = 0xFF000000;
   CShowMeMin->BGColor = 0xFF000000;
   TShowMeMin->BGColor = 0xFF000000;
   CUseRaidColors->BGColor = 0xFF000000;
   CLiveUpdate->BGColor = 0xFF000000;
   TFightIA->BGColor = 0xFF000000;
   TFightTO->BGColor = 0xFF000000;
   TEntTO->BGColor = 0xFF000000;
   CShowTotal->BGColor = 0xFF000000;
   if (CheckUI) {
      WriteChatf("\ar[MQ2DPSAdv] Incorrect UI File in use. Please update to latest and reload plugin.");
      WrongUI = true;
   } else WrongUI = false;
   
   LoadLoc();
   SetWndNotification(CDPSAdvWnd);
   //LTopList->SetColors(0xFFFFFFFF, 0xFFCC3333, 0xFF666666);
   //LFightList->SetColors(0xFFFFFFFF, 0xFFCC3333, 0xFF666666);
   CMobList->SetColors(0xFFCC3333, 0xFF666666, 0xFF000000);
   Tabs->UpdatePage();
   DrawCombo();
   //LFightList->AddString(&CXStr("1"), 0, 0, 0);
   //LFightList->AddString(&CXStr("2"), 0, 0, 0);
   //LFightList->AddString(&CXStr("3"), 0, 0, 0);
   //LFightList->ExtendSel(1);
   //LFightList->ExtendSel(2);
}

CDPSAdvWnd::~CDPSAdvWnd() {}

void CDPSAdvWnd::DrawCombo() {
   int CurSel = CMobList->GetCurChoice();
   CMobList->DeleteAll();
   CHAR szTemp[MAX_STRING] = { 0 };
   //sprintf_s(szTemp, "<Target> %s", CurListMob ? CurListMob->Name : "None");
   sprintf_s(szTemp, "<Target> %s%s", CListType == CLISTTARGET && CurListMob ? CurListMob->Tag : "", CListType == CLISTTARGET && CurListMob ? CurListMob->Name : "None");
   CMobList->InsertChoice(szTemp);
   sprintf_s(szTemp, "<MaxDmg> %s%s", CListType == CLISTMAXDMG && CurListMob ? CurListMob->Tag : "", CListType == CLISTMAXDMG && CurListMob ? CurListMob->Name : "None");
   CMobList->InsertChoice(szTemp);


   DPSMob *Mob = 0;
   int i = 0, ListSize = 0;
   for (i = 0; i < (int)MobList.size(); i++) {
      Mob = MobList[i];
      if (Mob->SpawnType == 1 && Mob->Active && !Mob->IsPet() && !Mob->Mercenary) {
         ListSize++;
         sprintf_s(szTemp, "%s(%i) %s", Mob->Tag, ListSize, Mob->Name);
         CMobList->InsertChoice(szTemp);
      }
   }

   if (ListSize < 6) CMobList->InsertChoice("");

   if (CListType == CLISTTARGET) CMobList->SetChoice(0);
   else if (CListType == CLISTMAXDMG) CMobList->SetChoice(1);
   else CMobList->SetChoice(CurSel >= 0 ? CurSel : 0);
}

void CDPSAdvWnd::SetTotal(int LineNum, DPSMob *Mob) {
   CHAR szTemp[MAX_STRING] = { 0 };
   SetLineColors(LineNum, 0, true);
   LTopList->SetItemText(LineNum, 0, &CXStr("-"));
   LTopList->SetItemText(LineNum, 1, &CXStr("Total"));
   sprintf_s(szTemp, "%i", CurListMob->Damage.Total);
   LTopList->SetItemText(LineNum, 2, &CXStr(szTemp));
   sprintf_s(szTemp, "%i", CurListMob->Damage.Total / (int)((CurListMob->Damage.Last - CurListMob->Damage.First) + 1));
   LTopList->SetItemText(LineNum, 3, &CXStr(szTemp));
}

void CDPSAdvWnd::DrawList(bool DoDead) {
   if (!CurListMob || (!DoDead && CurListMob->Dead)) return;
   int ScrollPos = LTopList->VScrollPos;
   int CurSel = LTopList->GetCurSel();
   CHAR szTemp[MAX_STRING] = { 0 };
   LTopList->DeleteAll();
   int i = 0, LineNum = 0, RankAdj = 0, ShowMeLineNum = 0;
   bool FoundMe = false, ThisMe = false;
   if (ShowTotal == TOTALABOVE) {
      LineNum = LTopList->AddString(&CXStr(""), 0, 0, 0);
      SetTotal(LineNum, CurListMob);
      RankAdj++;
   }
   if (ShowMeTop) {
      ShowMeLineNum = LTopList->AddString(&CXStr(""), 0, 0, 0);
      SetLineColors(ShowMeLineNum, 0, false, true);
      RankAdj++;
   }
   if (ShowTotal == TOTALSECOND) {
      LineNum = LTopList->AddString(&CXStr(""), 0, 0, 0);
      SetTotal(LineNum, CurListMob);
      RankAdj++;
   }
   for (i = 0; i < (int)CurListMob->EntList.size(); i++) {
      DPSMob::DPSEntry *Ent = CurListMob->EntList[i];
      if (Ent->Damage.Total == 0) break;
      if (ShowMeTop && !strcmp(Ent->Name, ((PSPAWNINFO)pCharSpawn)->DisplayedName)) {
         if (!ShowMeMin || (LineNum - RankAdj + 1) > ShowMeMinNum) FoundMe = true;
         ThisMe = true;
      } else ThisMe = false;
      LineNum = LTopList->AddString(&CXStr(""), 0, 0, 0);
      SetLineColors(LineNum, Ent);
      sprintf_s(szTemp, "%i", LineNum - RankAdj + 1);
      LTopList->SetItemText(LineNum, 0, &CXStr(szTemp));
      if (ThisMe) LTopList->SetItemText(ShowMeLineNum, 0, &CXStr(szTemp));
      sprintf_s(szTemp, "%s%s%s", Ent->Mercenary ? "[M] " : "", Ent->Name, Ent->Pets ? "*" : "");
      LTopList->SetItemText(LineNum, 1, &CXStr(szTemp));
      if (ThisMe) LTopList->SetItemText(ShowMeLineNum, 1, &CXStr(szTemp));
      sprintf_s(szTemp, "%i", Ent->Damage.Total);
      LTopList->SetItemText(LineNum, 2, &CXStr(szTemp));
      if (ThisMe) LTopList->SetItemText(ShowMeLineNum, 2, &CXStr(szTemp));
      sprintf_s(szTemp, "%i", Ent->GetDPS());
      LTopList->SetItemText(LineNum, 3, &CXStr(szTemp));
      if (ThisMe) LTopList->SetItemText(ShowMeLineNum, 3, &CXStr(szTemp));
   }
   if (ShowTotal == TOTALBOTTOM) {
      LineNum = LTopList->AddString(&CXStr(""), 0, 0, 0);
      SetTotal(LineNum, CurListMob);
   }
   if (ShowMeTop && !FoundMe) LTopList->RemoveLine(ShowMeLineNum);
   LTopList->VScrollPos = ScrollPos;
   LTopList->CalculateFirstVisibleLine();
   LTopList->SetCurSel(CurSel);
}

void CDPSAdvWnd::SetLineColors(int LineNum, DPSMob::DPSEntry *Ent, bool Total, bool MeTop) {
   if (MeTop) {
      LTopList->SetItemColor(LineNum, 0, MeTopColor);
      LTopList->SetItemColor(LineNum, 1, MeTopColor);
      LTopList->SetItemColor(LineNum, 2, MeTopColor);
      LTopList->SetItemColor(LineNum, 3, MeTopColor);
   } else if (Total) {
      LTopList->SetItemColor(LineNum, 0, TotalColor);
      LTopList->SetItemColor(LineNum, 1, TotalColor);
      LTopList->SetItemColor(LineNum, 2, TotalColor);
      LTopList->SetItemColor(LineNum, 3, TotalColor);
   } else if (!strcmp(Ent->Name, ((PSPAWNINFO)pCharSpawn)->DisplayedName)) {
      LTopList->SetItemColor(LineNum, 0, MeColor);
      LTopList->SetItemColor(LineNum, 1, MeColor);
      LTopList->SetItemColor(LineNum, 2, MeColor);
      LTopList->SetItemColor(LineNum, 3, MeColor);
   } else if (UseRaidColors && Ent->Class && (Ent->SpawnType == SPAWN_PLAYER || Ent->Mercenary)) {
      //WriteChatf("Setting Raid Color: %i, %i, %i", Ent->Class, Coloring[Ent->Class], ((PEQRAIDWINDOW)pRaidWnd)->ClassColors[ClassInfo.RaidColorOrder[Ent->Class]]);
      LTopList->SetItemColor(LineNum, 0, NormalColor);
      LTopList->SetItemColor(LineNum, 1, ((PEQRAIDWINDOW)pRaidWnd)->ClassColors[ClassInfo[Ent->Class].RaidColorOrder]);
      LTopList->SetItemColor(LineNum, 2, NormalColor);
      LTopList->SetItemColor(LineNum, 3, NormalColor);
   } else {
      LTopList->SetItemColor(LineNum, 0, NormalColor);
      LTopList->SetItemColor(LineNum, 1, NPCColor);
      LTopList->SetItemColor(LineNum, 2, NormalColor);
      LTopList->SetItemColor(LineNum, 3, NormalColor);
   }
}
/*
void CDPSAdvWnd::SaveSetting(PCHAR Key, PCHAR Value, ...) {
   char zOutput[MAX_STRING]; va_list vaList; va_start(vaList,Value);
   vsprintf_s(zOutput,Value,vaList);
   WritePrivateProfileString(GetCharInfo()->Name, Key, zOutput, INIFileName);
   if (!Saved) {
      Saved = true;
      WritePrivateProfileString(GetCharInfo()->Name, "Saved", "1", INIFileName);
   }
}
*/
void CDPSAdvWnd::SaveLoc() {
   if (!GetCharInfo()) return;
   CHAR szTemp[MAX_STRING] = { 0 };
   WritePrivateProfileString(GetCharInfo()->Name, "Saved", "1", INIFileName);
   sprintf_s(szTemp, "%i", Location.top);
   WritePrivateProfileString(GetCharInfo()->Name, "Top", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", Location.bottom);
   WritePrivateProfileString(GetCharInfo()->Name, "Bottom", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", Location.left);
   WritePrivateProfileString(GetCharInfo()->Name, "Left", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", Location.right);
   WritePrivateProfileString(GetCharInfo()->Name, "Right", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", Alpha);
   WritePrivateProfileString(GetCharInfo()->Name, "Alpha", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", FadeToAlpha);
   WritePrivateProfileString(GetCharInfo()->Name, "FadeToAlpha", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", CListType);
   WritePrivateProfileString(GetCharInfo()->Name, "CListType", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", LiveUpdate ? 1 : 0);
   WritePrivateProfileString(GetCharInfo()->Name, "LiveUpdate", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", dShow);
   WritePrivateProfileString(GetCharInfo()->Name, "Show", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", ShowMeTop ? 1 : 0);
   WritePrivateProfileString(GetCharInfo()->Name, "ShowMeTop", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", ShowMeMin ? 1 : 0);
   WritePrivateProfileString(GetCharInfo()->Name, "ShowMeMin", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", ShowMeMinNum);
   WritePrivateProfileString(GetCharInfo()->Name, "ShowMeMinNum", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", UseRaidColors ? 1 : 0);
   WritePrivateProfileString(GetCharInfo()->Name, "UseRaidColors", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", ShowTotal);
   WritePrivateProfileString(GetCharInfo()->Name, "ShowTotal", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", FightIA);
   WritePrivateProfileString(GetCharInfo()->Name, "FightIA", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", FightTO);
   WritePrivateProfileString(GetCharInfo()->Name, "FightTO", szTemp, INIFileName);
   sprintf_s(szTemp, "%i", EntTO);
   WritePrivateProfileString(GetCharInfo()->Name, "EntTO", szTemp, INIFileName);
}

void CDPSAdvWnd::LoadSettings() {
   CHAR szTemp[MAX_STRING] = { 0 };
   CShowMeTop->Checked = ShowMeTop ? 1 : 0;
   CShowMeMin->Checked = ShowMeMin ? 1 : 0;
   sprintf_s(szTemp, "%i", ShowMeMinNum);
   SetCXStr(&TShowMeMin->InputText, szTemp);
   CUseRaidColors->Checked = UseRaidColors ? 1 : 0;
   CLiveUpdate->Checked = LiveUpdate ? 1 : 0;
   sprintf_s(szTemp, "%i", FightIA);
   SetCXStr(&TFightIA->InputText, szTemp);
   sprintf_s(szTemp, "%i", FightTO);
   SetCXStr(&TFightTO->InputText, szTemp);
   sprintf_s(szTemp, "%i", EntTO);
   SetCXStr(&TEntTO->InputText, szTemp);
   CShowTotal->DeleteAll();
   CShowTotal->InsertChoice("Don't Show Total");
   CShowTotal->InsertChoice("Above ShowMeTop");
   CShowTotal->InsertChoice("Below ShowMeTop");
   CShowTotal->InsertChoice("Show Bottom");
   CShowTotal->InsertChoice("");
   CShowTotal->SetChoice(ShowTotal);
}

void CDPSAdvWnd::LoadLoc(char szChar[64]) {
   if (!GetCharInfo()) return;
   char szName[64];
   if (!szChar) strcpy_s(szName, GetCharInfo()->Name);
   else strcpy_s(szName, szChar);
   Saved = (GetPrivateProfileInt(szName, "Saved", 0, INIFileName) > 0 ? true : false);
   if (Saved) {
      Location.top = GetPrivateProfileInt(szName, "Top", 0, INIFileName);
      Location.bottom = GetPrivateProfileInt(szName, "Bottom", 0, INIFileName);
      Location.left = GetPrivateProfileInt(szName, "Left", 0, INIFileName);
      Location.right = GetPrivateProfileInt(szName, "Right", 0, INIFileName);
      Alpha = (BYTE)GetPrivateProfileInt(szName, "Alpha", 0, INIFileName);
      FadeToAlpha = (BYTE)GetPrivateProfileInt(szName, "FadeToAlpha", 0, INIFileName);
   }
   CListType = GetPrivateProfileInt(szName, "CListType", 0, INIFileName);
   LiveUpdate = (GetPrivateProfileInt(szName, "LiveUpdate", 0, INIFileName) > 0 ? true : false);
   WarnedYHO = (GetPrivateProfileInt(szName, "WarnedYHO", 0, INIFileName) > 0 ? true : false);
   WarnedOHO = (GetPrivateProfileInt(szName, "WarnedOHO", 0, INIFileName) > 0 ? true : false);
   Debug = (GetPrivateProfileInt(szName, "Debug", 0, INIFileName) > 0 ? true : false);
   dShow = (GetPrivateProfileInt(szName, "Show", 1, INIFileName) > 0 ? true : false);
   ShowMeTop = (GetPrivateProfileInt(szName, "ShowMeTop", 0, INIFileName) > 0 ? true : false);
   ShowMeMin = (GetPrivateProfileInt(szName, "ShowMeMin", 0, INIFileName) > 0 ? true : false);
   ShowMeMinNum = GetPrivateProfileInt(szName, "ShowMeMinNum", 0, INIFileName);
   UseRaidColors = (GetPrivateProfileInt(szName, "UseRaidColors", 0, INIFileName) > 0 ? true : false);
   ShowTotal = GetPrivateProfileInt(szName, "ShowTotal", 0, INIFileName);
   FightIA = GetPrivateProfileInt(szName, "FightIA", 8, INIFileName);
   FightTO = GetPrivateProfileInt(szName, "FightTO", 30, INIFileName);
   EntTO = GetPrivateProfileInt(szName, "EntTO", 8, INIFileName);
   MeColor = GetPrivateProfileInt(szName, "MeColor", 0xFF00CC00, INIFileName);
   MeTopColor = GetPrivateProfileInt(szName, "MeTopColor", 0xFF00CC00, INIFileName);
   NormalColor = GetPrivateProfileInt(szName, "NormalColor", 0xFFFFFFFF, INIFileName);
   NPCColor = GetPrivateProfileInt(szName, "NPCColor", 0xFFFFFFFF, INIFileName);
   TotalColor = GetPrivateProfileInt(szName, "TotalColor", 0xFF66FFFF, INIFileName);
   EntHover = GetPrivateProfileInt(szName, "EntHover", 0xFFCC3333, INIFileName);
   EntHighlight = GetPrivateProfileInt(szName, "EntHighlight", 0xFF666666, INIFileName);
   FightNormal = GetPrivateProfileInt(szName, "FightNormal", NormalColor, INIFileName);
   FightHover = GetPrivateProfileInt(szName, "FightHover", EntHover, INIFileName);
   FightHighlight = GetPrivateProfileInt(szName, "FightHighlight", EntHighlight, INIFileName);
   FightActive = GetPrivateProfileInt(szName, "FightActive", 0xFF00CC00, INIFileName);
   FightInActive = GetPrivateProfileInt(szName, "FightInActive", 0xFF777777, INIFileName);
   FightDead = GetPrivateProfileInt(szName, "FightDead", 0xFF330000, INIFileName);
   if (FightIA < 3) FightIA = 8;
   if (FightTO < 3) FightTO = 30;
   if (EntTO < 3) EntTO = 8;
   if (Debug) gSpewToFile = TRUE;
   if (CListType > 1) CListType = CLISTTARGET;
   LTopList->SetColors(NormalColor, EntHover, EntHighlight);
//   LFightList->SetColors(FightNormal, FightHover, FightHighlight);
   CMobList->SetChoice(CListType);
   LoadSettings();
}

int CDPSAdvWnd::WndNotification(CXWnd *pWnd, unsigned int Message, void *unknown) {
   if (Debug && Message != 21) WriteChatf("Notify: %i", Message);
   if (Message == 10) CheckActive();
   if (Message == 3 && pWnd == (CXWnd*)LTopList) LTopList->SetCurSel(-1);
   else if (Message == 10 && pWnd == (CXWnd*)DPSWnd) CheckActive();
   else if (Message == 1) {
      if (pWnd == (CXWnd*)Tabs) LoadSettings();
      else if (pWnd == (CXWnd*)CShowMeTop) ShowMeTop = CShowMeTop->Checked ? true : false;
      else if (pWnd == (CXWnd*)CShowMeMin) ShowMeMin = CShowMeMin->Checked ? true : false;
      else if (pWnd == (CXWnd*)CUseRaidColors) UseRaidColors = CUseRaidColors->Checked ? true : false;
      else if (pWnd == (CXWnd*)CLiveUpdate) LiveUpdate = CLiveUpdate->Checked ? true : false;
      //else if (pWnd == (CXWnd*)LTopList) WriteChatf("CurSel: %i", LTopList->GetCurSel());
      else if (pWnd == (CXWnd*)CShowTotal) {
         ShowTotal = CShowTotal->GetCurChoice();
         if (ShowTotal == 4) ShowTotal = 0;
         LoadSettings();
      } else if (pWnd == (CXWnd*)CMobList) {
         CurListMob = 0;
         LTopList->DeleteAll();
         bool FoundMob = false;
         if ((int)CMobList->GetCurChoice() > 1) {
            CListType = 2;
            DPSMob *ListMob = 0;
            int i = 0, x = 0;
            for (i = 0; i < (int)MobList.size() - 1; i++) {
               ListMob = MobList[i];
               if (ListMob->SpawnType == 1 && ListMob->Active && !ListMob->IsPet() && !ListMob->Mercenary) {
                  if (x + 2 == (int)CMobList->GetCurChoice()) {
                     FoundMob = true;
                     ListSwitch(ListMob);
                     break;
                  }
                  x++;
               }
            }
            if (!FoundMob) {
               CListType = 0;
               DPSWnd->DrawCombo();
            }
         } else CListType = (int)CMobList->GetCurChoice();
         Intervals -= 1; // Force update next Pulse.
      }
   } else if (Message == 14) {
	   CHAR szTemp[MAX_STRING] = { 0 };
      GetCXStr(((CEditWnd*)pWnd)->InputText, szTemp);
      if (pWnd == (CXWnd*)TShowMeMin) {
         if (strlen(szTemp)) {
            szTemp[2] = 0;
            ShowMeMinNum = atoi(szTemp);
            sprintf_s(szTemp, "%i", ShowMeMinNum);
            SetCXStr(&TShowMeMin->InputText, szTemp);
            TShowMeMin->SetSel(strlen(szTemp), 0);
         }
      } else if (pWnd == (CXWnd*)TFightIA) {
         if (strlen(szTemp)) {
            szTemp[2] = 0;
            FightIA = atoi(szTemp);
            if (FightIA < 3) FightIA = 8;
            sprintf_s(szTemp, "%i", FightIA);
            //SetCXStr(&TFightTO->InputText, szTemp);
            //TFightTO->SetSel(strlen(szTemp), 0);
         }
      } else if (pWnd == (CXWnd*)TFightTO) {
         if (strlen(szTemp)) {
            szTemp[2] = 0;
            FightTO = atoi(szTemp);
            if (FightTO < 3) FightTO = 30;
            sprintf_s(szTemp, "%i", FightTO);
            //SetCXStr(&TFightTO->InputText, szTemp);
            //TFightTO->SetSel(strlen(szTemp), 0);
         }
      } else if (pWnd == (CXWnd*)TEntTO) {
         if (strlen(szTemp)) {
            szTemp[2] = 0;
            EntTO = atoi(szTemp);
            if (EntTO < 3) EntTO = 8;
            sprintf_s(szTemp, "%i", EntTO);
            //SetCXStr(&TEntTO->InputText, szTemp);
            //TEntTO->SetSel(strlen(szTemp), 0);
         }
      }
   }
   

   return CSidlScreenWnd::WndNotification(pWnd,Message,unknown);
};

template <unsigned int _NameSize>DPSMob *GetMob(CHAR(&Name)[_NameSize], bool Create, bool Alive) {
   //ParsePet(Name);
   if (LastMob && (!Alive || (Alive && !LastMob->Dead)) && !_stricmp(LastMob->Name, Name)) return LastMob;
   else {
      if (LastMob && LastMob->LastEntry && LastMob->LastEntry->DoSort) LastMob->LastEntry->Sort();
      for (int i = 0; i < (int)MobList.size(); i++) {
         if ((!Alive || (Alive && !MobList[i]->Dead)) && !_stricmp(MobList[i]->Name, Name)) {
            LastMob = MobList[i];
            return LastMob;
         }
      }
   }
   if (Create) {
      LastMob = new DPSMob(Name,_NameSize);
      MobList.push_back(LastMob);
      return LastMob;
   }
   return 0;
}

template <unsigned int _EntSize, unsigned int _MobSize>bool SplitStringOtherHitOther(PCHAR Line, CHAR(&EntName)[_EntSize], CHAR(&MobName)[_MobSize], int *Damage) {
	if (strstr(Line, "injured by falling"))
		return false;
	int HitPos = 0, Action = 0;
	if (!strpbrk(Line, "1234567890"))
		return false;
	else
		*Damage = atoi(strpbrk(Line, "1234567890"));
   while (OtherHits[Action]) {
      HitPos = (int)(strstr(Line, OtherHits[Action]) - Line);
      if (HitPos > 0)
		  break;
      Action++;
   }
   if (HitPos <= 0){
      if (*Damage > 0 && !WarnedOHO) {
         WriteChatf("\ar[MQ2DPSAdv] Error: Can not use Other Hits Other: Numbers Only Hitmodes for DPS Parsing.");
         if (Debug) WriteChatf("[Debug] OtherHitOther - Line: \ap%s", Line);
         WarnedOHO = true;
      }
      return false;
   }
   strncpy_s(EntName, &Line[0], HitPos);
   EntName[HitPos] = 0;
   int DmgPos = (int)(strstr(Line, " for ") - Line);
   int MobStart = HitPos + strlen(OtherHits[Action]);
   int MobLength = DmgPos - MobStart;
   strncpy_s(MobName, &Line[MobStart], MobLength);
   MobName[MobLength] = 0;
   if (!_stricmp(MobName, "himself") || !_stricmp(MobName, "herself") || !_stricmp(MobName, "itself")) strcpy_s(MobName, EntName);
   return true;
}

template <unsigned int _MobSize> bool SplitStringYouHitOther(PCHAR Line, CHAR(&MobName)[_MobSize], int *Damage)
{
   int Action = 0, HitPos = 0, DmgPos, MobStart, MobLength;
   if (!strpbrk(Line, "1234567890"))
	   return false;
   else
	   *Damage = atoi(strpbrk(Line, "1234567890"));
   while (YourHits[Action]) {
      HitPos = (int)(strstr(Line, YourHits[Action]) - Line);
      if (HitPos >= 0) break;
      Action++;
   }
   if (HitPos < 0) {
      if (*Damage > 0 && CurTarMob) {
         strcpy_s(MobName, CurTarMob->Name);
         if (!WarnedYHO) {
            WriteChatf("\ay[MQ2DPSAdv] Warning: You Hit Other: Numbers Only may result in inaccuracy due to non-targetted attacks.");
			if (Debug) WriteChatf("[Debug] Line: %s", Line);
            WarnedYHO = true;
         }
         return true;
      }
      return false;
   }
   DmgPos = (int)(strstr(Line, " for ") - Line);
   MobStart = HitPos + strlen(YourHits[Action]);
   MobLength = DmgPos - MobStart;
   strncpy_s(MobName, &Line[MobStart], MobLength);
   MobName[MobLength] = 0;
   return true;
}

template <unsigned int _EntSize, unsigned int _MobSize>bool SplitStringNonMelee(PCHAR Line, CHAR(&EntName)[_EntSize], CHAR(&MobName)[_MobSize], int *Damage) {
	if (strstr(Line, "You were hit"))
		return false;
	if (!strpbrk(Line, "1234567890"))
		return false;

	*Damage = atoi(strpbrk(Line, "1234567890"));

	char temp[MAX_STRING] = "";
	sprintf_s(temp, " for %i", *Damage);
	int MobEnd = (int)(strstr(Line, temp) - Line);
	//If this was a direct damage ability [EntName] hit [MobName] for [Damage] points of [DamageType] damage.
	if (MobEnd <= 0) {
		if (Debug) WriteChatf("NonMelee Failed, No MobEnd - Line: %s", Line);
	}
   if (MobEnd > 0) {
	   if ((strstr(Line, ((PSPAWNINFO)pCharSpawn)->DisplayedName) || strstr(Line, "YOUR") || strstr(Line, " is "))) {
		   sprintf_s(temp, ((PSPAWNINFO)pCharSpawn)->DisplayedName);
		   if (strstr(Line, " YOUR ") || strstr(Line, " is ")) {
			   //Process your damage shields.
			   MobEnd = (int)(strstr(Line, " is ") - Line);
			   sprintf_s(temp, " is ");
			   strncpy_s(MobName, &Line[0], MobEnd);
		   }
		   else {
			   //Normal Non-Melee
			   strncpy_s(MobName, &Line[strlen(temp) + 5], MobEnd - strlen(temp) - 5);
		   }
		   if (strstr(Line, " is ") && !strstr(Line, " YOUR ")) {
			   //Process other's Damage Shields.
			   int EntEnd = 0;
			   int EntStart = 0;
			   if (strstr(Line, " pierced by ")) {
				   //Thorns DS
				   EntEnd = (int)(strstr(Line, "'s thorns for ") - Line);
				   EntStart = (int)(strstr(Line, " pierced by ") - Line) + 12;
				   strncpy_s(EntName, &Line[EntStart], EntEnd - EntStart);
			   }
			   else if (strstr(Line, " burned by ")) {
				   //Fire DS
				   EntEnd = (int)(strstr(Line, "'s flames for ") - Line);
				   EntStart = (int)(strstr(Line, " burned by ") - Line) + 11;
				   strncpy_s(EntName, &Line[EntStart], EntEnd - EntStart);
			   }
			   else if (strstr(Line, " tormented by ")) {
				   //Frost DS
				   EntEnd = (int)(strstr(Line, "'s frost for ") - Line);
				   EntStart = (int)(strstr(Line, " tormented by ") - Line) + 14;
				   strncpy_s(EntName, &Line[EntStart], EntEnd - EntStart);
			   }
			   if (EntEnd < 0 || EntStart < 0)
				   return false;
		   }
		   else {
			   strcpy_s(EntName, ((PSPAWNINFO)pCharSpawn)->DisplayedName);
		   }
		   if (!strlen(MobName) || !strlen(EntName)) {
			   if (Debug) WriteChatf("NonMelee Fail: %s", Line);
			   return false;
		   }
			MobName[MobEnd] = 0;
			if (Debug) WriteChatf("\ap%s\ax was hit by \ap%s \axfor \ap%i \axdamage", MobName, EntName, *Damage);
		    return true;
	   }
	   sprintf_s(temp, " hit %s", MobName);
	   int EntEnd = (int)(strstr(Line, temp) - Line);
	   if (EntEnd <= 0) {
		   if (Debug) WriteChatf("NonMelee Fail: %s",Line);
		   return false;
	   }
	   strncpy_s(EntName, &Line[0], EntEnd);
	   EntName[EntEnd] = 0;
	   MobEnd = (int)(strstr(Line, " for ") - Line);
	   int MobLength = MobEnd - 5 - strlen(EntName);
	   strncpy_s(MobName, &Line[EntEnd + 5], MobLength);
	   MobName[MobLength] = 0;
	   if (!strlen(MobName) || !strlen(EntName)) {
		   if (Debug) WriteChatf("NonMelee Fail: %s", Line);
		   return false;
	   }
	   return true;
   }
   return false;
}

template <unsigned int _MobSize>bool SplitStringDeath(PCHAR Line, CHAR(&MobName)[_MobSize])
{
   int MobStart = 0, MobLength = 0;
   MobLength = (int)(strstr(Line, " died.") - Line);
   if (MobLength <= 0) MobLength = (int)(strstr(Line, " has been slain by") - Line);
   if (MobLength <= 0) {
      MobStart = 15;
      MobLength = strlen(Line) - 16;
   }
   if (MobLength <= 0)
	   return false;
   strncpy_s(MobName, &Line[MobStart], MobLength);
   MobName[MobLength] = 0;
   return true;
}

template <unsigned int _EntSize, unsigned int _MobSize>bool SplitStringDOT(PCHAR Line, CHAR(&EntName)[_EntSize], CHAR(&MobName)[_MobSize], int *Damage) {
   if (!strpbrk(Line, "1234567890"))
	   return false;
   else
	   *Damage = atoi(strpbrk(Line, "1234567890"));
   if (*Damage <= 0 || strstr(Line, " damage by "))
	   return false;
   int MobEnd = (int)(strstr(Line, " has taken ") - Line);

   if (strstr(Line, " damage from your ")) {
	   strcpy_s(EntName, ((PSPAWNINFO)pCharSpawn)->DisplayedName);
   } else {
	  int EntEnd = (int)(strstr(Line, ".") - Line-6);
      if (MobEnd <= 0 || EntEnd <= 0) return false;
      int SpellNameStart = (int)(strstr(Line, " damage from ") - Line);
	  int SpellNameEnd = (int)(strstr(Line, " by ") - Line);
	  char temp[MAX_STRING] = "";
      strncpy_s(temp, &Line[SpellNameStart + 13], SpellNameEnd - SpellNameStart - 13);
	  int EntStart = (int)(strstr(Line,temp) - Line + strlen(temp)+4);
	  strcpy_s(EntName, &Line[EntEnd, EntStart]);
      EntName[EntEnd - EntStart+6] = 0;
   }
   strncpy_s(MobName, &Line[0], MobEnd);
   int Corpse = (int)(strstr(MobName, "'s corpse") - MobName);
   if (Corpse > 0)
	   MobName[Corpse] = 0;
   else
	   MobName[MobEnd] = 0;
   return true;
}

void HandleNonMelee(PCHAR Line) {
	char EntName[64] = { 0 }, MobName[64] = { 0 };
   int Damage;
   if (!SplitStringNonMelee(Line, EntName, MobName, &Damage))
	   return;
   if (Debug) WriteChatf("[HandleNonMelee] \ap%s hit \ap%s for \ar%i", EntName, MobName, Damage);
   GetMob(MobName, true, true)->GetEntry(EntName)->AddDamage(Damage);
}

void HandleDOT(PCHAR Line) {
	char EntName[64] = { 0 }, MobName[64] = { 0 };
	int Damage;
	if (!SplitStringDOT(Line, EntName, MobName, &Damage))
		return;
	if (Debug) WriteChatf("[HandleDot] \ap%s hit \ap%s for \ar%i", EntName, MobName, Damage);
	GetMob(MobName, true, true)->GetEntry(EntName)->AddDamage(Damage);
}

void HandleOtherHitOther(PCHAR Line) {
	char EntName[64] = { 0 }, MobName[64] = {0};
   int Damage;
   if (!SplitStringOtherHitOther(Line, EntName, MobName, &Damage))
	   return;
   if (Debug) WriteChatf("[OtherHitOther] \ap%s hit \ap%s for \ar%i", EntName, MobName, Damage);
   if (DPSMob *mob = GetMob(MobName, true, true)) {
	   if (DPSMob::DPSEntry *entry = mob->GetEntry(EntName)) {
		   entry->AddDamage(Damage);
	   }
   }
}

void HandleYouHitOther(PCHAR Line) {
	char MobName[64] = { 0 };
	int Damage;
	if (!SplitStringYouHitOther(Line, MobName, &Damage))
		return;
	if (Debug) WriteChatf("[YouHitOther] You hit \ap%s for \ar%i damage!", MobName, Damage);
	if (pCharSpawn) {
		if (DPSMob *mob = GetMob(MobName, true, true)) {
			if (DPSMob::DPSEntry *entry = mob->GetEntry(((PSPAWNINFO)pCharSpawn)->DisplayedName)) {
				entry->AddDamage(Damage);
			}
		}
	}
}

void HandleDeath(PCHAR Line) {
	char MobName[64] = { 0 };
   if (!SplitStringDeath(Line, MobName)) return;
   if (Debug) WriteChatf("Handling Dead Mob: \ap%s", MobName);
   if(DPSMob *DeadMob = GetMob(MobName, false, true)) {
      HandleDeath(DeadMob);
	  if (!DeadMob->IsPet() && !DeadMob->Mercenary) {
		  DPSWnd->DrawCombo();
	  }
   }
}

void HandleDeath(DPSMob *DeadMob) {
   if (DeadMob == CurListMob) {
      DPSWnd->DrawList();
      //CurListMob = 0;
   }
   if (DeadMob == CurTarMob) {
      CurTarMob = 0;
      CurTarget = 0;
   }
   if (DeadMob == CurMaxMob) {
      MaxDmgLast = (int)CurMaxMob->Damage.Last;
      CurMaxMob = 0;
   }
   sprintf_s(DeadMob->Tag, "[D] ");
   DeadMob->Dead = true;
}

#ifdef DPSDEV
void DPSTestCmd(PSPAWNINFO pChar, PCHAR szLine) {

}
#endif

void DPSAdvCmd(PSPAWNINFO pChar, PCHAR szLine) {
   char Arg1[MAX_STRING];
   GetArg(Arg1, szLine, 1);
   if (!_stricmp(Arg1, "show"))
      if (!DPSWnd) WriteChatf("\arDPSWnd does not exist. Try reloading your UI (/loadskin default).");
      else DPSWnd->dShow = 1;
   else if (!_stricmp(Arg1, "colors"))
      ((CXWnd*)pRaidOptionsWnd)->Show(1, 1);
   else if (DPSWnd && !_stricmp(Arg1, "reload"))
      DPSWnd->LoadLoc();
   else if (DPSWnd && !_stricmp(Arg1, "save"))
      DPSWnd->SaveLoc();
   else if (!_stricmp(Arg1, "listsize"))
      WriteChatf("\ayMobList Size: %i", MobList.size());
   else if (!_stricmp(Arg1, "copy")) {
      char szCopy[MAX_STRING];
      GetArg(szCopy, szLine, 2);
      if (DPSWnd) {
         DPSWnd->LoadLoc(szCopy);
         DPSWnd->SaveLoc();
      } else WriteChatf("\arFailed to Copy: DPS Window not loaded.");
   }
   else if (!_stricmp(Arg1, "Debug")) {
	   Debug = Debug ? false : true;
	   WriteChatf("Debug is now: %s", Debug ? "\agOn" : "\arOff");
   }
   CheckActive();
}

void CreateDPSWindow() {
   if (DPSWnd) DestroyDPSWindow();
   if (pSidlMgr->FindScreenPieceTemplate("DPSAdvWnd")) {
      DPSWnd = new CDPSAdvWnd();
      if (DPSWnd->dShow) ((CXWnd*)DPSWnd)->Show(1,1);
      char szTitle[MAX_STRING];
      sprintf_s(szTitle, "DPS Advanced v%s", DPSVERSION);
      SetCXStr(&DPSWnd->WindowText, szTitle);
   }
   CheckActive();
}

void DestroyDPSWindow() {
   if (DPSWnd) {
      DPSWnd->SaveLoc();
      delete DPSWnd;
      DPSWnd=0;
   }
   CheckActive();
}

PLUGIN_API VOID SetGameState(DWORD GameState) {
   DebugSpewAlways("GameState Change: %i", GameState);
   if (GameState==GAMESTATE_INGAME) {
      if (!DPSWnd) CreateDPSWindow();
   }
}

PLUGIN_API VOID OnCleanUI(VOID)   { DestroyDPSWindow(); }
PLUGIN_API VOID OnReloadUI(VOID) { if (gGameState == GAMESTATE_INGAME && pCharSpawn) CreateDPSWindow(); }

PLUGIN_API VOID InitializePlugin(VOID) {
   LastMob = 0;
   CurTarget = 0;
   CurTarMob = 0;
   CurListMob = 0;
   CurMaxMob = 0;
   Zoning = false;
   ShowMeTop = false;
   WrongUI = false;
   AddXMLFile("MQUI_DPSAdvWnd.xml");
   AddCommand("/dpsadv", DPSAdvCmd);
#ifdef DPSDEV
   AddCommand("/dpstest", DPSTestCmd);
#endif
   CheckActive();
   if (gGameState != GAMESTATE_INGAME || !pCharSpawn) return;
   else CreateDPSWindow();
}

PLUGIN_API VOID ShutdownPlugin(VOID) {
   DestroyDPSWindow();
   RemoveCommand("/dpsadv");
#ifdef DPSDEV
   RemoveCommand("/dpstest");
#endif
}



PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color) {
	if (gGameState != GAMESTATE_INGAME || !pCharSpawn) return 0;
	if (Active) {
		//WriteChatf("%i: %s", Color, Line);
		switch (Color) {
			case 0://Color: 0 - Task Updates

			case 2: //Color: 2 - Consider Message - Green

			case 4: //Color: 4 - Consider messages - Dark Blue

			case 6://Color: 6 - Consider message - Grey

			case 10://Color: 10 - Merchant says "Hello there, Soandso. How about a nice Iron Ration?"

			case 13://Color: 13 - Attack is on/off - Other invites to raid.

			case 15://Color: 15 - Help messages, Consider Message - Yellow (right click on the NPC to consider it, Targeted (NPC): a slatescale serpent, You have gained an ability point)

			case 18://Color: 18 - Consider message - Light blue

			case 20://Color: 20 - You regain some experience from resurrection.

			case 256://Color: 256 - Other player /say messages	
			case 257://Color: 257 - Other /tell's you
			case 258://Color: 258 - Other tells /group
			case 259://Color: 259 - Guild Chat - Incoming
			case 260://Color: 260 - Other /ooc
			case 261://Color: 261 - Other /auction
			case 262://Color: 262 - Other /shout	
			case 263://Color: 263 - other Emote commands/eat food messages (/cry, Chomp, chomp, chomp... Soandso takes a bite...etc)
			case 264://Color: 264 - You begin casting
				break;
			case 265://Color: 265 - You hit other (melee/melee crit)
				HandleYouHitOther(Line);
				break;
			case 266://Color: 266 - Other hits you	
			case 267://Color: 267 - You miss other	
			case 268://Color: 268 - Riposte/Dodge/Other Misses You
			case 269://Color: 269 - Announcement (GM Events etc)
			case 270://Color: 270 - You have become better at [Skill]! (Number)
			case 271://Color: 271 - Song wear off message?? (Ritual Scarification wear off message)
			case 272://Color: 272 - Pet thanks for stuff you give them?
			case 273://Color: 273 - Charged mercenary upkeep/Group Invites/Joins/Banker tells you welcome to bank.
			case 274://Color: 274 - Faction hits
			case 275://Color: 275 - [Merchant] tells you, that'll be [price] for the [item]
			case 276://Color: 276 - You sold [QTY] [Item Name] to [Player] for [Amount]/You Purchased [Item Name] from [NPC] for [Amount]
			case 277://Color: 277 - Your death message.
			case 278://Color: 278 - Other PC death message.
				break;
			case 279://Color: 279 - Others Hits Other
				HandleOtherHitOther(Line); //Other hits other
				break;
			case 280://Color: 280 - Other Misses Other	
			case 281://Color: 281 - /who
			case 282://Color: 282 - Other /yell
				break;
			case 283://Color: 283 - Your Non-Melee spell dmg.
				HandleNonMelee(Line); //Your Non-melee
				break;
			case 284://Color: 284 - Spell wear off messages
			case 285://Color: 285 - Coin collection/Split
			case 286://Color: 286 - Loot Related (Master looter actions, you loot)	
			case 287://Color: 287 - Dice Rolls	
			case 288://Color: 288 - Other Starts Casting Messages	
			case 289://Color: 289 - Target is out of range, get closer
			case 290://Color: 290 - Channel Lists after logged in
			case 291://Color: 291 - Chat Channel 1 - Incoming
			case 292://Color: 292 - Chat Channel 2 - Incoming
			case 293://Color: 293 - Chat Channel 3 - Incoming
			case 294://Color: 294 - Chat Channel 4 - Incoming
			case 295://Color: 295 - Chat Channel 5 - Incoming
			case 296://Color: 296 - Chat Channel 6 - Incoming
			case 297://Color: 297 - Chat Channel 7 - Incoming
			case 298://Color: 298 - Chat Channel 8 - Incoming
			case 299://Color: 299 - Chat Channel 9 - Incoming
			case 300://Color: 300 - Chat Channel 10 - Incoming

			case 303://Color: 303 - Errors (You must first click on the being you wish to attack, Can't hit them from here)

			case 306://Color: 306 - Enrage (Showed for pet)
			case 307://Color: 307 - Your /say messages, mob advances messages.	
			case 308://Color: 308 - You tell other.	
			case 309://Color: 309 - Group conversation
			case 310://Color: 310 - Guild conversation - Outgoing
			case 311://Color: 311 - you /ooc
			case 312://Color: 312 - you /auction
			case 313://Color: 313 - you shout.
			case 314://Color: 314 - /emote messages
			case 315://Color: 315 - Chat Channel 1 - Outgoing
			case 316://Color: 316 - Chat Channel 2 - Outgoing
			case 317://Color: 317 - Chat Channel 3 - Outgoing
			case 318://Color: 318 - Chat Channel 4 - Outgoing
			case 319://Color: 319 - Chat Channel 5 - Outgoing
			case 320://Color: 320 - Chat Channel 6 - Outgoing
			case 321://Color: 321 - Chat Channel 7 - Outgoing
			case 322://Color: 322 - Chat Channel 8 - Outgoing
			case 323://Color: 323 - Chat Channel 9 - Outgoing
			case 324://Color: 324 - Chat Channel 10 - Outgoing

			case 327://Color: 327 - Any /rsay
				break;
			case 328://Color: 328 - your pet misses
				HandleOtherHitOther(Line); // Your Pet
				break;
			case 329://Color: 329 - Damage Shield hits you.
			case 330://Color: 330 - Raid Role messages.

			case 333://Color: 333 - Item Focus messages	
			case 334://Color: 334 - You gain Experience messages
			case 335://Color: 335 - You have already finished collecting [Item].
				break;
			case 336://Color: 336 - Pet Non-Melee/Pet beings to cast
				HandleNonMelee(Line); //All pet non-melee (yours and others)
				break;
			case 337://Color: 337 - Pet messages (YourPet says, "Following you master.")

			case 339://Color: 339 - Strike thru (yours and others)	
			case 340://Color: 340 - You are stunned/unstunned messages.

			case 342://Color: 342 - fellowship messages	
			case 343://Color: 343 - corpse emote

			case 345://Color: 345 - Guild plants banner
			case 346://Color: 346 - Mercenary tells group

			case 348://Color: 348 - Achievement - you and other.
			case 349://Color: 349 - Achievement - Guildmate
				break;
			case 358://Color: 358 - NPC Death message
				HandleDeath(Line); //NPC died is Color: 358 Your death is Color: 277(this was 278)
				break;
			case 360://Color: 360 - Other Rolls Dice.
			case 361://Color: 361 - Injured by falling (self)
			case 362://Color: 362 - Injured by falling (other)
				break;
			case 363://Color: 363 - Your damage shield
				//HandleNonMelee(Line); //Your Damage shield
			case 364://Color: 364 - Other's damage shield
				HandleNonMelee(Line); //Other's damage shield
				break;
			case 366://Color: 366 - Your [Spell Name] has been overwritten - Detrimental
			case 367://Color: 367 - Your [Spell Name] has been overwritten - Beneficial
			case 368://Color: 368 - You cannot use that command right now (clicking disc too fast)
			case 369://Color: 369 - You can use [Ability Name] again in [Time till you can use it again]
			case 370://Color: 370 - AA Reuse Timer failed
			case 371://Color: 371 - Destroy item message

			case 373://Color: 373 - Heal over time on other?
			case 374://Color: 374 - You heal other	
			case 375://Color: 375 - Other buffs other/Other Heal Other
				break;
			case 376://Color: 376 - Your DoT's
				//HandleDOT(Line); //Your DoTs
			case 377://Color: 377 - Other's DoT's
				HandleDOT(Line); //Your DoTs
				break;
			case 378://Color: 378 - Song messages - Soandso begins to sing a song. <Selo's Sonata I>
				break;
			case 379://Color: 379 - Others Non-Melee
				HandleNonMelee(Line); //Others Non-melee
				break;
			case 380://Color: 380 - Your spell messages
				break;
			default:
				//Don't Know these, lets see what it is. 
				if (Debug) WriteChatf("[\ar%i\ax]\a-t: \ap%s", Color, Line);
				break;
		}
	}
	return 0;
}

bool CheckInterval() {
   if (!Intervals) Intervals = time(NULL);
   else if (Intervals != time(NULL)) {
      Intervals = time(NULL);
      return true;
   }
   return false;
}

void CheckActive() {
   if (DPSWnd && DPSWnd->dShow && !Zoning && !WrongUI) Active = true;
   else Active = false;
}

void ListSwitch(DPSMob *Switcher) {
   CurListMob = Switcher;
   DPSWnd->LTopList->SetCurSel(-1);
   DPSWnd->LTopList->VScrollPos = 0;
   DPSWnd->DrawList(true);
   DPSWnd->DrawCombo();
}

void TargetSwitch() {
   CurTarget = (PSPAWNINFO)pTarget;
   CurTarMob = GetMob(CurTarget->DisplayedName, true, CurTarget->Type == SPAWN_CORPSE ? false : true);
   //if (CurTarMob->Active && CurTarMob->SpawnType == 1) ListSwitch();
}

void IntPulse() {
   bool CChange = false;
   CurMaxMob = 0;
   for (int i = 0; i < (int)MobList.size(); i++) {
      DPSMob *Mob = MobList[i];
      if ((!Mob->Active || ((Mob->IsPet() || Mob->SpawnType == SPAWN_PLAYER || Mob->Mercenary) && Mob->InActive)) && Mob != CurTarMob && Mob != CurListMob && Mob != LastMob && Mob != CurMaxMob) {
         MobList.erase(MobList.begin() + i);
         delete Mob;
         i--;
      } else {
         if (Mob->Active && !Mob->Dead && time(NULL) - Mob->Damage.Last > FightTO) {
            HandleDeath(Mob);
            CChange = true;
         } else if (Mob->Active && !Mob->Dead && !Mob->InActive && time(NULL) - Mob->Damage.Last > FightIA) {
            Mob->InActive = true;
            sprintf_s(Mob->Tag, "[IA] ");
            if (!Mob->IsPet() && !Mob->Mercenary) CChange = true;
         } else if (Mob->Active && !Mob->Dead && Mob->InActive && time(NULL) - Mob->Damage.Last < FightIA) {
            Mob->InActive = false;
            sprintf_s(Mob->Tag, "[A] ");
            if (!Mob->IsPet() && !Mob->Mercenary) CChange = true;
         }         
         if (Mob->Active && !Mob->InActive && !Mob->Dead && !Mob->IsPet() && Mob->SpawnType == SPAWN_NPC && (int)Mob->Damage.Last > MaxDmgLast && (!CurMaxMob || Mob->Damage.Total > CurMaxMob->Damage.Total)) CurMaxMob = Mob;
      }
   }
   if (CListType == CLISTMAXDMG && CurMaxMob && CurMaxMob != CurListMob) ListSwitch(CurMaxMob);
   if (CChange) DPSWnd->DrawCombo();
   DPSWnd->DrawList();
   //WriteChatf("Active: %s", Active ? "Yes" : "No");
}

PLUGIN_API VOID OnPulse(VOID) {
   if (gGameState != GAMESTATE_INGAME || !pCharSpawn) return;

   if (Active) {
      if (LastMob && LastMob->LastEntry && LastMob->LastEntry->DoSort) LastMob->LastEntry->Sort();
      if ((PSPAWNINFO)pTarget && (PSPAWNINFO)pTarget != CurTarget) TargetSwitch();
      if (CListType == CLISTTARGET && CurTarMob && CurTarMob != CurListMob && CurTarMob->Active && CurTarMob->SpawnType == 1) ListSwitch(CurTarMob);
      if (CheckInterval()) IntPulse();
   }
}

PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn)
{
   if (!Zoning) {
      DPSMob *pMob = 0;
      DPSMob::DPSEntry *pEnt = 0;
      for (int i = 0; i < (int)MobList.size(); i++) {
         pMob = MobList[i];
         if (pMob->Spawn && pMob->Spawn->SpawnID == pSpawn->SpawnID) pMob->Spawn = 0;
         for (int x = 0; x < (int)pMob->EntList.size(); x++) {
            pEnt = pMob->EntList[x];
            if (pEnt->Spawn && pEnt->Spawn->SpawnID == pSpawn->SpawnID) pEnt->Spawn = 0;
         }
      }
   }
}

void ZoneProcess() {
   LastMob = 0;
   CurTarget = 0;
   CurTarMob = 0;
   CurListMob = 0;
   CurMaxMob = 0;
   DPSMob *pMob = 0;
   DPSMob::DPSEntry *pEnt = 0;
   for (int i = 0; i < (int)MobList.size(); i++) {
      pMob = MobList[i];
      pMob->Spawn = 0;
      for (int x = 0; x < (int)pMob->EntList.size(); x++) {
         pEnt = pMob->EntList[x];
         pEnt->Spawn = 0;
      }
      if (!pMob->Dead) HandleDeath(pMob);
   }
}

PLUGIN_API VOID OnBeginZone(VOID) {
   //DebugSpewAlways("START ZONING");
   ZoneProcess();
   Zoning = true;
   CheckActive();
}

PLUGIN_API VOID OnEndZone(VOID) {
   //DebugSpewAlways("END ZONING");
   Zoning = false;
   CheckActive();
} 