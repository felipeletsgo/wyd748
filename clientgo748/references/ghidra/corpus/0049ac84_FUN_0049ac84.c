// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049ac84 | Name: FUN_0049ac84


undefined4 __thiscall FUN_0049ac84(int param_1,undefined1 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  int iVar2;
  
  if (DAT_0067cf38 == param_1) {
    if ((*(int *)(param_1 + 0x28) != 0) &&
       (iVar2 = (**(code **)(**(int **)(param_1 + 0x28) + 0x14))
                          (CONCAT31((int3)((uint)DAT_0067cf38 >> 8),param_2),param_3), iVar2 == 1))
    {
      return 1;
    }
    uVar1 = FUN_0054ab66(param_2,param_3);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

