// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049ab73 | Name: FUN_0049ab73


undefined4 __thiscall
FUN_0049ab73(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            )

{
  int iVar1;
  
  if (DAT_0067cf38 == param_1) {
    if ((*(int *)(param_1 + 0x28) != 0) &&
       (iVar1 = (**(code **)(**(int **)(param_1 + 0x28) + 8))(param_2,param_3,param_4,param_5),
       iVar1 == 1)) {
      return 1;
    }
    FUN_0054ab39(param_2,param_3,param_4,param_5);
  }
  return 0;
}

