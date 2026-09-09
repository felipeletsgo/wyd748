// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004be1ef | Name: FUN_004be1ef


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004be1ef(int *param_1)

{
  float fVar1;
  undefined4 uVar2;
  int iVar3;
  int *piVar4;
  undefined4 *puVar5;
  int *piVar6;
  undefined4 *puVar7;
  float10 fVar8;
  undefined4 auStackY_290 [11];
  undefined4 uStackY_264;
  undefined4 uStackY_260;
  undefined4 uStackY_25c;
  undefined4 auStack_20c [8];
  int iStack_1ec;
  int iStack_1e8;
  int iStack_1e4;
  int iStack_1e0;
  float fStack_1dc;
  float fStack_1d8;
  float fStack_1d4;
  undefined4 uStack_1cc;
  
  if (param_1[1] == 0) {
    uVar2 = 0;
  }
  else if (param_1[0x2d] == 0) {
    uVar2 = 0;
  }
  else {
    if ((param_1[0xc] < 1) || (5 < param_1[0xc])) {
      FUN_004c5b40();
      FUN_00430f10();
      FUN_00430fc0();
      FUN_00430fc0();
      thunk_FUN_0056239d();
      uVar2 = FUN_013d5000();
      return uVar2;
    }
    piVar4 = param_1 + 0xd;
    piVar6 = (int *)(param_1[0x2d] + 8);
    for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
      *piVar6 = *piVar4;
      piVar4 = piVar4 + 1;
      piVar6 = piVar6 + 1;
    }
    FUN_004c5b40();
    FUN_00430f10();
    FUN_004c5b40();
    FUN_00430fc0();
    thunk_FUN_0056230d();
    thunk_FUN_0056239d();
    thunk_FUN_00561a5b();
    thunk_FUN_00561a5b();
    thunk_FUN_00561a5b();
    FUN_004c5b40();
    FUN_00430fc0();
    if (*param_1 == 0x2c) {
      if (param_1[0xd2] == 0) {
        iVar3 = (**(code **)(*DAT_0092e654 + 8))();
        param_1[0xd2] = iVar3;
      }
      (**(code **)(*DAT_0092e654 + 8))();
      fVar8 = (float10)FUN_00423b00();
      fVar1 = (float)(fVar8 / (float10)_DAT_005a365c);
      uStackY_25c = 0x4be648;
      _memset(auStack_20c,0,0x44);
      auStack_20c[0] = 0x3f800000;
      auStack_20c[1] = 0x3f800000;
      auStack_20c[2] = 0x3f800000;
      iStack_1ec = param_1[0xa2];
      iStack_1e8 = param_1[0xa3];
      iStack_1e4 = param_1[0xa4];
      iStack_1e0 = param_1[0xa5];
      uStack_1cc = 0;
      fStack_1dc = _DAT_005a4154 + fVar1;
      fStack_1d8 = _DAT_005a4154 + fVar1;
      fStack_1d4 = _DAT_005a4154 + fVar1;
      puVar5 = auStack_20c;
      puVar7 = auStackY_290;
      for (iVar3 = 0x11; iVar3 != 0; iVar3 = iVar3 + -1) {
        *puVar7 = *puVar5;
        puVar5 = puVar5 + 1;
        puVar7 = puVar7 + 1;
      }
      FUN_004be1cb();
    }
    (**(code **)(*DAT_005ccf9c + 0x30))();
    uStackY_25c = 0x4be728;
    FUN_004c21e8();
    uStackY_25c = 0x4be739;
    FUN_004c16cc();
    uStackY_25c = 0;
    uStackY_260 = 1;
    uStackY_264 = 0x4be74e;
    (**(code **)(*DAT_005ccf9c + 0x38))();
    uVar2 = 1;
  }
  return uVar2;
}

