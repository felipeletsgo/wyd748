// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052142d | Name: FUN_0052142d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_0052142d(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,int *param_7,
            undefined4 param_8,int param_9,int param_10)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  float10 fVar6;
  undefined1 *puVar7;
  float fVar8;
  undefined1 *puVar9;
  float fVar10;
  int local_c0;
  int local_bc;
  int local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  int local_a8;
  int local_a4;
  int local_a0;
  undefined1 local_90 [8];
  undefined1 local_88 [32];
  int local_68;
  int local_64;
  undefined4 local_60;
  float local_5c;
  undefined4 local_58;
  int local_54;
  int local_50;
  int local_4c;
  int local_48;
  undefined4 local_44;
  undefined4 local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  float local_24;
  float local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    if ((param_2 == param_4) && (param_3 == param_5)) {
      uVar1 = 0;
    }
    else {
      local_50 = DAT_0067cf38;
      if (*(int *)(DAT_0067cf38 + 0x4c) == param_1) {
        local_60 = 0;
      }
      local_8 = FUN_005563a0(param_2,param_3,param_4,param_5);
      FUN_00430f10();
      iVar2 = __ftol();
      if ((iVar2 == param_4) && (iVar2 = __ftol(), iVar2 == param_5)) {
        uVar1 = 0;
      }
      else {
        local_24 = *(float *)(param_1 + 0x28);
        local_20 = *(float *)(param_1 + 0x2c);
        FUN_00430f10();
        uVar1 = FUN_00430f20((float)(param_4 - param_2),(float)(param_5 - param_3));
        thunk_FUN_00560fa0(&local_44,uVar1);
        uVar1 = FUN_00430f20(((float)param_4 + _DAT_005a34a0) - local_24,
                             ((float)param_5 + _DAT_005a34a0) - local_20);
        fVar6 = (float10)FUN_0052fbc0(uVar1);
        local_5c = (float)fVar6;
        FUN_0040bd30(&local_24);
        local_58 = 0;
        local_1c = 0x30;
        iVar2 = __ftol();
        iVar3 = __ftol();
        local_18 = 1;
        if (param_4 - iVar2 < 0) {
          local_18 = -1;
        }
        local_3c = 1;
        if (param_5 - iVar3 < 0) {
          local_3c = -1;
        }
        local_a0 = iVar2;
        if (param_4 < iVar2) {
          local_a0 = param_4;
        }
        local_2c = local_a0;
        local_a4 = iVar3;
        if (param_5 < iVar3) {
          local_a4 = param_5;
        }
        local_38 = local_a4;
        local_a8 = iVar2;
        if (iVar2 <= param_4) {
          local_a8 = param_4;
        }
        local_48 = local_a8;
        local_ac = iVar3;
        if (iVar3 <= param_5) {
          local_ac = param_5;
        }
        local_4c = local_ac;
        local_30 = (int)*(char *)(param_9 +
                                 (((iVar3 - DAT_005b8928) * DAT_005b891c + iVar2) - DAT_005b8924));
        for (local_28 = local_a4; local_28 < local_ac; local_28 = local_28 + 1) {
          for (local_64 = local_a0; local_64 < local_a8; local_64 = local_64 + 1) {
            local_68 = (int)*(char *)(param_9 +
                                     (((local_28 - DAT_005b8928) * DAT_005b891c + local_64) -
                                     DAT_005b8924));
            if (local_68 == local_30 || local_68 - local_30 < 0) {
              local_b0 = -(local_68 - local_30);
            }
            else {
              local_b0 = local_68 - local_30;
            }
            if (param_10 < local_b0) {
              return 0;
            }
          }
        }
        for (local_34 = 1; local_34 < 0x30; local_34 = local_34 + 1) {
          FUN_0040bd30(&local_24);
          if (local_34 < local_1c) {
            fVar10 = (float)local_8;
            puVar9 = local_90;
            puVar7 = local_88;
            fVar8 = local_5c;
            FUN_00430f20(local_44,local_40);
            FUN_0052fb00(puVar7,fVar8);
            uVar1 = FUN_0052fc30(puVar9,fVar10);
            FUN_0052fbf0(uVar1);
            iVar2 = __ftol();
            iVar3 = __ftol();
            iVar4 = __ftol();
            iVar5 = __ftol();
            if (iVar4 == iVar5) {
              iVar4 = __ftol();
              iVar5 = __ftol();
              if (iVar4 != iVar5) goto LAB_00521807;
            }
            else {
LAB_00521807:
              local_30 = (int)*(char *)(param_9 +
                                       (((iVar3 - DAT_005b8928) * DAT_005b891c + iVar2) -
                                       DAT_005b8924));
              iVar4 = __ftol();
              if (iVar4 != iVar2 - local_18) {
                local_10 = (int)*(char *)(param_9 +
                                         ((((iVar3 - DAT_005b8928) * DAT_005b891c + iVar2) -
                                          local_18) - DAT_005b8924));
                if (local_30 == local_10 || local_30 - local_10 < 0) {
                  local_b4 = -(local_30 - local_10);
                }
                else {
                  local_b4 = local_30 - local_10;
                }
                if (param_10 + -2 <= local_b4) {
                  return 0;
                }
              }
              iVar4 = __ftol();
              if (iVar4 != iVar3 - local_3c) {
                local_14 = (int)*(char *)(param_9 +
                                         ((((iVar3 - local_3c) - DAT_005b8928) * DAT_005b891c +
                                          iVar2) - DAT_005b8924));
                if (local_30 == local_14 || local_30 - local_14 < 0) {
                  local_b8 = -(local_30 - local_14);
                }
                else {
                  local_b8 = local_30 - local_14;
                }
                if (param_10 + -2 <= local_b8) {
                  return 0;
                }
              }
              iVar4 = __ftol();
              if ((iVar4 != iVar3 - local_3c) && (iVar4 = __ftol(), iVar4 != iVar2 - local_18)) {
                local_54 = (int)*(char *)(param_9 +
                                         (((((iVar3 - local_3c) - DAT_005b8928) * DAT_005b891c +
                                           iVar2) - local_18) - DAT_005b8924));
                if (local_30 == local_54 || local_30 - local_54 < 0) {
                  local_bc = -(local_30 - local_54);
                }
                else {
                  local_bc = local_30 - local_54;
                }
                if (param_10 + -2 <= local_bc) {
                  return 0;
                }
              }
              if (local_34 != 0) {
                iVar2 = __ftol();
                iVar3 = (iVar2 - DAT_005b8928) * DAT_005b891c;
                iVar2 = __ftol();
                local_c = (int)*(char *)(param_9 + ((iVar3 + iVar2) - DAT_005b8924));
                if (local_30 == local_c || local_30 - local_c < 0) {
                  local_c0 = -(local_30 - local_c);
                }
                else {
                  local_c0 = local_30 - local_c;
                }
                if (param_10 + -2 <= local_c0) {
                  return 0;
                }
              }
            }
            if (local_8 <= local_34) {
              local_1c = local_34;
              *(float *)(param_6 + local_34 * 8) = (float)param_4 + _DAT_005a34a0;
              *(float *)(param_6 + 4 + local_34 * 8) = (float)param_5 + _DAT_005a34a0;
            }
          }
          FUN_0040bd30(param_6 + local_34 * 8);
        }
        if (param_7 != (int *)0x0) {
          *param_7 = local_1c;
        }
        *(undefined1 *)(param_1 + 0x774) = 2;
        uVar1 = 1;
      }
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

