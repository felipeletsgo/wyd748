// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005840da | Name: FUN_005840da


void FUN_005840da(int *param_1)

{
  *(undefined4 *)(*param_1 + 0x14) = 0x30;
  (**(code **)*param_1)(param_1);
  return;
}

