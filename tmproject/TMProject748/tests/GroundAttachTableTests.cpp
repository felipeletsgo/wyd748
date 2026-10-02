#include "../internal/render/world/terrain/TMGroundAttachTable.h"

#include <cstdio>

namespace
{
struct Flags
{
    char m_cLeftEnable;
    char m_cRightEnable;
    char m_cUpEnable;
    char m_cDownEnable;
};

// Literal copy of the original TMGround::SetAttatchEnable branch chain, kept as
// the oracle for the generated table. Only member access was qualified.
void LegacySetAttatchEnable(Flags& flags, int nX, int nY)
{
    if (nX == 1 && nY == 1)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 16 && nY == 15)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 16 && nY == 16)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 17 && nY == 16)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 18 && nY == 16)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 18 && nY == 17)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 19 && nY == 16)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 19 && nY == 15)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 20 && nY == 16)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 20 && nY == 15)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 19 && nY == 14)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 19 && nY == 13)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 19 && nY == 12)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 20 && nY == 13)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 18 && nY == 13)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 18 && nY == 12)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 17 && nY == 13)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 17 && nY == 12)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 17 && nY == 11)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 17 && nY == 10)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 17 && nY == 9)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 15 && nY == 13)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 16 && nY == 12)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 15 && nY == 12)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 14 && nY == 13)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 6 && nY == 28)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 13 && nY == 31)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 14 && nY == 30)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 15 && nY == 31)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 13 && nY == 13)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 12 && nY == 13)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 11 && nY == 13)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 10 && nY == 13)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 9 && nY == 13)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 8 && nY == 13)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 10 && nY == 14)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 13 && nY == 14)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 13 && nY == 12)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 1 && nY == 31)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 1 && nY == 29)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 2 && nY == 29)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 3 && nY == 29)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 4 && nY == 29)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 5 && nY == 29)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 3 && nY == 30)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 3 && nY == 31)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 5 && nY == 31)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 6 && nY == 31)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 7 && nY == 31)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 6 && nY == 30)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 7 && nY == 29)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 8 && nY == 29)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 10 && nY == 11)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 9 && nY == 31)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 10 && nY == 31)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 11 && nY == 31)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 10 && nY == 29)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 11 && nY == 29)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 9 && nY == 28)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 8 && nY == 27)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 10 && nY == 27)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 8 && nY == 2)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 9 && nY == 1)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 10 && nY == 2)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 13 && nY == 28)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 14 && nY == 28)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 17 && nY == 31)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 18 && nY == 31)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 19 && nY == 31)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 17 && nY == 30)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 18 && nY == 30)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 19 && nY == 30)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 17 && nY == 28)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 31 && nY == 31)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 25 && nY == 13)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 26 && nY == 8)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 26 && nY == 9)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 26 && nY == 10)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 26 && nY == 11)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 26 && nY == 12)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 27 && nY == 11)
    {
        flags.m_cLeftEnable = 2;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 2;
        flags.m_cDownEnable = 2;
    }
    else if (nX < 26)
    {
        if (nX == 8 && nY == 16)
        {
            flags.m_cLeftEnable = 0;
            flags.m_cRightEnable = 1;
            flags.m_cUpEnable = 1;
            flags.m_cDownEnable = 0;
        }
        else if (nX == 9 && nY == 16)
        {
            flags.m_cLeftEnable = 1;
            flags.m_cRightEnable = 0;
            flags.m_cUpEnable = 1;
            flags.m_cDownEnable = 0;
        }
        else if (nX == 8 && nY == 15)
        {
            flags.m_cLeftEnable = 0;
            flags.m_cRightEnable = 1;
            flags.m_cUpEnable = 0;
            flags.m_cDownEnable = 1;
        }
        else if (nX == 9 && nY == 15)
        {
            flags.m_cLeftEnable = 1;
            flags.m_cRightEnable = 0;
            flags.m_cUpEnable = 0;
            flags.m_cDownEnable = 1;
        }
    }
    else if (nX == 28 && nY == 24)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 28 && nY == 23)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 27 && nY == 23)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 0;
    }
    else if (nX == 29 && nY == 23)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 2;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 2;
    }
    else if (nX == 27 && nY == 22)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 28 && nY == 22)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 1;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 28 && nY == 21)
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 29 && nY == 22)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 1;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 1;
    }
    else if (nX == 30 && nY == 22)
    {
        flags.m_cLeftEnable = 1;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
    else if (nX != 29 || nY != 27)
    {
        if (nX == 28 && nY == 28)
        {
            flags.m_cLeftEnable = 0;
            flags.m_cRightEnable = 0;
            flags.m_cUpEnable = 0;
            flags.m_cDownEnable = 0;
        }
        else if (nX == 30 && nY == 28)
        {
            flags.m_cLeftEnable = 0;
            flags.m_cRightEnable = 0;
            flags.m_cUpEnable = 0;
            flags.m_cDownEnable = 0;
        }
    }
    else
    {
        flags.m_cLeftEnable = 0;
        flags.m_cRightEnable = 0;
        flags.m_cUpEnable = 0;
        flags.m_cDownEnable = 0;
    }
}

void TableSetAttatchEnable(Flags& flags, int nX, int nY)
{
    if (const auto* rule = ground_attach::Find(nX, nY))
    {
        flags.m_cLeftEnable = rule->left;
        flags.m_cRightEnable = rule->right;
        flags.m_cUpEnable = rule->up;
        flags.m_cDownEnable = rule->down;
    }
}
}

int RunGroundAttachTableTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL ground attach table: %s\n", message);
        }
    };

    // Every cell of a domain far larger than the 32x32 terrain index, starting
    // from a sentinel so unmatched cells must keep their previous flags.
    for (int y = -64; y <= 320; ++y) {
        for (int x = -64; x <= 320; ++x) {
            Flags legacy{-7, -7, -7, -7};
            Flags table{-7, -7, -7, -7};
            LegacySetAttatchEnable(legacy, x, y);
            TableSetAttatchEnable(table, x, y);
            check(legacy.m_cLeftEnable == table.m_cLeftEnable &&
                legacy.m_cRightEnable == table.m_cRightEnable &&
                legacy.m_cUpEnable == table.m_cUpEnable &&
                legacy.m_cDownEnable == table.m_cDownEnable,
                "table matches the original branch chain");
        }
    }

    // Each table row is a distinct cell, so lookup order cannot change results.
    for (int i = 0; i < static_cast<int>(sizeof(ground_attach::kRules) / sizeof(ground_attach::kRules[0])); ++i)
        for (int j = i + 1; j < static_cast<int>(sizeof(ground_attach::kRules) / sizeof(ground_attach::kRules[0])); ++j)
            check(ground_attach::kRules[i].x != ground_attach::kRules[j].x ||
                ground_attach::kRules[i].y != ground_attach::kRules[j].y,
                "table cells are unique");
    return failures;
}
