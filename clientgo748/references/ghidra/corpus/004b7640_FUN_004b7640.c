// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7640 | Name: FUN_004b7640


int __thiscall FUN_004b7640(int param_1,int param_2)

{
  FUN_004b6330(param_2);
  FUN_004b6330(param_2 + 0x10);
  *(undefined4 *)(param_1 + 0x20) = *(undefined4 *)(param_2 + 0x20);
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_2 + 0x24);
  return param_1;
}

