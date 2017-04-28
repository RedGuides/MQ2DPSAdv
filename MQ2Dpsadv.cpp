// DPS ADV CREATED BY WARNEN 2008-2009
// MQ2DPSAdv.cpp

#include "../MQ2Plugin.h"
PreSetup("MQ2DPSAdv");
#include <vector>
#include "MQ2DPSAdv.h"

#define DPSVERSION "1.2.03"
//#define DPSDEV

/*
 ### UPDATE THIS IF YOU MAKE A CHANGE ###

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
      GetCXStr(((CSidlScreenWnd*)pWnd)->InputText, szTemp);
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
	else {
		char *dmg = strpbrk(Line, "1234567890");
		*Damage = atoi(dmg);
	}
   while (OtherHits[Action]) {
      HitPos = (int)(strstr(Line, OtherHits[Action]) - Line);
      if (HitPos > 0)
		  break;
      Action++;
   }
   if (HitPos <= 0){
      if (*Damage > 0 && !WarnedOHO) {
         WriteChatf("\ar[MQ2DPSAdv] Error: Can not use Other Hits Other: Numbers Only Hitmodes for DPS Parsing.");
         if (Debug) WriteChatf("[Debug] Line: %s", Line);
         WarnedOHO = true;
      } //else if (Debug) WriteChatf("[Debug] Unparsed Line: %s", Line);
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
         } //else if (Debug) WriteChatf("[Debug] Unparsed Line: %s", Line);
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
   else
	   *Damage = atoi(strpbrk(Line, "1234567890"));
   int MobEnd = (int)(strstr(Line, " was hit by non-melee for ") - Line);
   if (MobEnd > 0) {
      strncpy_s(MobName, &Line[0], MobEnd);
      MobName[MobEnd] = 0;
      strcpy_s(EntName, ((PSPAWNINFO)pCharSpawn)->DisplayedName);
      return true;
   }
   int EntEnd = (int)(strstr(Line, " hit ") - Line);
   if (EntEnd <= 0) return false;
   strncpy_s(EntName, &Line[0], EntEnd);
   EntName[EntEnd] = 0;
   MobEnd = (int)(strstr(Line, " for ") - Line);
   int MobLength = MobEnd - 5 - strlen(EntName);
   strncpy_s(MobName, &Line[EntEnd + 5], MobLength);
   MobName[MobLength] = 0;
   return true;
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
   int EntEnd = (int)(strstr(Line, " by ") - Line);

   if (strstr(Line, " damage from your "))
	   strcpy_s(EntName, ((PSPAWNINFO)pCharSpawn)->DisplayedName);
   else {
      if (MobEnd <= 0 || EntEnd <= 0) return false;
      int EntStart = (int)(strstr(Line, " damage from ") - Line);
      strncpy_s(EntName, &Line[EntStart + 13], EntEnd - EntStart - 13);
      EntName[EntEnd - EntStart - 13] = 0;
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
   GetMob(MobName, true, true)->GetEntry(EntName)->AddDamage(Damage);
}

void HandleDOT(PCHAR Line) {
	char EntName[64] = { 0 }, MobName[64] = { 0 };
	int Damage;
	if (!SplitStringDOT(Line, EntName, MobName, &Damage))
		return;
	GetMob(MobName, true, true)->GetEntry(EntName)->AddDamage(Damage);
}

void HandleOtherHitOther(PCHAR Line) {
	char EntName[64] = { 0 }, MobName[64] = {0};
   int Damage;
   if (!SplitStringOtherHitOther(Line, EntName, MobName, &Damage))
	   return;
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
      if (!DPSWnd) WriteChatf("\arDPSWnd does not exist. Try reloading your UI.");
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
   CheckActive();
}

void CreateDPSWindow() {
   if (DPSWnd) DestroyDPSWindow();
   if (pSidlMgr->FindScreenPieceTemplate("DPSAdvWnd")) {
      //WriteChatf("\agCreating Window...");
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
      if (Color == 279) HandleOtherHitOther(Line);
      else if (Color == 265) HandleYouHitOther(Line);
      else if (Color == 278) HandleDeath(Line);
      else if (Color == 283) HandleNonMelee(Line);
      else if (Color == 264) HandleDOT(Line);
      else if (Color == 328) HandleOtherHitOther(Line); // Your Pet
      //else if (Color == 13) Handle13(Line);
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