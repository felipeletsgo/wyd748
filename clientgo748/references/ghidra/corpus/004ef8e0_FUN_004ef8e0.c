// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ef8e0 | Name: FUN_004ef8e0


void __fastcall FUN_004ef8e0(int *param_1)

{
  (**(code **)(*param_1 + 0x50))(1);
  param_1[0x15] = 0;
  param_1[0xc] = 0x40400000;
  param_1[0x11] = 0;
  param_1[0x12] = 0;
  param_1[0x13] = 0;
  param_1[0x14] = 0;
  param_1[0x1a] = 0;
  param_1[0x1b] = 0;
  param_1[0x23] = 0;
  param_1[0x1e] = 0;
  param_1[0x1f] = 0;
  param_1[0xe] = 0x40b00000;
  param_1[0x1c] = 0;
  param_1[0x1d] = 0;
  param_1[0x24] = 0;
  param_1[0x19] = param_1[0xc];
  param_1[0x2c] = 0;
  param_1[0x2e] = 0;
  param_1[0x2f] = 0x3fc00000;
  param_1[0x2d] = 0;
  return;
}

