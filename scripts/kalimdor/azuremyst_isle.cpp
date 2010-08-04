/* Copyright (C) 2006 - 2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Azuremyst_Isle
SD%Complete: 75
SDComment: Quest support: 9283, 9531, 9537, 9582, 9554(special flight path, proper model for mount missing). Injured Draenei cosmetic only
SDCategory: Azuremyst Isle
EndScriptData */

/* ContentData
npc_draenei_survivor
npc_geezle
npc_engineer_spark_overgrind
npc_injured_draenei
npc_magwin
npc_susurrus
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "TemporarySummon.h"
#include <cmath>

/*######
## npc_draenei_survivor
######*/

enum
{
    SAY_HEAL1           = -1000176,
    SAY_HEAL2           = -1000177,
    SAY_HEAL3           = -1000178,
    SAY_HEAL4           = -1000179,
    SAY_HELP1           = -1000180,
    SAY_HELP2           = -1000181,
    SAY_HELP3           = -1000182,
    SAY_HELP4           = -1000183,

    SPELL_IRRIDATION    = 35046,
    SPELL_STUNNED       = 28630
};

struct MANGOS_DLL_DECL npc_draenei_survivorAI : public ScriptedAI
{
    npc_draenei_survivorAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint64 m_uiCaster;

    uint32 m_uiSayThanksTimer;
    uint32 m_uiRunAwayTimer;
    uint32 m_uiSayHelpTimer;

    bool m_bCanSayHelp;

    void Reset()
    {
        m_uiCaster = 0;

        m_uiSayThanksTimer = 0;
        m_uiRunAwayTimer = 0;
        m_uiSayHelpTimer = 10000;

        m_bCanSayHelp = true;

        m_creature->CastSpell(m_creature, SPELL_IRRIDATION, true);

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
        m_creature->SetHealth(int(m_creature->GetMaxHealth()*.1));
        m_creature->SetStandState(UNIT_STAND_STATE_SLEEP);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (m_bCanSayHelp && pWho->GetTypeId() == TYPEID_PLAYER && m_creature->IsFriendlyTo(pWho) &&
            m_creature->IsWithinDistInMap(pWho, 25.0f))
        {
            //Random switch between 4 texts
            switch(urand(0, 3))
            {
                case 0: DoScriptText(SAY_HELP1, m_creature, pWho); break;
                case 1: DoScriptText(SAY_HELP2, m_creature, pWho); break;
                case 2: DoScriptText(SAY_HELP3, m_creature, pWho); break;
                case 3: DoScriptText(SAY_HELP4, m_creature, pWho); break;
            }

            m_uiSayHelpTimer = 20000;
            m_bCanSayHelp = false;
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        if (pSpell->SpellFamilyFlags2 & 0x080000000)
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);

            m_creature->CastSpell(m_creature, SPELL_STUNNED, true);

            m_uiCaster = pCaster->GetGUID();

            m_uiSayThanksTimer = 5000;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_uiSayThanksTimer)
        {
            if (m_uiSayThanksTimer <= uiDiff)
            {
                m_creature->RemoveAurasDueToSpell(SPELL_IRRIDATION);

                if (Player* pPlayer = (Player*)Unit::GetUnit(*m_creature,m_uiCaster))
                {
                    if (pPlayer->GetTypeId() != TYPEID_PLAYER)
                        return;

                    switch(urand(0, 3))
                    {
                        case 0: DoScriptText(SAY_HEAL1, m_creature, pPlayer); break;
                        case 1: DoScriptText(SAY_HEAL2, m_creature, pPlayer); break;
                        case 2: DoScriptText(SAY_HEAL3, m_creature, pPlayer); break;
                        case 3: DoScriptText(SAY_HEAL4, m_creature, pPlayer); break;
                    }

                    pPlayer->TalkedToCreature(m_creature->GetEntry(),m_creature->GetGUID());
                }

                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MovePoint(0, -4115.053711f, -13754.831055f, 73.508949f);

                m_uiRunAwayTimer = 10000;
                m_uiSayThanksTimer = 0;
            }else m_uiSayThanksTimer -= uiDiff;

            return;
        }

        if (m_uiRunAwayTimer)
        {
            if (m_uiRunAwayTimer <= uiDiff)
                m_creature->ForcedDespawn();
            else
                m_uiRunAwayTimer -= uiDiff;

            return;
        }

        if (m_uiSayHelpTimer < uiDiff)
        {
            m_bCanSayHelp = true;
            m_uiSayHelpTimer = 20000;
        }else m_uiSayHelpTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_draenei_survivor(Creature* pCreature)
{
    return new npc_draenei_survivorAI(pCreature);
}

/*######
## npc_geezle
######*/

enum
{
    C_SPARK                 = 17243,

    GO_NAGA_FLAG            = 181694,

    AREA_COVE               = 3579,
    AREA_ISLE               = 3639,

    SAY_GEEZLE1             = -1002050,
    SAY_GEEZLE2             = -1002051,
    SAY_GEEZLE3             = -1002052,

    Q_TREE_COMPANY          = 9531,

    AURA_TREE_DISGUISE      = 30298
};

float SparkSpawnPoint[]={-5046.07f,-11258.55f,7.508f};

struct MANGOS_DLL_DECL npc_geezleAI : public npc_escortAI
{
    npc_geezleAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        Reset();
    }

    bool EventStarted;
    uint8 Phase;
    uint16 RPTimer;
    Creature* EngSpark;

    void Reset()
    {
        Phase=0;
        RPTimer=23000;
        EventStarted=false;
        EngSpark=NULL;
    }

    void WaypointReached(uint32)
    {
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!EventStarted)
        {
            Creature* Spark = GetClosestCreatureWithEntry(m_creature,C_SPARK,1000.0f);
            if (Spark)
            {
                if ((Spark->GetAreaId()==AREA_COVE) || (Spark->GetAreaId()==AREA_ISLE)) //tree event already active
                {
                    ((TemporarySummon*)m_creature)->UnSummon();
                    error_log("SD2 : npc_geezle - Tree event is already active!");
                    return;
                }
            }
            if (Spark = m_creature->SummonCreature(C_SPARK,SparkSpawnPoint[0],SparkSpawnPoint[1],SparkSpawnPoint[2],0,TEMPSUMMON_TIMED_DESPAWN,150000))
            {
                EngSpark=Spark;
                EventStarted=true;
                Start();
            }
            return;
        }

        if (RPTimer<diff)
        {
            switch (Phase)
            {
            case 0:
                if (EngSpark)
                {
                    m_creature->SetInFront(EngSpark);
                    EngSpark->SetInFront(m_creature);
                }
                DoScriptText(SAY_GEEZLE1,m_creature);
                RPTimer=20000;
                Phase++;
                break;
            case 1:
                DoScriptText(SAY_GEEZLE2,m_creature);
                RPTimer=26000;
                Phase++;
                break;
            case 2:
                DoScriptText(SAY_GEEZLE3,m_creature);
                RPTimer=65000;
                Phase++;
                break;
            default: break;
            }
        }
        else RPTimer-=diff;
    }
};

CreatureAI* GetAI_npc_geezleAI(Creature* pCreature)
{
    return new npc_geezleAI(pCreature);
}

/*######
## npc_engineer_spark_overgrind
######*/

enum
{
    SAY_TEXT                = -1000184,
    EMOTE_SHELL             = -1000185,
    SAY_ATTACK              = -1000186,

    SAY_SPARK1              = -1002053,
    SAY_SPARK2              = -1002054,
    SAY_SPARK3              = -1002055,
    SAY_SPARK4              = -1002056,
    SAY_SPARK0              = -1002057,

    QUEST_GNOMERCY          = 9537,
    FACTION_HOSTILE         = 14,
    SPELL_DYNAMITE          = 7978
};

#define GOSSIP_FIGHT        "Traitor! You will be brought to justice!"

struct MANGOS_DLL_DECL npc_engineer_spark_overgrindAI : public npc_escortAI
{
    npc_engineer_spark_overgrindAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_uiNormFaction = pCreature->getFaction();
        m_uiNpcFlags = pCreature->GetUInt32Value(UNIT_NPC_FLAGS);
        Reset();

        if (pCreature->GetAreaId() == AREA_COVE || pCreature->GetAreaId() == AREA_ISLE)
            m_bIsTreeEvent = true;
    }

    uint32 m_uiNpcFlags;
    uint32 m_uiNormFaction;

    uint32 m_uiDynamiteTimer;
    uint32 m_uiEmoteTimer;

    uint8 Phase;

    uint16 RPTimer;

    bool m_bIsTreeEvent;
    bool EventStarted;

    void Reset()
    {
        m_creature->setFaction(m_uiNormFaction);
        m_creature->SetUInt32Value(UNIT_NPC_FLAGS, m_uiNpcFlags);

        m_uiDynamiteTimer = 8000;
        m_uiEmoteTimer = urand(120000, 150000);

        m_bIsTreeEvent = false;
        EventStarted=false;
        Phase=255;
        RPTimer=21000;
    }

    void Aggro(Unit *who)
    {
        DoScriptText(SAY_ATTACK, m_creature, who);
    }

    void DoComplete()
    {
        std::list<Player*> players;
        MaNGOS::AnyPlayerInObjectRangeCheck checker(m_creature,50.0f);
        MaNGOS::PlayerListSearcher<MaNGOS::AnyPlayerInObjectRangeCheck> searcher(m_creature,players,checker);
        Cell::VisitAllObjects(m_creature,searcher,50.0f);
        for (std::list<Player*>::const_iterator i=players.begin();i!=players.end();++i)
        {
            if ((*i)->GetQuestStatus(Q_TREE_COMPANY) == QUEST_STATUS_INCOMPLETE &&(*i)->HasAura(AURA_TREE_DISGUISE))
            {
            (*i)->KilledMonsterCredit(C_SPARK,0);
            }
        }
    }


    void WaypointReached(uint32)
    {
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!m_creature->isInCombat() && !m_bIsTreeEvent)
        {
            if (m_uiEmoteTimer < diff)
            {
                DoScriptText(SAY_TEXT, m_creature);
                DoScriptText(EMOTE_SHELL, m_creature);
                m_uiEmoteTimer = urand(120000, 150000);
            }
            else m_uiEmoteTimer -= diff;
        }
        else if (m_bIsTreeEvent)
        {
            if (!EventStarted)
            {
                m_creature->GetMotionMaster()->MoveIdle();
                m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                Start();
                EventStarted=true;
            }
            else
            {
                if (RPTimer<diff)
                {
                    GameObject* flag=NULL;
                    switch (Phase)
                    {                   
                    case 0:
                        DoScriptText(SAY_SPARK1,m_creature);
                        Phase++;
                        RPTimer=8000;
                        break;
                    case 1:
                        DoScriptText(SAY_SPARK2,m_creature);
                        Phase++;
                        RPTimer=15000;
                        break;
                    case 2:
                        DoScriptText(SAY_SPARK3,m_creature);
                        Phase++;
                        RPTimer=9000;
                        break;
                    case 3:
                        DoScriptText(SAY_SPARK4,m_creature);
                        Phase++;
                        RPTimer=20000;
                        break;
                    case 4:
                        DoComplete();
                        Phase++;
                        RPTimer=50000;
                        break;
                    case 255:
                        DoScriptText(SAY_SPARK0,m_creature);
                        flag = GetClosestGameObjectWithEntry(m_creature,GO_NAGA_FLAG,1000.0f);
                        if (flag)
                            flag->SetLootState(GO_JUST_DEACTIVATED);
                        Phase++;
                        RPTimer=7000;
                        break;
                    default:
                        break;
                    }
                }
                else RPTimer-=diff;
            }
            return;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiDynamiteTimer < diff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_DYNAMITE);
            m_uiDynamiteTimer = 8000;
        }
        else m_uiDynamiteTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_engineer_spark_overgrind(Creature* pCreature)
{
    return new npc_engineer_spark_overgrindAI(pCreature);
}

bool GossipHello_npc_engineer_spark_overgrind(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_GNOMERCY) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_FIGHT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_engineer_spark_overgrind(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->setFaction(FACTION_HOSTILE);
        ((npc_engineer_spark_overgrindAI*)pCreature->AI())->AttackStart(pPlayer);
    }
    return true;
}

/*######
## npc_injured_draenei
######*/

struct MANGOS_DLL_DECL npc_injured_draeneiAI : public ScriptedAI
{
    npc_injured_draeneiAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset()
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
        m_creature->SetHealth(int(m_creature->GetMaxHealth()*.15));
        switch(urand(0, 1))
        {
            case 0: m_creature->SetStandState(UNIT_STAND_STATE_SIT); break;
            case 1: m_creature->SetStandState(UNIT_STAND_STATE_SLEEP); break;
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        return;                                             //ignore everyone around them (won't aggro anything)
    }

    void UpdateAI(const uint32 diff)
    {
        return;
    }

};
CreatureAI* GetAI_npc_injured_draenei(Creature* pCreature)
{
    return new npc_injured_draeneiAI(pCreature);
}

/*######
## npc_magwin
######*/

#define SAY_START               -1000111
#define SAY_AGGRO               -1000112
#define SAY_PROGRESS            -1000113
#define SAY_END1                -1000114
#define SAY_END2                -1000115
#define EMOTE_HUG               -1000116

#define QUEST_A_CRY_FOR_HELP    9528

struct MANGOS_DLL_DECL npc_magwinAI : public npc_escortAI
{
    npc_magwinAI(Creature* pCreature) : npc_escortAI(pCreature) {Reset();}

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch(i)
        {
            case 0:
                DoScriptText(SAY_START, m_creature, pPlayer);
                break;
            case 17:
                DoScriptText(SAY_PROGRESS, m_creature, pPlayer);
                break;
            case 28:
                DoScriptText(SAY_END1, m_creature, pPlayer);
                break;
            case 29:
                DoScriptText(EMOTE_HUG, m_creature, pPlayer);
                DoScriptText(SAY_END2, m_creature, pPlayer);
                pPlayer->GroupEventHappens(QUEST_A_CRY_FOR_HELP, m_creature);
                break;
        }
    }

    void Aggro(Unit* who)
    {
        DoScriptText(SAY_AGGRO, m_creature, who);
    }

    void Reset() { }
};

bool QuestAccept_npc_magwin(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_A_CRY_FOR_HELP)
    {
        pCreature->setFaction(10);

        if (npc_magwinAI* pEscortAI = dynamic_cast<npc_magwinAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer->GetGUID(), pQuest);
    }
    return true;
}

CreatureAI* GetAI_npc_magwinAI(Creature* pCreature)
{
    return new npc_magwinAI(pCreature);
}

/*######
## npc_susurrus
######*/

enum
{
    ITEM_WHORL_OF_AIR       = 23843,
    SPELL_BUFFETING_WINDS   = 32474,
    TAXI_PATH_ID            = 506
};

#define GOSSIP_ITEM_READY   "I am ready."

bool GossipHello_npc_susurrus(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->HasItemCount(ITEM_WHORL_OF_AIR,1,true))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_READY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_susurrus(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF)
    {
        //spellId is correct, however it gives flight a somewhat funny effect
        pPlayer->CLOSE_GOSSIP_MENU();
        pPlayer->CastSpell(pPlayer,SPELL_BUFFETING_WINDS,true);
    }
    return true;
}

/*######
##
######*/

void AddSC_azuremyst_isle()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_draenei_survivor";
    newscript->GetAI = &GetAI_npc_draenei_survivor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_engineer_spark_overgrind";
    newscript->GetAI = &GetAI_npc_engineer_spark_overgrind;
    newscript->pGossipHello =  &GossipHello_npc_engineer_spark_overgrind;
    newscript->pGossipSelect = &GossipSelect_npc_engineer_spark_overgrind;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_injured_draenei";
    newscript->GetAI = &GetAI_npc_injured_draenei;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_magwin";
    newscript->GetAI = &GetAI_npc_magwinAI;
    newscript->pQuestAccept = &QuestAccept_npc_magwin;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_susurrus";
    newscript->pGossipHello =  &GossipHello_npc_susurrus;
    newscript->pGossipSelect = &GossipSelect_npc_susurrus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_geezle";
    newscript->GetAI = &GetAI_npc_geezleAI;
    newscript->RegisterSelf();
}
