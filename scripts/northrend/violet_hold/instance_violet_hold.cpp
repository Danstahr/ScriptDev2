#include "precompiled.h"
#include "violethold.h"

#define C_PRISONDOORSEAL 30896
#define C_LAVANTHOR 29312
#define C_MORAGG 29316
#define C_ZURAMAT 29314
#define C_EREKEM 29315
#define C_ICHORON 29313
#define C_XEVOZZ 29266
#define C_DEFSYSTEM 30837
#define DEF_SYSTEM_DAMAGE_AMOUNT 13000

struct MANGOS_DLL_DECL instance_violethold : public ScriptedInstance
{
    instance_violethold(Map* pMap) : ScriptedInstance(pMap) 
    {
        Initialize();
    };

    Map* pMap;
    uint64 DoorSealGUID;
    uint64 LavanthorGUID;
    uint64 MoraggGUID;
    uint64 ZuramatGUID;
    uint64 ErekemGUID;
    uint64 IchoronGUID;
    uint64 XevozzGUID;
    uint64 DefsystemGUID;
    uint16 WipeCheckTimer;
    uint16 BossStatus[PRISONBOSSES];
    uint16 SelectedBoss[2];
    uint16 Completed;
    uint8 PortalCounter;
    uint8 TickCounter;
    uint8 SealIntegrity;
    bool UpDate;
    bool Wiped;

    std::string strInstData;

    void DefenseSystemTrigger(bool CausedByWipe=false)
    {
        Creature* Door = instance->GetCreature(DoorSealGUID);

        if (Door)
        {
            std::list<Creature*> Creatures;
            GetCreatureListWithFactionInGrid(Creatures,Door,FACTION_VIOLET_HOLD_INVADER,1000.0f);
            for (std::list<Creature*>::const_iterator i=Creatures.begin();i!=Creatures.end();++i)
            {
                if (CausedByWipe)
                    if ((*i)->isAlive())
                        (*i)->RemoveFromWorld();
                else
                    (*i)->DealDamage((*i),DEF_SYSTEM_DAMAGE_AMOUNT,NULL,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_ARCANE,NULL,false);
            }
        }
    }

    void CheckCrystals()
    {
        Creature* Door = instance->GetCreature(DoorSealGUID);

        if (Door)
        {
            std::list<GameObject*> Crystals;
            GetGameObjectListWithEntryInGrid(Crystals,Door,193611,1000.0f);
            for (std::list<GameObject*>::const_iterator i = Crystals.begin();i!=Crystals.end();++i)
            {
                (*i)->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNK1);
                (*i)->SetGoState(GO_STATE_READY);
            }
        }
    }

    void CheckBosses()
    {
        Creature* Door = instance->GetCreature(DoorSealGUID);

        for (uint8 i=0;i<PRISONBOSSES;i++)
        {
            //close all cells etc.
            Creature* Door = instance->GetCreature(DoorSealGUID);

            if (!Door)
                break;

            switch(i)
            {
            case 0:
                if (GameObject* pCell = GetClosestGameObjectWithEntry(Door,191566,1000.0f))
                    pCell->SetGoState(GO_STATE_READY);
                break;
            case 1:
                if (GameObject* pCell = GetClosestGameObjectWithEntry(Door,191606,1000.0f))
                     pCell->SetGoState(GO_STATE_READY);
                break;
            case 2:
                if (GameObject* pCell = GetClosestGameObjectWithEntry(Door, 191565, 1000.0f))
                    pCell->SetGoState(GO_STATE_READY);
                break;
            case 3:
                if (GameObject* pCell = GetClosestGameObjectWithEntry(Door, 191564, 1000.0f))
                    pCell->SetGoState(GO_STATE_READY);
                if (GameObject* pCell = GetClosestGameObjectWithEntry(Door, 191563, 1000.0f))
                    pCell->SetGoState(GO_STATE_READY);
                if (GameObject* pCell = GetClosestGameObjectWithEntry(Door, 191562, 1000.0f))
                    pCell->SetGoState(GO_STATE_READY);
                break;
            case 4:
                if (GameObject* pCell = GetClosestGameObjectWithEntry(Door, 191722,1000.0f))
                    pCell->SetGoState(GO_STATE_READY);
                break;
            case 5:
                if (GameObject* pCell = GetClosestGameObjectWithEntry(Door, 191556,1000.0f))
                    pCell->SetGoState(GO_STATE_READY);
                break;
            }

            if (BossStatus[i]==DONE)
            {
                Creature* Boss=instance->GetCreature(GetData64(i));
                if (Boss)
                {
                    if (!(Boss->isAlive()))
                    {
                        Boss->Respawn();
                        Boss->setFaction(35);
                    }
                }
            }
        }
    }

    void OnPlayerEnter(Player* pPlayer)
    {
        if (!pPlayer)
            return;
        
        if (PortalCounter) //event is active
            pPlayer->SendUpdateWorldState(3816,1);
        else
            pPlayer->SendUpdateWorldState(3816,0);
    }

    void Wipe()
    {
        if (Wiped)
            return;
        //destroy portals and all door spawns, open the door
        Creature* Door = instance->GetCreature(DoorSealGUID);
        if (Door)
        {
            Door->AI()->DoAction(DESPAWN_WIPE);
        // Evade all pulled bosses
            for (uint8 i=0;i<2;i++)
            {
                if (SelectedBoss[i]!=255)
                {
                    Creature* Boss = instance->GetCreature(GetData64(SelectedBoss[i]));
                    if ((Boss)&&(BossStatus[SelectedBoss[i]]==IN_PROGRESS))
                    {
                        Boss->setFaction(35);
                        Boss->AI()->EnterEvadeMode();
                        BossStatus[SelectedBoss[i]]=NOT_STARTED;
                    }
                }
            }
        }
        // kill all event mobs arround (defense system AoE) TODO : Escort all enemy units to entrance
        DefenseSystemTrigger(true);
        //set crystals to passive
        if (Door)
        {
            std::list<GameObject*> Crystals;
            GetGameObjectListWithEntryInGrid(Crystals,Door,193611,1000.0f);
            for (std::list<GameObject*>::const_iterator i = Crystals.begin();i!=Crystals.end();++i)
            {
                (*i)->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNK1);
                (*i)->SetGoState(GO_STATE_ACTIVE);
            }
        }

        // reset all settings
        PortalCounter=0;
        TickCounter=0;
        SealIntegrity=100;
        UpDate=false;
        Wiped=true;

        if (instance && instance->IsDungeon())
        {
            Map::PlayerList const &PlayerList = instance->GetPlayers();
            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                ((Player*)i->getSource())->SendUpdateWorldState(3816,0);
            }   
        }
    }

    bool WipeCheck()
    {
        if (Wiped)
            return false;
        
        if (instance && instance->IsDungeon())
        {
            Map::PlayerList const &PlayerList = instance->GetPlayers();
            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                {
                    Player* pPlayer = i->getSource();
                    if (pPlayer->isGameMaster())
                        continue;

                    if (pPlayer->isAlive())
                        return false;
                }
            }
            else return true;
        }
        return true;
    }

    void Initialize()
    {
        PortalCounter=0;
        TickCounter=0;
        WipeCheckTimer=5000;
        DefsystemGUID=0;
        SealIntegrity=100;
        DoorSealGUID = 0;
        LavanthorGUID = 0;
        MoraggGUID=0;
        ZuramatGUID=0;
        ErekemGUID=0;
        IchoronGUID=0;
        XevozzGUID=0;
        for (uint8 i=0;i<2;i++)
            SelectedBoss[i]=255;
        for (uint8 i = 0; i < PRISONBOSSES; i++)
            BossStatus[i] = NOT_STARTED; 
        UpDate=false;
        Wiped=false;
        Completed=0;
    }

    void OnCreatureCreate(Creature* pCreature)
    {
        switch (pCreature->GetEntry())
        {
            case C_PRISONDOORSEAL:
                DoorSealGUID = pCreature->GetGUID();
                break;
            case C_LAVANTHOR:
                    LavanthorGUID = pCreature->GetGUID();
                    pCreature->setFaction(35);
                break;
            case C_MORAGG:
                    MoraggGUID=pCreature->GetGUID();
                    pCreature->setFaction(35);
                break;
            case C_ZURAMAT:
                    ZuramatGUID=pCreature->GetGUID();
                    pCreature->setFaction(35);
                break;
            case C_EREKEM:
                    ErekemGUID=pCreature->GetGUID();
                    pCreature->setFaction(35);
                break;
            case C_ICHORON:
                    IchoronGUID=pCreature->GetGUID();
                    pCreature->setFaction(35);
                break;
            case C_XEVOZZ:
                    XevozzGUID=pCreature->GetGUID();
                    pCreature->setFaction(35);
                break;            
            case C_DEFSYSTEM:
                if (pCreature->isAlive())
                {
                    DefsystemGUID=pCreature->GetGUID();
                    pCreature->SetVisibility(VISIBILITY_OFF);
                }
            default:
                break;
        }
    }

    void OnObjectCreate(GameObject* pGo)
    {
        if (pGo->GetEntry()==193611)
        {
            pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNK1);
            pGo->SetGoState(GO_STATE_ACTIVE);
        }
    }

    void InitWorldState(bool Enable = true)
    {
        DoUpdateWorldState(3815,100);
        DoUpdateWorldState(3816,0);        
    }

    uint32 GetData(uint32 action)
    {
        switch (action)
        {
        case DATA_COMPLETED:
            return Completed;
        case DATA_PORTALCOUNTER:
            return PortalCounter;
        case DATA_TICKCOUNTER:
            return TickCounter;
        case DATA_FIRSTBOSS:
            if (SelectedBoss[0]==255)
            {
                while (1)
                {
                    uint8 rnd=rand()%PRISONBOSSES;
                    if (!BossStatus[rnd])
                    {
                        BossStatus[rnd]=IN_PROGRESS;
                        SelectedBoss[0]=rnd;
                        SetData(12345,DONE);
                        return rnd;
                    }
                }
            }
            else return SelectedBoss[0];
        case DATA_SECONDBOSS:
            if (SelectedBoss[1]==255)
            {
                while (1)
                {
                    uint8 rnd=rand()%PRISONBOSSES;
                    if (!BossStatus[rnd])
                    {
                        BossStatus[rnd]=IN_PROGRESS;
                        SelectedBoss[1]=rnd;
                        SetData(12345,DONE);
                        return rnd;
                    }
                }
            }
            else return SelectedBoss[1];
        default:
            return 0;
        }
    }

    uint64 GetData64(uint32 action)
    {
        switch(action)
        {
            case LAVANTHOR_GUID:
                return LavanthorGUID;
            case MORAGG_GUID:
                return MoraggGUID;
            case ZURAMAT_GUID:
                return ZuramatGUID;
            case EREKEM_GUID:
                return ErekemGUID;
            case ICHORON_GUID:
                return IchoronGUID;
            case XEVOZZ_GUID:
                return XevozzGUID;
            case DOOR_GUID:
                return DoorSealGUID;
            case DEFSYSTEM_GUID:
                return DefsystemGUID;
            default:
                return 0;
        }
    }

    void SetData(uint32 type, uint32 data)
    {
        if (type<PRISONBOSSES)
            BossStatus[type] = data;

        switch(type)
        {
        case DATA_COMPLETED:
            Completed=data;
            break;
        case DATA_PORTALCOUNTER:
            PortalCounter=data;
            DoUpdateWorldState(3810, data);
            if (PortalCounter==1)
                CheckCrystals();
            break;
        case DATA_TICKCOUNTER:
            TickCounter=data;
            break;
        case DATA_CANWIPEAGAIN:
            Wiped=false;
            break;
        case DATA_DEFSYSTEM_TRIGGER:
            DefenseSystemTrigger();
            break;
        }

        if (data == DONE)
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << (uint32)BossStatus[0] << " " << (uint32)BossStatus[1] << " " << (uint32)BossStatus[2] << " "
                << (uint32)BossStatus[3] << " "<< (uint32)BossStatus[4] << " " << (uint32)BossStatus[5] << " " 
                << (uint32)SelectedBoss[0] << " " << (uint32)SelectedBoss[1] << " " << Completed;

            strInstData = saveStream.str();

            SaveToDB();
            OUT_SAVE_INST_DATA_COMPLETE;
        }
    }

    const char* Save()
    {
        return strInstData.c_str();
    }

    void Load(const char *chrIn)
    {
        if (!chrIn)
        {
            OUT_LOAD_INST_DATA_FAIL;
            return;
        }

        OUT_LOAD_INST_DATA(chrIn);

        std::istringstream loadStream(chrIn);
        loadStream >> BossStatus[0] >> BossStatus[1] >> BossStatus[2] >> BossStatus[3] >> BossStatus[4] >> BossStatus[5] >> SelectedBoss[0] >> SelectedBoss[1] >> Completed;

        for (uint8 i = 0; i < PRISONBOSSES; ++i)
            if (BossStatus[i] == IN_PROGRESS)
                BossStatus[i] = NOT_STARTED;
        UpDate=false;

        OUT_LOAD_INST_DATA_COMPLETE;
    }

    void Update(uint32 diff)
    {
        if (!UpDate)
        {
            UpDate=true;
            DoUpdateWorldState(3815,SealIntegrity);
            CheckBosses();
        }
        
        if (TickCounter>=18)
        {
            TickCounter=0;
            SealIntegrity--;
            if (!SealIntegrity)
                Wipe();
            DoUpdateWorldState(3815,SealIntegrity);
        }

        if (WipeCheckTimer<diff)
        {
            WipeCheckTimer=5000;
            if (WipeCheck()) 
                Wipe();
        }
        else WipeCheckTimer-=diff;
    }
};

bool GOHello_crystal(Player* pPlayer, GameObject* pGo)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData();
    if (pInstance)
    {
        if (pInstance->GetData(DATA_PORTALCOUNTER)!=0)
        {
            pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_UNK1);
            pGo->SetGoState(GO_STATE_ACTIVE);
            pInstance->SetData(DATA_DEFSYSTEM_TRIGGER,0);
        }
        else
            pGo->SetGoState(GO_STATE_READY);
    }

    return true;
};

InstanceData* GetInstanceData_instance_violethold(Map* pMap)
{
    return new instance_violethold(pMap);
}

void AddSC_instance_violethold()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "instance_violethold";
    newscript->GetInstanceData = &GetInstanceData_instance_violethold;
    newscript->RegisterSelf();

    newscript = new Script();
    newscript->Name = "go_violet_hold_crystal";
    newscript->pGOHello = &GOHello_crystal;
    newscript->RegisterSelf();
}
