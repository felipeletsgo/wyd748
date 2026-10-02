#include "pch.h"
// BASE_* functions split by domain; declarations stay in Basedef.h, global tables and loaders in Basedef.cpp.
#include "Basedef.h"
#include "TMGlobal.h"
#include "ServerListAsset.h"

/** Formats str and its arguments into a static 512-byte buffer via vsprintf_s.
 * Arguments must match the format. The borrowed result must not be freed;
 * the next call overwrites it. This is not reentrant or thread-safe; format
 * and capacity errors follow CRT handling rather than a dedicated status. */
char* strfmt(const char* str, ...)
{
    static char buffer[512] = { 0, };
    va_list va;
    va_start(va, str);
    vsprintf_s(buffer, str, va);
    va_end(va);
    return buffer;
}

void BASE_UnderBarToSpace(char* szStr)
{
	while (*szStr)
	{
		if (*szStr == '_')
			*szStr = ' ';

		++szStr;
	}
}

int IsClearString(char* str, int target)
{
	int len = strlen(str);
	for (int pos = 0; pos < len; ++pos)
	{
		if (str[pos] >= 0)
		{
			if (pos >= target)
				return 1;
		}
		else
		{
			if (pos == target)
				return 0;
			if (pos == target + 1)
				return 1;

			++pos;
		}
	}

	return 1;
}

int IsClearString2(char* str, int nTarget)
{
	if (!str)
		return 1;

	char* pNextRightChar = CharNext(&str[nTarget]);
	int nLen = pNextRightChar - &str[nTarget];
	int nLen2 = pNextRightChar - CharPrev(str, pNextRightChar);

	if (nLen == 1 && nLen2 == 2)
		return 0;

	if (nLen != 2 || nLen2 != 1)
		return 1;

	return 0;
}

char BASE_CheckValidString(char* name)
{
    int size = strlen(name);
    if (size < 4 || size >= 16)
        return 0;

    for (int j = 0; j < size; ++j)
    {
        char x = name[j];
        if (x < 0)
        {
            if (!name[++j])
                return 0;
        }
        else if ((x < 'a' || x > 'z') && (x < 'A' || x > 'Z') && (x < '0' || x > '9') && x != '-')
            return 0;
    }

    return 1;
}

char* BASE_TransCurse(char* sz)
{
    if (sz == nullptr)
        return 0;

    return sz;
}

char BASE_CheckChatValid(const char* Chat)
{
    return 1;
}

char CheckGuildName(const char* GuildName, bool bSubguild)
{
    int nLen = strlen(GuildName);
    if (!bSubguild)
    {
        for (int i = 0; i < nLen; ++i)
        {
            if (GuildName[i] == ' ')
                return 0;

            if (GuildName[i] == '_')
                return 0;
        }
    }

    if (nLen < 2)
        return 1;

    for (int i = 1; i < nLen; ++i)
        if (GuildName[i] == -95 && GuildName[i - 1] == -95)
            return 0;

    return 1;
}
