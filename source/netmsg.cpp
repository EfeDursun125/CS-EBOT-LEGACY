//
// Copyright (c) 2003-2009, by Yet Another POD-Bot Development Team.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// $Id:$
//

#include <core.h>

#define PTR_TO_BYTE(in) *reinterpret_cast<uint8_t*>(in)
#define PTR_TO_FLT(in) *reinterpret_cast<float*>(in)
#define PTR_TO_INT(in) *reinterpret_cast<int*>(in)
#define PTR_TO_STR(in) reinterpret_cast<char*>(in)

NetworkMsg::NetworkMsg(void)
{
    m_message = NETMSG_UNDEFINED;
    m_state = 0;
    m_bot = nullptr;

    for (auto& message : m_registerdMessages)
        message = NETMSG_UNDEFINED;
}

void NetworkMsg::HandleMessageIfRequired(const int messageType, const int requiredType)
{
    if (messageType == m_registerdMessages[requiredType])
        SetMessage(requiredType);
}

void NetworkMsg::Execute(void* p)
{
    if (m_message == NETMSG_UNDEFINED)
        return; // no message or not for bot, return

    // some needed variables
    static uint8_t r, g, b;
    static uint8_t enabled;

    static int index, numPlayers;
    static int state, id;

    static WeaponProperty weaponProp;

    // now starts of netmessage execution
    switch (m_message)
    {
    case NETMSG_VGUI:
    {
        // this message is sent when a VGUI menu is displayed
        if (m_state == 0 && m_bot != nullptr)
        {
            switch (PTR_TO_INT(p))
            {
            case GMENU_TEAM:
            {
                m_bot->m_startAction = CMENU_TEAM;
                break;
            }
            case GMENU_TERRORIST:
            case GMENU_COUNTER:
            {
                m_bot->m_startAction = CMENU_CLASS;
                break;
            }
            }
        }
        break;
    }
    case NETMSG_SHOWMENU:
    {
        // this message is sent when a text menu is displayed
        if (m_state < 3) // ignore first 3 fields of message
            break;

        if (m_bot != nullptr)
        {
            /*const auto search = VGUIMAP.find(PTR_TO_STR(p));
            if (search != VGUIMAP.end())
                m_bot->m_startAction = search->second;*/

            const char* x = PTR_TO_STR(p);
            if (cstrncmp(x, "#Team_Select", 13) == 0) // team select menu?
                m_bot->m_startAction = CMENU_TEAM;
            else if (cstrncmp(x, "#Team_Select_Spect", 19) == 0) // team select menu?
                m_bot->m_startAction = CMENU_TEAM;
            else if (cstrncmp(x, "#IG_Team_Select_Spect", 22) == 0) // team select menu?
                m_bot->m_startAction = CMENU_TEAM;
            else if (cstrncmp(x, "#IG_Team_Select", 16) == 0) // team select menu?
                m_bot->m_startAction = CMENU_TEAM;
            else if (cstrncmp(x, "#IG_VIP_Team_Select", 20) == 0) // team select menu?
                m_bot->m_startAction = CMENU_TEAM;
            else if (cstrncmp(x, "#IG_VIP_Team_Select_Spect", 26) == 0) // team select menu?
                m_bot->m_startAction = CMENU_TEAM;
            else if (cstrncmp(x, "#Terrorist_Select", 18) == 0) // T model select?
                m_bot->m_startAction = CMENU_CLASS;
            else if (cstrncmp(x, "#CT_Select", 11) == 0) // CT model select menu?
                m_bot->m_startAction = CMENU_CLASS;
        }

        break;
    }
    case NETMSG_WLIST:
    {
        // this message is sent when a client joins the game. All of the weapons are sent with the weapon ID and information about what ammo is used
        switch (m_state)
        {
        case 0:
        {
            cstrncpy(weaponProp.className, PTR_TO_STR(p), sizeof(weaponProp.className));
            break;
        }
        case 1:
        {
            weaponProp.ammo1 = PTR_TO_INT(p); // ammo index 1
            break;
        }
        case 2:
        {
            weaponProp.ammo1Max = PTR_TO_INT(p); // max ammo 1
            break;
        }
        case 5:
        {
            weaponProp.slotID = PTR_TO_INT(p); // slot for this weapon
            break;
        }
        case 6:
        {
            weaponProp.position = PTR_TO_INT(p); // position in slot
            break;
        }
        case 7:
        {
            weaponProp.id = PTR_TO_INT(p); // weapon ID
            break;
        }
        case 8:
        {
            
            if (weaponProp.id > -1 && weaponProp.id < Const_MaxWeapons)
            {
                weaponProp.flags = PTR_TO_INT(p); // flags for weapon (WTF???)
                g_weaponDefs[weaponProp.id] = weaponProp; // store away this weapon with it's ammo information...
            }
                
            break;
        }
        }
        break;
    }
    case NETMSG_CURWEAPON:
    {
        // this message is sent when a weapon is selected (either by the bot chosing a weapon or by the server auto assigning the bot a weapon). In CS it's also called when Ammo is increased/decreased
        switch (m_state)
        {
        case 0:
        {
            state = PTR_TO_INT(p); // state of the current weapon (WTF???)
            break;
        }
        case 1:
        {
            id = PTR_TO_INT(p); // weapon ID of current weapon
            break;
        }
        case 2:
        {
            if (id > -1 && id < Const_MaxWeapons)
            {
                if (state != 0)
                    m_bot->m_currentWeapon = id;
                m_bot->m_ammoInClip[id] = PTR_TO_INT(p);
            }
            break;
        }
        }
        break;
    }
    case NETMSG_AMMOX:
    {
        switch (m_state)
        {
        case 0:
        {
            index = PTR_TO_INT(p); // ammo index (for type of ammo)
            break;
        }
        case 1:
        {
            if (m_bot != nullptr && index > -1 && index < Const_MaxWeapons)
                m_bot->m_ammo[index] = PTR_TO_INT(p); // store it away
            break;
        }
        }
        break;
    }
    case NETMSG_AMMOPICK:
    {
        // this message is sent when the bot picks up some ammo (AmmoX messages are also sent so this message is probably
        // not really necessary except it allows the HUD to draw pictures of ammo that have been picked up
        // The bots don't really need pictures since they don't have any eyes anyway
        switch (m_state)
        {
        case 0:
        {
            index = PTR_TO_INT(p);
            break;
        }
        case 1:
        {
            if (m_bot != nullptr && index > -1 && index < Const_MaxWeapons)
                m_bot->m_ammo[index] = PTR_TO_INT(p);
            break;
        }
        }
        break;
    }
    case NETMSG_DAMAGE:
    {
        if (m_state == 2)
        {
            if (m_bot != nullptr)
                m_bot->TakeDamage(m_bot->pev->dmg_inflictor);
        }
        break;
    }
    case NETMSG_MONEY:
    {
        // this message gets sent when the bots money amount changes
        if (m_state == 0 && m_bot != nullptr)
            m_bot->m_moneyAmount = PTR_TO_INT(p); // amount of money
        break;
    }
    case NETMSG_STATUSICON:
    {
        switch (m_state)
        {
        case 0:
        {
            enabled = PTR_TO_BYTE(p);
            break;
        }
        case 1:
        {
            if (g_gameVersion != HALFLIFE && m_bot != nullptr)
            {
                const char* x = PTR_TO_STR(p);
                if (cstrncmp(x, "defuser", 8) == 0)
                    m_bot->m_hasDefuser = (enabled != 0);
                else if (cstrncmp(x, "buyzone", 8) == 0)
                {
                    m_bot->m_inBuyZone = (enabled != 0);
                    m_bot->EquipInBuyzone(0);
                }
                else if (cstrncmp(x, "vipsafety", 10) == 0)
                    m_bot->m_inVIPZone = (enabled != 0);
                else if (cstrncmp(x, "c4", 3) == 0)
                    m_bot->m_inBombZone = (enabled == 2);
            }
            break;
        }
        }
        break;
    }
    case NETMSG_DEATH: // this message sends on death
    {
        if (m_state == 1)
        {
            Bot* victimer = g_botManager->GetBot(PTR_TO_INT(p));
            if (victimer != nullptr)
            {
                victimer->m_isAlive = false;
                victimer->DeleteSearchNodes();
            }
        }
        break;
    }
    case NETMSG_SCREENFADE: // this message gets sent when the screen fades (flashbang)
    {
        switch (m_state)
        {
        case 3:
        {
            r = PTR_TO_BYTE(p);
            break;
        }
        case 4:
        {
            g = PTR_TO_BYTE(p);
            break;
        }
        case 5:
        {
            b = PTR_TO_BYTE(p);
            break;
        }
        case 6:
        {
            if (m_bot != nullptr)
                m_bot->TakeBlinded(Vector(r, g, b), PTR_TO_BYTE(p));
            break;
        }
        }
        break;
    }
    case NETMSG_HLTV: // round restart in steam cs
    {
        switch (m_state)
        {
        case 0:
        {
            numPlayers = PTR_TO_INT(p);
            break;
        }
        case 1:
        {
            if (numPlayers == 0 && PTR_TO_INT(p) == 0)
                RoundInit();
            break;
        }
        }
        break;
    }
    case NETMSG_TEXTMSG:
    {
        if (m_state == 1)
        {
            const char* x = PTR_TO_STR(p);
            if (cstrncmp(x, "#CTs_Win", 9) == 0 ||
                cstrncmp(x, "#Bomb_Defused", 14) == 0 ||
                cstrncmp(x, "#Terrorists_Win", 16) == 0 ||
                cstrncmp(x, "#Round_Draw", 12) == 0 ||
                cstrncmp(x, "#All_Hostages_Rescued", 22) == 0 ||
                cstrncmp(x, "#Target_Saved", 14) == 0 ||
                cstrncmp(x, "#Hostages_Not_Rescued", 22) == 0 ||
                cstrncmp(x, "#Terrorists_Not_Escaped", 24) == 0 ||
                cstrncmp(x, "#VIP_Not_Escaped", 17) == 0 ||
                cstrncmp(x, "#Escaping_Terrorists_Neutralized", 33) == 0 ||
                cstrncmp(x, "#VIP_Assassinated", 18) == 0 ||
                cstrncmp(x, "#VIP_Escaped", 13) == 0 ||
                cstrncmp(x, "#Terrorists_Escaped", 20) == 0 ||
                cstrncmp(x, "#CTs_PreventEscape", 19) == 0 ||
                cstrncmp(x, "#Target_Bombed", 15) == 0 ||
                cstrncmp(x, "#Game_Commencing", 17) == 0 ||
                cstrncmp(x, "#Game_will_restart_in", 22) == 0)
            {
                g_roundEnded = true;

                if (GetGameMode() == MODE_BASE)
                {
                    if (cstrncmp(x, "#CTs_Win", 9) == 0)
                        g_botManager->SetLastWinner(TEAM_COUNTER); // update last winner for economics
                    else if (cstrncmp(x, "#Terrorists_Win", 16) == 0)
                        g_botManager->SetLastWinner(TEAM_TERRORIST); // update last winner for economics
                }

                g_waypoint->SetBombPosition(true);
            }
            else if (!g_bombPlanted && cstrncmp(x, "#Bomb_Planted", 14) == 0)
            {
                g_bombPlanted = true;
                g_bombSayString = true;
                g_timeBombPlanted = engine->GetTime();
                g_waypoint->SetBombPosition();

                for (const auto& bot : g_botManager->m_bots)
                {
                    if (bot == nullptr)
                        continue;

                    if (!bot->m_isAlive)
                        continue;

                    bot->DeleteSearchNodes();
                }
            }
            else if (m_bot != nullptr)
            {
                if (cstrncmp(x, "#Switch_To_BurstFire", 21) == 0)
                    m_bot->m_weaponBurstMode = BURST_ENABLED;
                else if (cstrncmp(x, "#Switch_To_SemiAuto", 20) == 0)
                    m_bot->m_weaponBurstMode = BURST_DISABLED;
            }

            /*const auto search = HUDMAP.find(PTR_TO_STR(p));
            if (search != HUDMAP.end())
            {
                switch (search->second)
                {
                case RR::BombPlanted:
                {
                    if (!g_bombPlanted)
                    {
                        g_bombPlanted = true;
                        g_bombSayString = true;
                        g_timeBombPlanted = engine->GetTime();
                        g_waypoint->SetBombPosition();

                        for (const auto& bot : g_botManager->m_bots)
                        {
                            if (bot != nullptr && bot->m_isAlive)
                            {
                                bot->DeleteSearchNodes();
                                bot->ResetTasks();
                            }
                        }
                    }
                    break;
                }
                case RR::CTWin:
                {
                    g_roundEnded = true;
                    g_botManager->SetLastWinner(TEAM_COUNTER); // update last winner for economics
                    g_waypoint->SetBombPosition(true);
                    break;
                }
                case RR::TRWin:
                {
                    g_roundEnded = true;
                    g_botManager->SetLastWinner(TEAM_TERRORIST); // update last winner for economics
                    g_waypoint->SetBombPosition(true);
                    break;
                }
                case RR::BurstOn:
                {
                    if (m_bot != nullptr)
                        m_bot->m_weaponBurstMode = BURST_ENABLED;
                    break;
                }
                case RR::BurstOff:
                {
                    if (m_bot != nullptr)
                        m_bot->m_weaponBurstMode = BURST_DISABLED;
                    break;
                }
                case RR::Draw:
                {
                    g_roundEnded = true;
                    g_waypoint->SetBombPosition(true);
                    break;
                }
                }
            }*/
        }
        break;
    }
    case NETMSG_BARTIME:
    {
        if (m_state == 0 && m_bot != nullptr)
        {
            if (GetGameMode() == MODE_BASE)
            {
                if (PTR_TO_INT(p))
                    m_bot->m_hasProgressBar = true; // the progress bar on a hud
                else
                    m_bot->m_hasProgressBar = false; // no progress bar or disappeared
            }
            else
                m_bot->m_hasProgressBar = false;
        }
        break;
    }
    }
    m_state++; // and finally update network message state
}
