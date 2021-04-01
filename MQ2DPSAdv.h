// DPS ADV CREATED BY WARNEN 2008-2009
// MQ2DPSAdv.h

char* OtherHits[] = { " punches "," slashes "," crushes "," pierces "," hits "," kicks "," backstabs "," frenzies on "," bashes ", " strikes "," claws "," slices "," bites "," mauls "," stings ", " shoots ", " smashes ", " rends ", " gores ", 0 };
char* YourHits[] = { " hit "," slash "," pierce "," crush "," kick "," punch "," backstab "," frenzy on "," bash ", " strike "," claw "," slice "," bite "," maul "," sting ", " shoot ", " smash ", " rend ", " gore ", 0 };
enum { CLISTTARGET, CLISTMAXDMG, SINGLE };
enum { NOTOTAL, TOTALABOVE, TOTALSECOND, TOTALBOTTOM };

PSPAWNINFO CurTarget;
time_t Intervals;
int CListType;
int MaxDmgLast;
int MeColor;
int MeTopColor;
int NormalColor;
int NPCColor;
int TotalColor;
int EntHover;
int EntHighlight;
int FightNormal;
int FightHover;
int FightHighlight;
int FightActive;
int FightInActive;
int FightDead;
bool Saved;
bool WarnedYHO, WarnedOHO;
bool Debug;
bool MyDebug;
bool Active;
bool Zoning;
bool WrongUI;
bool ShowMeTop;
bool ShowMeMin;
int ShowMeMinNum;
bool UseRaidColors;//Use colors from set raid colors for each class display.
bool LiveUpdate;//Update Total/DPS/Time/% as it happens instead of a set pulse.
int ShowTotal;
int FightIA;//Fight Inactive.
int FightTO;//Fight Timeout.
int EntTO;//Entity Timeout.
bool UseTBMKOutputs = false;//Intended to show 1.4t, 1.5m, 343k etc outputs for total and DPS.
int HistoryLimit = 25; // Maximum number of mobs to store before deleting the closest to the beginning

//----------------------------------------------------------------------------
// new below here for current character DPS totals, this does NOT show up in the DPS Window.
char MyName[64];
uint64_t MyTotal;
uint64_t MyPetTotal;
float MyDPSValue;
float MyPetDPS;
float TotalDPSValue;
bool MyActive; // This is to turn on and off the DPS accumlator for just yourself.
time_t MyFirst;
time_t MyLast;
uint64_t MyTime;
//----------------------------------------------------------------------------

struct EntDamage {
	uint64_t Total;
	time_t First;
	time_t Last;
	int AddTime;
};

void CreateDPSWindow();
void DestroyDPSWindow();
template <unsigned int _EntSize, unsigned int _MobSize>bool SplitStringOtherHitOther(PCHAR Line, CHAR(&EntName)[_EntSize], CHAR(&MobName)[_MobSize], int* Damage);
template <unsigned int _EntSize, unsigned int _MobSize>bool SplitStringDOT(PCHAR Line, CHAR(&EntName)[_EntSize], CHAR(&MobName)[_MobSize], int* Damage);
template <unsigned int _EntSize, unsigned int _MobSize>bool SplitStringNonMelee(PCHAR Line, CHAR(&EntName)[_EntSize], CHAR(&MobName)[_MobSize], int* Damage);
void HandleYouHitOther(PCHAR Line);
void HandleOtherHitOther(PCHAR Line);
void HandleNonMelee(PCHAR Line);
void HandleDOT(PCHAR Line);
void HandleDeath(PCHAR Line);
void TargetSwitch();
void CheckActive();
void DPSAdvCmd(PSPAWNINFO pChar, PCHAR szLine);
void MakeItTBMK(PCHAR szLine);
void DisplayHelp(PCHAR hTemp);

class DPSMob {
public:
	class DPSEntry {
	public:
		char Name[64];
		bool DoSort;
		bool Pets;
		bool CheckPetName;
		bool UsingPetName;
		int SpawnType;
		int Class;
		bool Mercenary;
		EntDamage Damage;
		DPSMob* Parent;
		PSPAWNINFO Spawn;
		DPSEntry* Master;

		DPSEntry();
		DPSEntry(char EntName[], DPSMob* pParent, bool PCOnly = false);
		void Init();
		void AddDamage(int Damage1);
		uint64_t GetDPS();
		uint64_t GetSDPS();
		void Sort();
		void GetSpawn(bool PCOnly = false);
		bool CheckMaster();
	};

	char Name[64];
	char Tag[8];
	int SpawnType;
	PSPAWNINFO Spawn;
	bool Active;
	bool InActive;
	bool Dead;
	bool PetName;
	bool Mercenary;
	EntDamage Damage;
	DPSEntry* LastEntry;
	std::vector<DPSEntry*> EntList;

	DPSMob();
	DPSMob::DPSMob(PCHAR MobName, size_t MobLen);
	void Init();
	void AddDamage(int Damage1);
	void GetSpawn();
	bool IsPet();
	DPSEntry* GetEntry(char EntName[], bool Create = true, bool PCOnly = false);
};

std::vector<DPSMob*> MobList;
DPSMob* LastMob;
DPSMob* CurTarMob;
DPSMob* CurListMob;
DPSMob* CurMaxMob;
template <unsigned int _NameSize>DPSMob* GetMob(CHAR(&Name)[_NameSize], bool Create = true, bool Alive = false);
bool MobListMaint(DPSMob* Mob, int ListLoc);
void HandleDeath(DPSMob* DeadMob);
void ListSwitch(DPSMob* Switcher);

class CDPSAdvWnd : public CCustomWnd {
public:
	CTabWnd* Tabs;
	CListWnd* LTopList;
	CComboWnd* CMobList;
	CCheckBoxWnd* CShowMeTop;
	CCheckBoxWnd* CShowMeMin;
	CEditWnd* TShowMeMin;
	CCheckBoxWnd* CUseRaidColors;
	CCheckBoxWnd* CLiveUpdate;
	CCheckBoxWnd* CUseTBMKOutput;
	CEditWnd* TFightIA;
	CEditWnd* TFightTO;
	CEditWnd* TEntTO;
	CEditWnd* THistoryLimit;
	CComboWnd* CShowTotal;
	bool ReSort;

	CDPSAdvWnd();
	~CDPSAdvWnd();
	void DrawList(bool DoDead = false);
	void SetTotal(int LineNum, DPSMob* Mob);
	void DrawCombo();
	void LoadLoc(char szChar[] = 0);
	void LoadSettings();
	void SaveLoc();
	void SetLineColors(int LineNum, DPSMob::DPSEntry* Ent, bool Total = false, bool MeTop = false);
	int WndNotification(CXWnd* pWnd, unsigned int Message, void* unknown);
	void SaveSetting(PCHAR Key, PCHAR Value, ...);

};
CDPSAdvWnd* DPSWnd = nullptr;
