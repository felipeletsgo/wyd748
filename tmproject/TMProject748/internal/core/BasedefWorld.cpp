#include "pch.h"
// BASE_* functions split by domain; declarations stay in Basedef.h, global tables and loaders in Basedef.cpp.
#include "Basedef.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "ItemEffect.h"
#include "NativeItemVolatile.h"
#include "WYD748Assets.h"
#include "ServerListAsset.h"
#include <WinInet.h>

// Navigation, terrain, and coordinate projection.
int BASE_GetRoute(int x, int y, int* targetx, int* targety, char* Route, int distance, char* pHeight, int MH)
{
    int lastx = x;
    int lasty = y;
    int tx = *targetx;
    int ty = *targety;
    memset(Route, 0, 24);

    for (int i = 0; i < distance && i < 23; ++i)
    {
        if (x - g_HeightPosX < 1 || y - g_HeightPosY < 1 || x - g_HeightPosX > g_HeightWidth - 2 || y - g_HeightPosY > g_HeightHeight - 2)
        {
            Route[i] = 0;
            break;
        }

        int cul = pHeight[x + g_HeightWidth * (y - g_HeightPosY) - g_HeightPosX];
        int n = pHeight[x + g_HeightWidth * (y - g_HeightPosY - 1) - g_HeightPosX];
        int ne = pHeight[x + g_HeightWidth * (y - g_HeightPosY - 1) - g_HeightPosX + 1];
        int e = pHeight[x + g_HeightWidth * (y - g_HeightPosY) - g_HeightPosX + 1];
        int se = pHeight[x + g_HeightWidth * (y - g_HeightPosY + 1) - g_HeightPosX + 1];
        int s = pHeight[x + g_HeightWidth * (y - g_HeightPosY + 1) - g_HeightPosX];
        int sw = pHeight[x + g_HeightWidth * (y - g_HeightPosY + 1) - g_HeightPosX - 1];
        int w = pHeight[x + g_HeightWidth * (y - g_HeightPosY) - g_HeightPosX - 1];
        int nw = pHeight[x + g_HeightWidth * (y - g_HeightPosY - 1) - g_HeightPosX - 1];
        if (tx == x && ty == y)
        {
            Route[i] = 0;
            break;
        }
        if (tx == x && ty > y && s < MH + cul && s > cul - MH)
        {
            Route[i] = 56;
            ++y;
        }
        else if (tx == x && ty < y && n < MH + cul && n > cul - MH)
        {
            Route[i] = 50;
            --y;
        }
        else if (tx > x
            && ty < y
            && ne < MH + cul
            && ne > cul - MH
            && (n < MH + cul && n > cul - MH || e < MH + cul && e > cul - MH))
        {
            Route[i] = 51;
            ++x;
            --y;
        }
        else if (tx > x && ty == y && e < MH + cul && e > cul - MH)
        {
            Route[i] = 54;
            ++x;
        }
        else if (tx > x
            && ty > y
            && se < MH + cul
            && se > cul - MH
            && (s < MH + cul && s > cul - MH || e < MH + cul && e > cul - MH))
        {
            Route[i] = 57;
            ++x;
            ++y;
        }
        else if (tx < x
            && ty > y
            && sw < MH + cul
            && sw > cul - MH
            && (s < MH + cul && s > cul - MH || w < MH + cul && w > cul - MH))
        {
            Route[i] = 55;
            --x;
            ++y;
        }
        else if (tx < x && ty == y && w < MH + cul && w > cul - MH)
        {
            Route[i] = 52;
            --x;
        }
        else if (tx < x
            && ty < y
            && nw < MH + cul
            && nw > cul - MH
            && (n < MH + cul && n > cul - MH || w < MH + cul && w > cul - MH))
        {
            Route[i] = 49;
            --x;
            --y;
        }
        else if (tx > x && ty < y && e < MH + cul && e > cul - MH)
        {
            Route[i] = 54;
            ++x;
        }
        else if (tx > x && ty < y && n < MH + cul && n > cul - MH)
        {
            Route[i] = 50;
            --y;
        }
        else if (tx > x && ty > y && e < MH + cul && e > cul - MH)
        {
            Route[i] = 54;
            ++x;
        }
        else if (tx > x && ty > y && s < MH + cul && s > cul - MH)
        {
            Route[i] = 56;
            ++y;
        }
        else if (tx < x && ty > y && w < MH + cul && w > cul - MH)
        {
            Route[i] = 52;
            --x;
        }
        else if (tx < x && ty > y && s < MH + cul && s > cul - MH)
        {
            Route[i] = 56;
            ++y;
        }
        else if (tx < x && ty < y && w < MH + cul && w > cul - MH)
        {
            Route[i] = 52;
            --x;
        }
        else if (tx < x && ty < y && n < MH + cul && n > cul - MH)
        {
            Route[i] = 50;
            --y;
        }
        else
        {
            if (tx == x + 1 || ty == y + 1 || tx == x - 1 || ty == y - 1)
            {
                Route[i] = 0;
                break;
            }
            if (tx == x
                && ty > y
                && se < MH + cul
                && se > cul - MH
                && (s < MH + cul && s > cul - MH || e < MH + cul && e > cul - MH))
            {
                Route[i] = 57;
                ++x;
                ++y;
            }
            else if (tx == x
                && ty > y
                && sw < MH + cul
                && sw > cul - MH
                && (s < MH + cul && s > cul - MH || w < MH + cul && w > cul - MH))
            {
                Route[i] = 55;
                --x;
                ++y;
            }
            else if (tx == x
                && ty < y
                && ne < MH + cul
                && ne > cul - MH
                && (n < MH + cul && n > cul - MH || e < MH + cul && e > cul - MH))
            {
                Route[i] = 51;
                ++x;
                --y;
            }
            else if (tx == x
                && ty < y
                && nw < MH + cul
                && nw > cul - MH
                && (n < MH + cul && n > cul - MH || w < MH + cul && w > cul - MH))
            {
                Route[i] = 49;
                --x;
                --y;
            }
            else if (tx < x
                && ty == y
                && sw < MH + cul
                && sw > cul - MH
                && (s < MH + cul && s > cul - MH || w < MH + cul && w > cul - MH))
            {
                Route[i] = 55;
                --x;
                ++y;
            }
            else if (tx < x
                && ty == y
                && nw < MH + cul
                && nw > cul - MH
                && (n < MH + cul && n > cul - MH || w < MH + cul && w > cul - MH))
            {
                Route[i] = 49;
                --x;
                --y;
            }
            else if (tx > x
                && ty == y
                && se < MH + cul
                && se > cul - MH
                && (s < MH + cul && s > cul - MH || e < MH + cul && e > cul - MH))
            {
                Route[i] = 57;
                ++x;
                ++y;
            }
            else
            {
                if (tx <= x
                    || ty != y
                    || ne >= MH + cul
                    || ne <= cul - MH
                    || (n >= MH + cul || n <= cul - MH) && (e >= MH + cul || e <= cul - MH))
                {
                    Route[i] = 0;
                    break;
                }
                Route[i] = 51;
                ++x;
                --y;
            }
        }
    }

    if (lastx == x && lasty == y)
        return 0;

    *targetx = x;
    *targety = y;
    return lastx != x || lasty != y;
}

int BASE_GetDistance(int x1, int y1, int x2, int y2)
{
    int dy;
    int dx;
    if (x1 <= x2)
        dx = x2 - x1;
    else
        dx = x1 - x2;
    if (y1 <= y2)
        dy = y2 - y1;
    else
        dy = y1 - y2;
    if (dx <= 6 && dy <= 6)
        return g_pDistanceTable[dy][dx];
    if (dx <= dy)
        return dy + 1;

    return dx + 1;
}

int BASE_GetVillage(int x, int y)
{
    for (int i = 0; i < 5; ++i)
    {
        if (x >= g_pGuildZone[i].vx1 && g_pGuildZone[i].vx2 && g_pGuildZone[i].vy1 && g_pGuildZone[i].vy2)
            return i;
    }

    return MAX_GUILDZONE;
}

char BASE_GetAttribute(int x, int y)
{
    if (x >= 0 && x <= 4096 && y >= 0 && x <= 4096)
        return g_pAttribute[y / 4 & 1023][x / 4 & 1023];

    return 0;
}

char BASE_GetAttr(int nX, int nY)
{
    return g_pAttribute[nY / 4 % 1024][nX / 4 % 1024];
}

int BASE_IsInLowZone(int nX, int nY)
{
    int nX4 = nX / 4;
    int nY4 = nY / 4;
    if (nY / 4 < 1024 && nX4 < 1024 && nY4 >= 0 && nX4 >= 0)
        return g_pAttribute[nY4][nX4] < 0;

    LOG_WRITELOG("\nWrong Position [X:%d Y:%d]\n");
    MessageBox(g_pApp->m_hWnd, "Wrong Character Information.", "Error", MB_SYSTEMMODAL);
    PostMessage(g_pApp->m_hWnd, 16, 0, 0);
    return 0;
}

void BASE_GetHitPosition(int sx, int sy, int* tx, int* ty, char* pHeight, int MH)
{
    if ((sx == *tx && sy == *ty) || !sx || !sy || !*tx || !*ty)
        return;

    int dx = sx <= *tx ? *tx - sx : sx - *tx;
    int dy = sy <= *ty ? *ty - sy : sy - *ty;
    int dis = BASE_GetDistance(sx, sy, *tx, *ty);

    if (dis <= 0)
        return;

    if (dis > 30)
    {
        *tx = 0;
        *ty = 0;
        return;
    }

    if (dx > dy)
    {
        if (*tx == sx)
            return;
        int a = 1000 * (*ty - sy) / (*tx - sx);
        int b = 1000 * sy - sx * a;
        int dir = sx >= *tx ? -1 : 1;

        int sxa = dir + sx;
        int This = pHeight[sxa + g_HeightWidth * ((b + sxa * a) / 1000 - g_HeightPosY) - g_HeightPosX];
        if (This == 127)
        {
            *tx = 0;
            *ty = 0;
            return;
        }

        int leng = dx;
        for (int x = sxa; x != *tx; x += dir)
        {
            if (x != sxa)
            {
                int Last = This;
                This = pHeight[x + g_HeightWidth * ((b + x * a) / 1000 - g_HeightPosY) - g_HeightPosX];
                if (This == 127)
                {
                    *tx = 0;
                    *ty = 0;
                    return;
                }
                if (This > MH + Last || This < Last - MH)
                {
                    *tx = x;
                    *ty = (b + x * a) / 1000;
                    return;
                }
                if (--leng < 1)
                    return;
            }
            else if (--leng < 1)
            {
                return;
            }
        }

        return;
    }
    if (*ty != sy)
    {
        int a = 1000 * (*tx - sx) / (*ty - sy);
        int b = 1000 * sx - sy * a;
        int dir = sy >= *ty ? -1 : 1;

        int sya = dir + sy;

        int This = pHeight[(b + sya * a) / 1000 + g_HeightWidth * (sya - g_HeightPosY) - g_HeightPosX];
        if (This == 127)
        {
            *tx = 0;
            *ty = 0;
            return;
        }

        int leng = dy;
        for (int y = sya; y != *ty; y += dir)
        {
            if (y != sya)
            {
                int xp = (b + y * a) / 1000;
                int Last = This;
                This = pHeight[xp + g_HeightWidth * (y - g_HeightPosY) - g_HeightPosX];
                if (This == 127)
                {
                    *tx = 0;
                    *ty = 0;
                    return;
                }
                if (This > MH + Last || This < Last - MH)
                {
                    *tx = xp;
                    *ty = y;
                    return;
                }
                if (--leng < 1)
                    return;
            }
            else if (--leng < 1)
            {
                return;
            }
        }

        return;
    }
}

int BASE_Get3DTo2DPos(float fX, float fY, float fZ, int* pX, int* pY)
{
    D3DXVECTOR3 vTemp;
    D3DXVECTOR3 vPosTransformed;
    D3DXVECTOR3 vecPos;

    vecPos.x = fX;
    vecPos.y = fY;
    vecPos.z = fZ;

    D3DXVec3TransformCoord(&vTemp, &vecPos, &g_pDevice->m_matView);
    D3DXVec3TransformCoord(&vPosTransformed, &vTemp, &g_pDevice->m_matProj);

    if (vPosTransformed.z < 0.0f)
        return 0;
    if (vPosTransformed.z >= 1.0f)
        return 0;

    int vPosInX = (int)(((vPosTransformed.x + 1.0f) * (float)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift)) / 2.0f);
    int vPosInY = (int)(((-vPosTransformed.y + 1.0f) * (float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift)) / 2.0f);

    if (vPosInX <= 0 || vPosInX >= (int)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift) ||
        vPosInY <= 0 || vPosInY >= (int)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift))
    {
        return 0;
    }

    *pX = vPosInX;
    *pY = vPosInY;
    return 1;
}

void BASE_GetHitPosition2(int sx, int sy, int* tx, int* ty, char* pHeight, int MH)
{
    if ((sx != *tx || sy != *ty) && sx && sy && *tx && *ty)
    {
        int dx = sx <= *tx ? *tx - sx : sx - *tx;
        int dy = sy <= *ty ? *ty - sy : sy - *ty;
        int This = pHeight[sx + g_HeightWidth * (sy - g_HeightPosY) - g_HeightPosX];

        if (dx > dy)
        {
            int dir = sx >= *tx ? -1 : 1;
            int leng = dx;
            for (int x = sx; x - dir != *tx; x += dir)
            {
                if (x == sx)
                {
                    if (--leng < 0)
                        return;

                    continue;
                }

                int Last = This;
                int a = 1000 * (*ty - sy) / (*tx - sx);
                This = pHeight[x + g_HeightWidth * ((1000 * sy - sx * a + x * a) / 1000 - g_HeightPosY) - g_HeightPosX];
                if (This > MH + Last || This < Last - MH)
                {
                    *tx = 0;
                    *ty = 0;
                    return;
                }
                if (--leng < 0)
                    return;
            }

            return;
        }


        int dir = sy >= *ty ? - 1 : 1;
        int leng = dy;
        for (int y = sy; y - dir != *ty; y += dir)
        {
            if (y == sy)
            {
                if (--leng < 0)
                    return;

                continue;
            }

            int xt = 1000 * (*tx - sx) / (*ty - sy);
            int xp = (1000 * sx - sy * xt + y * xt) / 1000;
            int Last = This;
            if (y - g_HeightPosY < 0 || y - g_HeightPosY >= 256 || xp - g_HeightPosX < 0 || xp - g_HeightPosX >= 256)
            {
                *tx = 0;
                *ty = 0;
                return;
            }

            This = pHeight[xp + g_HeightWidth * (y - g_HeightPosY) - g_HeightPosX];
            if (y == sy)
            {
                --leng;
                continue;
            }

            if (This > MH + Last || This < Last - MH)
            {
                *tx = 0;
                *ty = 0;
                return;
            }
            if (--leng < 0)
                return;

        }
    }
}

void BASE_SetBit(char* byte, int pos)
{
    byte[pos / 8] |= 1 << pos % 8;
}

int BASE_UpdateItem2(int maskidx, int CurrentState, int NextState, int xx, int yy, char* pHeight, int rotate, int height)
{
    constexpr int maskCount = static_cast<int>(sizeof(g_pGroundMask) / sizeof(g_pGroundMask[0]));
    constexpr int rotationCount = static_cast<int>(sizeof(g_pGroundMask[0]) / sizeof(g_pGroundMask[0][0]));
    if (maskidx < 0 || maskidx >= maskCount || rotate < 0 || rotate >= rotationCount || !pHeight)
        return 0;

    for (int y = 0; y <= 5; ++y)
    {
        for (int x = 0; x <= 5; ++x)
        {
            int xp = xx + x - 2;
            int  yp = yy + y - 2;
            if (xp - g_HeightPosX < 1 || yp - g_HeightPosY < 1 || xp - g_HeightPosX > g_HeightWidth - 2 || yp - g_HeightPosY > g_HeightHeight - 2)
                break;

            if (g_pGroundMask[maskidx][rotate][y][x])
                pHeight[xp + g_HeightWidth * (yp - g_HeightPosY) - g_HeightPosX] = height;
        }
    }

    return 1;
}
