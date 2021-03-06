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
SDName: Old_Hillsbrad
SD%Complete: 40
SDComment: Quest support: 10283, 10284. All friendly NPC's. Thrall waypoints fairly complete, missing many details, but possible to complete escort.
SDCategory: Caverns of Time, Old Hillsbrad Foothills
EndScriptData */

/* ContentData
npc_brazen
npc_erozion
npc_thrall_old_hillsbrad
npc_taretha
EndContentData */

#include "precompiled.h"
#include "old_hillsbrad.h"
#include "escort_ai.h"

enum
{
    QUEST_ENTRY_HILLSBRAD   = 10282,
    QUEST_ENTRY_DIVERSION   = 10283,
    QUEST_ENTRY_ESCAPE      = 10284,
    QUEST_ENTRY_RETURN      = 10285,
    ITEM_ENTRY_BOMBS        = 25853,

    TAXI_PATH_ID            = 534
};

struct MANGOS_DLL_DECL npc_tarethaAI : public npc_escortAI
{
    npc_tarethaAI(Creature* pCreature);

    ScriptedInstance* m_pInstance;

    void WaypointReached(uint32 i);

    void Reset() {}
};

/*######
## npc_brazen
######*/

#define GOSSIP_ITEM_READY       "I am ready to go to Durnholde Keep."

bool GossipHello_npc_brazen(Player* pPlayer, Creature* pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_READY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_brazen(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        if (!pPlayer->HasItemCount(ITEM_ENTRY_BOMBS,1))
            pPlayer->SEND_GOSSIP_MENU(9780, pCreature->GetGUID());
        else
        {
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->ActivateTaxiPathTo(TAXI_PATH_ID);
        }
    }
    return true;
}

/*######
## npc_erozion
######*/

bool GossipHello_npc_erozion(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    ScriptedInstance* pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());

    if (pInstance && pInstance->GetData(TYPE_BARREL_DIVERSION) != DONE && !pPlayer->HasItemCount(ITEM_ENTRY_BOMBS,1))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I need a pack of Incendiary Bombs.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    if (!pPlayer->GetQuestRewardStatus(QUEST_ENTRY_RETURN) && pPlayer->GetQuestStatus(QUEST_ENTRY_RETURN) == QUEST_STATUS_COMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "[PH] Teleport please, i'm tired.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    pPlayer->SEND_GOSSIP_MENU(9778, pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_erozion(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        if (Item* pItem = pPlayer->StoreNewItemInInventorySlot(ITEM_ENTRY_BOMBS, 1))
            pPlayer->SendNewItem(pItem, 1, true, false);

        pPlayer->SEND_GOSSIP_MENU(9515, pCreature->GetGUID());
    }
    if (uiAction == GOSSIP_ACTION_INFO_DEF+2)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
    }
    return true;
}

/*######
## npc_thrall_old_hillsbrad
######*/

//Thrall texts
#define SAY_TH_START_EVENT_PART1    -1560023
#define SAY_TH_ARMORY               -1560024
#define SAY_TH_SKARLOC_MEET         -1560025
#define SAY_TH_SKARLOC_TAUNT        -1560026
#define SAY_TH_START_EVENT_PART2    -1560027
#define SAY_TH_MOUNTS_UP            -1560028
#define SAY_TH_CHURCH_END           -1560029
#define SAY_TH_MEET_TARETHA         -1560030
#define SAY_TH_EPOCH_WONDER         -1560031
#define SAY_TH_EPOCH_KILL_TARETHA   -1560032
#define SAY_TH_EVENT_COMPLETE       -1560033

#define SAY_TH_RANDOM_LOW_HP1       -1560034
#define SAY_TH_RANDOM_LOW_HP2       -1560035

#define SAY_TH_RANDOM_DIE1          -1560036
#define SAY_TH_RANDOM_DIE2          -1560037

#define SAY_TH_RANDOM_AGGRO1        -1560038
#define SAY_TH_RANDOM_AGGRO2        -1560039
#define SAY_TH_RANDOM_AGGRO3        -1560040
#define SAY_TH_RANDOM_AGGRO4        -1560041

#define SAY_TH_RANDOM_KILL1         -1560042
#define SAY_TH_RANDOM_KILL2         -1560043
#define SAY_TH_RANDOM_KILL3         -1560044

#define SAY_TH_LEAVE_COMBAT1        -1560045
#define SAY_TH_LEAVE_COMBAT2        -1560046
#define SAY_TH_LEAVE_COMBAT3        -1560047

//Taretha texts
#define SAY_TA_FREE                 -1560048
#define SAY_TA_ESCAPED              -1560049

//Misc for Thrall
#define SPELL_STRIKE                14516
#define SPELL_SHIELD_BLOCK          12169
#define SPELL_SUMMON_EROZION_IMAGE  33954                   //if thrall dies during escort?

#define SPEED_WALK                  (0.5f)
#define SPEED_RUN                   (1.0f)
#define SPEED_MOUNT                 (1.6f)

#define EQUIP_ID_WEAPON             927
#define EQUIP_ID_SHIELD             20913
#define THRALL_MODEL_UNEQUIPPED     17292
#define THRALL_MODEL_EQUIPPED       18165

//misc creature entries
#define ENTRY_ARMORER               18764
#define ENTRY_SCARLOC               17862

#define MOB_ENTRY_RIFLE             17820
#define MOB_ENTRY_WARDEN            17833
#define MOB_ENTRY_VETERAN           17860
#define MOB_ENTRY_WATCHMAN          17814
#define MOB_ENTRY_SENTRY            17815

#define MOB_ENTRY_BARN_GUARDSMAN    18092
#define MOB_ENTRY_BARN_PROTECTOR    18093
#define MOB_ENTRY_BARN_LOOKOUT      18094

#define MOB_ENTRY_CHURCH_GUARDSMAN  23175
#define MOB_ENTRY_CHURCH_PROTECTOR  23179
#define MOB_ENTRY_CHURCH_LOOKOUT    23177

#define MOB_ENTRY_INN_GUARDSMAN     23176
#define MOB_ENTRY_INN_PROTECTOR     23180
#define MOB_ENTRY_INN_LOOKOUT       23178

#define SKARLOC_MOUNT               18798
#define SKARLOC_MOUNT_MODEL         18223
#define EROZION_ENTRY               18723
#define ENTRY_EPOCH                 18096
#define NPC_THRALL_QUEST_TRIGGER    20156

//gossip items
#define GOSSIP_ID_START             9568
#define GOSSIP_ID_SKARLOC1          9614                    //I'm glad Taretha is alive. We now must find a way to free her...
#define GOSSIP_ITEM_SKARLOC1        "Taretha cannot see you, Thrall."
#define GOSSIP_ID_SKARLOC2          9579                    //What do you mean by this? Is Taretha in danger?
#define GOSSIP_ITEM_SKARLOC2        "The situation is rather complicated, Thrall. It would be best for you to head into the mountains now, before more of Blackmoore's men show up. We'll make sure Taretha is safe."
#define GOSSIP_ID_SKARLOC3          9580

#define GOSSIP_ID_TARREN            9597                    //tarren mill is beyond these trees
#define GOSSIP_ITEM_TARREN          "We're ready, Thrall."

#define GOSSIP_ID_COMPLETE          9578                    //Thank you friends, I owe my freedom to you. Where is Taretha? I hoped to see her

struct MANGOS_DLL_DECL npc_thrall_old_hillsbradAI : public npc_escortAI
{
    npc_thrall_old_hillsbradAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        HadMount = false;
        Reset();
    }

    ScriptedInstance* m_pInstance;
    uint64 TarethaGUID;

    bool LowHp;
    bool HadMount;

    void WaypointReached(uint32 i)
    {
        if (!m_pInstance)
            return;

        switch(i)
        {
            case 8:
                SetRun(false);
                m_creature->SummonCreature(ENTRY_ARMORER, 2181.87f, 112.46f, 89.45f, 0.26f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                break;
            case 9:
                DoScriptText(SAY_TH_ARMORY, m_creature);
                SetEquipmentSlots(false, EQUIP_ID_WEAPON, EQUIP_ID_SHIELD, EQUIP_NO_CHANGE);
                break;
            case 10:
                m_creature->SetDisplayId(THRALL_MODEL_EQUIPPED);
                break;
            case 11:
                SetRun();
                break;
            case 15:
                m_creature->SummonCreature(MOB_ENTRY_RIFLE, 2200.28f, 137.37f, 87.93f, 5.07f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_WARDEN, 2197.44f, 131.83f, 87.93f, 0.78f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_VETERAN, 2203.62f, 135.40f, 87.93f, 3.70f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_VETERAN, 2200.75f, 130.13f, 87.93f, 1.48f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                break;
            case 21:
                m_creature->SummonCreature(MOB_ENTRY_RIFLE, 2135.80f, 154.01f, 67.45f, 4.98f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_WARDEN, 2144.36f, 151.87f, 67.74f, 4.46f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_VETERAN, 2142.12f, 154.41f, 67.12f, 4.56f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_VETERAN, 2138.08f, 155.38f, 67.24f, 4.60f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                break;
            case 25:
                m_creature->SummonCreature(MOB_ENTRY_RIFLE, 2102.98f, 192.17f, 65.24f, 6.02f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_WARDEN, 2108.48f, 198.75f, 65.18f, 5.15f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_VETERAN, 2106.11f, 197.29f, 65.18f, 5.63f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_VETERAN, 2104.18f, 194.82f, 65.18f, 5.75f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                break;
            case 29:
                DoScriptText(SAY_TH_SKARLOC_MEET, m_creature);
                m_creature->SummonCreature(ENTRY_SCARLOC, 2036.48f, 271.22f, 63.43f, 5.27f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,30000);
                break;
            case 30:
                SetEscortPaused(true);
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                SetRun(false);
                break;
            case 31:
                DoScriptText(SAY_TH_MOUNTS_UP, m_creature);
                DoMount();
                SetRun();
                break;
            case 37:
                //possibly regular patrollers? If so, remove this and let database handle them
                m_creature->SummonCreature(MOB_ENTRY_WATCHMAN, 2124.26f, 522.16f, 56.87f, 3.99f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_WATCHMAN, 2121.69f, 525.37f, 57.11f, 4.01f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_SENTRY, 2124.65f, 524.55f, 56.63f, 3.98f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                break;
            case 59:
                m_creature->SummonCreature(SKARLOC_MOUNT, 2488.64f, 625.77f, 58.26f, 4.71f, TEMPSUMMON_TIMED_DESPAWN,10000);
                DoUnmount();
                HadMount = false;
                SetRun(false);
                break;
            case 60:
                m_creature->HandleEmote(EMOTE_ONESHOT_EXCLAMATION);
                //make horsie run off
                SetEscortPaused(true);
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                m_pInstance->SetData(TYPE_THRALL_PART2, DONE);
                SetRun();
                break;
            case 64:
                SetRun(false);
                break;
            case 68:
                m_creature->SummonCreature(MOB_ENTRY_BARN_PROTECTOR, 2500.22f, 692.60f, 55.50f, 2.84f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_BARN_LOOKOUT, 2500.13f, 696.55f, 55.51f, 3.38f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_BARN_GUARDSMAN, 2500.55f, 693.64f, 55.50f, 3.14f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_BARN_GUARDSMAN, 2500.94f, 695.81f, 55.50f, 3.14f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                break;
            case 71:
                SetRun();
                break;
            case 81:
                SetRun(false);
                break;
            case 83:
                m_creature->SummonCreature(MOB_ENTRY_CHURCH_PROTECTOR, 2627.33f, 646.82f, 56.03f, 4.28f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,5000);
                m_creature->SummonCreature(MOB_ENTRY_CHURCH_LOOKOUT, 2624.14f, 648.03f, 56.03f, 4.50f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,5000);
                m_creature->SummonCreature(MOB_ENTRY_CHURCH_GUARDSMAN, 2625.32f, 649.60f, 56.03f, 4.38f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,5000);
                m_creature->SummonCreature(MOB_ENTRY_CHURCH_GUARDSMAN, 2627.22f, 649.00f, 56.03f, 4.34f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,5000);
                break;
            case 84:
                DoScriptText(SAY_TH_CHURCH_END, m_creature);
                SetRun();
                break;
            case 91:
                SetRun(false);
                break;
            case 93:
                m_creature->SummonCreature(MOB_ENTRY_INN_PROTECTOR, 2652.71f, 660.31f, 61.93f, 1.67f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_INN_LOOKOUT, 2648.96f, 662.59f, 61.93f, 0.79f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_INN_GUARDSMAN, 2657.36f, 662.34f, 61.93f, 2.68f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                m_creature->SummonCreature(MOB_ENTRY_INN_GUARDSMAN, 2656.39f, 659.77f, 61.93f, 2.61f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000);
                break;
            case 94:
                if (uint64 TarethaGUID = m_pInstance->GetData64(DATA_TARETHA))
                {
                    if (Unit* Taretha = Unit::GetUnit((*m_creature), TarethaGUID))
                        DoScriptText(SAY_TA_ESCAPED, Taretha, m_creature);
                }
                break;
            case 95:
                DoScriptText(SAY_TH_MEET_TARETHA, m_creature);
                m_pInstance->SetData(TYPE_THRALL_PART3,DONE);
                SetEscortPaused(true);
                break;
            case 96:
                DoScriptText(SAY_TH_EPOCH_WONDER, m_creature);
                break;
            case 97:
                DoScriptText(SAY_TH_EPOCH_KILL_TARETHA, m_creature);
                SetRun();
                break;
            case 98:
                //trigger epoch Yell("Thrall! Come outside and face your fate! ....")
                //from here, thrall should not never be allowed to move to point 106 which he currently does.
                break;
            case 106:
                //trigger taretha to run down outside
                if (Creature* pTaretha = m_pInstance->instance->GetCreature(m_pInstance->GetData64(DATA_TARETHA)))
                {
                    if (Player* pPlayer = GetPlayerForEscort())
                    {
                        if (npc_tarethaAI* pTarethaAI = dynamic_cast<npc_tarethaAI*>(pTaretha->AI()))
                            pTarethaAI->Start(true, pPlayer->GetGUID());
                    }
                }

                //kill credit creature for quest
                Map *map = m_creature->GetMap();
                Map::PlayerList const& players = map->GetPlayers();
                if (!players.isEmpty() && map->IsDungeon())
                {
                    for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    {
                        if (Player* pPlayer = itr->getSource())
                            pPlayer->KilledMonsterCredit(NPC_THRALL_QUEST_TRIGGER, m_creature->GetGUID());
                    }
                }

                //alot will happen here, thrall and taretha talk, erozion appear at spot to explain
                m_creature->SummonCreature(EROZION_ENTRY, 2646.47f, 680.416f, 55.38f, 4.16f, TEMPSUMMON_TIMED_DESPAWN,120000);
                break;
        }
    }

    void Reset()
    {
        LowHp = false;

        if (HadMount)
            DoMount();

        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            DoUnmount();
            HadMount = false;
            SetEquipmentSlots(true);
            m_creature->SetDisplayId(THRALL_MODEL_UNEQUIPPED);
        }

        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            switch(urand(0, 2))
            {
                case 0: DoScriptText(SAY_TH_LEAVE_COMBAT1, m_creature); break;
                case 1: DoScriptText(SAY_TH_LEAVE_COMBAT2, m_creature); break;
                case 2: DoScriptText(SAY_TH_LEAVE_COMBAT3, m_creature); break;
            }
        }
    }

    void StartWP()
    {
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        SetEscortPaused(false);
    }

    void DoMount()
    {
        m_creature->Mount(SKARLOC_MOUNT_MODEL);
        m_creature->SetSpeedRate(MOVE_RUN, SPEED_MOUNT);
    }

    void DoUnmount()
    {
        m_creature->Unmount();
        m_creature->SetSpeedRate(MOVE_RUN, SPEED_RUN);
    }

    void Aggro(Unit* who)
    {
        switch(urand(0, 3))
        {
            case 0: DoScriptText(SAY_TH_RANDOM_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_TH_RANDOM_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_TH_RANDOM_AGGRO3, m_creature); break;
            case 3: DoScriptText(SAY_TH_RANDOM_AGGRO4, m_creature); break;
        }

        if (m_creature->IsMounted())
        {
            DoUnmount();
            HadMount = true;
        }
    }

    void JustSummoned(Creature* summoned)
    {
        switch(summoned->GetEntry())
        {
            //TODO: make Scarloc start into event instead, and not start attack directly
            case MOB_ENTRY_BARN_GUARDSMAN:
            case MOB_ENTRY_BARN_PROTECTOR:
            case MOB_ENTRY_BARN_LOOKOUT:
            case SKARLOC_MOUNT:
            case EROZION_ENTRY:
                break;
            default:
                summoned->AI()->AttackStart(m_creature);
                break;
        }
    }

    void KilledUnit(Unit *victim)
    {
        switch(urand(0, 2))
        {
            case 0: DoScriptText(SAY_TH_RANDOM_KILL1, m_creature); break;
            case 1: DoScriptText(SAY_TH_RANDOM_KILL2, m_creature); break;
            case 2: DoScriptText(SAY_TH_RANDOM_KILL3, m_creature); break;
        }
    }

    void JustDied(Unit *slayer)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_THRALL_EVENT,FAIL);

        // Don't do a yell if he kills self (if player goes too far or at the end).
        if (slayer == m_creature)
            return;

        DoScriptText(urand(0, 1) ? SAY_TH_RANDOM_DIE1 : SAY_TH_RANDOM_DIE2, m_creature);
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        //TODO: add his abilities'n-crap here

        if (!LowHp && m_creature->GetHealthPercent() < 20.0f)
        {
            DoScriptText(urand(0, 1) ? SAY_TH_RANDOM_LOW_HP1 : SAY_TH_RANDOM_LOW_HP2, m_creature);
            LowHp = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_thrall_old_hillsbrad(Creature* pCreature)
{
    return new npc_thrall_old_hillsbradAI(pCreature);
}

bool GossipHello_npc_thrall_old_hillsbrad(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
    {
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());
        pPlayer->SendPreparedQuest(pCreature->GetGUID());
    }

    ScriptedInstance* pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());

    if (pInstance)
    {
        if (pInstance->GetData(TYPE_BARREL_DIVERSION) == DONE && !pInstance->GetData(TYPE_THRALL_EVENT))
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "[PH] Start walking.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_START, pCreature->GetGUID());
        }

        if (pInstance->GetData(TYPE_THRALL_PART1) == DONE && !pInstance->GetData(TYPE_THRALL_PART2))
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SKARLOC1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_SKARLOC1, pCreature->GetGUID());
        }

        if (pInstance->GetData(TYPE_THRALL_PART2) == DONE && !pInstance->GetData(TYPE_THRALL_PART3))
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TARREN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_TARREN, pCreature->GetGUID());
        }
    }
    return true;
}

bool GossipSelect_npc_thrall_old_hillsbrad(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    ScriptedInstance* pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());

    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->CLOSE_GOSSIP_MENU();
            pInstance->SetData(TYPE_THRALL_EVENT,IN_PROGRESS);
            pInstance->SetData(TYPE_THRALL_PART1,IN_PROGRESS);

            DoScriptText(SAY_TH_START_EVENT_PART1, pCreature);

            if (npc_thrall_old_hillsbradAI* pThrallAI = dynamic_cast<npc_thrall_old_hillsbradAI*>(pCreature->AI()))
                pThrallAI->Start(true, pPlayer->GetGUID());

            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SKARLOC2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+20);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_SKARLOC2, pCreature->GetGUID());
            break;

        case GOSSIP_ACTION_INFO_DEF+20:
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_SKARLOC3, pCreature->GetGUID());
            pCreature->SummonCreature(SKARLOC_MOUNT, 2038.81f, 270.26f, 63.20f, 5.41f, TEMPSUMMON_TIMED_DESPAWN,12000);
            pInstance->SetData(TYPE_THRALL_PART2,IN_PROGRESS);

            DoScriptText(SAY_TH_START_EVENT_PART2, pCreature);

            if (npc_thrall_old_hillsbradAI* pThrallAI = dynamic_cast<npc_thrall_old_hillsbradAI*>(pCreature->AI()))
                pThrallAI->StartWP();
            break;

        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->CLOSE_GOSSIP_MENU();
            pInstance->SetData(TYPE_THRALL_PART3,IN_PROGRESS);
            if (npc_thrall_old_hillsbradAI* pThrallAI = dynamic_cast<npc_thrall_old_hillsbradAI*>(pCreature->AI()))
                pThrallAI->StartWP();
            break;
    }
    return true;
}

/*######
## npc_taretha
######*/

#define GOSSIP_ID_EPOCH1        9610                        //Thank you for helping Thrall escape, friends. Now I only hope
#define GOSSIP_ITEM_EPOCH1      "Strange wizard?"
#define GOSSIP_ID_EPOCH2        9613                        //Yes, friends. This man was no wizard of
#define GOSSIP_ITEM_EPOCH2      "We'll get you out. Taretha. Don't worry. I doubt the wizard would wander too far away."

npc_tarethaAI::npc_tarethaAI(Creature* pCreature) : npc_escortAI(pCreature)
{
    m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
    Reset();
}

void npc_tarethaAI::WaypointReached(uint32 i)
{
    switch(i)
    {
        case 6:
            DoScriptText(SAY_TA_FREE, m_creature);
            break;
        case 7:
            m_creature->HandleEmote(EMOTE_ONESHOT_CHEER);
            break;
    }
}

CreatureAI* GetAI_npc_taretha(Creature* pCreature)
{
    return new npc_tarethaAI(pCreature);
}

bool GossipHello_npc_taretha(Player* pPlayer, Creature* pCreature)
{
    ScriptedInstance* pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());

    if (pInstance && pInstance->GetData(TYPE_THRALL_PART3) == DONE && pInstance->GetData(TYPE_THRALL_PART4) == NOT_STARTED)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_EPOCH1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_EPOCH1, pCreature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_taretha(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    ScriptedInstance* pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());

    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_EPOCH2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_ID_EPOCH2, pCreature->GetGUID());
    }
    if (uiAction == GOSSIP_ACTION_INFO_DEF+2)
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        if (pInstance->GetData(TYPE_THRALL_EVENT) == IN_PROGRESS)
        {
            pInstance->SetData(TYPE_THRALL_PART4,IN_PROGRESS);
            pCreature->SummonCreature(ENTRY_EPOCH, 2639.13f, 698.55f, 65.43f, 4.59f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,120000);

            if (uint64 ThrallGUID = pInstance->GetData64(DATA_THRALL))
            {
                Creature* Thrall = ((Creature*)Unit::GetUnit((*pCreature), ThrallGUID));
                if (Thrall)
                    ((npc_thrall_old_hillsbradAI*)Thrall->AI())->StartWP();
            }
        }
    }
    return true;
}

/*######
## AddSC
######*/

void AddSC_old_hillsbrad()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_brazen";
    newscript->pGossipHello =   &GossipHello_npc_brazen;
    newscript->pGossipSelect =  &GossipSelect_npc_brazen;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_erozion";
    newscript->pGossipHello =   &GossipHello_npc_erozion;
    newscript->pGossipSelect =  &GossipSelect_npc_erozion;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_thrall_old_hillsbrad";
    newscript->pGossipHello =  &GossipHello_npc_thrall_old_hillsbrad;
    newscript->pGossipSelect = &GossipSelect_npc_thrall_old_hillsbrad;
    newscript->GetAI = &GetAI_npc_thrall_old_hillsbrad;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_taretha";
    newscript->pGossipHello =   &GossipHello_npc_taretha;
    newscript->pGossipSelect =  &GossipSelect_npc_taretha;
    newscript->GetAI = &GetAI_npc_taretha;
    newscript->RegisterSelf();
}
