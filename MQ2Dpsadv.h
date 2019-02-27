// DPS ADV CREATED BY WARNEN 2008-2009
// MQ2DPSAdv.h

char* OtherHits[] = {" punches "," slashes "," crushes "," pierces "," hits "," kicks "," backstabs "," frenzies on "," bashes ", " strikes "," claws "," slices "," bites "," mauls "," stings ", " shoots ", " smashes ", 0};
char* YourHits[] = {" hit "," slash "," pierce "," crush "," kick "," punch "," backstab "," frenzy on "," bash ", " strike "," claw "," slice "," bite "," maul "," sting ", " shoot ", " smash ", 0};
enum { CLISTTARGET, CLISTMAXDMG, SINGLE };
enum { NOTOTAL, TOTALABOVE, TOTALSECOND, TOTALBOTTOM };

static int Coloring []= {
   {0},   //unk
   {13},   //war
   {4},   //clr
   {10},   //pal
   {11},   //rng
   {13},   //shd
   {5},   //dru
   {8},   //mnk
   {2},   //brd
   {12},   //rog
   {14},   //shm
   {9},   //nec
   {14},   //wiz
   {7},   //mag
   {6},   //enc
   {3},   //bst
   {15},   //ber
};

int ColorTest[17];

PSPAWNINFO   CurTarget;
time_t      Intervals;
int         CListType;
int         MaxDmgLast;
int         MeColor;
int         MeTopColor;
int         NormalColor;
int         NPCColor;
int         TotalColor;
int         EntHover;
int         EntHighlight;
int         FightNormal;
int         FightHover;
int         FightHighlight;
int         FightActive;
int         FightInActive;
int         FightDead;
bool      Saved;
bool      WarnedYHO, WarnedOHO;
bool      Debug;
bool      Active;
bool      Zoning;
bool      WrongUI;

bool      ShowMeTop;
bool      ShowMeMin;
int         ShowMeMinNum;
bool      UseRaidColors;
bool      LiveUpdate;
int         ShowTotal;
int         FightIA;
int         FightTO;
int         EntTO;

struct EntDamage {
   int Total;
   time_t First;
   time_t Last;
   int AddTime;
};

PLUGIN_API VOID SetGameState(DWORD GameState);
PLUGIN_API VOID OnCleanUI(VOID);
PLUGIN_API VOID OnReloadUI(VOID);
PLUGIN_API VOID InitializePlugin(VOID);
PLUGIN_API VOID ShutdownPlugin(VOID);
PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color);
PLUGIN_API VOID OnPulse(VOID);
void         DestroyDPSWindow();
void         CreateDPSWindow();
void         CreateDPSWindow();
void         DestroyDPSWindow();
template <unsigned int _EntSize, unsigned int _MobSize>bool SplitStringOtherHitOther(PCHAR Line, CHAR(&EntName)[_EntSize], CHAR(&MobName)[_MobSize], int *Damage);
template <unsigned int _EntSize, unsigned int _MobSize>bool SplitStringDOT(PCHAR Line, CHAR(&EntName)[_EntSize], CHAR(&MobName)[_MobSize], int *Damage);
template <unsigned int _EntSize, unsigned int _MobSize>bool SplitStringNonMelee(PCHAR Line, CHAR(&EntName)[_EntSize], CHAR(&MobName)[_MobSize], int *Damage);
void         HandleYouHitOther(PCHAR Line);
void         HandleOtherHitOther(PCHAR Line);
void         HandleNonMelee(PCHAR Line);
void         HandleDOT(PCHAR Line);
void         HandleDeath(PCHAR Line);
void         TargetSwitch();
void         CheckActive();
void         DPSAdvCmd(PSPAWNINFO pChar, PCHAR szLine);
#ifdef DPSDEV
void         DPSTestCmd(PSPAWNINFO pChar, PCHAR szLine);
#endif

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
      DPSMob *Parent;
      PSPAWNINFO Spawn;
      DPSEntry *Master;

      DPSEntry();
      DPSEntry(char EntName[64], DPSMob *pParent);
      void      Init();
      void      AddDamage(int aDamage);
      int         GetDPS();
      void      Sort();
      void      GetSpawn();
      bool      CheckMaster();
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
   DPSEntry *LastEntry;
   vector<DPSEntry*> EntList;

            DPSMob();
			DPSMob::DPSMob(PCHAR MobName, size_t MobLen);
   void      Init();
   void      AddDamage(int aDamage);
   void      GetSpawn();
   bool      IsPet();
   DPSEntry   *GetEntry(char EntName[64], bool Create = true);
};
vector<DPSMob*> MobList;
DPSMob   *LastMob;
DPSMob   *CurTarMob;
DPSMob   *CurListMob;
DPSMob   *CurMaxMob;
template <unsigned int _NameSize>DPSMob *GetMob(CHAR(&Name)[_NameSize], bool Create = true, bool Alive = false);
bool         MobListMaint(DPSMob *Mob, int ListLoc);
void         HandleDeath(DPSMob *DeadMob);
void         ListSwitch(DPSMob *Switcher);

class CDPSAdvWnd : public CCustomWnd {
public:
   CTabWnd *Tabs;
   CListWnd *LTopList;
   CComboWnd *CMobList;
   CCheckBoxWnd *CShowMeTop;
   CCheckBoxWnd *CShowMeMin;
   CTextEntryWnd *TShowMeMin;
   CCheckBoxWnd *CUseRaidColors;
   CCheckBoxWnd *CLiveUpdate;
   CTextEntryWnd *TFightIA;
   CTextEntryWnd *TFightTO;
   CTextEntryWnd *TEntTO;
   CComboWnd *CShowTotal;
//   CListWnd *LFightList;
   bool ReSort;

   CDPSAdvWnd();
   ~CDPSAdvWnd();
   void DrawList(bool DoDead = false);
   void SetTotal(int LineNum, DPSMob *Mob);
   void DrawCombo();
   void LoadLoc(char szChar[64] = 0);
   void LoadSettings();
   void SaveLoc();
   void SetLineColors(int LineNum, DPSMob::DPSEntry *Ent, bool Total = false, bool MeTop = false);
   int WndNotification(CXWnd *pWnd, unsigned int Message, void *unknown);
   void SaveSetting(PCHAR Key, PCHAR Value, ...);

};
CDPSAdvWnd *DPSWnd = 0;
