// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040104c | Name: FUN_0040104c


undefined4 * __thiscall
FUN_0040104c(undefined4 *param_1,float param_2,float param_3,float param_4,float param_5)

{
  float local_c;
  float local_8;
  
  FUN_0054a9e0(0);
  param_1[9] = 0;
  param_1[10] = 1;
  param_1[0xb] = 1;
  param_1[0xc] = 0;
  param_1[0xd] = 0;
  param_1[0xe] = 0;
  param_1[0xf] = 1;
  param_1[0x10] = 0xffffffff;
  param_1[0x11] = 0;
  param_1[0x17] = 0;
  param_1[0x18] = 0;
  *param_1 = &PTR_FUN_005a3420;
  if (DAT_005b892c == 2) {
    local_8 = 1.0;
    local_c = 1.0;
  }
  else {
    local_c = DAT_005b490c;
    local_8 = DAT_005b4910;
  }
  param_1[0x13] = param_2 * local_c;
  param_1[0x14] = param_3 * local_8;
  param_1[0x15] = param_4 * local_c;
  param_1[0x16] = param_5 * local_8;
  param_1[0x12] = DAT_005ccec4;
  DAT_005ccec4 = DAT_005ccec4 + 1;
  return param_1;
}

