// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578e51 | Name: FUN_00578e51


void FUN_00578e51(int param_1)

{
  (**(code **)(*(int *)(param_1 + 4) + 0x24))(param_1,1);
  *(uint *)(param_1 + 0x10) = (-(uint)(*(char *)(param_1 + 0xc) != '\0') & 100) + 100;
  return;
}

