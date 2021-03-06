/***
 * Demonstrike Core
 */

#include "StdAfx.h"

const char * AreaTriggerFailureMessages[] = {
    "-",
    "This instance is currently not available",                                         //AREA_TRIGGER_FAILURE_UNAVAILABLE
    "You must have the \"Burning Crusade\" expansion to access this content.",          //AREA_TRIGGER_FAILURE_NO_BC
    "Heroic mode currently not available for this instance.",                           //AREA_TRIGGER_FAILURE_NO_HEROIC
    "You must be in a raid group to pass through here.",                                //AREA_TRIGGER_FAILURE_NO_RAID
    "You must complete the quest \"%s\" to pass through here.",                         //AREA_TRIGGER_FAILURE_NO_ATTUNE_Q
    "You must have item \"%s\" to pass through here.",                                  //AREA_TRIGGER_FAILURE_NO_ATTUNE_I
    "You must have reached level %u before you can pass through here.",                 //AREA_TRIGGER_FAILURE_LEVEL
    "You must be in a party to pass through here.",                                     //AREA_TRIGGER_FAILURE_NO_GROUP
    "You do not have a required key(s) \"%s\" to pass through here.",                   //AREA_TRIGGER_FAILURE_NO_KEY
    "You must have reached level %u before you can enter heroic mode.",                 //AREA_TRIGGER_FAILURE_LEVEL_HEROIC
    "Don\'t have any idea why you can\'t pass through here.",                           //AREA_TRIGGER_FAILURE_NO_CHECK
    "You must have the \"Wrath of the Lich King\" expansion to access this content.",   //AREA_TRIGGER_FAILURE_NO_WOTLK
    "You are in queue for this raid group.",                                            //AREA_TRIGGER_FAILURE_IN_QUEUE
    "Another group is already active inside.",                                          //AREA_TRIGGER_FAILURE_WRONG_GROUP
};

void WorldSession::HandleAreaTriggerOpcode(WorldPacket & recv_data)
{
    CHECK_INWORLD_RETURN();
    CHECK_PACKET_SIZE(recv_data, 4);
    uint32 id;
    recv_data >> id;
    _HandleAreaTriggerOpcode(id);
}

void WorldSession::_HandleAreaTriggerOpcode(uint32 id)
{
    sLog.Debug( "WorldSession","HandleAreaTriggerOpcode: %u", id);

    // Are we REALLY here?
    CHECK_INWORLD_RETURN();

    // Search quest log, find any exploration quests
    sQuestMgr.OnPlayerExploreArea(GetPlayer(),id);

    AreaTrigger* pAreaTrigger = AreaTriggerStorage.LookupEntry( id );

    sHookInterface.OnAreaTrigger(_player, id);
    CALL_INSTANCE_SCRIPT_EVENT( _player->GetMapMgr(), OnAreaTrigger )( _player, id );

    // if in BG handle is triggers
    if( _player->m_bg )
    {
        _player->m_bg->HookOnAreaTrigger(_player, id);
        return;
    }

    // Hook for Scripted Areatriggers
    _player->GetMapMgr()->HookOnAreaTrigger(_player, id);

    if( _player->GetSession()->CanUseCommand('z') )
    {
        if( pAreaTrigger != NULL )
            sChatHandler.BlueSystemMessage( this, "[%sSystem%s] |rEntered areatrigger: %s%u (%s).", MSG_COLOR_WHITE, MSG_COLOR_LIGHTBLUE, MSG_COLOR_SUBWHITE, id, pAreaTrigger->Name );
        else
            sChatHandler.BlueSystemMessage( this, "[%sSystem%s] |rEntered areatrigger: %s%u", MSG_COLOR_WHITE, MSG_COLOR_LIGHTBLUE, MSG_COLOR_SUBWHITE, id);
    }

    if( pAreaTrigger == NULL )
    {
        sLog.outDebug("Missing AreaTrigger: %u", id);
        return;
    }
    if(pAreaTrigger->RequiredTeam != -1)
        if(pAreaTrigger->RequiredTeam != _player->GetTeam())
            return;

    _player->SetLastAreaTrigger(pAreaTrigger);
    switch(pAreaTrigger->Type)
    {
    case ATTYPE_INSTANCE:
        {
            if(_player->GetPlayerStatus() != TRANSFER_PENDING) //only ports if player is out of pendings
            {
                MapInfo * pMi = LimitedMapInfoStorage.LookupEntry(pAreaTrigger->Mapid);
                MapEntry* map = dbcMap.LookupEntry(pAreaTrigger->Mapid);
                if(!pMi)
                    return;

                //do we meet the map requirements?
                uint8 reason = CheckTeleportPrerequisites(pAreaTrigger, this, _player, pAreaTrigger->Mapid);
                if(reason != AREA_TRIGGER_FAILURE_OK)
                {
                    const char * pReason = AreaTriggerFailureMessages[reason];
                    char msg[200];
                    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 50);
                    data << uint32(0);

                    switch (reason)
                    {
                    case AREA_TRIGGER_FAILURE_LEVEL:
                        {
                            snprintf(msg,200,pReason,pAreaTrigger->required_level);
                            data << msg;
                        }break;
                    case AREA_TRIGGER_FAILURE_NO_ATTUNE_I:
                        {
                            ItemPrototype * pItem = ItemPrototypeStorage.LookupEntry(pMi->required_item);
                            snprintf(msg, 200, pReason, pItem ? pItem->Name1 : "UNKNOWN");
                            data << msg;
                        }break;
                    case AREA_TRIGGER_FAILURE_NO_ATTUNE_Q:
                        {
                            Quest * pQuest = sQuestMgr.GetQuestPointer(pMi->required_quest);
                            snprintf(msg, 200, pReason, pQuest ? pQuest->qst_title : "UNKNOWN");

                            data << msg;
                        }break;
                    case AREA_TRIGGER_FAILURE_NO_KEY:
                        {
                            string temp_msg[2];
                            string tmp_msg;
                            for(uint32 i = 0; i < 2; i++)
                            {
                                if (pMi->heroic_key[i] && _player->GetItemInterface()->GetItemCount(pMi->heroic_key[i], false)==0)
                                {
                                    ItemPrototype * pKey = ItemPrototypeStorage.LookupEntry(pMi->heroic_key[i]);
                                    if(pKey)
                                        temp_msg[i] += pKey->Name1;
                                    else
                                        temp_msg[i] += "UNKNOWN";
                                }
                            }
                            tmp_msg += temp_msg[0];
                            if(temp_msg[0].size() && temp_msg[1].size())
                                tmp_msg += "\" and \"";
                            tmp_msg += temp_msg[1];

                            snprintf(msg, 200, pReason, tmp_msg.c_str());
                            data << msg;
                        }break;
                    case AREA_TRIGGER_FAILURE_LEVEL_HEROIC:
                        {
                            snprintf(msg, 200, pReason, pMi->HasFlag(WMI_INSTANCE_XPACK_02) ? 80 : 70);
                            data << msg;
                        }break;
                    default:
                        {
                            data << pReason;
                        }break;
                    }

                    data << uint8(0);
                    SendPacket(&data);
                    return;
                }

                if( _player->IsMounted())
                    TO_UNIT(_player)->Dismount();

                uint32 InstanceID = 0;

                // Try to find a saved instance and
                // do not handle Hyjal Inn (trigger 4319), since we need a unique mapid when generating our instance_id.
                if( id != 4319 && pMi && ( map->IsRaid() || _player->iRaidType >= MODE_25PLAYER_NORMAL && pMi->type == INSTANCE_MULTIMODE ) )
                {
                    //Do we have a saved instance we should use?
                    Instance * in = sInstanceMgr.GetSavedInstance( pMi->mapid,_player->GetLowGUID(), _player->iRaidType );
                    if( in != NULL  && in->m_instanceId )
                    {
                        //If we are the first to enter this instance, also set our current group id.
                        if( in->m_mapMgr == NULL || (!in->m_mapMgr->HasPlayers() && _player->GetGroupID() != in->m_creatorGroup))
                            in->m_creatorGroup =_player->GetGroupID();
                        InstanceID = in->m_instanceId;
                    }
                }

                // teleport to our instance
                _player->SafeTeleport(pAreaTrigger->Mapid, InstanceID, LocationVector(pAreaTrigger->x, pAreaTrigger->y, pAreaTrigger->z, pAreaTrigger->o));
            }
        }break;
    case ATTYPE_QUESTTRIGGER:
        {

        }break;
    case ATTYPE_INN:
        {

        }break;
    case ATTYPE_TELEPORT:
        {
            if( _player->GetPlayerStatus() != TRANSFER_PENDING) //only ports if player is out of pendings
            {
                if( _player->IsMounted() )
                    TO_UNIT(_player)->Dismount();

                _player->SafeTeleport(pAreaTrigger->Mapid, 0, LocationVector(pAreaTrigger->x, pAreaTrigger->y, pAreaTrigger->z, pAreaTrigger->o));
            }
        }break;
    default:
        break;
    }
}
