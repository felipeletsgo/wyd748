#pragma once

#include "ProcessClientMessage.h"
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <ctime>
#include "SendFunc.h"
#include "Functions.h"
#include "wMySQL.h"

void Freeze() {

    for (int nY = 0; nY < MAX_GRIDY; nY++)
    {
        for (int nX = 0; nX < MAX_GRIDX; nX++)
        {
            int Target = pMobGrid[nY][nX];

            if (Target < MAX_USER)
                continue;

            if (pMob[Target].Mode == MOB_EMPTY || pMob[Target].MOB.CurrentScore.Hp <= FALSE)
            {
                if (pMob[Target].TargetX != 0 && pMob[Target].TargetY != 0)
                {
                    MSG_RemoveMob sm;
                    memset(&sm, 0, sizeof(MSG_RemoveMob));
                    sm.Size = sizeof(MSG_RemoveMob);

                    sm.Type = _MSG_RemoveMob;
                    sm.ID = Target;
                    sm.RemoveType = 0;

                    GridMulticast(pMob[Target].TargetX, pMob[Target].TargetY, (MSG_STANDARD*)&sm, Target < MAX_USER ? Target : 0);
                }

                pMobGrid[nY][nX] = 0;
            }
        }
    }
}