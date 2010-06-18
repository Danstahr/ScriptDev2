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
SDName: Swamp_of_Sorrows
SD%Complete: 10
SDComment: Quest support: 1393
SDCategory: Swamp of Sorrows
EndScriptData */

/* ContentData
npc_galen
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

enum
{
    QUEST_GALENS_ESCAPE         = 1393,

    GO_GALENS_CAGE              = 37118,

    SAY_START                   = -1002070,
    SAY_AGGRO1                  = -1002071,
    SAY_AGGRO2                  = -1002072,
    SAY_AGGRO3                  = -1002073,
    SAY_AGGRO4                  = -1002074,
    SAY_END                     = -1002075,
    SAY_END2                    = -1002076
};

struct MANGOS_DLL_DECL npc_galenAI : public npc_escortAI
{
    npc_galenAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        ResetTimers();
    }

    uint16 RPTimer;

    void ResetTimers()
    {
        RPTimer=0;
    }

    void Reset()
    {
    }

    void Aggro(Unit* who)
    {
        switch(rand()%3)
        {
        case 0:
            DoScriptText(SAY_AGGRO1,m_creature,who);
            break;
        case 1:
            DoScriptText(SAY_AGGRO3, m_creature,who);
            break;
        case 2:
            DoScriptText(SAY_AGGRO4, m_creature,who);
            break;
        }
    }

    void WaypointReached(uint32 id)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        GameObject* Cage;
        
        switch (id)
        {
        case 0:
            Cage = GetClosestGameObjectWithEntry(m_creature, GO_GALENS_CAGE, 50.0f);
            if (Cage)
                Cage->SetGoState(GO_STATE_READY);
            m_creature->setFaction(250);
            break;
        case 22:
            DoScriptText(SAY_END,m_creature,pPlayer);
            pPlayer->GroupEventHappens(QUEST_GALENS_ESCAPE,m_creature);
            m_creature->setFaction(35);
            RPTimer=5000;
            break;
        case 23:
            m_creature->SetSpeedRate(MOVE_WALK,1.05f,true);
            break;
        }

    }

    void UpdateEscortAI(const uint32 diff)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;
        
        if (RPTimer)
        {
            if (RPTimer<=diff)
            {
                DoScriptText(SAY_END2,m_creature,pPlayer);
                m_creature->SetSpeedRate(MOVE_WALK,3.0f,true);
                RPTimer=0;
            }
            else RPTimer-=diff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;       

        DoMeleeAttackIfReady();        
    }

};

bool QuestAccept_npc_galen(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_GALENS_ESCAPE)
    {
        if (npc_galenAI* pEscortAI = dynamic_cast<npc_galenAI*>(pCreature->AI()))
        {
            GameObject* Cage = GetClosestGameObjectWithEntry(pCreature, GO_GALENS_CAGE, 50.0f);
            if (Cage)
                Cage->SetGoState(GO_STATE_ACTIVE);
            DoScriptText(SAY_START,pCreature);
            if (pPlayer->isGameMaster())
                pEscortAI->Start(true, true, pPlayer->GetGUID(), pQuest);
            else
                pEscortAI->Start(true, false, pPlayer->GetGUID(), pQuest);
        }
    }
    return true;
}

CreatureAI* GetAI_npc_galen(Creature* pCreature)
{
    return new npc_galenAI(pCreature);
}

void AddSC_swamp_of_sorrows()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_galen";
    newscript->GetAI = &GetAI_npc_galen;
    newscript->pQuestAccept = &QuestAccept_npc_galen;
    newscript->RegisterSelf();
    
}