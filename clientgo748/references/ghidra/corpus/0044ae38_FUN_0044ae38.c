// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044ae38 | Name: FUN_0044ae38


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0044ae38(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_e0 [4];
  undefined2 local_dc;
  undefined2 local_da;
  undefined1 local_d4 [128];
  int *local_54;
  int local_50;
  int *local_4c;
  undefined4 *local_48;
  int local_44;
  int local_40;
  int local_3c;
  int *local_38;
  int *local_34;
  int local_30;
  int *local_2c;
  int *local_28;
  undefined4 local_24;
  int *local_20;
  int *local_1c;
  int *local_18;
  undefined4 local_14;
  int *local_10;
  int *local_c;
  int *local_8;
  
  DAT_005b12bc = 0xffff;
  iVar1 = FUN_0040c0f0();
  if (iVar1 == 1) {
    FUN_00447594();
  }
  local_34 = *(int **)(param_1 + 0x27b38);
  if (local_34 != (int *)0x0) {
    local_18 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x139);
    local_2c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x29c);
    local_24 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x259);
    local_14 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x269);
    local_c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x126);
    local_20 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x127);
    local_8 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x125);
    local_28 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x32c);
    local_1c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x32d);
    local_10 = *(int **)(param_1 + 0x27c84);
    if (local_2c != (int *)0x0) {
      (**(code **)(*local_2c + 0x60))(0);
    }
    local_30 = 0;
    if (local_18 != (int *)0x0) {
      (**(code **)(*local_18 + 0x8c))(param_2);
    }
    if (local_34 != (int *)0x0) {
      iVar1 = FUN_0040c0f0();
      if (((iVar1 == 1) && (param_2 == 0)) && (iVar1 = FUN_0040c0f0(), iVar1 == 0)) {
        local_30 = 1;
      }
      (**(code **)(*local_34 + 0x60))(param_2);
      if (DAT_005b892c == 2) {
        (**(code **)(*local_34 + 0x70))
                  ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                   *(float *)(*(int *)(param_1 + 0x27b38) + 0x54) * _DAT_005a34a0,
                   (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                   *(float *)(*(int *)(param_1 + 0x27b38) + 0x58) * _DAT_005a34a0);
      }
    }
    if (param_2 == 1) {
      (**(code **)(*DAT_005ccec0 + 0x98))();
      (**(code **)(**(int **)(param_1 + 0x27c70) + 0x60))(0);
      (**(code **)(**(int **)(param_1 + 0x27c7c) + 0x60))(0);
      (**(code **)(*local_10 + 0x60))(0);
      (**(code **)(**(int **)(param_1 + 0x285dc) + 0x60))(0);
      (**(code **)(*local_c + 0x8c))(0);
      (**(code **)(*local_20 + 0x8c))(0);
      (**(code **)(*local_8 + 0x8c))(0);
      FUN_0044b890(0);
      if (DAT_005b892c != 2) {
        uVar2 = __ftol();
        *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar2;
        FUN_004012c6((float)*(uint *)(DAT_005ccf9c + 0x2a504) -
                     (float)*(int *)(DAT_005ccf9c + 0x2a5e4),local_34[0x14]);
      }
      local_38 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x29b);
      if (param_3 == 1) {
        if (DAT_005b892c == 2) {
          (**(code **)(**(int **)(param_1 + 0x285d8) + 0x70))
                    ((float)local_34[0x15] * _DAT_005a34a0 +
                     (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 + _DAT_005a3670,
                     (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                     *(float *)(*(int *)(param_1 + 0x285d8) + 0x58) * _DAT_005a34a0);
        }
        else {
          (**(code **)(**(int **)(param_1 + 0x285d8) + 0x70))(0x44048000,0);
        }
        (**(code **)(**(int **)(param_1 + 0x285d8) + 0x60))(1);
        (**(code **)(**(int **)(param_1 + 0x27b34) + 0x60))(0);
        if (local_38 != (int *)0x0) {
          (**(code **)(*local_38 + 0x60))(1);
        }
        if (local_28 != (int *)0x0) {
          (**(code **)(*local_28 + 0x60))(1);
        }
        if (local_1c != (int *)0x0) {
          (**(code **)(*local_1c + 0x60))(1);
        }
        for (local_3c = 0; local_3c < 0xc; local_3c = local_3c + 1) {
          if (*(int *)(param_1 + 0x27b3c + local_3c * 4) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x27b3c + local_3c * 4) + 0x400) = 9;
          }
        }
      }
      else {
        (**(code **)(**(int **)(param_1 + 0x285d8) + 0x60))(0);
        if (local_28 != (int *)0x0) {
          (**(code **)(*local_28 + 0x60))(0);
        }
        if (local_1c != (int *)0x0) {
          (**(code **)(*local_1c + 0x60))(0);
        }
        (**(code **)(**(int **)(param_1 + 0x27b34) + 0x60))(1);
        if (local_38 != (int *)0x0) {
          (**(code **)(*local_38 + 0x60))(0);
        }
        for (local_40 = 0; local_40 < 0xc; local_40 = local_40 + 1) {
          if (*(int *)(param_1 + 0x27b3c + local_40 * 4) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x27b3c + local_40 * 4) + 0x400) = 0xb;
          }
        }
        *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 10;
        FUN_00447f6f(0);
      }
      *(undefined4 *)(*(int *)(param_1 + 0x28738) + 0x400) = 10;
    }
    else {
      FUN_0058f078(*(int *)(param_1 + 0x4c) + 0x724,&DAT_005d043c);
      (**(code **)(**(int **)(*(int *)(param_1 + 0x4c) + 0x424) + 0x80))
                (*(int *)(param_1 + 0x4c) + 0x724,0);
      iVar1 = FUN_0040c0f0();
      if (iVar1 == 1) {
        *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 0;
        FUN_00447f6f(1);
      }
      for (local_44 = 0; local_44 < 0xc; local_44 = local_44 + 1) {
        FUN_0058f078(local_d4,&DAT_005b7d44,0);
        local_54 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_44 + 800);
        *(undefined1 *)((int)local_54 + 0xcdd) = 1;
        (**(code **)(*local_54 + 0x80))(local_d4,0);
        *(undefined2 *)(param_1 + 0x274f2) = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
        *(undefined2 *)(param_1 + 0x275ae) = 0;
        local_4c = *(int **)(param_1 + 0x27b3c + local_44 * 4);
        local_48 = (undefined4 *)(**(code **)(*local_4c + 0xa8))(0,0);
        local_50 = (**(code **)(**(int **)(param_1 + 0x28738) + 0xb8))
                             ((int)*(char *)(param_1 + local_44 + 0x27570) % 9,
                              (int)*(char *)(param_1 + local_44 + 0x27570) / 9);
        if (local_50 != 0) {
          *(undefined4 *)(local_50 + 0x94) = 0xffffffff;
        }
        if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_48)) {
          DAT_005ccec0[0x7a] = 0;
        }
        if (local_48 != (undefined4 *)0x0) {
          if (local_48 != (undefined4 *)0x0) {
            (**(code **)*local_48)(1);
          }
          local_48 = (undefined4 *)0x0;
        }
      }
      if (DAT_005b892c == 2) {
        (**(code **)(**(int **)(param_1 + 0x285d8) + 0x70))
                  ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                   *(float *)(*(int *)(param_1 + 0x285d8) + 0x54) * _DAT_005a34a0,
                   (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                   *(float *)(*(int *)(param_1 + 0x285d8) + 0x58) * _DAT_005a34a0);
      }
      else {
        (**(code **)(**(int **)(param_1 + 0x285d8) + 0x70))(0x438c0000,0);
      }
      (**(code **)(**(int **)(param_1 + 0x27b34) + 0x60))(0);
      (**(code **)(**(int **)(param_1 + 0x285d8) + 0x60))(0);
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x28738) + 0x400) = 2;
      _memset((void *)(param_1 + 0x274ec),0,0xc4);
      if (local_30 == 1) {
        _memset(local_e0,0,0xc);
        local_dc = 900;
        local_da = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
        FUN_0055f2dd(local_e0,0xc);
      }
    }
  }
  return;
}

