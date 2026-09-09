// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057ad9a | Name: FUN_0057ad9a


uint FUN_0057ad9a(uint param_1,byte *param_2,uint param_3)

{
  uint uVar1;
  uint uVar2;
  
  if (param_2 == (byte *)0x0) {
    return 0;
  }
  param_1 = ~param_1;
  if (7 < param_3) {
    uVar1 = param_3 >> 3;
    do {
      param_3 = param_3 - 8;
      uVar2 = *(uint *)(&DAT_005a8fa0 + ((*param_2 ^ param_1) & 0xff) * 4) ^ param_1 >> 8;
      uVar2 = *(uint *)(&DAT_005a8fa0 + ((param_2[1] ^ uVar2) & 0xff) * 4) ^ uVar2 >> 8;
      uVar2 = *(uint *)(&DAT_005a8fa0 + ((param_2[2] ^ uVar2) & 0xff) * 4) ^ uVar2 >> 8;
      uVar2 = *(uint *)(&DAT_005a8fa0 + ((param_2[3] ^ uVar2) & 0xff) * 4) ^ uVar2 >> 8;
      uVar2 = *(uint *)(&DAT_005a8fa0 + ((param_2[4] ^ uVar2) & 0xff) * 4) ^ uVar2 >> 8;
      uVar2 = *(uint *)(&DAT_005a8fa0 + ((param_2[5] ^ uVar2) & 0xff) * 4) ^ uVar2 >> 8;
      uVar2 = *(uint *)(&DAT_005a8fa0 + ((param_2[6] ^ uVar2) & 0xff) * 4) ^ uVar2 >> 8;
      param_1 = uVar2 >> 8 ^ *(uint *)(&DAT_005a8fa0 + ((param_2[7] ^ uVar2) & 0xff) * 4);
      param_2 = param_2 + 8;
      uVar1 = uVar1 - 1;
    } while (uVar1 != 0);
  }
  for (; param_3 != 0; param_3 = param_3 - 1) {
    param_1 = param_1 >> 8 ^ *(uint *)(&DAT_005a8fa0 + ((*param_2 ^ param_1) & 0xff) * 4);
    param_2 = param_2 + 1;
  }
  return ~param_1;
}

