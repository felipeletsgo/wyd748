// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005588e2 | Name: FUN_005588e2


void FUN_005588e2(byte *param_1,byte param_2)

{
  *param_1 = *param_1 | (byte)(1 << (param_2 & 0x1f));
  return;
}

