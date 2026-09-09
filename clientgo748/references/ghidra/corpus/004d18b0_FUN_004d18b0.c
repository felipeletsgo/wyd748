// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d18b0 | Name: FUN_004d18b0


int __thiscall FUN_004d18b0(int param_1,int param_2)

{
  FUN_004310a0(param_2);
  FUN_004310a0(param_2 + 0xc);
  FUN_004310a0(param_2 + 0x18);
  FUN_004310a0(param_2 + 0x24);
  *(undefined4 *)(param_1 + 0x30) = *(undefined4 *)(param_2 + 0x30);
  *(undefined4 *)(param_1 + 0x34) = *(undefined4 *)(param_2 + 0x34);
  *(undefined2 *)(param_1 + 0x38) = *(undefined2 *)(param_2 + 0x38);
  *(undefined4 *)(param_1 + 0x3c) = *(undefined4 *)(param_2 + 0x3c);
  return param_1;
}

