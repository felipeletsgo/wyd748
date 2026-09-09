// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052c5eb | Name: FUN_0052c5eb


undefined4 __thiscall FUN_0052c5eb(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_d8 [32];
  undefined4 local_b8;
  undefined1 local_b4 [32];
  undefined4 local_94;
  undefined1 local_90 [32];
  undefined1 local_70 [32];
  undefined1 local_50 [32];
  undefined1 local_30 [32];
  int local_10;
  int local_c;
  int local_8;
  
  local_8 = param_2;
  iVar1 = FUN_00494dcf();
  if (iVar1 == 30000) {
    *(undefined2 *)(param_1 + 0x450) = *(undefined2 *)(local_8 + 0xc);
    *(undefined2 *)(param_1 + 0x452) = *(undefined2 *)(local_8 + 0xe);
    local_c = DAT_0067cf38;
    local_10 = DAT_0067cf38;
    if (((*(char *)(DAT_0067cf38 + 0x26eb4) == '\x01') &&
        (param_1 == *(int *)(DAT_0067cf38 + 0x26e90))) && (*(short *)(local_8 + 0xc) < 1)) {
      *(undefined4 *)(DAT_0067cf38 + 0x26e90) = 0;
    }
    if (*(short *)(param_1 + 0x44c) < *(short *)(param_1 + 0x450)) {
      *(undefined2 *)(param_1 + 0x450) = *(undefined2 *)(param_1 + 0x44c);
    }
    (**(code **)(**(int **)(param_1 + 0x434) + 0x88))((int)*(short *)(param_1 + 0x450));
    FUN_004ff37b((int)*(short *)(param_1 + 0x450));
    FUN_004ff365();
    if ((param_1 == *(int *)(DAT_0067cf38 + 0x4c)) && (*(int *)(local_10 + 0x26e1c) == 0)) {
      *(undefined2 *)(local_10 + 0x26e80) = *(undefined2 *)(local_8 + 0x10);
      *(undefined2 *)(local_10 + 0x26e82) = *(undefined2 *)(local_8 + 0x12);
      FUN_0058f220(DAT_013b71e8 + 0x72c,param_1 + 0x444,0x1c);
      if (*(int *)(local_10 + 0x27acc) != 0) {
        (**(code **)(**(int **)(local_10 + 0x27acc) + 0x88))((int)*(short *)(param_1 + 0x450));
      }
      if (*(int *)(local_10 + 0x27ad0) != 0) {
        (**(code **)(**(int **)(local_10 + 0x27ad0) + 0x88))((int)*(short *)(param_1 + 0x452));
      }
      if (*(int *)(local_10 + 0x27ad8) != 0) {
        (**(code **)(**(int **)(local_10 + 0x27ad8) + 0x88))((int)*(short *)(local_10 + 0x26e80));
      }
      if (*(int *)(local_10 + 0x27adc) != 0) {
        (**(code **)(**(int **)(local_10 + 0x27adc) + 0x88))((int)*(short *)(local_10 + 0x26e82));
      }
      if (((DAT_005b892c == 2) && (*(int *)(param_1 + 0x438) != 0)) &&
         ((*(int *)(local_10 + 0x27ad4) != 0 && (*(int *)(local_10 + 0x27ae0) != 0)))) {
        uVar2 = (**(code **)(**(int **)(param_1 + 0x438) + 0x90))();
        (**(code **)(**(int **)(local_10 + 0x27ad4) + 0x88))(uVar2);
      }
      if ((*(int *)(local_10 + 0x27ab0) != 0) &&
         (FUN_0058f078(local_30,&DAT_005bb828,DAT_013c1f14), *(int *)(local_10 + 0x27ab0) != 0)) {
        (**(code **)(**(int **)(local_10 + 0x27ab0) + 0x80))(local_30,0);
      }
      if ((*(int *)(local_10 + 0x27ab4) != 0) &&
         (FUN_0058f078(local_50,&DAT_005bb82c,DAT_013c1f0c), *(int *)(local_10 + 0x27ab4) != 0)) {
        (**(code **)(**(int **)(local_10 + 0x27ab4) + 0x80))(local_50,0);
      }
      if ((*(int *)(local_10 + 0x27ab8) != 0) &&
         (FUN_0058f078(local_70,&DAT_005bb830,DAT_013c1f18), *(int *)(local_10 + 0x27ab8) != 0)) {
        (**(code **)(**(int **)(local_10 + 0x27ab8) + 0x80))(local_70,0);
      }
      if ((*(int *)(local_10 + 0x27abc) != 0) &&
         (FUN_0058f078(local_90,&DAT_005bb834,DAT_013c1f10), *(int *)(local_10 + 0x27abc) != 0)) {
        (**(code **)(**(int **)(local_10 + 0x27abc) + 0x80))(local_90,0);
      }
      if ((*(int *)(local_10 + 0x27ac0) != 0) && (*(int *)(param_1 + 0x438) != 0)) {
        local_94 = (**(code **)(**(int **)(param_1 + 0x438) + 0x90))();
        if (*(char *)(param_1 + 0x79c) == '\0') {
          local_94 = 0;
        }
        FUN_0058f078(local_b4,&DAT_005bb838,local_94);
        (**(code **)(**(int **)(local_10 + 0x27ac0) + 0x80))(local_b4,0);
      }
      if ((*(int *)(local_10 + 0x27ac4) != 0) && (*(int *)(param_1 + 0x438) != 0)) {
        local_b8 = (**(code **)(**(int **)(param_1 + 0x438) + 0x94))();
        if (*(char *)(param_1 + 0x79c) == '\0') {
          local_b8 = 0;
        }
        FUN_0058f078(local_d8,&DAT_005bb83c,local_b8);
        (**(code **)(**(int **)(local_10 + 0x27ac4) + 0x80))(local_d8,0);
      }
    }
    if ((0 < *(short *)(param_1 + 0x450)) &&
       ((*(char *)(param_1 + 0x245) == '\x01' || (*(int *)(param_1 + 0x740) == 0xc)))) {
      *(undefined1 *)(param_1 + 0x245) = 0;
      FUN_0051dba4(0xe,0);
    }
  }
  return 1;
}

