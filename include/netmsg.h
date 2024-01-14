#pragma once
/*#include <unordered_map>

enum RR
{
    Unknown,
    CTWin,
    TRWin,
    Draw,
    BombPlanted,
    BurstOn,
    BurstOff
};*/

// netmessage handler class
class NetworkMsg : public Singleton <NetworkMsg>
{
private:
	Bot* m_bot;
	int m_state;
	int m_message;
	int m_registerdMessages[NETMSG_NUM];
    /*std::unordered_map<char*, int> VGUIMAP =
    {
        {"#Team_Select", CMENU_TEAM},
        {"#Team_Select_Spect", CMENU_TEAM},
        {"#IG_Team_Select", CMENU_TEAM},
        {"#IG_VIP_Team_Select", CMENU_TEAM},
        {"#IG_VIP_Team_Select_Spect", CMENU_TEAM},
        {"#Terrorist_Select", CMENU_CLASS},
        {"#CT_Select", CMENU_CLASS}
    };
    std::unordered_map<char*, int> HUDMAP =
    {
        {"#CTs_Win", RR::CTWin},
        {"#Terrorists_Win", RR::TRWin},
        {"#Bomb_Defused", RR::Draw},
        {"#Round_Draw", RR::Draw},
        {"#All_Hostages_Rescued", RR::Draw},
        {"#Hostages_Not_Rescued", RR::Draw},
        {"#Terrorists_Not_Escaped", RR::Draw},
        {"#VIP_Not_Escaped", RR::Draw},
        {"#Escaping_Terrorists_Neutralized", RR::Draw},
        {"#VIP_Assassinated", RR::Draw},
        {"#VIP_Escaped", RR::Draw},
        {"#Terrorists_Escaped", RR::Draw},
        {"#CTs_PreventEscape", RR::Draw},
        {"#Target_Bombed", RR::Draw},
        {"#Game_Commencing", RR::Draw},
        {"#Game_will_restart_in", RR::Draw},
        {"#Bomb_Planted", RR::BombPlanted},
        {"#Switch_To_BurstFire", RR::BurstOn},
        {"#Switch_To_SemiAuto", RR::BurstOff}
    };*/
public:
	NetworkMsg(void);
	~NetworkMsg(void) {};

	void Execute(void* p);
	void Reset(void) { m_message = NETMSG_UNDEFINED; m_state = 0; m_bot = nullptr; };
	void HandleMessageIfRequired(const int messageType, const int requiredType);

	void SetMessage(const int message) { m_message = message; }
	void SetBot(Bot* bot) { m_bot = bot; }

	int GetId(const int messageType) { return m_registerdMessages[messageType]; }
	void SetId(const int messageType, const int messsageIdentifier) { m_registerdMessages[messageType] = messsageIdentifier; }
};

#define g_netMsg NetworkMsg::GetObjectPtr()

/*class NetTri
{
    int x;
    float y;
    char z;
};

// netmessage handler class
class NetworkMsg : public Singleton <NetworkMsg>
{
private:
	Bot* m_bot;
    int m_message;
    NetTri* m_messageArray;
    uint_fast8_t m_messageNumber;
	int m_registerdMessages[NETMSG_NUM];
public:
    NetworkMsg(void)
    {
        m_bot = nullptr;
        safeloc(m_messageArray, 10);
        m_messageNumber = 0;

        for (auto& message : m_registerdMessages)
            message = NETMSG_UNDEFINED;
    };

    ~NetworkMsg(void)
    {
        m_bot = nullptr;
        safedel(m_messageArray);
        m_messageNumber = 0;
    };

	void Execute(void* p);
	void Reset(void) { m_message = NETMSG_UNDEFINED; m_state = 0; m_bot = nullptr; };
	void HandleMessageIfRequired(const int messageType, const int requiredType);

	void SetMessage(const int message) { m_message = message; }
	void SetBot(Bot* bot) { m_bot = bot; }

	int GetId(const int messageType) { return m_registerdMessages[messageType]; }
	void SetId(const int messageType, const int messsageIdentifier) { m_registerdMessages[messageType] = messsageIdentifier; }
};*/