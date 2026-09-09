// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00575739 | Name: FUN_00575739


void FUN_00575739(int param_1,undefined4 param_2,undefined4 param_3)

{
  if (*(code **)(param_1 + 0x50) != (code *)0x0) {
    (**(code **)(param_1 + 0x50))(param_1,param_2,param_3);
    return;
  }
  FUN_005733d1(param_1,"Call to NULL read function");
  return;
}

