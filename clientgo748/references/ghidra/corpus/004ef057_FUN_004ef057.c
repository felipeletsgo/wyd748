// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ef057 | Name: FUN_004ef057


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004ef057(int param_1)

{
  float fVar1;
  int local_c;
  
  fVar1 = DAT_005b490c * _DAT_005a3704;
  *(undefined4 *)(param_1 + 0x24) = 0xce;
  *(undefined4 *)(param_1 + 0x28) = 0xbf19999a;
  *(undefined4 *)(param_1 + 0x34) = 0xd4;
  *(undefined4 *)(param_1 + 0x38) = 0xbecccccd;
  *(undefined4 *)(param_1 + 0x44) = 0xd4;
  *(undefined4 *)(param_1 + 0x48) = 0xbe99999a;
  *(undefined4 *)(param_1 + 0x54) = 0xd3;
  *(undefined4 *)(param_1 + 0x58) = 0;
  *(undefined4 *)(param_1 + 100) = 0xd2;
  *(undefined4 *)(param_1 + 0x68) = 0;
  *(undefined4 *)(param_1 + 0x74) = 0xd3;
  *(undefined4 *)(param_1 + 0x78) = 0;
  *(undefined4 *)(param_1 + 0x84) = 0xd4;
  *(undefined4 *)(param_1 + 0x88) = 0;
  *(undefined4 *)(param_1 + 0x94) = 0xd5;
  *(undefined4 *)(param_1 + 0x98) = 0;
  *(undefined4 *)(param_1 + 0xa4) = 0xd2;
  *(undefined4 *)(param_1 + 0xa8) = 0x3dcccccd;
  *(undefined4 *)(param_1 + 0xb4) = 0xd1;
  *(undefined4 *)(param_1 + 0xb8) = 0x3e4ccccd;
  *(undefined4 *)(param_1 + 0xc4) = 0xce;
  *(undefined4 *)(param_1 + 200) = 0x3e99999a;
  *(undefined4 *)(param_1 + 0xd4) = 0xd0;
  *(undefined4 *)(param_1 + 0xd8) = 0x3f19999a;
  *(float *)(param_1 + 0x2c) = _DAT_005a4154 * fVar1;
  *(undefined **)(param_1 + 0x30) = &DAT_00aa8888;
  *(float *)(param_1 + 0x3c) = _DAT_005a5308 * fVar1;
  *(undefined **)(param_1 + 0x40) = &DAT_00aaaa22;
  *(float *)(param_1 + 0x4c) = _DAT_005a5304 * fVar1;
  *(undefined **)(param_1 + 0x50) = &DAT_00aa22aa;
  *(float *)(param_1 + 0x5c) = _DAT_005a46d8 * fVar1;
  *(undefined4 *)(param_1 + 0x60) = 0xffffff;
  *(float *)(param_1 + 0x6c) = _DAT_005a5300 * fVar1;
  *(undefined **)(param_1 + 0x70) = &DAT_00aaaaaa;
  *(float *)(param_1 + 0x7c) = _DAT_005a52fc * fVar1;
  *(undefined **)(param_1 + 0x80) = &DAT_00aaaaaa;
  *(float *)(param_1 + 0x8c) = _DAT_005a52f8 * fVar1;
  *(undefined **)(param_1 + 0x90) = &DAT_00aaaaaa;
  *(float *)(param_1 + 0x9c) = _DAT_005a52f4 * fVar1;
  *(undefined4 *)(param_1 + 0xa0) = 0xffffff;
  *(float *)(param_1 + 0xac) = _DAT_005a52f0 * fVar1;
  *(undefined **)(param_1 + 0xb0) = &DAT_00aaaaaa;
  *(float *)(param_1 + 0xbc) = _DAT_005a52ec * fVar1;
  *(undefined **)(param_1 + 0xc0) = &DAT_00aaaaaa;
  *(float *)(param_1 + 0xcc) = _DAT_005a52e8 * fVar1;
  *(undefined **)(param_1 + 0xd0) = &DAT_00aaaaaa;
  *(float *)(param_1 + 0xdc) = _DAT_005a52e4 * fVar1;
  *(undefined **)(param_1 + 0xe0) = &DAT_00aaaaaa;
  *(undefined4 *)(param_1 + 0x104) = 0xbf800000;
  *(undefined4 *)(param_1 + 0x108) = 0xbf800000;
  *(undefined4 *)(param_1 + 0x120) = 0;
  *(undefined4 *)(param_1 + 0x124) = 0xbf800000;
  *(undefined4 *)(param_1 + 0x13c) = 0;
  *(undefined4 *)(param_1 + 0x140) = 0;
  *(undefined4 *)(param_1 + 0x158) = 0xbf800000;
  *(undefined4 *)(param_1 + 0x15c) = 0;
  for (local_c = 0; local_c < 4; local_c = local_c + 1) {
    *(undefined4 *)(param_1 + 0xfc + local_c * 0x1c) = 0x3f800000;
  }
  return 1;
}

