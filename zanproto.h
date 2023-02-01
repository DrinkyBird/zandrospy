#pragma once

#define	SQF_NAME					0x00000001
#define	SQF_URL						0x00000002
#define	SQF_EMAIL					0x00000004
#define	SQF_MAPNAME					0x00000008
#define	SQF_MAXCLIENTS				0x00000010
#define	SQF_MAXPLAYERS				0x00000020
#define	SQF_PWADS					0x00000040
#define	SQF_GAMETYPE				0x00000080
#define	SQF_GAMENAME				0x00000100
#define	SQF_IWAD					0x00000200
#define	SQF_FORCEPASSWORD			0x00000400
#define	SQF_FORCEJOINPASSWORD		0x00000800
#define	SQF_GAMESKILL				0x00001000
#define	SQF_BOTSKILL				0x00002000
#define	SQF_DMFLAGS					0x00004000
#define	SQF_LIMITS					0x00010000
#define	SQF_TEAMDAMAGE				0x00020000
#define	SQF_TEAMSCORES				0x00040000
#define	SQF_NUMPLAYERS				0x00080000
#define	SQF_PLAYERDATA				0x00100000
#define SQF_TEAMINFO_NUMBER			0x00200000
#define SQF_TEAMINFO_NAME			0x00400000
#define SQF_TEAMINFO_COLOR			0x00800000
#define SQF_TEAMINFO_SCORE			0x01000000
#define SQF_TESTING_SERVER			0x02000000
#define SQF_DATA_MD5SUM				0x04000000
#define SQF_ALL_DMFLAGS				0x08000000
#define SQF_SECURITY_SETTINGS		0x10000000
#define SQF_OPTIONAL_WADS			0x20000000
#define SQF_DEH						0x40000000
#define SQF_EXTENDED_INFO			0x80000000

#define SQF2_PWAD_HASHES			0x00000001
#define SQF2_COUNTRY				0x00000002

#define	SQF_ALL						( SQF_NAME|SQF_URL|SQF_EMAIL|SQF_MAPNAME|SQF_MAXCLIENTS|SQF_MAXPLAYERS| \
									  SQF_PWADS|SQF_GAMETYPE|SQF_GAMENAME|SQF_IWAD|SQF_FORCEPASSWORD|SQF_FORCEJOINPASSWORD|SQF_GAMESKILL| \
									  SQF_BOTSKILL|SQF_DMFLAGS|SQF_LIMITS|SQF_TEAMDAMAGE|SQF_TEAMSCORES|SQF_NUMPLAYERS|SQF_PLAYERDATA|SQF_TEAMINFO_NUMBER|SQF_TEAMINFO_NAME|SQF_TEAMINFO_COLOR|SQF_TEAMINFO_SCORE| \
									  SQF_TESTING_SERVER|SQF_DATA_MD5SUM|SQF_ALL_DMFLAGS|SQF_SECURITY_SETTINGS|SQF_OPTIONAL_WADS|SQF_DEH|SQF_EXTENDED_INFO )

#define SQF2_ALL					( SQF2_PWAD_HASHES|SQF2_COUNTRY )

enum
{
    MSC_BEGINSERVERLIST,
    MSC_SERVER,
    MSC_ENDSERVERLIST,
    MSC_IPISBANNED,
    MSC_REQUESTIGNORED,
    MSC_WRONGVERSION,
    MSC_BEGINSERVERLISTPART,
    MSC_ENDSERVERLISTPART,
    MSC_SERVERBLOCK,

};

//*****************************************************************************
enum
{
    // Server is letting master server of its existence.
    SERVER_MASTER_CHALLENGE = 5660020,

    // [RC] This is no longer used.
    /*
        // Server is letting master server of its existence, along with sending an IP the master server
        // should use for this server.
        SERVER_MASTER_CHALLENGE_OVERRIDE = 5660021,
    */

    // Server is sending some statistics to the master server.
    SERVER_MASTER_STATISTICS = 5660022,

    // Server is sending its info to the launcher.
    SERVER_LAUNCHER_CHALLENGE,

    // Server is telling a launcher that it's ignoring it.
    SERVER_LAUNCHER_IGNORING,

    // Server is telling a launcher that his IP is banned from the server.
    SERVER_LAUNCHER_BANNED,

    // Client is trying to create a new account with the master server.
    CLIENT_MASTER_NEWACCOUNT,

    // Client is trying to log in with the master server.
    CLIENT_MASTER_LOGIN,

    // [BB] Launcher is querying the master server for a full server list, possibly split into several packets.
    LAUNCHER_MASTER_CHALLENGE,

    // [BB] Server is answering a MasterBanlistVerificationString verification request.
    SERVER_MASTER_VERIFICATION,

    // [BB] Server is acknowledging the receipt of a ban list.
    SERVER_MASTER_BANLIST_RECEIPT,
};