// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054a0f0 | Name: FUN_0054a0f0


undefined4 __fastcall FUN_0054a0f0(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if (DAT_005d03f0 == 1) {
    uVar2 = 0;
  }
  else if ((((param_1[6] == -1) || (param_1[6] == DAT_005bdef0)) || (param_1[6] == DAT_005bdef4)) ||
          ((param_1[6] == DAT_005bdef8 || (param_1[6] == DAT_005bdefc)))) {
    if ((param_1[0x18] != 0) && (param_1[0x12] == 1)) {
      if (DAT_005ccf9c[0xa944] == 0x20) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xaa);
      }
      else {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xff);
      }
      iVar1 = DAT_0067cf38;
      (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
      if ((*(int *)(iVar1 + 0x24) == 30000) &&
         (((param_1[9] == 0x162 || (param_1[9] == 0x179)) || (param_1[9] == 0x17a)))) {
        if ((*(char *)(iVar1 + 0x26ebc) == '\a') && (param_1[9] != 0x17a)) {
          if (param_1[0x18] != 0) {
            if (param_1[0x18] != 0) {
              FUN_00493d20(1);
            }
            param_1[0x18] = 0;
          }
          FUN_00549dd6(0x17a);
          (**(code **)(*param_1 + 0x38))();
          (**(code **)(*param_1 + 0x40))(0,param_1[0xd],0);
          (**(code **)(*param_1 + 0x3c))(param_1[10],param_1[0xc],param_1[0xb]);
        }
        else if ((*(char *)(iVar1 + 0x26ebc) == '\b') && (param_1[9] != 0x179)) {
          if (param_1[0x18] != 0) {
            if (param_1[0x18] != 0) {
              FUN_00493d20(1);
            }
            param_1[0x18] = 0;
          }
          FUN_00549dd6(0x179);
          (**(code **)(*param_1 + 0x38))();
          (**(code **)(*param_1 + 0x40))(0,param_1[0xd],0);
          (**(code **)(*param_1 + 0x3c))(param_1[10],param_1[0xc],param_1[0xb]);
        }
        else if (((*(char *)(iVar1 + 0x26ebc) != '\a') && (*(char *)(iVar1 + 0x26ebc) != '\b')) &&
                (param_1[9] != 0x162)) {
          if (param_1[0x18] != 0) {
            if (param_1[0x18] != 0) {
              FUN_00493d20(1);
            }
            param_1[0x18] = 0;
          }
          FUN_00549dd6(0x162);
          (**(code **)(*param_1 + 0x38))();
          (**(code **)(*param_1 + 0x40))(0,param_1[0xd],0);
          (**(code **)(*param_1 + 0x3c))(param_1[10],param_1[0xc],param_1[0xb]);
        }
      }
      FUN_004be1ef(0,0x3f800000,0);
    }
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

