// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00577773 | Name: FUN_00577773


void FUN_00577773(undefined4 *param_1)

{
  byte bVar1;
  byte extraout_AH;
  
  bVar1 = FUN_005776a0();
  if (((bVar1 & 0x20) != 0) && ((char)bVar1 < '\0')) {
    *param_1 = &LAB_00583718;
    param_1[1] = &LAB_00582c5a;
    param_1[2] = &LAB_005839d9;
    param_1[3] = &LAB_0057eeaa;
    param_1[5] = &LAB_0058376d;
    param_1[6] = &LAB_00582ccb;
    param_1[7] = FUN_00582d32;
    param_1[8] = &LAB_00583a5b;
    param_1[9] = &LAB_005837a6;
    param_1[10] = &LAB_00583212;
    param_1[0xb] = &LAB_0057f0dc;
    param_1[0xc] = &LAB_00583e03;
    param_1[0xd] = &LAB_00583e46;
    param_1[0xe] = FUN_00583217;
    param_1[0xf] = FUN_0058339f;
    param_1[0x10] = &LAB_00583aba;
    param_1[0x11] = &LAB_0058380a;
    param_1[0x12] = &LAB_00582c3e;
    param_1[0x13] = &LAB_0057d960;
    param_1[0x14] = &LAB_0057d9f9;
    param_1[0x15] = FUN_00583fe8;
    param_1[0x16] = &LAB_00583f2a;
    param_1[0x17] = FUN_0057da56;
    param_1[0x18] = &LAB_0057db09;
    param_1[0x19] = &LAB_0057df3a;
    param_1[0x1a] = &LAB_0057f1ab;
    param_1[0x1b] = &LAB_0057f1fb;
    param_1[0x1c] = &LAB_0057f2e6;
    param_1[0x1d] = FUN_0057dfff;
    param_1[0x1e] = &LAB_0057f3c2;
    param_1[0x1f] = &LAB_0057f425;
    param_1[0x20] = &LAB_0057f483;
    param_1[0x21] = &LAB_0057f4dc;
    param_1[0x22] = &LAB_0058404c;
    param_1[0x23] = FUN_0057e06b;
    param_1[0x24] = FUN_0057e1a2;
    param_1[0x25] = &LAB_0057f523;
    param_1[0x26] = &LAB_0057f5cb;
    param_1[0x27] = FUN_0057f752;
    param_1[0x28] = FUN_005808ea;
    param_1[0x29] = &LAB_00580a8f;
    param_1[0x2a] = FUN_00580fc3;
    param_1[0x2b] = FUN_0057e22f;
    param_1[0x2c] = &LAB_0057e2c1;
    param_1[0x36] = &LAB_00583bec;
    param_1[0x33] = &LAB_00582d91;
    param_1[0x30] = &LAB_005838a2;
    param_1[0x2e] = &LAB_005838eb;
    param_1[0x34] = &LAB_00583c5d;
    param_1[0x31] = &LAB_00582e02;
    param_1[0x37] = FUN_0057e325;
    param_1[0x38] = &LAB_0057e3cc;
    param_1[0x35] = &LAB_00583d25;
    param_1[0x2f] = &LAB_0058392e;
    param_1[0x32] = &LAB_00582ed6;
    param_1[4] = &LAB_00582512;
    param_1[0x2d] = FUN_0057e471;
    if (((extraout_AH & 1) != 0) && ((extraout_AH & 2) != 0)) {
      param_1[0xf] = FUN_005835c5;
      param_1[0x12] = FUN_00582230;
      param_1[0x26] = &LAB_0057f69d;
      param_1[0x27] = FUN_005810dc;
      param_1[0x18] = &LAB_0057dd25;
    }
  }
  return;
}

