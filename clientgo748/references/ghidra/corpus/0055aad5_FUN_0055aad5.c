// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055aad5 | Name: FUN_0055aad5


undefined4 FUN_0055aad5(byte *param_1,char *param_2)

{
  uint local_20;
  uint local_1c;
  uint local_18;
  uint local_c;
  uint local_8;
  
  local_8 = (uint)*param_1;
  local_c = (uint)param_1[1];
  local_18 = (uint)(byte)((byte)((local_8 & 3) << 4) | (byte)((int)local_c >> 4));
  local_1c = (uint)(byte)((byte)((local_c & 0xf) << 2) | (byte)((int)(uint)param_1[2] >> 6));
  local_20 = (uint)(param_1[2] & 0x3f);
  *param_2 = s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914[(int)local_8 >> 2];
  param_2[1] = s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914[local_18];
  param_2[2] = s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914[local_1c];
  param_2[3] = s_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef_005c4914[local_20];
  return 1;
}

