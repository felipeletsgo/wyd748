// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004081a9 | Name: FUN_004081a9


void __thiscall
FUN_004081a9(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            ,undefined4 param_6)

{
  (**(code **)(**(int **)(param_1 + 0xe50) + 0x58))(param_2,param_3,param_4,param_5,param_6);
  (**(code **)(**(int **)(param_1 + 0xe54) + 0x58))(param_2,param_3,param_4,param_5,param_6);
  (**(code **)(**(int **)(param_1 + 0xe58) + 0x58))(param_2,param_3,param_4,param_5,param_6);
  (**(code **)(**(int **)(param_1 + 0xe5c) + 0x58))(param_2,param_3,param_4,param_5,param_6);
  FUN_00407281(param_2,param_3,param_4,param_5,param_6);
  return;
}

