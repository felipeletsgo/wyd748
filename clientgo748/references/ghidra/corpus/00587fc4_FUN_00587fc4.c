// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00587fc4 | Name: FUN_00587fc4


void FUN_00587fc4(void)

{
  short sVar1;
  int iVar2;
  int in_EAX;
  short *psVar3;
  short *psVar4;
  int iVar5;
  int iVar6;
  int *unaff_ESI;
  int iVar7;
  int local_2c;
  int local_24;
  short *local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_24 = unaff_ESI[3];
  iVar2 = *(int *)(*(int *)(in_EAX + 0x1a4) + 0x18);
  local_1c = *unaff_ESI;
  local_10 = unaff_ESI[4];
  local_14 = unaff_ESI[1];
  local_2c = unaff_ESI[2];
  local_18 = unaff_ESI[5];
  iVar5 = local_1c;
  if (local_1c < local_14) {
    do {
      if (local_2c <= local_24) {
        psVar4 = (short *)(*(int *)(iVar2 + iVar5 * 4) + (local_2c * 0x20 + local_10) * 2);
        iVar6 = local_10;
        local_c = local_2c;
        psVar3 = psVar4;
        do {
          for (; iVar6 <= local_18; iVar6 = iVar6 + 1) {
            sVar1 = *psVar4;
            psVar4 = psVar4 + 1;
            if (sVar1 != 0) {
              *unaff_ESI = iVar5;
              local_1c = iVar5;
              goto LAB_00588062;
            }
          }
          local_c = local_c + 1;
          psVar4 = psVar3 + 0x20;
          iVar6 = local_10;
          psVar3 = psVar4;
        } while (local_c <= local_24);
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 <= local_14);
LAB_00588062:
    iVar5 = local_14;
    if (local_1c < local_14) {
      do {
        if (local_2c <= local_24) {
          psVar4 = (short *)(*(int *)(iVar2 + iVar5 * 4) + (local_2c * 0x20 + local_10) * 2);
          iVar6 = local_10;
          local_c = local_2c;
          psVar3 = psVar4;
          do {
            for (; iVar6 <= local_18; iVar6 = iVar6 + 1) {
              sVar1 = *psVar4;
              psVar4 = psVar4 + 1;
              if (sVar1 != 0) {
                unaff_ESI[1] = iVar5;
                local_14 = iVar5;
                goto LAB_005880ca;
              }
            }
            local_c = local_c + 1;
            psVar4 = psVar3 + 0x20;
            iVar6 = local_10;
            psVar3 = psVar4;
          } while (local_c <= local_24);
        }
        iVar5 = iVar5 + -1;
      } while (local_1c <= iVar5);
    }
  }
LAB_005880ca:
  if (local_2c < local_24) {
    iVar6 = (local_2c * 0x20 + local_10) * 2;
    iVar5 = local_1c;
    local_c = local_2c;
    do {
      for (; iVar5 <= local_14; iVar5 = iVar5 + 1) {
        psVar4 = (short *)(*(int *)(iVar2 + iVar5 * 4) + iVar6);
        for (iVar7 = local_10; iVar7 <= local_18; iVar7 = iVar7 + 1) {
          sVar1 = *psVar4;
          psVar4 = psVar4 + 1;
          if (sVar1 != 0) {
            local_2c = local_c;
            unaff_ESI[2] = local_c;
            goto LAB_00588137;
          }
        }
      }
      local_c = local_c + 1;
      iVar6 = iVar6 + 0x40;
      iVar5 = local_1c;
    } while (local_c <= local_24);
LAB_00588137:
    if (local_2c < local_24) {
      iVar6 = (local_24 * 0x20 + local_10) * 2;
      iVar5 = local_1c;
      local_c = local_24;
      do {
        for (; iVar5 <= local_14; iVar5 = iVar5 + 1) {
          psVar4 = (short *)(*(int *)(iVar2 + iVar5 * 4) + iVar6);
          for (iVar7 = local_10; iVar7 <= local_18; iVar7 = iVar7 + 1) {
            sVar1 = *psVar4;
            psVar4 = psVar4 + 1;
            if (sVar1 != 0) {
              local_24 = local_c;
              unaff_ESI[3] = local_c;
              goto LAB_005881a3;
            }
          }
        }
        local_c = local_c + -1;
        iVar6 = iVar6 + -0x40;
        iVar5 = local_1c;
      } while (local_2c <= local_c);
    }
  }
LAB_005881a3:
  local_8 = local_10;
  if (local_10 < local_18) {
    do {
      local_20 = (short *)local_1c;
      if (local_1c <= local_14) {
        do {
          psVar4 = (short *)(*(int *)(iVar2 + (int)local_20 * 4) + (local_2c * 0x20 + local_8) * 2);
          for (iVar5 = local_2c; iVar5 <= local_24; iVar5 = iVar5 + 1) {
            if (*psVar4 != 0) {
              local_10 = local_8;
              unaff_ESI[4] = local_8;
              goto LAB_00588207;
            }
            psVar4 = psVar4 + 0x20;
          }
          local_20 = (short *)((int)local_20 + 1);
        } while ((int)local_20 <= local_14);
      }
      local_8 = local_8 + 1;
    } while (local_8 <= local_18);
LAB_00588207:
    local_8 = local_18;
    if (local_10 < local_18) {
      do {
        local_20 = (short *)local_1c;
        if (local_1c <= local_14) {
          do {
            psVar4 = (short *)(*(int *)(iVar2 + (int)local_20 * 4) + (local_2c * 0x20 + local_8) * 2
                              );
            for (iVar5 = local_2c; iVar5 <= local_24; iVar5 = iVar5 + 1) {
              if (*psVar4 != 0) {
                local_18 = local_8;
                unaff_ESI[5] = local_8;
                goto LAB_00588267;
              }
              psVar4 = psVar4 + 0x20;
            }
            local_20 = (short *)((int)local_20 + 1);
          } while ((int)local_20 <= local_14);
        }
        local_8 = local_8 + -1;
      } while (local_10 <= local_8);
    }
  }
LAB_00588267:
  iVar7 = (local_18 - local_10) * 8;
  iVar5 = (local_24 - local_2c) * 0xc;
  iVar6 = (local_14 - local_1c) * 0x10;
  local_c = 0;
  unaff_ESI[6] = iVar7 * iVar7 + iVar5 * iVar5 + iVar6 * iVar6;
  for (iVar5 = local_1c; iVar5 <= local_14; iVar5 = iVar5 + 1) {
    if (local_2c <= local_24) {
      psVar4 = (short *)(*(int *)(iVar2 + iVar5 * 4) + (local_2c * 0x20 + local_10) * 2);
      iVar6 = (local_24 - local_2c) + 1;
      do {
        if (local_10 <= local_18) {
          local_1c = (local_18 - local_10) + 1;
          local_20 = psVar4;
          do {
            if (*local_20 != 0) {
              local_c = local_c + 1;
            }
            local_20 = local_20 + 1;
            local_1c = local_1c + -1;
          } while (local_1c != 0);
        }
        psVar4 = psVar4 + 0x20;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
  }
  unaff_ESI[7] = local_c;
  return;
}

