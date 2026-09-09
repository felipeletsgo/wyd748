// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004be3cb | Name: FUN_004be3cb


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_004be3cb(void)

{
  size_t sVar1;
  undefined4 uVar2;
  int iVar3;
  int unaff_EBP;
  undefined4 *puVar4;
  undefined4 *puVar5;
  float10 fVar6;
  undefined4 auStackY_5c [11];
  undefined4 uStackY_30;
  undefined4 uStackY_2c;
  undefined4 uStackY_28;
  
  thunk_FUN_0056230d();
  if (((*(int *)(*(int *)(unaff_EBP + -0x214) + 0x2f4) != 0) &&
      (*(short *)(*(int *)(unaff_EBP + -0x214) + 0x354) == 1)) &&
     (*(undefined4 *)(unaff_EBP + -0x184) = *(undefined4 *)(*(int *)(unaff_EBP + -0x214) + 0x2f4),
     *(int *)(unaff_EBP + -0x184) != 0)) {
    if (*(int *)(*(int *)(unaff_EBP + -0x184) + 0x160) == 0x65) {
      *(undefined4 *)(*(int *)(unaff_EBP + -0x214) + 0x350) = 1;
      if ((*(int *)(*(int *)(unaff_EBP + -0x184) + 0x198) != 0) &&
         (sVar1 = _strlen((char *)(*(int *)(unaff_EBP + -0x184) + 0x168)), 3 < sVar1)) {
        *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x184) + 0x198) + 0x350) = 1;
      }
    }
    else {
      *(undefined4 *)(*(int *)(unaff_EBP + -0x214) + 0x350) = 0;
      if ((*(int *)(*(int *)(unaff_EBP + -0x184) + 0x198) != 0) &&
         (sVar1 = _strlen((char *)(*(int *)(unaff_EBP + -0x184) + 0x168)), 3 < sVar1)) {
        *(undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x184) + 0x198) + 0x350) = 0;
      }
      FUN_004c5b40();
      FUN_00430fc0();
      *(undefined4 *)(unaff_EBP + -0x19c) = 0xbf800000;
      thunk_FUN_00561a5b();
    }
  }
  thunk_FUN_00561a5b();
  thunk_FUN_00561a5b();
  FUN_004c5b40();
  FUN_00430fc0();
  if (**(int **)(unaff_EBP + -0x214) == 0x2c) {
    if (*(int *)(*(int *)(unaff_EBP + -0x214) + 0x348) == 0) {
      uVar2 = (**(code **)(*DAT_0092e654 + 8))();
      *(undefined4 *)(*(int *)(unaff_EBP + -0x214) + 0x348) = uVar2;
    }
    uVar2 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(unaff_EBP + -0x20c) = uVar2;
    *(uint *)(unaff_EBP + -0x21c) =
         (uint)(*(int *)(*(int *)(unaff_EBP + -0x214) + 0x348) - *(int *)(unaff_EBP + -0x20c)) %
         3000;
    *(undefined4 *)(unaff_EBP + -0x218) = 0;
    *(float *)(unaff_EBP + -0x210) = (float)*(longlong *)(unaff_EBP + -0x21c) / _DAT_005a35c0;
    fVar6 = (float10)FUN_00423b00();
    *(float *)(unaff_EBP + -0x210) = (float)(fVar6 / (float10)_DAT_005a365c);
    uStackY_28 = 0x4be648;
    _memset((void *)(unaff_EBP + -0x208),0,0x44);
    *(undefined4 *)(unaff_EBP + -0x208) = 0x3f800000;
    *(undefined4 *)(unaff_EBP + -0x204) = 0x3f800000;
    *(undefined4 *)(unaff_EBP + -0x200) = 0x3f800000;
    iVar3 = *(int *)(unaff_EBP + -0x214);
    *(undefined4 *)(unaff_EBP + -0x1e8) = *(undefined4 *)(iVar3 + 0x288);
    *(undefined4 *)(unaff_EBP + -0x1e4) = *(undefined4 *)(iVar3 + 0x28c);
    *(undefined4 *)(unaff_EBP + -0x1e0) = *(undefined4 *)(iVar3 + 0x290);
    *(undefined4 *)(unaff_EBP + -0x1dc) = *(undefined4 *)(iVar3 + 0x294);
    *(undefined4 *)(unaff_EBP + -0x1c8) = 0;
    *(float *)(unaff_EBP + -0x1d8) = _DAT_005a4154 + *(float *)(unaff_EBP + -0x210);
    *(float *)(unaff_EBP + -0x1d4) = _DAT_005a4154 + *(float *)(unaff_EBP + -0x210);
    *(float *)(unaff_EBP + -0x1d0) = _DAT_005a4154 + *(float *)(unaff_EBP + -0x210);
    puVar4 = (undefined4 *)(unaff_EBP + -0x208);
    puVar5 = auStackY_5c;
    for (iVar3 = 0x11; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar5 = *puVar4;
      puVar4 = puVar4 + 1;
      puVar5 = puVar5 + 1;
    }
    FUN_004be1cb();
  }
  (**(code **)(*DAT_005ccf9c + 0x30))();
  uStackY_28 = 0x4be728;
  FUN_004c21e8();
  uStackY_28 = 0x4be739;
  FUN_004c16cc();
  uStackY_28 = 0;
  uStackY_2c = 1;
  uStackY_30 = 0x4be74e;
  (**(code **)(*DAT_005ccf9c + 0x38))();
  return 1;
}

