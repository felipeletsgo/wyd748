// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005225c0 | Name: FUN_005225c0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_005225c0(int param_1,int param_2,int param_3)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  int local_11c;
  undefined1 local_fc [32];
  undefined4 local_dc;
  undefined1 local_d8 [32];
  undefined4 local_b8;
  undefined1 local_b4 [32];
  undefined1 local_94 [32];
  undefined1 local_74 [32];
  undefined1 local_54 [32];
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  float local_24;
  int local_20;
  int local_1c;
  float local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2166;
  local_10 = ExceptionList;
  if ((*(int *)(param_1 + 0x214) == 0) && (param_3 != 0)) {
    if (*(short *)(param_1 + 0x450) < 1) {
      local_18 = (float)param_2;
    }
    else {
      local_18 = (float)param_2 / (float)(int)*(short *)(param_1 + 0x450);
    }
    ExceptionList = &local_10;
    local_14 = FUN_004f9a46();
    if (((_DAT_005a4158 < local_18) ||
        ((iVar1 = _rand(), iVar1 % 10 == 0 && (iVar1 = __ftol(), 0 < iVar1)))) &&
       (0 < *(short *)(param_1 + 0x450))) {
      if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
        local_1c = (**(code **)(*DAT_0092e654 + 8))();
        if ((((*(int *)(param_1 + 0x740) < 4) || (9 < *(int *)(param_1 + 0x740))) ||
            (iVar1 = _rand(), iVar1 % 5 == 0)) &&
           (600 < (uint)(local_1c - *(int *)(param_1 + 0x600)))) {
          FUN_0051dba4(10,0);
        }
      }
      else {
        FUN_0051dba4(10,0);
      }
    }
    if (_DAT_005a4214 < local_18) {
      local_24 = *(float *)(&DAT_0092e7d4 + *(int *)(param_1 + 0x5c) * 8) *
                 *(float *)(param_1 + 0x38) + _DAT_005a45d4;
      pvVar2 = operator_new(0x180);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_11c = 0;
      }
      else {
        local_11c = FUN_004c8f70(local_14,600,_DAT_005a4214 * local_24,_DAT_005a4214 * local_24,
                                 _DAT_005a4214 * local_24,0x3ba3d70a,1,0x50);
      }
      local_8 = 0xffffffff;
      local_20 = local_11c;
      if (local_11c != 0) {
        uVar3 = FUN_00430f50(*(undefined4 *)(param_1 + 0x28),
                             *(float *)(param_1 + 0x30) + _DAT_005a3660,
                             *(undefined4 *)(param_1 + 0x2c));
        FUN_004310a0(uVar3);
        if ((*(int *)(DAT_005ccf9c + 0x2a39c) == 1) || (*(int *)(DAT_005ccf9c + 0x2a3a0) == 1)) {
          *(undefined4 *)(local_20 + 0x6c) = 1;
        }
        if (local_14 == 0x38) {
          *(undefined4 *)(local_20 + 0x6c) = 1;
        }
        FUN_0054ac09(local_20);
      }
    }
    if (*(short *)(param_1 + 0x450) < 0) {
      *(undefined2 *)(param_1 + 0x450) = 0;
    }
    if (*(int *)(DAT_0067cf38 + 0x4c) == param_1) {
      local_30 = DAT_0067cf38;
      iVar1 = FUN_00494dcf();
      if (iVar1 == 30000) {
        local_34 = local_30;
        if (*(int *)(local_30 + 0x27ab0) != 0) {
          FUN_0058f078(local_54,&DAT_005bb7bc,(int)*(short *)(param_1 + 0x450));
          (**(code **)(**(int **)(local_34 + 0x27ab0) + 0x80))(local_54,0);
        }
        if (*(int *)(local_34 + 0x27ab4) != 0) {
          FUN_0058f078(local_74,&DAT_005bb7c0,(int)*(short *)(param_1 + 0x44c));
          (**(code **)(**(int **)(local_34 + 0x27ab4) + 0x80))(local_74,0);
        }
        if (*(int *)(local_34 + 0x27ab8) != 0) {
          FUN_0058f078(local_94,&DAT_005bb7c4,(int)*(short *)(param_1 + 0x452));
          (**(code **)(**(int **)(local_34 + 0x27ab8) + 0x80))(local_94,0);
        }
        if (*(int *)(local_34 + 0x27abc) != 0) {
          FUN_0058f078(local_b4,&DAT_005bb7c8,(int)*(short *)(param_1 + 0x44e));
          (**(code **)(**(int **)(local_34 + 0x27abc) + 0x80))(local_b4,0);
        }
        if ((*(int *)(local_34 + 0x27ac0) != 0) && (*(int *)(param_1 + 0x438) != 0)) {
          local_b8 = (**(code **)(**(int **)(param_1 + 0x438) + 0x90))();
          if (*(char *)(param_1 + 0x79c) == '\0') {
            local_b8 = 0;
          }
          FUN_0058f078(local_d8,&DAT_005bb7cc,local_b8);
          (**(code **)(**(int **)(local_34 + 0x27ac0) + 0x80))(local_d8,0);
        }
        if ((*(int *)(local_34 + 0x27ac4) != 0) && (*(int *)(param_1 + 0x438) != 0)) {
          local_dc = (**(code **)(**(int **)(param_1 + 0x438) + 0x94))();
          if (*(char *)(param_1 + 0x79c) == '\0') {
            local_dc = 0;
          }
          FUN_0058f078(local_fc,&DAT_005bb7d0,local_dc);
          (**(code **)(**(int **)(local_34 + 0x27ac4) + 0x80))(local_fc,0);
        }
        (**(code **)(**(int **)(local_34 + 0x27acc) + 0x8c))((int)*(short *)(param_1 + 0x44c));
        (**(code **)(**(int **)(local_34 + 0x27acc) + 0x88))((int)*(short *)(param_1 + 0x450));
        (**(code **)(**(int **)(local_34 + 0x27ad0) + 0x8c))((int)*(short *)(param_1 + 0x44e));
        (**(code **)(**(int **)(local_34 + 0x27ad0) + 0x88))((int)*(short *)(param_1 + 0x452));
        if ((((DAT_005b892c == 2) && (*(int *)(param_1 + 0x438) != 0)) &&
            (*(int *)(local_34 + 0x27ad4) != 0)) && (*(int *)(local_34 + 0x27ae0) != 0)) {
          uVar3 = (**(code **)(**(int **)(param_1 + 0x438) + 0x94))();
          (**(code **)(**(int **)(local_34 + 0x27ad4) + 0x8c))(uVar3);
          uVar3 = (**(code **)(**(int **)(param_1 + 0x438) + 0x90))();
          (**(code **)(**(int **)(local_34 + 0x27ad4) + 0x88))(uVar3);
        }
      }
    }
    else {
      local_28 = DAT_0067cf38;
      iVar1 = FUN_00494dcf();
      if (iVar1 == 30000) {
        local_2c = local_28;
        (**(code **)(**(int **)(param_1 + 0x434) + 0x88))((int)*(short *)(param_1 + 0x450));
        FUN_004ff37b((int)*(short *)(param_1 + 0x450));
        FUN_004ff365();
        if (*(int *)(local_2c + 0x27aac) != 0) {
          (**(code **)(**(int **)(local_2c + 0x27aac) + 0x80))(param_1 + 0x168,1);
        }
      }
    }
  }
  ExceptionList = local_10;
  return;
}

