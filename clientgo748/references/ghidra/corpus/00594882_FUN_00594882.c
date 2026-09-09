// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594882 | Name: FUN_00594882


undefined4 FUN_00594882(int *param_1)

{
  *param_1 = *param_1 + 4;
  return CONCAT22((short)((uint)*param_1 >> 0x10),*(undefined2 *)(*param_1 + -4));
}

