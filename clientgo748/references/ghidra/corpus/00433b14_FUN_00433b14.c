// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00433b14 | Name: FUN_00433b14


int FUN_00433b14(short *param_1)

{
  uint local_18;
  uint local_14;
  uint local_10;
  uint local_c;
  uint local_8;
  
  local_c = (uint)(ushort)(*param_1 * -0x1700);
  local_14 = (uint)(ushort)(param_1[1] * -0x5740);
  local_8 = (uint)(ushort)(param_1[3] * 0x5a0);
  local_18 = (uint)(ushort)(param_1[4] * 0x3c);
  local_10 = (uint)(ushort)param_1[5];
  return local_c + local_14 + local_8 + local_18 + local_10;
}

