#include "pch.h"
// TMGround split by responsibility; terrain lifecycle and loading stay in TMGround.cpp.
#include "TMSea.h"
#include "TMGlobal.h"
#include "TMGround.h"
#include "TMGroundAttachTable.h"

int TMGround::Attach(TMGround* pGround)
{
    if (!pGround)
        return 0;

    const int deltaX = pGround->m_vecOffsetIndex.x - m_vecOffsetIndex.x;
    const int deltaY = pGround->m_vecOffsetIndex.y - m_vecOffsetIndex.y;
    if (!((deltaX == 1 || deltaX == -1) && deltaY == 0) &&
        !((deltaY == 1 || deltaY == -1) && deltaX == 0))
        return 0;

    m_pLeftGround = 0;
    m_pRightGround = 0;
    m_pUpGround = 0;
    m_pDownGround = 0;

    if (deltaX == 1)
    {
        m_pRightGround = pGround;
        m_pRightGround->m_pLeftGround = this;

        for (int i = 0; i < 64; ++i)
        {
            pGround->m_TileMapData[64 * i].cHeight = m_TileMapData[(i << 6) + 63].cHeight;
            pGround->m_TileMapData[64 * i].dwColor = m_TileMapData[(i << 6) + 63].dwColor;

            pGround->m_TileNormalVector[64 * i] = m_TileNormalVector[64 * i + 63];
        }

        m_nMiniMapPos = 0;
        m_pRightGround->m_nMiniMapPos = 1;
        return 1;
    }
    if (deltaX == -1)
    {
        m_pLeftGround = pGround;
        m_pLeftGround->m_pRightGround = this;

        for (int i = 0; i < 64; ++i)
        {
            m_TileMapData[64 * i].cHeight = pGround->m_TileMapData[(i << 6) + 63].cHeight;
            m_TileMapData[64 * i].dwColor = pGround->m_TileMapData[(i << 6) + 63].dwColor;

           m_TileNormalVector[64 * i] = pGround->m_TileNormalVector[64 * i + 63];
        }

        m_nMiniMapPos = 1;
        m_pLeftGround->m_nMiniMapPos = 0;
        return 1;
    }
    if (deltaY == 1)
    {
        m_pDownGround = pGround;
        m_pDownGround->m_pUpGround = this;

        for (int i = 0; i < 64; ++i)
        {
            pGround->m_TileMapData[i].cHeight = m_TileMapData[i + 4032].cHeight;
            pGround->m_TileMapData[i].dwColor = m_TileMapData[i + 4032].dwColor;

            pGround->m_TileNormalVector[i] = m_TileNormalVector[i + 4032];
        }

        m_nMiniMapPos = 0;
        m_pDownGround->m_nMiniMapPos = 2;
        return 1;
    }
    if (deltaY == -1)
    {
        m_pUpGround = pGround;
        m_pUpGround->m_pDownGround = this;

        for (int i = 0; i < 64; ++i)
        {
            m_TileMapData[i].cHeight = pGround->m_TileMapData[i + 4032].cHeight;
            m_TileMapData[i].dwColor = pGround->m_TileMapData[i + 4032].dwColor;

            m_TileNormalVector[i] = pGround->m_TileNormalVector[i + 4032];
        }

        m_nMiniMapPos = 2;
        m_pUpGround->m_nMiniMapPos = 0;
        return 1;
    }

    return 0;
}

void TMGround::SetAttatchEnable(int nX, int nY)
{
    // Cell flags are data in TMGroundAttachTable.h. Cells outside the table
    // derive each side from the field files shipped for their neighbours.
    const auto hasField = [](int x, int y)
    {
        if (x < 0 || y < 0 || x > 99 || y > 99)
            return false;
        char path[32]{};
        sprintf_s(path, "Env\\Field%02d%02d.trn", x, y);
        return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
    };
    const auto* tableRule = ground_attach::Find(nX, nY);
    const auto derived = ground_attach::FromNeighbours(nX, nY, hasField(nX - 1, nY),
        hasField(nX + 1, nY), hasField(nX, nY - 1), hasField(nX, nY + 1));
    const auto* rule = tableRule ? tableRule : &derived;
    m_cLeftEnable = rule->left;
    m_cRightEnable = rule->right;
    m_cUpEnable = rule->up;
    m_cDownEnable = rule->down;
}
