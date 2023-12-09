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

    static int damageArmor, damageTaken, damageBits;
    static int killerIndex, victimIndex, playerIndex;
    static int index, numPlayers;
    static int state, id, clip;

    static Vector damageOrigin;
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
            const auto search = VGUIMAP.find(PTR_TO_STR(p));
            if (search != VGUIMAP.end())
                m_bot->m_startAction = search->second;
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
            weaponProp.flags = PTR_TO_INT(p); // flags for weapon (WTF???)
            if (weaponProp.id > -1 && weaponProp.id < Const_MaxWeapons)
                g_weaponDefs[weaponProp.id] = weaponProp; // store away this weapon with it's ammo information...
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
            clip = PTR_TO_INT(p); // ammo currently in the clip for this weapon
            if (id > -1 && id < Const_MaxWeapons)
            {
                if (state != 0)
                    m_bot->m_currentWeapon = id;
                m_bot->m_ammoInClip[id] = clip;
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
            if (index > -1 && index < Const_MaxWeapons)
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
            if (index > -1 && index < Const_MaxWeapons)
                m_bot->m_ammo[index] = PTR_TO_INT(p);
            break;
        }
        }
        break;
    }
    case NETMSG_DAMAGE:
    {
        // this message gets sent when the bots are getting damaged
        switch (m_state)
        {
        case 0:
            damageArmor = PTR_TO_INT(p);
            break;

        case 1:
            damageTaken = PTR_TO_INT(p);
            break;

        case 2:
        {
            if (m_bot != nullptr)
            {
                damageBits = PTR_TO_INT(p);
                if (damageArmor > 0 || damageTaken > 0)
                    m_bot->TakeDamage(m_bot->pev->dmg_inflictor, damageTaken, damageArmor, damageBits);
            }
            break;
        }
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
            if (m_bot != nullptr && g_gameVersion != HALFLIFE)
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
        switch (m_state)
        {
        case 0:
        {
            killerIndex = PTR_TO_INT(p);
            break;
        }
        case 1:
        {
            victimIndex = PTR_TO_INT(p);
            break;
        }
        case 2:
        {
            Bot* victimer = g_botManager->GetBot(victimIndex);
            if (victimer != nullptr)
            {
                victimer->m_isAlive = false;
                victimer->DeleteSearchNodes();
            }
            break;
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
            const auto search = HUDMAP.find(PTR_TO_STR(p));
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
            }
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
