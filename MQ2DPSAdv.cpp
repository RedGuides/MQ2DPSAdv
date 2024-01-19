// DPS ADV CREATED BY WARNEN 2008-2009
// MQ2DPSAdv.cpp

#include <mq/Plugin.h>
#include "MQ2DPSAdv.h"

PreSetup("MQ2DPSAdv");
PLUGIN_VERSION(1.6);

// ############################### DPSEntry Start ############################################

DPSMob::DPSEntry::DPSEntry() {
	Init();
}

DPSMob::DPSEntry::DPSEntry(CHAR EntName[], DPSMob* pParent, bool PCOnly /* = false */) {
	Init();
	Parent = pParent;
	strncpy_s(Name, EntName, sizeof(Name) - 1);
	GetSpawn(PCOnly);
}

void DPSMob::DPSEntry::Init() {
	Parent = nullptr;
	SpawnType = -1;
	Mercenary = false;
	Class = 0;
	Spawn = nullptr;
	Master = nullptr;
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

void DPSMob::DPSEntry::GetSpawn(bool PCOnly /* = false */) {
	PSPAWNINFO pSpawn = (PSPAWNINFO)pSpawnList;
	while (pSpawn) {
		if (!_stricmp(pSpawn->DisplayedName, Name) && (!PCOnly || pSpawn->Type == SPAWN_PLAYER)) {
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
		if (strstr(Name, "`s pet")) {
			UsingPetName = true;
			CHAR szMaster[MAX_STRING] = { 0 };
			strcpy_s(szMaster, Name);
			if (char* pDest = strstr(szMaster, "`s pet")) {
				pDest[0] = '\0';
			}
			Master = Parent->GetEntry(szMaster);
			return true;
		}
	}
	if (Master != nullptr && (Spawn == nullptr || Spawn->MasterID <= 0 || Master->Spawn == nullptr || Master->Spawn->SpawnID != Spawn->MasterID)) Master = nullptr;
	if (Master != nullptr) return true;
	if (Spawn && Spawn->MasterID > 0) {
		PSPAWNINFO NewMaster = (PSPAWNINFO)GetSpawnByID(Spawn->MasterID);
		if (NewMaster) {
			Master = Parent->GetEntry(NewMaster->DisplayedName, true, true);
			return Master != nullptr;
		}
	}
	return false;
}

void DPSMob::DPSEntry::AddDamage(int Damage1) {
	if (CheckMaster()) {
		Master->Pets = true;
		Master->AddDamage(Damage1);
		DoSort = true;
		return;
	}

	DoSort = true;
	time_t timenow = time(nullptr);

	if (Damage.First && (timenow - Damage.Last >= EntTO)) {
		Damage.AddTime += (int)(Damage.Last - Damage.First) + 1;
		Damage.First = 0;
	}

	Damage.Total += Damage1;
	if (!Damage.First)
		Damage.First = timenow;

	Damage.Last = timenow;

	Parent->AddDamage(Damage1);
}

uint64_t DPSMob::DPSEntry::GetDPS() {
	uint64_t dividetotal = (((Damage.Last - Damage.First) + 1) + Damage.AddTime);
	if (dividetotal == 0)
		return 0;

	return (Damage.Total / dividetotal);
}

uint64_t DPSMob::DPSEntry::GetSDPS() {
	uint64_t dividetotal = ((CurListMob->Damage.Last - CurListMob->Damage.First) + 1) + CurListMob->Damage.AddTime;
	if (dividetotal == 0)
		return 0;

	return (Damage.Total / dividetotal);
}

void DPSMob::DPSEntry::Sort() {
	DoSort = false;
	if (Master && Master->DoSort) Master->Sort();
	if (Damage.Total == 0) return;
	int i = 0, x = -1;
	bool Inserted = false;
	for (i = 0; i < (int)Parent->EntList.size(); i++) {
		if (Parent->EntList[i] == this) {
			Inserted = true;
			if (x == -1) break;
			Parent->EntList.erase(Parent->EntList.begin() + i);
			Parent->EntList.insert(Parent->EntList.begin() + x, this);
			break;
		}
		else if (x == -1 && Parent->EntList[i]->Damage.Total < Damage.Total) x = i;
	}
	if (!Inserted)
		if (x != -1) Parent->EntList.insert(Parent->EntList.begin() + x, this);
		else Parent->EntList.push_back(this);
	if (LiveUpdate && Parent == CurListMob && !CurListMob->Dead) DPSWnd->DrawList();
};

// ############################### DPSMob START ############################################

DPSMob::DPSMob() {
	Init();
}

DPSMob::DPSMob(const char* MobName, size_t MobLen) {
	Init();
	strncpy_s(Name, MobName, sizeof(Name) - 1);
	GetSpawn();
	if (strstr(Name, "`s pet"))
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
	PSPAWNINFO pSpawn = (PSPAWNINFO)pSpawnList;
	while (pSpawn) {
		if (!_stricmp(pSpawn->DisplayedName, Name)) {
			SpawnType = pSpawn->Type;
			Mercenary = pSpawn->Mercenary;
			Spawn = pSpawn;
			if (SpawnType != SPAWN_CORPSE) {
				Dead = false;
				return;
			}
			Dead = true;
		}
		pSpawn = pSpawn->pNext;
	}
	if (Debug && !Spawn) WriteChatf("\arError: Could not find Mob Spawn (%s)", Name);
}

bool DPSMob::IsPet() {
	return PetName || (Spawn && Spawn->MasterID > 0);
}

void DPSMob::AddDamage(int Damage1) {
	Damage.Total += Damage1;
	time_t timenow = time(nullptr);

	if (!Damage.First)
		Damage.First = timenow;

	Damage.Last = timenow;

	if (!Active) {
		Active = true;
		sprintf_s(Tag, "[A] ");

		if (!IsPet() && !Mercenary)
			DPSWnd->DrawCombo();
	}
}

DPSMob::DPSEntry* DPSMob::GetEntry(char EntName[], bool Create /* = true */, bool PCOnly /* = false */) {
	char szBuffer[256] = { 0 };
	strcpy_s(szBuffer, 256, EntName);
	if (!_stricmp(szBuffer, "You")) {
		strcpy_s(szBuffer, 256, ((PSPAWNINFO)pLocalPlayer)->Name);
	}

	if (LastEntry && !strcmp(LastEntry->Name, szBuffer) && (!PCOnly || LastEntry->SpawnType == SPAWN_PLAYER)) return LastEntry;

	if (LastEntry && LastEntry->DoSort) LastEntry->Sort();
	// TODO: This and other areas like this should probably be range based for loops.
	for (int i = 0; i < (int)EntList.size(); i++) {
		if (EntList[i] != nullptr && !strcmp(EntList[i]->Name, szBuffer) && (!PCOnly || EntList[i]->SpawnType == SPAWN_PLAYER)) {
			LastEntry = EntList[i];
			return LastEntry;
		}
	}

	if (Create) {
		LastEntry = new DPSEntry(szBuffer, this, PCOnly);
		EntList.push_back(LastEntry);
		return LastEntry;
	}
	return nullptr;
}

// ############################### CDPSAdvWnd START ############################################

CDPSAdvWnd::CDPSAdvWnd() :CCustomWnd("DPSAdvWnd") {
	// TODO:  This looks like it can all be done with WrongUI
	bool CheckUI = false;

	// Tabs holder exists?
	if (!(Tabs = (CTabWnd*)GetChildItem("DPS_Tabs"))) CheckUI = true;

	// DPS Settings Tab
	if (!(CShowMeTop = (CCheckBoxWnd*)GetChildItem("DPS_ShowMeTopBox"))) CheckUI = true;
	if (!(CShowMeMin = (CCheckBoxWnd*)GetChildItem("DPS_ShowMeMinBox"))) CheckUI = true;
	if (!(TShowMeMin = (CEditWnd*)GetChildItem("DPS_ShowMeMinInput"))) CheckUI = true;
	if (!(CUseRaidColors = (CCheckBoxWnd*)GetChildItem("DPS_UseRaidColorsBox"))) CheckUI = true;
	if (!(CLiveUpdate = (CCheckBoxWnd*)GetChildItem("DPS_LiveUpdateBox"))) CheckUI = true;
	if (!(CUseTBMKOutput = (CCheckBoxWnd*)GetChildItem("DPS_UseTBMKOutputBox"))) CheckUI = true;
	if (!(TFightIA = (CEditWnd*)GetChildItem("DPS_FightIAInput"))) CheckUI = true;
	if (!(TFightTO = (CEditWnd*)GetChildItem("DPS_FightTOInput"))) CheckUI = true;
	if (!(TEntTO = (CEditWnd*)GetChildItem("DPS_EntTOInput"))) CheckUI = true;
	if (!(THistoryLimit = (CEditWnd*)GetChildItem("DPS_HistoryLimitInput"))) CheckUI = true;
	if (!(CShowTotal = (CComboWnd*)GetChildItem("DPS_ShowTotal"))) CheckUI = true;
	// End DPS Settings Tab

	// DPS Tab
	if (!(LTopList = (CListWnd*)GetChildItem("DPS_TopList"))) CheckUI = true;
	if (!(CMobList = (CComboWnd*)GetChildItem("DPS_MobList"))) CheckUI = true;

	// Setting this here sets it for the entire window. Setting everything individually was blacking out the checkboxes!
	this->SetBGColor(MQColor(0, 0, 0));

	if (CheckUI) {
		WriteChatf("\ar[MQ2DPSAdv] Incorrect UI File in use. Please update to latest and reload plugin.");
		WrongUI = true;
	}
	else
	{
		WrongUI = false;

		LoadLoc();
		CMobList->SetColors(MQColor(204, 51, 51), MQColor(102, 102, 102), MQColor(0, 0, 0));
		Tabs->UpdatePage();
		DrawCombo();
	}
}

CDPSAdvWnd::~CDPSAdvWnd() {}

void CDPSAdvWnd::DrawCombo() {
	int CurSel = CMobList->GetCurChoice();
	CMobList->DeleteAll();
	CHAR szTemp[MAX_STRING] = { 0 };
	// TODO:  There's probably a better way to do this since this code is duplicated, but this moves the previous ternaries into a more readable format.
	strcpy_s(szTemp, "<Target> ");
	if (CListType == CLISTTARGET && CurListMob != nullptr)
	{
		strcat_s(szTemp, CurListMob->Tag);
		strcat_s(szTemp, CurListMob->Name);
	}
	else
	{
		strcat_s(szTemp, "None");
	}
	CMobList->InsertChoice(szTemp);

	strcpy_s(szTemp, "<MaxDmg> ");
	if (CListType == CLISTMAXDMG && CurListMob != nullptr)
	{
		strcat_s(szTemp, CurListMob->Tag);
		strcat_s(szTemp, CurListMob->Name);
	}
	else
	{
		strcat_s(szTemp, "None");
	}
	CMobList->InsertChoice(szTemp);

	DPSMob* Mob = nullptr;
	int ListSize = 0;
	for (int i = 0; i < (int)MobList.size(); ++i) {
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

void CDPSAdvWnd::SetTotal(int LineNum, DPSMob* Mob) {
	CHAR szTemp[MAX_STRING] = { 0 };
	SetLineColors(LineNum, 0, true);
	//Index
	LTopList->SetItemText(LineNum, 0, "-");
	//Name
	LTopList->SetItemText(LineNum, 1, "Total");
	//Total
	sprintf_s(szTemp, "%llu", CurListMob->Damage.Total);
	if (!UseTBMKOutputs)
		PrettifyNumber(szTemp, sizeof(szTemp));
	else {
		MakeItTBMK(szTemp);
		//Need to turn the strings from like 125556 to 125.5k
	}
	LTopList->SetItemText(LineNum, 2, szTemp);
	//DPS
	sprintf_s(szTemp, "%llu", CurListMob->Damage.Total / (int)((CurListMob->Damage.Last - CurListMob->Damage.First) + 1));
	if (!UseTBMKOutputs)
		PrettifyNumber(szTemp, sizeof(szTemp));
	else {
		MakeItTBMK(szTemp);
		//Need to turn the strings from like 125556 to 125.5k
	}
	LTopList->SetItemText(LineNum, 3, szTemp);
	//Time
	sprintf_s(szTemp, "%I64is", CurListMob->Damage.Last - CurListMob->Damage.First);
	LTopList->SetItemText(LineNum, 4, szTemp);
	//This is where Percentage would go, if it wasn't always going to be 100%, if more columns are added, make sure to skip 5!
}

void MakeItTBMK(char* szLine) {
	uint64_t value = GetInt64FromString(szLine, 0);
	if (value >= 1000000000000) {
		value = value / 100000000000;
		_i64toa_s(value, szLine, MAX_STRING, 10);
		std::string temp = szLine;
		temp.insert(temp.end() - 1, '.');
		sprintf_s(szLine, MAX_STRING, temp.c_str());
		strcat_s(szLine, MAX_STRING, " t");
	}
	else if (value >= 1000000000) {
		value = value / 100000000;
		_i64toa_s(value, szLine, MAX_STRING, 10);
		std::string temp = szLine;
		temp.insert(temp.end() - 1, '.');
		sprintf_s(szLine, MAX_STRING, temp.c_str());
		strcat_s(szLine, MAX_STRING, " b");
	}
	else if (value >= 1000000) {
		value = value / 100000;
		_i64toa_s(value, szLine, MAX_STRING, 10);
		std::string temp = szLine;
		temp.insert(temp.end() - 1, '.');
		sprintf_s(szLine, MAX_STRING, temp.c_str());
		strcat_s(szLine, MAX_STRING, " m");
	}
	else if (value >= 1000) {
		value = value / 100;
		_i64toa_s(value, szLine, MAX_STRING, 10);
		std::string temp = szLine;
		temp.insert(temp.end() - 1, '.');
		sprintf_s(szLine, MAX_STRING, temp.c_str());
		strcat_s(szLine, MAX_STRING, " k");
	}
}

void CDPSAdvWnd::DrawList(bool DoDead) {
	if (!CurListMob || (!DoDead && CurListMob->Dead)) return;
	int ScrollPos = LTopList->GetVScrollPos();
	int CurSel = LTopList->GetCurSel();
	CHAR szTemp[MAX_STRING] = { 0 };
	LTopList->DeleteAll();
	int i = 0, LineNum = 0, RankAdj = 0, ShowMeLineNum = 0;
	bool FoundMe = false, ThisMe = false;
	if (ShowTotal == TOTALABOVE) {
		LineNum = LTopList->AddString(CXStr(""), 0, 0, 0);
		SetTotal(LineNum, CurListMob);
		RankAdj++;
	}
	if (ShowMeTop) {
		ShowMeLineNum = LTopList->AddString(CXStr(""), 0, 0, 0);
		SetLineColors(ShowMeLineNum, 0, false, true);
		RankAdj++;
	}
	if (ShowTotal == TOTALSECOND) {
		LineNum = LTopList->AddString(CXStr(""), 0, 0, 0);
		SetTotal(LineNum, CurListMob);
		RankAdj++;
	}
	for (i = 0; i < (int)CurListMob->EntList.size(); i++) {
		DPSMob::DPSEntry* Ent = CurListMob->EntList[i];
		if (Ent->Damage.Total == 0) break;
		if (ShowMeTop && !strcmp(Ent->Name, pLocalPlayer->DisplayedName)) {
			if (!ShowMeMin || (LineNum - RankAdj + 1) > ShowMeMinNum) FoundMe = true;
			ThisMe = true;
		}
		else ThisMe = false;
		LineNum = LTopList->AddString(CXStr(""), 0, 0, 0);
		SetLineColors(LineNum, Ent);
		//Placement Index
		sprintf_s(szTemp, "%i", LineNum - RankAdj + 1);
		LTopList->SetItemText(LineNum, 0, szTemp);
		if (ThisMe) LTopList->SetItemText(ShowMeLineNum, 0, szTemp);
		sprintf_s(szTemp, "%s%s%s", Ent->Mercenary ? "[M] " : "", Ent->Name, Ent->Pets ? "*" : "");
		LTopList->SetItemText(LineNum, 1, szTemp);
		if (ThisMe) LTopList->SetItemText(ShowMeLineNum, 1, szTemp);
		//Total Damage
		sprintf_s(szTemp, "%llu", Ent->Damage.Total);
		if (!UseTBMKOutputs)
			PrettifyNumber(szTemp, sizeof(szTemp));
		else {
			MakeItTBMK(szTemp);
			//Need to turn the strings from like 125556 to 125.5k
		}
		LTopList->SetItemText(LineNum, 2, szTemp);
		if (ThisMe) LTopList->SetItemText(ShowMeLineNum, 2, szTemp);
		//DPS
		char DPSoutput[MAX_STRING] = { 0 };
		sprintf_s(DPSoutput, "%llu", Ent->GetDPS());
		if (!UseTBMKOutputs)
			PrettifyNumber(DPSoutput, sizeof(DPSoutput));
		else {
			MakeItTBMK(DPSoutput);
			//Need to turn the strings from like 125556 to 125.5k
		}

		//SDPS (DPS over duration of entire fight?)
		char SDPSoutput[MAX_STRING] = { 0 };
		sprintf_s(SDPSoutput, "%llu", Ent->GetSDPS());
		if (!UseTBMKOutputs)
			PrettifyNumber(SDPSoutput, sizeof(SDPSoutput));
		else {
			MakeItTBMK(SDPSoutput);
			//Need to turn the strings from like 125556 to 125.5k
		}
		sprintf_s(szTemp, "%s (%s)", DPSoutput, SDPSoutput);
		LTopList->SetItemText(LineNum, 3, szTemp);
		if (ThisMe) LTopList->SetItemText(ShowMeLineNum, 3, szTemp);
		//Time
		sprintf_s(szTemp, "%I64is", Ent->Damage.Last - Ent->Damage.First);
		LTopList->SetItemText(LineNum, 4, szTemp);
		if (ThisMe) LTopList->SetItemText(ShowMeLineNum, 4, szTemp);
		//Percentage of Damage
		sprintf_s(szTemp, "%2.0f", (((float)Ent->Damage.Total / (float)CurListMob->Damage.Total) * 100.0f));
		strcat_s(szTemp, "%");
		LTopList->SetItemText(LineNum, 5, szTemp);
		if (ThisMe) LTopList->SetItemText(ShowMeLineNum, 5, szTemp);

	}
	if (ShowTotal == TOTALBOTTOM) {
		LineNum = LTopList->AddString(CXStr(""), 0, 0, 0);
		SetTotal(LineNum, CurListMob);
	}
	if (ShowMeTop && !FoundMe) LTopList->RemoveLine(ShowMeLineNum);
	LTopList->SetVScrollPos(ScrollPos);
	LTopList->CalculateFirstVisibleLine();
	LTopList->SetCurSel(CurSel);
}

void CDPSAdvWnd::SetLineColors(int LineNum, DPSMob::DPSEntry* Ent, bool Total, bool MeTop) {
	if (MeTop) {
		LTopList->SetItemColor(LineNum, 0, MeTopColor);
		LTopList->SetItemColor(LineNum, 1, MeTopColor);
		LTopList->SetItemColor(LineNum, 2, MeTopColor);
		LTopList->SetItemColor(LineNum, 3, MeTopColor);
		LTopList->SetItemColor(LineNum, 4, MeTopColor);
		LTopList->SetItemColor(LineNum, 5, MeTopColor);
	}
	else if (Total) {
		LTopList->SetItemColor(LineNum, 0, TotalColor);
		LTopList->SetItemColor(LineNum, 1, TotalColor);
		LTopList->SetItemColor(LineNum, 2, TotalColor);
		LTopList->SetItemColor(LineNum, 3, TotalColor);
		LTopList->SetItemColor(LineNum, 4, TotalColor);
		LTopList->SetItemColor(LineNum, 5, TotalColor);
	}
	else if (!strcmp(Ent->Name, pLocalPlayer->DisplayedName)) {
		LTopList->SetItemColor(LineNum, 0, MeColor);
		LTopList->SetItemColor(LineNum, 1, MeColor);
		LTopList->SetItemColor(LineNum, 2, MeColor);
		LTopList->SetItemColor(LineNum, 3, MeColor);
		LTopList->SetItemColor(LineNum, 4, MeColor);
		LTopList->SetItemColor(LineNum, 5, MeColor);
	}
	else if (UseRaidColors && Ent->Class && (Ent->SpawnType == SPAWN_PLAYER || Ent->Mercenary)) {
		LTopList->SetItemColor(LineNum, 0, NormalColor);
		LTopList->SetItemColor(LineNum, 1, pRaidWnd->ClassColors[ClassInfo[Ent->Class].RaidColorOrder]);
		LTopList->SetItemColor(LineNum, 2, NormalColor);
		LTopList->SetItemColor(LineNum, 3, NormalColor);
		LTopList->SetItemColor(LineNum, 4, NormalColor);
		LTopList->SetItemColor(LineNum, 5, NormalColor);
	}
	else {
		LTopList->SetItemColor(LineNum, 0, NormalColor);
		LTopList->SetItemColor(LineNum, 1, NPCColor);
		LTopList->SetItemColor(LineNum, 2, NormalColor);
		LTopList->SetItemColor(LineNum, 3, NormalColor);
		LTopList->SetItemColor(LineNum, 4, NormalColor);
		LTopList->SetItemColor(LineNum, 5, NormalColor);
	}
}

void CDPSAdvWnd::SaveLoc() {
	if (!pLocalPC || pLocalPC->Name[0] == '\0')
		return;

	WritePrivateProfileString(pLocalPC->Name, "Saved", "1", INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "Top", GetLocation().top, INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "Bottom", GetLocation().bottom, INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "Left", GetLocation().left, INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "Right", GetLocation().right, INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "Alpha", GetAlpha(), INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "FadeToAlpha", GetFadeToAlpha(), INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "CListType", CListType, INIFileName);
	WritePrivateProfileBool(pLocalPC->Name, "LiveUpdate", LiveUpdate, INIFileName);
	WritePrivateProfileBool(pLocalPC->Name, "Show", IsVisible(), INIFileName);
	WritePrivateProfileBool(pLocalPC->Name, "ShowMeTop", ShowMeTop, INIFileName);
	WritePrivateProfileBool(pLocalPC->Name, "ShowMeMin", ShowMeMin, INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "ShowMeMinNum", ShowMeMinNum, INIFileName);
	WritePrivateProfileBool(pLocalPC->Name, "UseRaidColors", UseRaidColors, INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "ShowTotal", ShowTotal, INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "FightIA", FightIA, INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "FightTO", FightTO, INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "EntTO", EntTO, INIFileName);
	WritePrivateProfileBool(pLocalPC->Name, "UseTBMKOutputs", UseTBMKOutputs, INIFileName);
	WritePrivateProfileInt(pLocalPC->Name, "HistoryLimit", HistoryLimit, INIFileName);

	//Save the column widths
	for (int i = 0; i <= 5; i++) {
		char szColumn[16] = { 0 };
		sprintf_s(szColumn, 16, "Column%iWidth", i);
		WritePrivateProfileInt(pLocalPC->Name, szColumn, LTopList->GetColumnWidth(i), INIFileName);
	}
}

void CDPSAdvWnd::LoadSettings() {
	//Set the check boxes to their current settings.
	CShowMeTop->SetCheck(ShowMeTop);
	CShowMeMin->SetCheck(ShowMeMin);
	CUseRaidColors->SetCheck(UseRaidColors);
	CLiveUpdate->SetCheck(LiveUpdate);
	CUseTBMKOutput->SetCheck(UseTBMKOutputs);

	//Set the text in the Inputboxes to their current setting.
	TShowMeMin->InputText = std::to_string(ShowMeMinNum);
	TFightIA->InputText = std::to_string(FightIA);
	TFightTO->InputText = std::to_string(FightTO);
	TEntTO->InputText = std::to_string(EntTO);
	THistoryLimit->InputText = std::to_string(HistoryLimit);

	//Clear and populate the ComboBox options for Show Total
	CShowTotal->DeleteAll();
	CShowTotal->InsertChoice("Don't Show Total");
	CShowTotal->InsertChoice("Above ShowMeTop");
	CShowTotal->InsertChoice("Below ShowMeTop");
	CShowTotal->InsertChoice("Show Bottom");
	CShowTotal->InsertChoice("");
	CShowTotal->SetChoice(ShowTotal);
	//Make Columns resizable.
	LTopList->SetColumnsSizable(true);
}

void CDPSAdvWnd::LoadLoc(char szChar[256]) {
	if (!pLocalPC) return;
	if (WrongUI)
		return;
	char szName[256] = { 0 };
	if (!szChar) strcpy_s(szName, pLocalPC->Name);
	else strcpy_s(szName, szChar);
	Saved = GetPrivateProfileBool(szName, "Saved", false, INIFileName);
	if (Saved) {
		SetLocation({
			GetPrivateProfileInt(szName, "Left", 0, INIFileName),
			GetPrivateProfileInt(szName, "Top", 0, INIFileName),
			GetPrivateProfileInt(szName, "Right", 0, INIFileName),
			GetPrivateProfileInt(szName, "Bottom", 0, INIFileName)
		});

		SetAlpha((uint8_t)GetPrivateProfileInt(szName, "Alpha", 0, INIFileName));
		SetFadeToAlpha((uint8_t)GetPrivateProfileInt(szName, "FadeToAlpha", 0, INIFileName));
	}

	CListType = GetPrivateProfileInt(szName, "CListType", 0, INIFileName);
	LiveUpdate = GetPrivateProfileBool(szName, "LiveUpdate", false, INIFileName);
	WarnedYHO = GetPrivateProfileBool(szName, "WarnedYHO", false, INIFileName);
	WarnedOHO = GetPrivateProfileBool(szName, "WarnedOHO", false, INIFileName);
	Debug = GetPrivateProfileBool(szName, "Debug", false, INIFileName);
	SetVisible(GetPrivateProfileBool(szName, "Show", true, INIFileName));
	ShowMeTop = GetPrivateProfileBool(szName, "ShowMeTop", false, INIFileName);
	ShowMeMin = GetPrivateProfileBool(szName, "ShowMeMin", false, INIFileName);
	ShowMeMinNum = GetPrivateProfileInt(szName, "ShowMeMinNum", 0, INIFileName);
	UseRaidColors = GetPrivateProfileBool(szName, "UseRaidColors", false, INIFileName);
	ShowTotal = GetPrivateProfileInt(szName, "ShowTotal", 0, INIFileName);
	FightIA = GetPrivateProfileInt(szName, "FightIA", 8, INIFileName);
	FightTO = GetPrivateProfileInt(szName, "FightTO", 30, INIFileName);
	EntTO = GetPrivateProfileInt(szName, "EntTO", 8, INIFileName);
	UseTBMKOutputs = GetPrivateProfileBool(szName, "UseTBMKOutputs", false, INIFileName);
	HistoryLimit = GetPrivateProfileInt(szName, "HistoryLimit", 25, INIFileName);

	// Load colors
	MeColor = GetPrivateProfileColor(szName, "MeColor", MQColor(0, 204, 0), INIFileName);
	MeTopColor = GetPrivateProfileColor(szName, "MeTopColor", MQColor(0, 204, 0), INIFileName);
	NormalColor = GetPrivateProfileColor(szName, "NormalColor", MQColor(255, 255, 255), INIFileName);
	NPCColor = GetPrivateProfileColor(szName, "NPCColor", MQColor(255, 255, 255), INIFileName);
	TotalColor = GetPrivateProfileColor(szName, "TotalColor", MQColor(102, 255, 255), INIFileName);
	EntHover = GetPrivateProfileColor(szName, "EntHover", MQColor(204, 51, 51), INIFileName);
	EntHighlight = GetPrivateProfileColor(szName, "EntHighlight", MQColor(102, 102, 102), INIFileName);
	FightNormal = GetPrivateProfileColor(szName, "FightNormal", NormalColor, INIFileName);
	FightHover = GetPrivateProfileColor(szName, "FightHover", EntHover, INIFileName);
	FightHighlight = GetPrivateProfileColor(szName, "FightHighlight", EntHighlight, INIFileName);
	FightActive = GetPrivateProfileColor(szName, "FightActive", MQColor(0, 204, 0), INIFileName);
	FightInActive = GetPrivateProfileColor(szName, "FightInActive", MQColor(119, 119, 119), INIFileName);
	FightDead = GetPrivateProfileColor(szName, "FightDead", MQColor(51, 0, 0), INIFileName);

	//Restore the column widths
	for (int i = 0; i <= 5; i++) {
		char szTemp[16] = { 0 };
		sprintf_s(szTemp, 16, "Column%iWidth", i);
		const int temp = GetPrivateProfileInt(szName, szTemp, 0, INIFileName);
		if (temp != 0) {
			LTopList->SetColumnWidth(i, temp);
		}
	}

	if (FightIA < 3) FightIA = 8;
	if (FightTO < 3) FightTO = 30;
	if (EntTO < 3) EntTO = 8;
	if (CListType > 1) CListType = CLISTTARGET;
	LTopList->SetColors(NormalColor, EntHover, EntHighlight);
	CMobList->SetChoice(CListType);
	LoadSettings();
}

int CDPSAdvWnd::WndNotification(CXWnd* pWnd, unsigned int Message, void* unknown) {
	if (Debug && Message != 21) WriteChatf("Notify: %i", Message);
	if (Message == XWM_CLOSE) CheckActive();
	if (Message == XWM_RCLICK && pWnd == LTopList) LTopList->SetCurSel(-1);
	else if (Message == XWM_CLOSE && pWnd == DPSWnd) CheckActive();
	else if (Message == XWM_LCLICK) {
		if (pWnd == Tabs) LoadSettings();
		else if (pWnd == CShowMeTop) ShowMeTop = CShowMeTop->bChecked;
		else if (pWnd == CShowMeMin) ShowMeMin = CShowMeMin->bChecked;
		else if (pWnd == CUseRaidColors) UseRaidColors = CUseRaidColors->bChecked;
		else if (pWnd == CLiveUpdate) LiveUpdate = CLiveUpdate->bChecked;
		else if (pWnd == CUseTBMKOutput) UseTBMKOutputs = CUseTBMKOutput->bChecked;
		//else if (pWnd == LTopList) WriteChatf("CurSel: %i", LTopList->GetCurSel());
		else if (pWnd == CShowTotal) {
			ShowTotal = CShowTotal->GetCurChoice();
			if (ShowTotal == 4) ShowTotal = 0;
			LoadSettings();
		}
		else if (pWnd == CMobList) {
			CurListMob = nullptr;
			LTopList->DeleteAll();
			bool FoundMob = false;
			if ((int)CMobList->GetCurChoice() > 1) {
				CListType = 2;
				DPSMob* ListMob = nullptr;
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
			}
			else CListType = (int)CMobList->GetCurChoice();
			Intervals -= 1; // Force update next Pulse.
		}
	}
	else if (Message == XWM_NEWVALUE) {
		if (pWnd == TShowMeMin) {
			if (!TShowMeMin->InputText.empty()) {
				ShowMeMinNum = GetIntFromString(TShowMeMin->InputText, 0);
				TShowMeMin->SetSel((int)TShowMeMin->InputText.length(), 0);
			}
		}
		else if (pWnd == TFightIA) {
			if (!TFightIA->InputText.empty()) {
				FightIA = GetIntFromString(TFightIA->InputText, 0);
				if (FightIA < 3) FightIA = 8;
			}
		}
		else if (pWnd == TFightTO) {
			if (!TFightTO->InputText.empty()) {
				FightTO = GetIntFromString(TFightTO->InputText, 0);
				if (FightTO < 3) FightTO = 30;
			}
		}
		else if (pWnd == TEntTO) {
			if (!TEntTO->InputText.empty()) {
				EntTO = GetIntFromString(TEntTO->InputText, 0);
				if (EntTO < 3) EntTO = 8;
			}
		}
		else if (pWnd == THistoryLimit) {
			if (!THistoryLimit->InputText.empty()) {
				HistoryLimit = GetIntFromString(THistoryLimit->InputText, 1);
				if (HistoryLimit < 1) HistoryLimit = 1;
			}
		}
	}

	return CSidlScreenWnd::WndNotification(pWnd, Message, unknown);
};

template <unsigned int _NameSize>
DPSMob* GetMob(CHAR(&Name)[_NameSize], bool Create, bool Alive)
{
	//ParsePet(Name);
	if (LastMob && (!Alive || (Alive && !LastMob->Dead)) && !_stricmp(LastMob->Name, Name))
		return LastMob;

	if (LastMob && LastMob->LastEntry && LastMob->LastEntry->DoSort) LastMob->LastEntry->Sort();
	// TODO: Range based for loop.
	for (int i = 0; i < (int)MobList.size(); i++) {
		if ((!Alive || (Alive && !MobList[i]->Dead)) && !_stricmp(MobList[i]->Name, Name)) {
			LastMob = MobList[i];
			return LastMob;
		}
	}
	if (Create) {
		LastMob = new DPSMob(Name, _NameSize);
		MobList.push_back(LastMob);
		return LastMob;
	}
	return nullptr;
}

template <unsigned int _EntSize, unsigned int _MobSize>
bool SplitStringOtherHitOther(const char* Line, char(&EntName)[_EntSize], char(&MobName)[_MobSize], int* Damage)
{
	if (strstr(Line, "injured by falling"))
		return false;
	int HitPos = 0, Action = 0;
	if (!strpbrk(Line, "1234567890"))
		return false;

	*Damage = GetIntFromString(strpbrk(Line, "1234567890"), 0);
	while (OtherHits[Action][0]) {
		HitPos = find_substr(Line, OtherHits[Action]);
		if (HitPos > 0)
			break;
		Action++;
	}
	if (!OtherHits[Action][0]) {
		return false;
	}
	if (HitPos <= 0 || HitPos > 2048) {
		if (*Damage > 0 && !WarnedOHO) {
			WriteChatf("\ar[MQ2DPSAdv] Error: Can not use Other Hits Other: Numbers Only Hitmodes for DPS Parsing.");
			if (Debug) WriteChatf("[Debug] OtherHitOther - Line: \ap%s", Line);
			WarnedOHO = true;
		}
		return false;
	}

	strncpy_s(EntName, &Line[0], HitPos);
	EntName[HitPos] = 0;
	int DmgPos = find_substr(Line, " for ");
	if (DmgPos == -1) {
		return false;
	}
	int MobStart = HitPos + (int)strlen(OtherHits[Action]);
	int MobLength = DmgPos - MobStart;
	strncpy_s(MobName, &Line[MobStart], MobLength);
	MobName[MobLength] = 0;

	if (!_stricmp(MobName, "himself") || !_stricmp(MobName, "herself") || !_stricmp(MobName, "itself"))
		strcpy_s(MobName, EntName);
	return true;
}

template <unsigned int _MobSize>
bool SplitStringYouHitOther(const char* Line, char(&MobName)[_MobSize], int* Damage)
{
	int Action = 0, HitPos = 0, DmgPos, MobStart, MobLength;
	if (!strpbrk(Line, "1234567890"))
		return false;

	*Damage = GetIntFromString(strpbrk(Line, "1234567890"), 0);
	while (YourHits[Action][0] != 0) {
		HitPos = find_substr(Line, YourHits[Action]);
		if (HitPos >= 0) break;
		Action++;
	}
	if (!YourHits[Action][0]) {
		return false;
	}
	if (HitPos < 0 || HitPos > 2048) {
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

	DmgPos = find_substr(Line, " for ");
	if (DmgPos == -1) {
		return false;
	}

	MobStart = HitPos + (int)strlen(YourHits[Action]);
	MobLength = DmgPos - MobStart;
	strncpy_s(MobName, &Line[MobStart], MobLength);
	MobName[MobLength] = 0;
	return true;
}

template <unsigned int _EntSize, unsigned int _MobSize>
bool SplitStringNonMelee(const char* Line, char(&EntName)[_EntSize], char(&MobName)[_MobSize], int* Damage)
{
	if (strstr(Line, "You were hit"))
		return false;
	if (!strpbrk(Line, "1234567890"))
		return false;
	if (strstr(Line, " healed "))
		return false;

	*Damage = GetIntFromString(strpbrk(Line, "1234567890"), 0);

	char temp[MAX_STRING] = "";
	sprintf_s(temp, " for %i", *Damage);
	int MobEnd = find_substr(Line, temp);

	//If this was a direct damage ability [EntName] hit [MobName] for [Damage] points of [DamageType] damage.
	if (MobEnd <= 0) {
		if (Debug) WriteChatf("NonMelee Failed, No MobEnd - Line: %s", Line);
	} else {
		if ((strstr(Line, pLocalPlayer->DisplayedName) || strstr(Line, "YOUR") || strstr(Line, " is "))) {
			sprintf_s(temp, pLocalPlayer->DisplayedName);
			if (strstr(Line, " YOUR ") || strstr(Line, " is ")) {
				//Process your damage shields.
				MobEnd = find_substr(Line, " is ");
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
					EntEnd = find_substr(Line, "'s thorns for ");
					EntStart = find_substr(Line, " pierced by ") + 12;
					strncpy_s(EntName, &Line[EntStart], EntEnd - EntStart);
				}
				else if (strstr(Line, " burned by ")) {
					//Fire DS
					EntEnd = find_substr(Line, "'s flames for ");
					EntStart = find_substr(Line, " burned by ") + 11;
					strncpy_s(EntName, &Line[EntStart], EntEnd - EntStart);
				}
				else if (strstr(Line, " tormented by ")) {
					//Frost DS
					EntEnd = find_substr(Line, "'s frost for ");
					EntStart = find_substr(Line, " tormented by ") + 14;
					strncpy_s(EntName, &Line[EntStart], EntEnd - EntStart);
				}
				if (EntEnd < 0 || EntStart < 0)
					return false;
			}
			else {
				strcpy_s(EntName, pLocalPlayer->DisplayedName);
			}
			if (!strlen(MobName) || !strlen(EntName)) {
				if (Debug) WriteChatf("NonMelee Fail: %s", Line);
				return false;
			}
			if (MobEnd > 0 && MobEnd < 256) {
				MobName[MobEnd] = 0;
				return true;
			}
			else {
				return false;
			}
		}

		sprintf_s(temp, " hit %s", MobName);
		int EntEnd = find_substr(Line, temp);
		if (EntEnd == -1) {
			if (Debug) WriteChatf("NonMelee Fail: %s", Line);
			return false;
		}

		strncpy_s(EntName, &Line[0], EntEnd);
		EntName[EntEnd] = 0;

		MobEnd = find_substr(Line, " for ");
		if (MobEnd != -1) {
			int MobLength = (int)(MobEnd - 5 - strlen(EntName));
			strncpy_s(MobName, Line + EntEnd + 5, MobLength);
			MobName[MobLength] = 0;
			if (!strlen(MobName) || !strlen(EntName)) {
				if (Debug) WriteChatf("NonMelee Fail: %s", Line);
				return false;
			}
			return true;
		}
	}
	return false;
}

template <unsigned int _MobSize>
bool SplitStringDeath(const char* Line, char(&MobName)[_MobSize])
{
	int MobStart = 0, MobLength = 0;
	MobLength = find_substr(Line, " died.");
	if (MobLength <= 0)
		MobLength = find_substr(Line, " has been slain by");

	if (MobLength <= 0) {
		MobStart = 15;
		MobLength = (int)(strlen(Line) - 16);
	}
	if (MobLength <= 0)
		return false;
	strncpy_s(MobName, Line + MobStart, MobLength);
	MobName[MobLength] = 0;
	return true;
}

template <unsigned int _EntSize, unsigned int _MobSize>
bool SplitStringDOT(const char* Line, char(&EntName)[_EntSize], char(&MobName)[_MobSize], int* Damage)
{
	if (!strpbrk(Line, "1234567890"))
		return false;
	if (strstr(Line, " healed "))
		return false;

	*Damage = GetIntFromString(strpbrk(Line, "1234567890"), 0);
	if (*Damage <= 0 || strstr(Line, " damage by "))
		return false;
	int MobEnd = find_substr(Line, " has taken ");

	if (strstr(Line, " damage from your ")) {
		strcpy_s(EntName, pLocalPlayer->DisplayedName);
	}
	else {
		int EntEnd = 0;
		if (!strstr(Line, "Rk. I")) {
			EntEnd = find_substr(Line, ".") - 6;
		}
		else {
			EntEnd = (int)(strrchr(Line, '.') - Line) - 6;
		}

		if (MobEnd <= 0 || EntEnd <= 0)
			return false;

		int SpellNameStart = find_substr(Line, " damage from ");
		int SpellNameEnd = find_substr(Line, " by ");
		char temp[MAX_STRING] = "";
		strncpy_s(temp, &Line[SpellNameStart + 13], SpellNameEnd - SpellNameStart - 13);
		int EntStart = find_substr(Line, temp) + (int)strlen(temp) + 4;
		strcpy_s(EntName, &Line[EntEnd, EntStart]);
		EntName[EntEnd - EntStart + 6] = 0;
	}
	strncpy_s(MobName, &Line[0], MobEnd);
	int Corpse = find_substr(MobName, "'s corpse");
	if (Corpse > 0)
		MobName[Corpse] = 0;
	else
		MobName[MobEnd] = 0;
	return true;
}

void AddMyDamage(char* EntName, int Damage1) {
	//if Me Do this
	uint64_t MyTotalBefore = MyTotal;
	uint64_t MyPetTotalBefore = MyPetTotal;
	time_t timenow = time(nullptr);
	int flag1 = 0;
	if ((!_stricmp(MyName, EntName) || !_stricmp(EntName, "You"))
		|| (strstr(EntName, "`s pet") && strstr(EntName, MyName))) {
		if (MyDebug) WriteChatf("[AddMyDamage] 1 -%s-", EntName);
		if (!MyFirst) MyFirst = timenow;
		MyLast = timenow;
		if (!_stricmp(MyName, EntName) || !_stricmp(EntName, "You")) {
			MyTotal += Damage1;
			flag1 = 1;
		}
		else {
			MyPetTotal += Damage1;
			flag1 = 2;
		}
	}
	// If I have a pet is it my pet
	else {
		if (pLocalPlayer && pLocalPlayer->PetID != -1) {
			int iPetID = pLocalPlayer->PetID;
			if (MyDebug) WriteChatf("[AddMyDamage] 3 -%s- -%s- %i", EntName, MyName, iPetID);
			if (PSPAWNINFO dPet = (PSPAWNINFO)GetSpawnByID(iPetID)) {
				if (MyDebug) WriteChatf("[AddMyDamage] 4 -%s-", dPet->DisplayedName);
				if (!_stricmp(dPet->DisplayedName, EntName)) {
					if (MyDebug) WriteChatf("[AddMyDamage] 5 %i", Damage1);
					if (!MyFirst) MyFirst = timenow;
					MyLast = timenow;
					MyPetTotal += Damage1;
					flag1 = 3;
				}
			}
		}
	}
	if (MyDebug) WriteChatf("[AddMyDamage] %s did %i Damage  My B: %lu A: %lu  Pet B: %lu A: %lu Flag: %i", EntName, Damage1, MyTotalBefore, MyTotal, MyPetTotalBefore, MyPetTotal, flag1);
}

void HandleNonMelee(const char* Line) {
	CHAR EntName[256] = { 0 };
	CHAR MobName[256] = { 0 };
	int Damage;
	if (!SplitStringNonMelee(Line, EntName, MobName, &Damage))
		return;
	if (Debug) WriteChatf("[HandleNonMelee] \ap%s \awhit \ap%s \awfor \ar%i", EntName, MobName, Damage);
	if (Active) GetMob(MobName, true, true)->GetEntry(EntName)->AddDamage(Damage);
	if (MyActive) AddMyDamage(EntName, Damage);
}

void HandleDOT(const char* Line) {
	CHAR EntName[256] = { 0 };
	CHAR MobName[256] = { 0 };
	int Damage;
	if (!SplitStringDOT(Line, EntName, MobName, &Damage))
		return;
	if (Debug) WriteChatf("[HandleDot] \ap%s \awhit \ap%s \awfor \ar%i", EntName, MobName, Damage);
	if (Active) GetMob(MobName, true, true)->GetEntry(EntName)->AddDamage(Damage);
	if (MyActive) AddMyDamage(EntName, Damage);
}

void HandleOtherHitOther(const char* Line) {
	char EntName[256] = { 0 }, MobName[256] = { 0 };
	int Damage;
	if (!SplitStringOtherHitOther(Line, EntName, MobName, &Damage))
		return;
	if (Debug) WriteChatf("[OtherHitOther] \ap%s \awhit \ap%s \awfor \ar%i", EntName, MobName, Damage);
	if (Active) {
		if (DPSMob* mob = GetMob(MobName, true, true)) {
			if (DPSMob::DPSEntry* entry = mob->GetEntry(EntName)) {
				entry->AddDamage(Damage);
			}
		}
	}
	if (MyActive) AddMyDamage(EntName, Damage);
}

void HandleYouHitOther(const char* Line) {
	char MobName[256] = { 0 };
	int Damage;
	if (!SplitStringYouHitOther(Line, MobName, &Damage))
		return;
	if (Debug) WriteChatf("[YouHitOther] \apYou \awhit \ap%s \awfor \ar%i \awdamage!", MobName, Damage);
	if (pLocalPlayer) {
		if (Active) {
			if (DPSMob* mob = GetMob(MobName, true, true)) {
				if (DPSMob::DPSEntry* entry = mob->GetEntry(pLocalPlayer->DisplayedName)) {
					entry->AddDamage(Damage);
				}
			}
		}
		if (MyActive) AddMyDamage(MyName, Damage);
	}
}

void HandleDeath(const char* Line) {
	char MobName[256] = { 0 };
	if (!SplitStringDeath(Line, MobName)) return;
	if (Debug) WriteChatf("[HandleDeath] Death Name: \ap%s", MobName);
	if (Active) {
		if (DPSMob* DeadMob = GetMob(MobName, false, true)) {
			HandleDeath(DeadMob);
			if (!DeadMob->IsPet() && !DeadMob->Mercenary) {
				DPSWnd->DrawCombo();
			}
		}
	}
	if (MyActive) MyActive = false;
}

void HandleDeath(DPSMob* DeadMob) {
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

void DPSAdvCmd(PSPAWNINFO pChar, PCHAR szLine) {
	char Arg1[MAX_STRING];
	GetArg(Arg1, szLine, 1);
	if (!_stricmp(Arg1, "show")) {
		if (!DPSWnd || WrongUI) {
			if (!DPSWnd) WriteChatf("\arDPSWnd does not exist. Try reloading your UI (/loadskin default).");
			if (WrongUI) WriteChatf("\arIncorrect UI File in use. Make sure you don't have a copy of MQUI_DPSAdvWnd stored in the EQ\\uifiles\\Default or in your custom UI Folder.");
		}
		else {
			DPSWnd->SetVisible(1);
		}
	}
	else if (!_stricmp(Arg1, "colors") && !WrongUI)
		pRaidOptionsWnd->Show(1, 1);
	else if (DPSWnd && !_stricmp(Arg1, "reload") && !WrongUI)
		DPSWnd->LoadLoc();
	else if (DPSWnd && !_stricmp(Arg1, "save") && !WrongUI)
		DPSWnd->SaveLoc();
	else if (!_stricmp(Arg1, "listsize") && !WrongUI)
		WriteChatf("\ayMobList Size: %i", MobList.size());
	else if (!_stricmp(Arg1, "limit")) {
		GetArg(Arg1, szLine, 2);
		const int i = GetIntFromString(Arg1, 0);
		if (i <= 0) {
			WriteChatf("\ar[MQ2DPSAdv] Invalid history limit");
		}
		else {
			HistoryLimit = i;
			WriteChatf("[MQ2DPSAdv] History limit set to %s", Arg1);
		}
	}
	else if (!_stricmp(Arg1, "copy") && !WrongUI) {
		char szCopy[MAX_STRING];
		GetArg(szCopy, szLine, 2);
		if (DPSWnd) {
			DPSWnd->LoadLoc(szCopy);
			DPSWnd->SaveLoc();
		}
		else
			WriteChatf("\arFailed to Copy: DPS Window not loaded.");
	}
	else if (!_stricmp(Arg1, "Debug")) {
		Debug = Debug ? false : true;
		WriteChatf("Debug is now: %s", Debug ? "\agOn" : "\arOff");
	}
	else if (!_stricmp(Arg1, "MyDebug")) {
		MyDebug = MyDebug ? false : true;
		WriteChatf("MyDebug is now: %s", MyDebug ? "\agOn" : "\arOff");
	}
	else if (!_stricmp(Arg1, "MyStart")) {
		MyFirst = 0;
		MyLast = 0;
		MyTime = 0;
		MyTotal = 0;
		MyPetTotal = 0;
		MyDPSValue = 0;
		MyPetDPS = 0;
		TotalDPSValue = 0;
		MyActive = true;
	}
	else if (!_stricmp(Arg1, "MyStop")) {
		if (MyActive) {
			MyLast = time(nullptr);
			MyActive = false;
			MyTime = (MyLast - MyFirst);
			MyDPSValue = MyTime ? (float)MyTotal / MyTime : MyTotal;
			MyPetDPS = MyTime ? (float)MyPetTotal / MyTime : MyPetTotal;
			TotalDPSValue = MyTime ? (float)(MyTotal + MyPetTotal) / MyTime : MyTotal + MyPetTotal;
		}
	}
	else if (!_stricmp(Arg1, "MyReset")) {
		MyActive = false;
		MyFirst = 0;
		MyLast = 0;
		MyTime = 0;
		MyTotal = 0;
		MyPetTotal = 0;
		MyDPSValue = 0;
		MyPetDPS = 0;
		TotalDPSValue = 0;
	}
	else if (!_stricmp(Arg1, "tlo")) {
		DisplayHelp("tlo");
	}
	else {
		DisplayHelp("dps");
	}
	CheckActive();
}

void DisplayHelp(const char* hTemp) {
	if (!_stricmp(hTemp, "dps")) {
		WriteChatf("[DPSAdv] - Valid Commands are:");
		WriteChatf("[DPSAdv]     show - Opens DPSAdv Window.");
		WriteChatf("[DPSAdv]     colors - switch to Raid tab in Window.");
		WriteChatf("[DPSAdv]     reload - reload your ini settings.");
		WriteChatf("[DPSAdv]     save - Save your current settings to ini.");
		WriteChatf("[DPSAdv]     listsize - displays the number of mobs in the DPS list.");
		WriteChatf("[DPSAdv]     copy - Copy the DPS window.");
		WriteChatf("[DPSAdv]     debug - Toggles on/off debug messages.");
		WriteChatf("[DPSAdv]     mydebug - Toggles on/off myDPS debug messages");
		WriteChatf("[DPSAdv]     mystart - Starts My DPS capture process.");
		WriteChatf("[DPSAdv]     mystop - Stops my DPS capture process");
		WriteChatf("[DPSAdv]     myreset - Turns off my DPS capture and set myDPS totals to zero.");
		WriteChatf("[DPSAdv]     tlo - get a list of the DPSAdv TLO members.");
		WriteChatf("[DPSAdv]     limit - Limit the history of mobs in the DPS list.");
	}
	else if (!_stricmp(hTemp, "tlo")) {
		WriteChatf("[DPSAdv] - DPSAdv TLO Members:");
		WriteChatf("[DPSAdv]    MyDamage - My Damage as a number");
		WriteChatf("[DPSAdv]    PetDamage - My pet Damage as a number");
		WriteChatf("[DPSAdv]    TotalDamage - My Damage and Pet Damage together as a number");
		WriteChatf("[DPSAdv]    MyDPS - My DPS as a number");
		WriteChatf("[DPSAdv]    PetDPS - My pet DPS as a number");
		WriteChatf("[DPSAdv]    TotalDPS - My DPS and pet DPS together as a number");
		WriteChatf("[DPSAdv]    TimeElapsed - Number in seconds of time of fight");
	}
}

void CreateDPSWindow() {
	if (DPSWnd) DestroyDPSWindow();
	if (pSidlMgr->FindScreenPieceTemplate("DPSAdvWnd")) {
		DPSWnd = new CDPSAdvWnd();
		if (DPSWnd->IsVisible()) {
			DPSWnd->Show(1, 1);
		}
		char szTitle[MAX_STRING];
		sprintf_s(szTitle, "DPS Advanced v%.1f", MQ2Version);
		DPSWnd->SetWindowText(szTitle);
	}
	CheckActive();
}

void DestroyDPSWindow() {
	if (DPSWnd) {
		DPSWnd->SaveLoc();
		delete DPSWnd;
		DPSWnd = nullptr;
	}
	CheckActive();
}

// ***********************************************************************************************************
// Adding TLO for DPS Meter
// ***********************************************************************************************************

class MQ2DPSAdvType : public MQ2Type
{
public:
	enum DpsAdvMembers {
		MyDamage = 1,
		PetDamage,
		TotalDamage,
		MyDPS,
		PetDPS,
		TotalDPS,
		TimeElapsed,
		MyStatus,
		MyPetID
	};

	MQ2DPSAdvType() :MQ2Type("DPSAdv")
	{
		TypeMember(MyDamage);
		TypeMember(PetDamage);
		TypeMember(TotalDamage);
		TypeMember(MyDPS);
		TypeMember(PetDPS);
		TypeMember(TotalDPS);
		TypeMember(TimeElapsed);
		TypeMember(MyStatus);
		TypeMember(MyPetID);
	}

	virtual bool GetMember(MQVarPtr VarPtr, const char* Member, char* Index, MQTypeVar& Dest) override
	{
		MQTypeMember* pMember = MQ2DPSAdvType::FindMember(Member);
		if (!pMember)
			return false;

		switch ((DpsAdvMembers)pMember->ID)
		{
		case MyDamage:
			Dest.Int64 = MyTotal;
			Dest.Type = mq::datatypes::pInt64Type;
			return true;

		case PetDamage:
			Dest.Int64 = MyPetTotal;
			Dest.Type = mq::datatypes::pInt64Type;
			return true;

		case TotalDamage:
			Dest.Int64 = MyTotal + MyPetTotal;
			Dest.Type = mq::datatypes::pInt64Type;
			return true;

		case MyDPS:
			if (MyActive) {
				if (MyTotal > 0 && MyFirst > 0) {
					MyTime = time(nullptr) - MyFirst;
					MyDPSValue = MyTime ? (float)MyTotal / MyTime : MyTotal;
				}
				else {
					MyTime = 0;
					MyDPSValue = 0;
				}
			}
			else {
				if (MyTotal > 0 && MyFirst > 0 && MyLast > 0) {
					MyTime = MyLast - MyFirst;
					MyDPSValue = MyTime ? (float)MyTotal / MyTime : MyTotal;
				}
				else {
					MyTime = 0;
					MyDPSValue = 0;
				}

			}

			Dest.Float = MyDPSValue;
			Dest.Type = mq::datatypes::pFloatType;
			return true;

		case PetDPS:
			if (MyActive) {
				if (MyPetTotal > 0 && MyFirst > 0) {
					MyTime = time(nullptr) - MyFirst;
					MyPetDPS = MyTime ? (float)MyPetTotal / MyTime : MyPetTotal;
				}
				else {
					MyTime = 0;
					MyPetDPS = 0;
				}
			}
			else {
				if (MyPetTotal > 0 && MyFirst > 0 && MyLast > 0) {
					MyTime = MyLast - MyFirst;
					MyPetDPS = MyTime ? (float)MyPetTotal / MyTime : MyPetTotal;
				}
				else {
					MyTime = 0;
					MyPetDPS = 0;
				}
			}

			Dest.Float = MyPetDPS;
			Dest.Type = mq::datatypes::pFloatType;
			return true;

		case TotalDPS:
			if (MyActive) {
				if (MyTotal > 0 && MyFirst > 0) {
					MyTime = time(nullptr) - MyFirst;
					TotalDPSValue = MyTime ? (float)(MyTotal + MyPetTotal) / MyTime : MyTotal + MyPetTotal;
				}
				else {
					MyTime = 0;
					TotalDPSValue = 0;
				}
			}
			else {
				if (MyTotal > 0 && MyFirst > 0 && MyLast > 0) {
					MyTime = MyLast - MyFirst;
					TotalDPSValue = MyTime ? (float)(MyTotal + MyPetTotal) / MyTime : MyTotal + MyPetTotal;
				}
				else {
					MyTime = 0;
					TotalDPSValue = 0;
				}
			}

			Dest.Float = TotalDPSValue;
			Dest.Type = mq::datatypes::pFloatType;
			return true;

		case TimeElapsed:
			if (MyActive) {
				if (MyFirst) {
					MyTime = time(nullptr) - MyFirst;
				}
				else {
					MyTime = 0;
				}
			}
			else if (MyFirst & MyLast) {
				MyTime = MyLast - MyFirst;
			}
			else {
				MyTime = 0;
			}
			Dest.UInt64 = static_cast<uint64_t>(MyTime);
			Dest.Type = mq::datatypes::pTimeStampType;
			return true;

		case MyStatus:
			if (MyActive) {
				Dest.Int = 1;
			}
			else {
				Dest.Int = 0;
			}
			Dest.Type = mq::datatypes::pIntType;

			return true;

		case MyPetID:
			if (pLocalPlayer && pLocalPlayer->PetID != -1)
			{
				Dest.Int = pLocalPlayer->PetID;
				Dest.Type = mq::datatypes::pIntType;
				return true;
			}
			else {
				Dest.Int = 0;
				Dest.Type = mq::datatypes::pIntType;
				return true;
			}
		}
		return false;
	}

	bool ToString(MQVarPtr VarPtr, PCHAR Destination) override
	{
		strcpy_s(Destination, MAX_STRING, Active ? "TRUE" : "FALSE");
		return true;
	}
};

MQ2DPSAdvType* pDpsAdvType = nullptr;

bool dataDPSAdv(const char* szName, MQTypeVar &Dest)
{
	Dest.DWord = 1;
	Dest.Type = pDpsAdvType;
	return true;
}

PLUGIN_API void SetGameState(int GameState)
{
	if (GameState == GAMESTATE_INGAME) {
		if (!DPSWnd) CreateDPSWindow();
		// Name could change without the gamestate changing, but it's rare
		if (pLocalPC && pLocalPlayer) {
			strcpy_s(MyName, pLocalPlayer->DisplayedName);
			return;
		}

		// MyName is used for matching DPS and we don't want false positive matches.
		strcpy_s(MyName, "NameNotFound");
	}
}

PLUGIN_API void OnCleanUI()
{
	DestroyDPSWindow();
}

PLUGIN_API void OnReloadUI()
{
	if (gGameState == GAMESTATE_INGAME && pLocalPlayer) CreateDPSWindow();
}

PLUGIN_API void InitializePlugin()
{
	LastMob = nullptr;
	CurTarget = nullptr;
	CurTarMob = nullptr;
	CurListMob = nullptr;
	CurMaxMob = nullptr;
	Zoning = false;
	ShowMeTop = false;
	WrongUI = false;
	// TODO:  Add as a resource to automatically unpack
	AddXMLFile("MQUI_DPSAdvWnd.xml");
	AddCommand("/dpsadv", DPSAdvCmd);
	// additions for DPS Meter
	pDpsAdvType = new MQ2DPSAdvType;
	AddMQ2Data("DPSAdv", dataDPSAdv);
	// Additions End
	CheckActive();
	MyActive = false;
}

PLUGIN_API void ShutdownPlugin()
{
	DestroyDPSWindow();
	RemoveCommand("/dpsadv");
	// additions for DPS Meter
	RemoveMQ2Data("DPSAdv");
	delete pDpsAdvType;
	// Additions End
}

PLUGIN_API bool OnIncomingChat(const char* Line, DWORD Color)
{
	if (gGameState != GAMESTATE_INGAME || !pLocalPlayer) return 0;
	if (Active || MyActive) {
		if (Debug) {
			char szMsg[MAX_STRING] = { 0 };
			strcpy_s(szMsg, Line);

			if (strchr(szMsg, '\x12'))
			{
				StripTextLinks(szMsg);
			}
			WriteChatf("[\ar%i\ax]\a-t: \ap%s", Color, szMsg);
		}
		switch (Color)
		{
		case COLOR_DEFAULT:                      // Color: 0   - Task Updates
		case CONCOLOR_GREEN:                     // Color: 2   - Consider Message - Green
		case CONCOLOR_BLUE:                      // Color: 4   - Consider messages - Dark Blue
		case CONCOLOR_GREY:                      // Color: 6   - Consider message - Grey
		case CONCOLOR_WHITE:                     // Color: 10  - Merchant says "Hello there, Soandso. How about a nice Iron Ration?"
		case CONCOLOR_RED:                       // Color: 13  - Attack is on/off - Other invites to raid.
		case CONCOLOR_YELLOW:                    // Color: 15  - Help messages, Consider Message - Yellow (right click on the NPC to consider it, Targeted (NPC): a slatescale serpent, You have gained an ability point)
		case CONCOLOR_LIGHTBLUE:                 // Color: 18  - Consider message - Light blue
		case CONCOLOR_BLACK:                     // Color: 20  - You regain some experience from resurrection.
		case USERCOLOR_SAY:                      // Color: 256 - Other player /say messages
		case USERCOLOR_TELL:                     // Color: 257 - Other /tell's you
		case USERCOLOR_GROUP:                    // Color: 258 - Other tells /group
		case USERCOLOR_GUILD:                    // Color: 259 - Guild Chat - Incoming
		case USERCOLOR_OOC:                      // Color: 260 - Other /ooc
		case USERCOLOR_AUCTION:                  // Color: 261 - Other /auction
		case USERCOLOR_SHOUT:                    // Color: 262 - Other /shout
		case USERCOLOR_EMOTE:                    // Color: 263 - other Emote commands/eat food messages (/cry, Chomp, chomp, chomp... Soandso takes a bite...etc)
		case USERCOLOR_SPELLS:                   // Color: 264 - You begin casting
			break;

		case USERCOLOR_YOU_HIT_OTHER:            // Color: 265 - You hit other (melee/melee crit)
			HandleYouHitOther(Line);
			break;

		case USERCOLOR_OTHER_HIT_YOU:            // Color: 266 - Other hits you
		case USERCOLOR_YOU_MISS_OTHER:           // Color: 267 - You miss other
		case USERCOLOR_OTHER_MISS_YOU:           // Color: 268 - Riposte/Dodge/Other Misses You
		case USERCOLOR_DUELS:                    // Color: 269 - Announcement (GM Events etc)
		case USERCOLOR_SKILLS:                   // Color: 270 - You have become better at [Skill]! (Number)
		case USERCOLOR_DISCIPLINES:              // Color: 271 - Song wear off message?? (Ritual Scarification wear off message)
		case USERCOLOR_UNUSED001:                // Color: 272 - Pet thanks for stuff you give them?
		case USERCOLOR_DEFAULT:                  // Color: 273 - Charged mercenary upkeep/Group Invites/Joins/Banker tells you welcome to bank.
		case USERCOLOR_FACTION:                  // Color: 274 - Faction hits
		case USERCOLOR_MERCHANT_OFFER:           // Color: 275 - [Merchant] tells you, that'll be [price] for the [item]
		case USERCOLOR_MERCHANT_EXCHANGE:        // Color: 276 - You sold [QTY] [Item Name] to [Player] for [Amount]/You Purchased [Item Name] from [NPC] for [Amount]
		case USERCOLOR_YOUR_DEATH:               // Color: 277 - Your death message.
		case USERCOLOR_OTHER_DEATH:              // Color: 278 - Other PC death message.
			break;

		case USERCOLOR_OTHER_HIT_OTHER:          // Color: 279 - Others Hits Other
			HandleOtherHitOther(Line);
			break;

		case USERCOLOR_OTHER_MISS_OTHER:         // Color: 280 - Other Misses Other
		case USERCOLOR_WHO:                      // Color: 281 - /who
		case USERCOLOR_YELL:                     // Color: 282 - Other /yell
			break;

		case USERCOLOR_NON_MELEE:                // Color: 283 - Your Non-Melee spell dmg.
			HandleNonMelee(Line); 
			break;

		case USERCOLOR_SPELL_WORN_OFF:           // Color: 284 - Spell wear off messages
		case USERCOLOR_MONEY_SPLIT:              // Color: 285 - Coin collection/Split
		case USERCOLOR_LOOT:                     // Color: 286 - Loot Related (Master looter actions, you loot)
		case USERCOLOR_RANDOM:                   // Color: 287 - Dice Rolls
		case USERCOLOR_OTHERS_SPELLS:            // Color: 288 - Other Starts Casting Messages
		case USERCOLOR_SPELL_FAILURE:            // Color: 289 - Target is out of range, get closer/Interupt/Fizzle/other spell failure
		case USERCOLOR_CHAT_CHANNEL:             // Color: 290 - Channel Lists after logged in
		case USERCOLOR_CHAT_1:                   // Color: 291 - Chat Channel 1 - Incoming
		case USERCOLOR_CHAT_2:                   // Color: 292 - Chat Channel 2 - Incoming
		case USERCOLOR_CHAT_3:                   // Color: 293 - Chat Channel 3 - Incoming
		case USERCOLOR_CHAT_4:                   // Color: 294 - Chat Channel 4 - Incoming
		case USERCOLOR_CHAT_5:                   // Color: 295 - Chat Channel 5 - Incoming
		case USERCOLOR_CHAT_6:                   // Color: 296 - Chat Channel 6 - Incoming
		case USERCOLOR_CHAT_7:                   // Color: 297 - Chat Channel 7 - Incoming
		case USERCOLOR_CHAT_8:                   // Color: 298 - Chat Channel 8 - Incoming
		case USERCOLOR_CHAT_9:                   // Color: 299 - Chat Channel 9 - Incoming
		case USERCOLOR_CHAT_10:                  // Color: 300 - Chat Channel 10 - Incoming
			break;

		case USERCOLOR_MELEE_CRIT:               // Color: 301 - Critical (YouHitOther)
			HandleYouHitOther(Line);
			break;

		case USERCOLOR_SPELL_CRIT:               // Color: 302 - Direct Damage Crits (Yours)
			HandleNonMelee(Line);
#if !IS_EXPANSION_LEVEL(EXPANSION_LEVEL_COTF) // Not contained in ROF2
			// This requires checking both handle nonmelee and dot
			// Using both of these here can cause an error in debug reporting, but that is expected.
			HandleDOT(Line);
#endif
			break;

		case USERCOLOR_TOO_FAR_AWAY:             // Color: 303 - Errors (You must first click on the being you wish to attack, Can't hit them from here)
		case USERCOLOR_NPC_RAMPAGE:              // Color: 304 - NPC Rampage
			break;

		case USERCOLOR_NPC_FLURRY:               // Color: 305 - Other Pet Flurry (OtherHitOther)
			HandleOtherHitOther(Line);
			break;

		case USERCOLOR_NPC_ENRAGE:               // Color: 306 - Enrage (Showed for pet)
		case USERCOLOR_ECHO_SAY:                 // Color: 307 - Your /say messages, mob advances messages.
		case USERCOLOR_ECHO_TELL:                // Color: 308 - You tell other.
		case USERCOLOR_ECHO_GROUP:               // Color: 309 - Group conversation
		case USERCOLOR_ECHO_GUILD:               // Color: 310 - Guild conversation - Outgoing
		case USERCOLOR_ECHO_OOC:                 // Color: 311 - you /ooc
		case USERCOLOR_ECHO_AUCTION:             // Color: 312 - you /auction
		case USERCOLOR_ECHO_SHOUT:               // Color: 313 - you shout.
		case USERCOLOR_ECHO_EMOTE:               // Color: 314 - /emote messages
		case USERCOLOR_ECHO_CHAT_1:              // Color: 315 - Chat Channel 1 - Outgoing
		case USERCOLOR_ECHO_CHAT_2:              // Color: 316 - Chat Channel 2 - Outgoing
		case USERCOLOR_ECHO_CHAT_3:              // Color: 317 - Chat Channel 3 - Outgoing
		case USERCOLOR_ECHO_CHAT_4:              // Color: 318 - Chat Channel 4 - Outgoing
		case USERCOLOR_ECHO_CHAT_5:              // Color: 319 - Chat Channel 5 - Outgoing
		case USERCOLOR_ECHO_CHAT_6:              // Color: 320 - Chat Channel 6 - Outgoing
		case USERCOLOR_ECHO_CHAT_7:              // Color: 321 - Chat Channel 7 - Outgoing
		case USERCOLOR_ECHO_CHAT_8:              // Color: 322 - Chat Channel 8 - Outgoing
		case USERCOLOR_ECHO_CHAT_9:              // Color: 323 - Chat Channel 9 - Outgoing
		case USERCOLOR_ECHO_CHAT_10:             // Color: 324 - Chat Channel 10 - Outgoing
		case USERCOLOR_AVATAR_CMD:               // Color: 325
		case USERCOLOR_LINK:                     // Color: 326
		case USERCOLOR_RAID:                     // Color: 327 - Any /rsay
			break;

		case USERCOLOR_PET:                      // Color: 328 - My pet hits (OtherHitOther)
			HandleOtherHitOther(Line); // Your Pet
			break;

		case USERCOLOR_DAMAGESHIELD:             // Color: 329 - Damage Shield hits you.
		case USERCOLOR_LEADER:                   // Color: 330 - Raid Role messages.
			break;

		case USERCOLOR_PETRAMPFLURRY:            // Color: 331 - My Pet Flurry (Other Hit Other) My Pet
		case USERCOLOR_PETCRITS:                 // Color: 332 - My Pet Critical (Other Hit Other)
			HandleOtherHitOther(Line);
			break;

		case USERCOLOR_FOCUS:                    // Color: 333 - Item Focus messages
		case USERCOLOR_XP:                       // Color: 334 - You gain Experience messages
		case USERCOLOR_SYSTEM:                   // Color: 335 - You have already finished collecting [Item].
			break;

		case USERCOLOR_PET_SPELLS:               // Color: 336 - Pet Non-Melee/Pet beings to cast
			HandleNonMelee(Line); // All pet non-melee (yours and others)
			break;

		case USERCOLOR_PET_RESPONSES:            // Color: 337 - Pet messages (YourPet says, "Following you master.")
		case USERCOLOR_ITEM_SPEECH:              // Color: 338
		case USERCOLOR_STRIKETHROUGH:            // Color: 339 - Strike thru (yours and others)
		case USERCOLOR_STUN:                     // Color: 340 - You are stunned/unstunned messages.
		case USERCOLOR_SWARM_PET_DEATH:          // Color: 341
		case USERCOLOR_FELLOWSHIP:               // Color: 342 - fellowship messages
		case USERCOLOR_NPC_SPEECH:               // Color: 343 - corpse emote
		case USERCOLOR_NPC_SPEECH_TO_YOU:        // Color: 344
		case USERCOLOR_GUILD_MSG:                // Color: 345 - Guild plants banner
			break;

#if IS_EXPANSION_LEVEL(EXPANSION_LEVEL_COTF) // Not contained in ROF2
		case USERCOLOR_MERCENARY_GRP:            // Color: 346 - Mercenary tells group
		case USERCOLOR_ACHIEVEMENT:              // Color: 347
		case USERCOLOR_ACHIEVEMENTS_YOU:         // Color: 348 - Achievement - you and other.
		case USERCOLOR_ACHIEVEMENTS_OTHERS:      // Color: 349 - Achievement - Guildmate
		case USERCOLOR_PVP:                      // Color: 350
		case USERCOLOR_HOTBUTTON_COOLDOWN:       // Color: 351
		case USERCOLOR_AGGRO_LOW:                // Color: 352
		case USERCOLOR_AGGRO_WARNING:            // Color: 353
		case USERCOLOR_AGGRO_MOST:               // Color: 354
		case USERCOLOR_DIALOG_LINK:              // Color: 355
			break;
	
		case USERCOLOR_FLURRY:                   // Color: 356 - Flurry (Yours)
			HandleYouHitOther(Line);

		case USERCOLOR_DEBUG:                    // Color: 357
			break;

		case USERCOLOR_NPC_DEATH:                // Color: 358 - NPC Death message
			HandleDeath(Line);
			break;

		case USERCOLOR_DICE_OTHER:               // Color: 359
		case USERCOLOR_DICE_GROUP:               // Color: 360 - Other Rolls Dice.
		case USERCOLOR_FALL_DAMAGE_SELF:         // Color: 361 - Injured by falling (self)
		case USERCOLOR_FALL_DAMAGE_OTHER:        // Color: 362 - Injured by falling (other)
			break;

		case USERCOLOR_DAMAGESHIELD_SELF:        // Color: 363 - Your damage shield
		case USERCOLOR_DAMAGESHIELD_OTHER:       // Color: 364 - Other's damage shield
			HandleNonMelee(Line);
			break;

		case USERCOLOR_EVENT:                    // Color: 365
		case USERCOLOR_OVERWRITTEN_DET:          // Color: 366 - Your [Spell Name] has been overwritten - Detrimental
		case USERCOLOR_OVERWRITTEN_BENE:         // Color: 367 - Your [Spell Name] has been overwritten - Beneficial
		case USERCOLOR_CANT_USE_COMMAND:         // Color: 368 - You cannot use that command right now (clicking disc too fast)
		case USERCOLOR_ABILITY_COOLDOWN:         // Color: 369 - You can use [Ability Name] again in [Time till you can use it again]
		case USERCOLOR_AA_REUSE_TIMER:           // Color: 370 - AA Reuse Timer failed
		case USERCOLOR_DESTROY_ITEM:             // Color: 371 - Destroy item message
		case USERCOLOR_AURAS_YOU:                // Color: 372
		case USERCOLOR_AURAS_OTHER:              // Color: 373 - Heal over time on other?
		case USERCOLOR_HEALS_YOU:                // Color: 374 - You heal other
		case USERCOLOR_HEALS_OTHERS:             // Color: 375 - Other buffs other/Other Heal Other
			break;

		case USERCOLOR_DOTS_YOURS:               // Color: 376 - Your DoT's
		case USERCOLOR_DOTS_OTHERS:              // Color: 377 - Other's DoT's
			HandleDOT(Line);
			break;

		case USERCOLOR_BARD_SONG_PETS:           // Color: 378 - Song messages - Soandso begins to sing a song. <Selo's Sonata I>
			break;

		case USERCOLOR_DIRECT_DAMAGE_OTHERS:     // Color: 379 - Others Non-Melee
			HandleNonMelee(Line);
			break;

		case USERCOLOR_SPELL_MESSAGES:           // Color: 380 - Your spell messages
		case USERCOLOR_FACTION_LINK:             // Color: 381 - Faction Links
		case USERCOLOR_TAUNT_MESSAGES:           // Color: 382 - Taunt Messages
		case USERCOLOR_DISCIPLINES_OTHER:        // Color: 383 - Combat Abilities / Disciplines (Others)
		case USERCOLOR_ITEM_STAT_POSITIVE:       // Color: 384 - Item Stat Positive
		case USERCOLOR_ITEM_STAT_NEGATIVE:       // Color: 385 - Item Stat Negative
		case USERCOLOR_ENCOUNTER_LOCK_ATTACKABLE: // Color: 386 - Encounter Lock Attackable
		case USERCOLOR_ENCOUNTER_LOCK_UNATTACKABLE: // Color 387 - Encounter Lock Unattackable
		case USERCOLOR_FOOD_AND_DRINK:           // Color: 388 - Food And Drink
		case USERCOLOR_RAID_VICTORY:             // Color: 389 - Raid Victory
			break;
#endif
#if IS_EXPANSION_LEVEL(EXPANSION_LEVEL_LS)
		case USERCOLOR_DIRECT_DAMAGE_YOU:        // Color: 397 - Direct Damage (Yours)
		case USERCOLOR_DIRECT_DAMAGE_OTHERS_CRITICAL: // Color 398 - Direct Damage (Other Critical Hits)
			HandleNonMelee(Line);
			break;

		case USERCOLOR_DOT_YOURS_CRITICAL:       // Color: 399 - DoTs (Your Critical Hits)
		case USERCOLOR_DOT_OTHERS_CRITICAL:      // Color: 400 - DoTs (Other Critical Hits)
			HandleDOT(Line);
			break;

		case USERCOLOR_DOT_DAMAGE_TAKEN:         // Color: 401 - DoTs (You Being Hit)
		case USERCOLOR_HEALS_RECEIVED:           // Color: 402 - Heals Received
		case USERCOLOR_HEALS_YOURS_CRITICAL:     // Color: 403 - Heals (Your Critical Heals)
		case USERCOLOR_HEALS_OTHERS_CRITICAL:    // Color: 404 - Heals (Other Critical Heals)
			break;

		case USERCOLOR_MELEE_OTHERS_CRITICAL:    // Color: 405 - Others Hits (Critical)
			HandleOtherHitOther(Line);
			break;
#endif // IS_EXPANSION_LEVEL(EXPANSION_LEVEL_LS)

		default:
			// Don't Know these, lets see what it is.
			if (Debug) WriteChatf("[\ar%i\ax]\a-t: \ap%s", Color, Line);
			break;
		}
	}

	return false;
}

bool CheckInterval() {
	time_t timenow = time(nullptr);
	if (!Intervals)
		Intervals = timenow;
	else if (Intervals != timenow) {
		Intervals = timenow;
		return true;
	}
	return false;
}

void CheckActive() {
	if (DPSWnd && DPSWnd->IsVisible() && !Zoning && !WrongUI) Active = true;
	else Active = false;
}

void ListSwitch(DPSMob* Switcher) {
	CurListMob = Switcher;
	DPSWnd->LTopList->SetCurSel(-1);
	DPSWnd->LTopList->SetVScrollPos(0);
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
	CurMaxMob = nullptr;
	const auto MobListSizeStart = MobList.size();
	for (int i = 0; i < (int)MobList.size(); i++) {
		DPSMob* Mob = MobList[i];
		if (Mob == nullptr // Somehow this was a null pointer
		        || (MobList.size() > static_cast<size_t>(HistoryLimit + 1) // We have exceeded the limit of mobs in the config (+1 due to display of the "Active" mob)
		                || (!Mob->Active || (Mob->IsPet() || Mob->SpawnType == SPAWN_PLAYER || Mob->Mercenary) && Mob->InActive)) // It's not active or it's a pet/player/merc that's inactive?
		            && Mob != CurTarMob && Mob != CurListMob && Mob != LastMob && Mob != CurMaxMob) // It's not one of our UI mobs
		{
			MobList.erase(MobList.begin() + i);
			delete Mob;
			i--;
		}
		else {
			const time_t timenow = time(nullptr);
			if (Mob->Active && !Mob->Dead && timenow - Mob->Damage.Last > FightTO) {
				HandleDeath(Mob);
				CChange = true;
			}
			else if (Mob->Active && !Mob->Dead && !Mob->InActive && timenow - Mob->Damage.Last > FightIA) {
				Mob->InActive = true;
				sprintf_s(Mob->Tag, "[IA] ");
				if (!Mob->IsPet() && !Mob->Mercenary) CChange = true;
			}
			else if (Mob->Active && !Mob->Dead && Mob->InActive && timenow - Mob->Damage.Last < FightIA) {
				Mob->InActive = false;
				sprintf_s(Mob->Tag, "[A] ");
				if (!Mob->IsPet() && !Mob->Mercenary) CChange = true;
			}
			if (Mob->Active && !Mob->InActive && !Mob->Dead && !Mob->IsPet() && Mob->SpawnType == SPAWN_NPC && (int)Mob->Damage.Last > MaxDmgLast && (!CurMaxMob || Mob->Damage.Total > CurMaxMob->Damage.Total)) CurMaxMob = Mob;
		}
	}
	if (MobListSizeStart > MobList.size() || CListType == CLISTMAXDMG && CurMaxMob && CurMaxMob != CurListMob)
		ListSwitch(CurMaxMob);
	if (CChange) DPSWnd->DrawCombo();
	DPSWnd->DrawList();
	//WriteChatf("Active: %s", Active ? "Yes" : "No");
}

PLUGIN_API void OnPulse() {
	if (gGameState != GAMESTATE_INGAME || !pLocalPlayer) return;

	if (Active && !WrongUI) {
		if (LastMob && LastMob->LastEntry && LastMob->LastEntry->DoSort) LastMob->LastEntry->Sort();
		if ((PSPAWNINFO)pTarget && (PSPAWNINFO)pTarget != CurTarget) TargetSwitch();
		if (CListType == CLISTTARGET && CurTarMob && CurTarMob != CurListMob && CurTarMob->Active && CurTarMob->SpawnType == 1) ListSwitch(CurTarMob);
		if (CheckInterval()) IntPulse();
	}
}

PLUGIN_API void OnRemoveSpawn(PSPAWNINFO pSpawn)
{
	if (!Zoning) {
		DPSMob* pMob = nullptr;
		DPSMob::DPSEntry* pEnt = nullptr;
		// TODO: Both of these should probably be range based for loops.
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
	LastMob = nullptr;
	CurTarget = nullptr;
	CurTarMob = nullptr;
	CurListMob = nullptr;
	CurMaxMob = nullptr;
	DPSMob* pMob = nullptr;
	DPSMob::DPSEntry* pEnt = nullptr;
	// TODO: Both of these should probably be range based for loops.
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

PLUGIN_API void OnBeginZone() {
	ZoneProcess();
	Zoning = true;
	CheckActive();
}

PLUGIN_API void OnEndZone() {
	Zoning = false;
	CheckActive();
}
