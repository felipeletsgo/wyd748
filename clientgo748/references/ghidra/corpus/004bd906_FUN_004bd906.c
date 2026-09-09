// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bd906 | Name: FUN_004bd906


undefined4 __thiscall FUN_004bd906(int *param_1,uint param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_8;
  
  if (param_2 < *(uint *)(&DAT_0065a4a4 + *param_1 * 900)) {
    if (param_1[0xb4] == param_2) {
      uVar1 = 0;
    }
    else {
      param_1[0xb5] = param_1[0xb4];
      param_1[0xb6] = (param_1[0xb3] + param_1[0xb8]) * *(int *)(&DAT_0065a790 + *param_1 * 900);
      iVar2 = (**(code **)(*DAT_0092e654 + 8))();
      param_1[0xba] = iVar2;
      param_1[0xb4] = param_2;
      param_1[0xb3] = 0;
      for (local_8 = 0; local_8 < param_1[0xb4]; local_8 = local_8 + 1) {
        param_1[0xb3] = param_1[0xb3] + *(int *)(&DAT_0065a4a8 + local_8 * 4 + *param_1 * 900);
      }
      if ((param_1[0xbd] != 0) && (*param_1 < 0x13)) {
        FUN_004c0451();
      }
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

