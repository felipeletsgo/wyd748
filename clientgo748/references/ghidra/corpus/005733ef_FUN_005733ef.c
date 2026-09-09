// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005733ef | Name: FUN_005733ef


void FUN_005733ef(int param_1,undefined4 param_2)

{
  if (*(code **)(param_1 + 0x44) != (code *)0x0) {
    (**(code **)(param_1 + 0x44))(param_1,param_2);
  }
  return;
}

