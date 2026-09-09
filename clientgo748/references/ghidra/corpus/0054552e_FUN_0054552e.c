// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054552e | Name: FUN_0054552e


undefined4 __thiscall FUN_0054552e(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  int local_1c;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  for (local_c = 0; local_c < *(int *)(param_1 + 0x48); local_c = local_c + 1) {
    local_8 = local_c;
    local_14 = *(int *)(param_1 + 0x4c + local_c * 4);
    local_10 = local_c;
    while (local_10 = local_10 + 1, local_10 < *(int *)(param_1 + 0x48)) {
      if (((local_14 != 0) && (*(int *)(param_1 + 0x4c + local_10 * 4) != 0)) &&
         (*(float *)(*(int *)(param_1 + 0x4c + local_10 * 4) + 0x2c) < *(float *)(local_14 + 0x2c)))
      {
        local_14 = *(int *)(param_1 + 0x4c + local_10 * 4);
        local_8 = local_10;
      }
    }
    *(undefined4 *)(param_1 + 0x4c + local_8 * 4) = *(undefined4 *)(param_1 + 0x4c + local_c * 4);
    *(int *)(param_1 + 0x4c + local_c * 4) = local_14;
  }
  iVar1 = FUN_0058f716(param_2,&DAT_005bdfe0);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    for (local_1c = 0; local_1c < *(int *)(param_1 + 0x48); local_1c = local_1c + 1) {
      (**(code **)(**(int **)(param_1 + 0x4c + local_1c * 4) + 0x68))(iVar1);
    }
    FUN_0058f62c(iVar1);
    uVar2 = 1;
  }
  return uVar2;
}

