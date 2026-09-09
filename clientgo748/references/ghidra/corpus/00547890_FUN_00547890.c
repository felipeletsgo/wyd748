// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00547890 | Name: FUN_00547890


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __fastcall FUN_00547890(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  float10 fVar3;
  undefined1 *puVar4;
  undefined4 uVar5;
  undefined1 local_58 [24];
  undefined1 local_40 [24];
  uint local_28;
  void *local_24 [6];
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2a6f;
  local_24[5] = ExceptionList;
  ExceptionList = local_24 + 5;
  FUN_0053e1d2();
  local_8 = 0;
  FUN_00590340(param_1 + 0x5d,0x180,0x14,FUN_00549d20,FUN_004c92e9);
  local_8._0_1_ = 1;
  FUN_00590340(param_1 + 0x7dd,0x180,2,FUN_00549d20,FUN_004c92e9);
  local_8 = CONCAT31(local_8._1_3_,2);
  *param_1 = &PTR_FUN_005a5c08;
  param_1[0x5c] = 0;
  param_1[9] = 1;
  param_1[0xc] = 0xc0a00000;
  param_1[0x58] = 0;
  param_1[0xe] = 0x3f000000;
  param_1[0xd] = 0x40490fdb;
  param_1[0x5a] = 0;
  param_1[0x59] = 0;
  param_1[0x5b] = 2000;
  if (*(int *)(DAT_0067cf38 + 0x24) == 0x7535) {
    *(undefined4 *)(DAT_005ccf9c + 0x2b044) = 0x41980000;
    *(undefined4 *)(DAT_005ccf9c + 0x2b048) = 0x41f00000;
    param_1[0xd] = 0x3fc90fdb;
  }
  param_1[0x89d] = 0x1c;
  param_1[0x8a1] = 0x78;
  param_1[0x8a5] = 0xbd;
  param_1[0x89e] = 0x33;
  param_1[0x8a2] = 0x36;
  param_1[0x8a6] = 0x2a;
  param_1[0x89f] = 0x62;
  param_1[0x8a3] = 0x2f;
  param_1[0x8a7] = 4;
  param_1[0x8a0] = 0x13;
  param_1[0x8a4] = 0x2e;
  param_1[0x8a8] = 0x33;
  local_24[0] = (void *)0xffffffff;
  local_24[1] = (void *)0xff8888aa;
  local_24[2] = (void *)0xffff9955;
  local_24[3] = (void *)0xffffffaa;
  local_24[4] = (void *)0xffaaffaa;
  for (local_28 = 0; (int)local_28 < 0x14; local_28 = local_28 + 1) {
    param_1[local_28 * 0x60 + 0xa1] = 0x38;
    param_1[local_28 * 0x60 + 0x9b] = 0;
    param_1[local_28 * 0x60 + 0x95] = 0x3dcccccd;
    param_1[local_28 * 0x60 + 0x96] = 0x3dcccccd;
    param_1[local_28 * 0x60 + 0x97] = 0x3dcccccd;
    iVar1 = _rand();
    param_1[local_28 * 0x60 + 0x7a] = (float)(iVar1 % 12000 + -6000) * _DAT_005a5c04;
    iVar1 = _rand();
    param_1[local_28 * 0x60 + 0x7c] = (float)(iVar1 % 12000 + -6000) * _DAT_005a5c04;
    fVar3 = (float10)FUN_004d3cd0((float)param_1[local_28 * 0x60 + 0x7c] *
                                  (float)param_1[local_28 * 0x60 + 0x7c] +
                                  (float)param_1[local_28 * 0x60 + 0x7a] *
                                  (float)param_1[local_28 * 0x60 + 0x7a]);
    param_1[local_28 * 0x60 + 0x7b] =
         (float)(((float10)_DAT_005a34a0 - fVar3 / (float10)_DAT_005a3704) * (float10)_DAT_005a4314)
    ;
    (**(code **)(param_1[local_28 * 0x60 + 0x5d] + 0x3c))(local_24[(int)local_28 % 5]);
    param_1[local_28 * 0x60 + 0xa4] = 0x3e99999a;
    uVar2 = local_28 & 0x80000001;
    if ((int)uVar2 < 0) {
      uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
    }
    if (uVar2 == 0) {
      param_1[local_28 * 0x60 + 0x9e] = 2;
    }
    else {
      param_1[local_28 * 0x60 + 0x9e] = 0;
    }
    param_1[local_28 * 0x60 + 0x78] = 1;
  }
  param_1[0x821] = 0xd6;
  param_1[0x881] = 2;
  uVar5 = 0x41700000;
  puVar4 = local_40;
  FUN_00430f50(0xbf800000,0x3f000000,0x3e99999a);
  uVar5 = FUN_004c8c60(puVar4,uVar5);
  FUN_004310a0(uVar5);
  uVar5 = 0x41700000;
  puVar4 = local_58;
  FUN_00430f50(0xbf800000,0x3f000000,0x3e99999a);
  uVar5 = FUN_004c8c60(puVar4,uVar5);
  FUN_004310a0(uVar5);
  param_1[0x81b] = 0;
  param_1[0x87b] = 0;
  param_1[0x815] = 0x40a00000;
  param_1[0x816] = 0x40a00000;
  param_1[0x817] = 0x40a00000;
  param_1[0x875] = 0x41400000;
  param_1[0x876] = 0x41400000;
  param_1[0x877] = 0x41400000;
  (**(code **)(param_1[0x7dd] + 0x3c))(0xffffffff);
  (**(code **)(param_1[0x83d] + 0x3c))(0x554433);
  param_1[0x81e] = 0;
  param_1[0x87e] = 0;
  param_1[0x7f8] = 0;
  param_1[0x858] = 1;
  ExceptionList = local_24[5];
  return param_1;
}

