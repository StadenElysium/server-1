/* 
 * Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2018 TrinityCore <http://www.trinitycore.org/>
 *
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
SDName: Elwynn_Forest
SD%Complete: 50
SDComment: Quest support: 1786
SDCategory: Elwynn Forest
EndScriptData */

/* ContentData
npc_henze_faulk
EndContentData */

#include "scriptPCH.h"
#include "CreatureAIImpl.h"


/*######
## npc_henze_faulk
######*/

#define SAY_HEAL    -1000187

struct npc_henze_faulkAI : public ScriptedAI
{
    uint32 lifeTimer;
    bool spellHit;

    npc_henze_faulkAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    void Reset()
    {
        lifeTimer = 120000;
        m_creature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);   // lay down
        spellHit = false;
    }

    void MoveInLineOfSight(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if (m_creature->IsStandState())
        {
            if (lifeTimer < diff)
                m_creature->AI()->EnterEvadeMode();
            else
                lifeTimer -= diff;
        }
    }

    void SpellHit(Unit *Hitter, const SpellEntry *Spellkind)
    {
        if (Spellkind->Id == 8593 && !spellHit)
        {
            DoCastSpellIfCan(m_creature, 32343);
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
            m_creature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
            //m_creature->RemoveAllAuras();
            DoScriptText(SAY_HEAL, m_creature, Hitter);
            spellHit = true;
        }
    }

};

CreatureAI* GetAI_npc_henze_faulk(Creature* pCreature)
{
    return new npc_henze_faulkAI(pCreature);
}

/*######
## go_marshal_haggards_chest
######*/

enum
{
    //guid 12609 entry 177673 Serpant statue
    NPC_SPIRIT                  = 2044,
    QUEST_STALVANS_LEGEND       = 75

};

struct go_marshal_haggards_chestAI: public GameObjectAI
{
    go_marshal_haggards_chestAI(GameObject* pGo) : GameObjectAI(pGo)
    {
        timer = 0;
        state = 0;
        guid_spirit = 0;
    }
    uint64 guid_spirit;
    uint32 timer;
    bool state; //0 = usual, can launch. //1 = in use, cannot launch

    void UpdateAI(const uint32 uiDiff)
    {
        if (state)
        {
            if (timer < uiDiff)
                state = 0;
            else
                timer -= uiDiff;
        }
    }

    bool CheckCanStartEvent()
    {
        if (!state && !me->GetMap()->GetCreature(guid_spirit))
            return true;
        return false;
    }

    void SetInUse(Creature* spirit)
    {
        guid_spirit = spirit->GetGUID();
        state = 1;
        timer = 60000;
    }
};

GameObjectAI* GetAIgo_marshal_haggards_chest(GameObject *pGo)
{
    return new go_marshal_haggards_chestAI(pGo);
}

bool GOHello_go_marshal_haggards_chest(Player* pPlayer, GameObject* pGo)
{
    if (go_marshal_haggards_chestAI* pMarkAI = dynamic_cast<go_marshal_haggards_chestAI*>(pGo->AI()))
    {
        if (pMarkAI->CheckCanStartEvent())
        {
            if (pPlayer->GetQuestStatus(QUEST_STALVANS_LEGEND) == QUEST_STATUS_INCOMPLETE)
            {
                if (Creature* spirit = pGo->SummonCreature(NPC_SPIRIT, -9552.67f, -1431.93f, 62.3f, 5.03f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000))
                {
                    pMarkAI->SetInUse(spirit);
                    spirit->AI()->AttackStart(pPlayer);
                }
            }
        }
    }
    return true;
}

/*######
## npc_cameron
######*/

enum COG_Waypoints
{
    STORMWIND_WAYPOINT = 57,
    GOLDSHIRE_WAYPOINT = 89,
    WOODS_WAYPOINT = 111,
    HOUSE_WAYPOINT = 146
};

enum COG_Sounds
{
    BANSHEE_DEATH = 1171,
    BANSHEEPREAGGRO = 1172,
    CTHUN_YOU_WILL_DIE = 8585,
    CTHUN_DEATH_IS_CLOSE = 8580,
    HUMAN_FEMALE_EMOTE_CRY = 6916,
    GHOSTDEATH = 3416
};

enum COG_Creatures
{
    NPC_DANA = 804,
    NPC_CAMERON = 805,
    NPC_JOHN = 806,
    NPC_LISA = 807,
    NPC_AARON = 810,
    NPC_JOSE = 811
};

enum COG_Events
{
    EVENT_PLAY_SOUNDS = 1,
    EVENT_BEGIN_EVENT = 2
};

enum COG_GameEvent
{
    GAME_EVENT_CHILDEREN_OF_GOLDSHIRE = 87
};

struct COG_Positions
{
    float x;
    float y;
    float z;
    float o;
};

struct npc_cameron : public ScriptedAI
{
    npc_cameron(Creature* creature) : ScriptedAI(creature), _started(false)
    {
        //me->GetMotionMaster()->MovementExpired();
        //_events.ScheduleEvent(EVENT_BEGIN_EVENT, Seconds(2));
    }

    void Reset()
    {
    }

    static uint32 SoundPicker()
    {
        return RAND(
            BANSHEE_DEATH,
            BANSHEEPREAGGRO,
            CTHUN_YOU_WILL_DIE,
            CTHUN_DEATH_IS_CLOSE,
            HUMAN_FEMALE_EMOTE_CRY,
            GHOSTDEATH
        );
    }

    void MoveTheChildren()
    {
        std::vector<COG_Positions> MovePosPositions =
        {
            { -9373.521f, -67.71767f, 69.201965f, 1.117011f },
            { -9374.94f, -62.51654f, 69.201965f, 5.201081f },
            { -9371.013f, -71.20811f, 69.201965f, 1.937315f },
            { -9368.419f, -66.47543f, 69.201965f, 3.141593f },
            { -9372.376f, -65.49946f, 69.201965f, 4.206244f },
            { -9377.477f, -67.8297f, 69.201965f, 0.296706f }
        };

        std::random_shuffle(MovePosPositions.begin(), MovePosPositions.end());

        // first we break formation because children will need to move on their own now
        for (auto guid : _childrenGUIDs)
            if (CreatureGroup* myGroup = me->GetCreatureGroup())
                myGroup->RemoveMember(guid);

        // Move each child to an random position
        for (uint32 i = 0; i < _childrenGUIDs.size(); ++i)
        {
            if (Creature* children = m_creature->GetMap()->GetCreature(_childrenGUIDs[i]))
            {
                children->SetWalk(true);
                children->GetMotionMaster()->MovePoint(0, MovePosPositions[i].x, MovePosPositions[i].y, MovePosPositions[i].z, MOVE_PATHFINDING, 0.f, MovePosPositions[i].o);
            }
        }
        me->SetWalk(true);
        me->GetMotionMaster()->MovePoint(0, MovePosPositions.back().x, MovePosPositions.back().y, MovePosPositions.back().z, MOVE_PATHFINDING, 0.f, MovePosPositions.back().o);
    }

    void WaypointReached(uint32 PointId)
    {
        switch (PointId)
        {
            case STORMWIND_WAYPOINT:
            case GOLDSHIRE_WAYPOINT:
            case WOODS_WAYPOINT:
            {
                me->GetMotionMaster()->MoveRandom();
                break;
            }
            case HOUSE_WAYPOINT:
            {
                // Move childeren at last point 
                MoveTheChildren();

                // After 30 seconds a random sound should play
                _events.ScheduleEvent(EVENT_PLAY_SOUNDS, Seconds(30));
                break;
            }
        }
    }

    void UpdateAI(uint32 diff)
    {
        if (_started == false)
        {
            _events.ScheduleEvent(EVENT_BEGIN_EVENT, Seconds(2));
            _started = true;
        }

        /*if (sGameEventMgr.IsActiveEvent(GAME_EVENT_CHILDEREN_OF_GOLDSHIRE) && _started == false)
        {
            _started = true;
            _events.ScheduleEvent(EVENT_BEGIN_EVENT, Seconds(2));
        }

        if (!sGameEventMgr.IsActiveEvent(GAME_EVENT_CHILDEREN_OF_GOLDSHIRE) && _started == true)
        {
            _started = false;
            _events.Reset();
        }*/

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_PLAY_SOUNDS:
                me->PlayDistanceSound(SoundPicker());
                break;
            case EVENT_BEGIN_EVENT:
            {
                _childrenGUIDs.clear();

                // Get all childeren's guid's.
                if (Creature* dana = me->FindNearestCreature(NPC_DANA, 25.0f))
                    _childrenGUIDs.push_back(dana->GetGUID());

                if (Creature* john = me->FindNearestCreature(NPC_JOHN, 25.0f))
                    _childrenGUIDs.push_back(john->GetGUID());

                if (Creature* lisa = me->FindNearestCreature(NPC_LISA, 25.0f))
                    _childrenGUIDs.push_back(lisa->GetGUID());

                if (Creature* aaron = me->FindNearestCreature(NPC_AARON, 25.0f))
                    _childrenGUIDs.push_back(aaron->GetGUID());

                if (Creature* jose = me->FindNearestCreature(NPC_JOSE, 25.0f))
                    _childrenGUIDs.push_back(jose->GetGUID());

                // If Formation was disbanded, respawn.
                if (!me->GetCreatureGroup()->IsFormation())
                    if (CreatureGroup* group = me->GetCreatureGroup())
                        group->OnRespawn(me);

                // Start movement
                me->SetWalk(false);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->MoveWaypoint(false);

                break;
            }
            default:
                break;
            }
        }
    }

private:
    EventMap _events;
    bool _started;
    std::vector<ObjectGuid> _childrenGUIDs;
};

CreatureAI* GetAI_npc_cameron(Creature* Creature)
{
    return new npc_cameron(Creature);
}

void AddSC_elwynn_forest()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_henze_faulk";
    newscript->GetAI = &GetAI_npc_henze_faulk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_marshal_haggards_chest";
    newscript->GOGetAI = &GetAIgo_marshal_haggards_chest;
    newscript->pGOHello = &GOHello_go_marshal_haggards_chest;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_cameron";
    newscript->GetAI = &GetAI_npc_cameron;
    newscript->RegisterSelf();
}
