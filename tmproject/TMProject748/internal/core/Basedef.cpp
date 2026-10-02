#include "pch.h"
#include "Basedef.h"

// Client global tables and their loaders. BASE_* queries live in the domain
// units BasedefItems, BasedefAppearance, BasedefCombat, BasedefWorld,
// BasedefText and BasedefSystem; all are declared in Basedef.h.
#include "TMGlobal.h"
#include "TMLog.h"
#include "ItemEffect.h"
#include "NativeItemVolatile.h"
#include "WYD748Assets.h"
#include "ServerListAsset.h"
#include <WinInet.h>

namespace
{
    static_assert(EF_VOLATILE == native_item_volatile::Effect,
        "Native volatile ability type changed");
    static_assert(MAX_ITEMLIST == native_item_volatile::CatalogLimit,
        "Native volatile catalog bound changed");
    constexpr int SharedHeightMapWidth = 4096;
    constexpr std::size_t SharedHeightMapSize =
        SharedHeightMapWidth * SharedHeightMapWidth;
    std::array<char, SharedHeightMapSize> sharedHeightMap{};
    bool sharedHeightMapLoaded = false;
}

char g_pAffectTable[MAX_EFFECT_STRING_TABLE][24];
char g_pAffectSubTable[MAX_SUB_EFFECT_STRING_TABLE][24];
int g_pHitRate[1024];

HWND hWndMain;
char EncodeByte[4];
int g_nChannelWidth;
int g_nServerGroupNum;
char g_pServerList[MAX_SERVERGROUP][MAX_SERVERNUMBER][64];
int g_nSelServerWeather;
char g_pMessageStringTable[MAX_STRING][MAX_STRING_LENGTH];
STRUCT_ITEMLIST g_pItemList[MAX_ITEMLIST];
STRUCT_SPELL g_pSpell[MAX_SPELL_LIST];
STRUCT_TOTOLIST g_pTOTOList[80];
int g_nTOTOListCount;

static_assert(sizeof(STRUCT_TOTOLIST) == 96, "7.48 TOTO list entry must remain 96 bytes");

STRUCT_GUILDZONE g_pGuildZone[MAX_GUILDZONE] =
{
    {0, 0, 2088, 2148, 2086, 2093, 2052, 2052, 2171, 2163, 197, 213, 238, 230, 205, 220, 228, 220, 5, 0}, // Armia
    {0, 0, 2531, 1700, 2494, 1707, 2432, 1672, 2675, 1767, 197, 149, 238, 166, 205, 157, 228, 157, 5, 0}, // Azran
    {0, 0, 2460, 1976, 2453, 2000, 2448, 1966, 2476, 2024, 141, 213, 182, 230, 146, 220, 173, 220, 5, 0}, // Erion
    {0, 0, 3614, 3124, 3652, 3122, 3605, 3090, 3690, 3260, 141, 149, 182, 166, 146, 157, 173, 157, 5, 0}, // Nippleheim
    {0, 0, 1066, 1760, 1050, 1706, 1036, 1700, 1072, 1760, 4000, 4000, 4010, 4010, 4005, 4005, 4005, 4005, 5, 0} // Noatum
};

/** Delegates asset-root setup to the WYD748 adapter.
 * Returns no status; path policy belongs to the adapter. */
void BASE_InitModuleDir()
{
    // WYD_ASSET_ROOT allows this source build to read the existing 7.48 tree.
    WYD748_InitializeAssetRoot();
}

/** Rebuilds the 1,024 values of the global hit-rate table deterministically.
 * Overwrites prior contents, caps values at 999, and sets entry zero to 512.
 * Uses no RNG; run before consumers of g_pHitRate. */
void BASE_InitializeHitRate()
{
    memset(g_pHitRate, 0, sizeof(g_pHitRate));

    int Jump = 512;
    int Start = 0;
    int Quad = 0;

    do
    {
        for (int i = 0; i < 1024; i += Jump)
        {
            if (!g_pHitRate[i])
            {
                if (!Quad)
                    g_pHitRate[i] = Start;
                else if (Quad == 1)
                    g_pHitRate[i] = 512 - Start;
                else if (Quad == 2)
                    g_pHitRate[i] = Start + 512;
                else
                    g_pHitRate[i] = 1024 - Start;

                if (g_pHitRate[i] > 999)
                    g_pHitRate[i] = 999;
                if (++Quad >= 4)
                    Quad = 0;
                if (!Quad)
                    ++Start;
            }
        }
        Jump /= 2;
    } while (Jump);

    g_pHitRate[0] = 512;
}

/** Loads g_pAttribute from Env/AttributeMap.dat, falling back to TMSRV/Run.
 * Closes the opened file. Returns 0 for a missing or truncated map and leaves
 * the existing table unchanged when the selected file is truncated. */
int BASE_InitializeAttribute()
{
    char FileName[256]{};
    strcpy(FileName, "./Env/AttributeMap.dat");

    FILE* fp = nullptr;
    fopen_s(&fp, FileName, "rb");
    if (fp == nullptr)
        fopen_s(&fp, "../../TMSRV/Run/AttributeMap.dat", "rb");

    if (fp == nullptr)
    {
        MessageBox(0, "There is no file", "Attributemap.dat", MB_OK);
        return 0;
    }

    const bool loaded = WYD748_ReadAttributeMap(
        fp, reinterpret_cast<char*>(g_pAttribute), sizeof(g_pAttribute));
    fclose(fp);

    return loaded ? 1 : 0;
}

/** Loads the server's world-height payload packaged with the client.
 * A missing or truncated map must fail startup instead of silently restoring
 * a different client-side collision surface. */
int BASE_InitializeHeightMap()
{
    FILE* file = nullptr;
    fopen_s(&file, "./Env/HeightMap.dat", "rb");
    if (file == nullptr)
        return 0;

    const bool loaded = std::fseek(file, 0, SEEK_END) == 0 &&
        std::ftell(file) == static_cast<long>(SharedHeightMapSize) &&
        std::fseek(file, 0, SEEK_SET) == 0 &&
        std::fread(sharedHeightMap.data(), 1, sharedHeightMap.size(), file) ==
        sharedHeightMap.size();
    std::fclose(file);
    sharedHeightMapLoaded = loaded;
    return loaded ? 1 : 0;
}

/** Copies the shared server height map into the local window, then applies
 * attribute-map bit 2. Out-of-world cells are blocked. The caller supplies
 * a valid size and the g_HeightWidth stride. */
void BASE_ApplyAttribute(char* pHeight, int size)
{
    if (sharedHeightMapLoaded && pHeight != nullptr)
    {
        for (int localY = 0; localY < size; ++localY)
        {
            char* row = pHeight + localY * g_HeightWidth;
            const int worldY = g_HeightPosY + localY;
            for (int localX = 0; localX < size; ++localX)
            {
                const int worldX = g_HeightPosX + localX;
                row[localX] = worldX >= 0 && worldX < SharedHeightMapWidth &&
                    worldY >= 0 && worldY < SharedHeightMapWidth
                    ? sharedHeightMap[worldY * SharedHeightMapWidth + worldX]
                    : 127;
            }
        }
    }

    int endx = size + g_HeightPosX;
    int endy = size + g_HeightPosY;

    for (int y = g_HeightPosY; y < endy; ++y)
    {
        for (int x = g_HeightPosX; x < endx; ++x)
        {
            if (g_pAttribute[(y >> 2) & 0x3FF][(x >> 2) & 0x3FF] & 2)
                pHeight[x + g_HeightWidth * (y - g_HeightPosY) - g_HeightPosX] = 127;
        }
    }
}

/** Asks the adapter to load ItemList.bin into the global g_pItemList table.
 * Returns 1 on success or 0 with a dialog on failure. Record conversion is
 * handled by WYD748_LoadItemList rather than reading the runtime struct raw. */
int BASE_ReadItemList()
{
    // ItemList 7.48 stores 140-byte rows, whereas this newer source keeps a
    // 156-byte runtime struct. The adapter translates fields instead of
    // reading across record boundaries and corrupting meshes and slot masks.
    if (!WYD748_LoadItemList(".\\ItemList.bin", g_pItemList, _countof(g_pItemList)))
    {
        MessageBoxA(0, "Can't read ItemList.bin", "ERROR", 0);
        return 0;
    }

    return 1;
}

/** Loads g_pSpell through WYD748_LoadSkillData using SkillData_Path.
 * Returns TRUE on success or FALSE with a dialog on failure. The adapter
 * receives table capacity and owns the read/conversion logic. */
int BASE_ReadSkillBin()
{
    // SkillData 7.48 has 104 compact rows; translate it before gameplay uses it.
    if (!WYD748_LoadSkillData(SkillData_Path, g_pSpell, _countof(g_pSpell)))
    {
        MessageBox(NULL, "Can't read SkillData.bin", "ERROR", NULL);
        return FALSE;
    }

    return TRUE;
}

/** Overrides local prices at g_pItemList indexes 412, 413, 419, and 420.
 * Run after table loading so the overrides remain. Does not persist values
 * or change server authority over transactions. */
void BASE_InitialItemRePrice()
{
    g_pItemList[412].nPrice = 4000000;
    g_pItemList[413].nPrice = 8000000;
    g_pItemList[419].nPrice = 400000;
    g_pItemList[420].nPrice = 800000;
}

/** Computes the legacy sum over p[0..size), alternating by index modulo 7.
 * p is borrowed and read-only, and must cover size bytes when size > 0.
 * Returns 0 for size <= 0. Preserves char/int arithmetic; this is not a secure hash. */
int BASE_GetSum(char* p, int size)
{
	int sum = 0;
	for (int i = 0; i < size; ++i)
	{
		int mod = i % 7;
		if (!(i % 7))
			sum += p[i] / 2;
		if (mod == 1)
			sum += p[i] ^ 0xFF;
		if (mod == 2)
			sum += 3 * p[i];
		if (mod == 3)
			sum += 2 * p[i];
		if (mod == 4)
			sum -= p[i] / 7;
		// The else belongs only to mod == 5, not to the earlier conditions.
		if (mod == 5)
			sum -= p[i];
		else
			sum += p[i] / 3;
	}

	return sum;
}

/** Legacy-sum variant with a modulo-9 cycle; returns 0 for size <= 0.
 * p is read-only and must cover size bytes when size > 0.
 * Does not replace BASE_GetSum: its factors and complementary division differ. */
int BASE_GetSum2(char* p, int size)
{
    int sum = 0;

    for (int i = 0; i < size; ++i)
    {
        int mod = i % 9;
        if (mod == 0)
            sum += 2 * p[i];
        if (mod == 1)
            sum += p[i] ^ 0xFF;
        if (mod == 2)
            sum += p[i] / 3;
        if (mod == 3)
            sum += 2 * p[i];
        if (mod == 4)
            sum -= p[i] ^ 0x5A;
        // Complementary division also applies to cases 0..4 and 6..8.
        if (mod == 5)
            sum -= p[i];
        else
            sum += p[i] / 5;
    }

    return sum;
}

/** Delegates loading Strdef_Path into the global message table.
 * Supplies row count and capacity and forwards the loader result without a
 * dialog or a second interpretation of the file. */
int BASE_ReadMessageBin()
{
	// The 7.48 strdef contains fewer rows than TMProject but keeps 128-byte indices.
	return WYD748_LoadMessageStrings(
		Strdef_Path,
		&g_pMessageStringTable[0][0],
		_countof(g_pMessageStringTable),
		_countof(g_pMessageStringTable[0]));
}

/** Loads effect and subeffect names into their fixed-width global tables.
 * Missing or malformed assets leave the corresponding table untouched. */
void BASE_InitEffectString()
{
    WYD748_LoadEffectStrings(EffectString_Path, &g_pAffectTable[0][0],
        MAX_EFFECT_STRING_TABLE, sizeof(g_pAffectTable[0]), 1);
    WYD748_LoadEffectStrings(EffectSubString_Path, &g_pAffectSubTable[0][0],
        MAX_SUB_EFFECT_STRING_TABLE, sizeof(g_pAffectSubTable[0]), 0);

    /* There's a loading of the GuildString.txt file, but is not used */
}

/** Initializes server list, skills, items, and language in that order.
 * Runs every step even after failure (bitwise AND, no short-circuit) and
 * returns the accumulated result seeded by the first result's low bit.
 * Does not roll back tables already loaded if a later step fails. */
int BASE_InitializeBaseDef()
{
    int ret = 0;
    ret = BASE_InitializeServerList() & 1;
    ret = BASE_ReadSkillBin() & ret;
    ret = BASE_ReadItemList() & ret;
    ret = BASE_GetLanguage() & ret;
	return ret;
}

/*
 * Data, rules, and geometry API below:
 * - loaders (`BASE_ReadItemPrice`, `BASE_ReadTOTOList`, language, and filters)
 *   mutate only global tables and return legacy status;
 * - BASE_Get..., Is..., and Check... queries do not own their pointers or
 *   persist server-side state;
 * - item/equipment routines write only when given a destination pointer;
 *   buffers and structs belong to the caller;
 * - navigation/geometry writes to explicitly supplied buffers and requires
 *   valid dimensions from the caller;
 * - combat calculations retain historical integer and global-table behavior.
 * Individual comments cover exceptions; this common contract avoids repeating
 * the same explanation across small functions.
 */
void BASE_ReadItemPrice()
{
    int itemprice[MAX_ITEM_PRICE_REPLACE][2]{};

    FILE* fp = nullptr;
    fopen_s(&fp, ItemPrice_Path, "rb");

    if (fp)
    {
        fread(itemprice, sizeof(itemprice), 1, fp);
        fclose(fp);
    }

    for (int k = 0; k < MAX_ITEM_PRICE_REPLACE && itemprice[k][0]; ++k)
    {
        int idx = itemprice[k][0];
        int bufprice = g_pItemList[idx].nPrice;
        g_pItemList[idx].nPrice = itemprice[k][1];   
    }
}

void ReadItemName()
{
    WYD748_LoadItemNames(ItemName_Path, &g_pItemList[0].Name[0],
        _countof(g_pItemList), sizeof(g_pItemList[0]),
        sizeof(g_pItemList[0].Name));
}

void ReadUIString()
{
	WYD748_LoadUIStrings(UIString_Path, &g_UIString[0][0],
		_countof(g_UIString), sizeof(g_UIString[0]));
}

char ReadNameFiltraDataBase()
{
	return 1;
}

char ReadChatFiltraDataBase()
{
	return 1;
}

int BASE_InitializeServerList()
{
	FILE* fpBin = WYD748_OpenServerListAsset("./serverlist.local.bin", "./serverlist.bin");
	const bool loaded = WYD748_ReadServerList(fpBin, g_pServerList);
	if (fpBin)
		fclose(fpBin);
	if (!loaded)
		return 0;

	char szList[65] = { "¤¡¤¤¤§¤©¤±¤²¤µ¤·¤¸¤º¤»¤¼¤½¤¾¤¿¤Á¤Ã¤Å¤Ç¤Ë¤Ì¤Ð¤Ñ¤Ó¤¿¤Ä¤Ó¤Ç¤Ì°¡³ª´Ù"};
	for (int k = 0; k < MAX_SERVERGROUP; k++)
		for (int j = 0; j < MAX_SERVERNUMBER; j++)
			for (int i = 0; i < 64; i++)
				g_pServerList[k][j][i] -= szList[63 - i];
	return 1;
}


int BASE_GetLanguage()
{
    FILE* fpFont = nullptr;
    fopen_s(&fpFont, "Lang.txt", "rt");

    if (fpFont != nullptr)
    {
        char szTemp[256]{};
        fgets(szTemp, 256, fpFont);
        sscanf(szTemp, "%d", &g_nLangIndex);
        if (g_nFontBold < 0 || g_nLangIndex > 4)
            g_nFontBold = 0;

        fclose(fpFont);
        return 1;
    }
    // The 7.48 client has no Lang.txt; language zero is its implicit default.
    g_nLangIndex = 0;
    return 1;
}

int BASE_ReadTOTOList(char* szFileName)
{
    if (szFileName == nullptr)
        return 0;

    FILE* file = nullptr;
    fopen_s(&file, szFileName, "rt");
    if (file == nullptr)
        return 0;

    memset(g_pTOTOList, 0, sizeof(g_pTOTOList));
    g_nTOTOListCount = 0;

    char line[1024]{};
    if (fgets(line, sizeof(line), file) == nullptr)
    {
        fclose(file);
        return 0;
    }

    int declaredCount = 0;
    if (sscanf_s(line, "%d", &declaredCount) != 1 || declaredCount < 0 || declaredCount > 80)
    {
        fclose(file);
        return 0;
    }

    g_nTOTOListCount = declaredCount;
    for (int row = 0; row < declaredCount && fgets(line, sizeof(line), file) != nullptr; ++row)
    {
        for (char* cursor = line; *cursor != '\0'; ++cursor)
        {
            if (*cursor == ',')
                *cursor = ' ';
        }

        int index = -1;
        STRUCT_TOTOLIST entry{};
        const int parsedFields = sscanf_s(
            line,
            "%d %31s %31s %31s",
            &index,
            entry.szTime, static_cast<unsigned int>(_countof(entry.szTime)),
            entry.szTeamA, static_cast<unsigned int>(_countof(entry.szTeamA)),
            entry.szTeamB, static_cast<unsigned int>(_countof(entry.szTeamB)));
        if (parsedFields != 4)
            continue;

        BASE_UnderBarToSpace(entry.szTime);
        BASE_UnderBarToSpace(entry.szTeamA);
        BASE_UnderBarToSpace(entry.szTeamB);

        // The native table names matches 1..80. Keep the public number while
        // storing it in the complete zero-based array used by the source UI.
        if (index >= 1 && index <= 80)
            g_pTOTOList[index - 1] = entry;
    }

    fclose(file);
    return 1;
}
