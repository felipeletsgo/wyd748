// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00407c03 | Name: FUN_00407c03


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_00407c03(int param_1,undefined4 param_2,float param_3,float param_4,undefined4 param_5,
            undefined4 param_6)

{
  uint uVar1;
  
  (**(code **)(**(int **)(param_1 + 0xe60) + 0x58))(param_2,param_3,param_4,param_5,param_6);
  (**(code **)(**(int **)(param_1 + 0xe68) + 0x58))(param_2,param_3,param_4,param_5,param_6);
  (**(code **)(**(int **)(param_1 + 0xe64) + 0x58))(param_2,param_3,param_4,param_5,param_6);
  param_3 = param_3 - _DAT_005a3834;
  param_4 = param_4 - _DAT_005a3660;
  if (*(int *)(param_1 + 0xe5c) == 1) {
    uVar1 = (**(code **)(*DAT_0092e654 + 8))();
    if ((int)(((ulonglong)uVar1 % 1000) / 500) == 0) {
      *(undefined4 *)(param_1 + 0x754) = 0xffffffff;
      FUN_00421b8f(param_1 + 0x76a,*(undefined4 *)(param_1 + 0x754),0);
    }
    else {
      *(undefined4 *)(param_1 + 0x754) = 0xffff0000;
      FUN_00421b8f(param_1 + 0x76a,*(undefined4 *)(param_1 + 0x754),0);
    }
  }
  FUN_00407281(param_2,param_3,param_4,param_5,param_6);
  return;
}

