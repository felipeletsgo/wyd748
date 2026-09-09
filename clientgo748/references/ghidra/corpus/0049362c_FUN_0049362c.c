// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049362c | Name: FUN_0049362c


void __fastcall FUN_0049362c(int param_1)

{
  short sVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  uint uVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uStack_58;
  undefined4 uStack_54;
  undefined4 uStack_50;
  int local_44;
  int local_14;
  int iStack_10;
  
  if (DAT_005d03f4 == 0) {
    FUN_00493a9d();
    return;
  }
  if (*(int *)(DAT_0067cf38 + 0x24) != 30000) {
    DAT_005d03f4 = 0;
    FUN_00493a9d();
    return;
  }
  if (*(int *)(param_1 + 0x27144) != 0) {
    FUN_00493a9d();
    return;
  }
  if (*(int *)(param_1 + 0x27140) != 0) {
    FUN_00493a9d();
    return;
  }
  if (*(int *)(param_1 + 0x27138) != 0) {
    FUN_00493a9d();
    return;
  }
  uVar5 = (**(code **)(**(int **)(DAT_013b71e0 + 0xf4) + 8))();
  sVar1 = *(short *)(DAT_013b71e8 + 0x738);
  if (sVar1 < 1) {
    DAT_005d03f4 = 0;
    FUN_00493a9d();
    return;
  }
  __ftol();
  __ftol();
  sVar2 = *(short *)(DAT_013b71e8 + 0x734);
  sVar3 = *(short *)(DAT_013b71e8 + 0x73a);
  sVar4 = *(short *)(DAT_013b71e8 + 0x736);
  iVar6 = FUN_0054cd07(DAT_013b71e8 + 0x7b8,0x50);
  if (uVar5 % 3 == 0) {
    if ((int)sVar1 < (int)sVar2 / 2) {
      FUN_00452601(0x71,0);
      FUN_00493a9d();
      return;
    }
    if ((iVar6 != 0) && (iVar6 < 6000)) {
      FUN_004525d1(0x76,0);
      FUN_00493a9d();
      return;
    }
    if ((int)sVar3 < (int)sVar4 / 2) {
      FUN_00452631(0x77,0);
      FUN_00493a9d();
      return;
    }
  }
  if (1 < *(int *)(*(int *)(param_1 + 0x4c) + 0x740)) {
    FUN_00493a9d();
    return;
  }
  local_14 = 0;
  do {
    if (9 < local_14) {
      iStack_10 = *(int *)(*(int *)(param_1 + 0x34) + 0x10);
      while( true ) {
        while( true ) {
          if ((iStack_10 == 0) || (*(int *)(iStack_10 + 0xc) == 0)) {
            return;
          }
          if (iStack_10 != *(int *)(param_1 + 0x4c)) break;
          iStack_10 = *(int *)(iStack_10 + 0xc);
        }
        if ((DAT_005d03f4 == 1) && (iVar6 = FUN_0052ee94(iStack_10), iVar6 == 1)) break;
        if (DAT_005d03f4 == 2) {
          uVar7 = __ftol();
          uVar8 = __ftol();
          FUN_00430f50(*(undefined4 *)(iStack_10 + 0x28),*(undefined4 *)(iStack_10 + 0x30),
                       *(undefined4 *)(iStack_10 + 0x2c));
          iVar6 = FUN_00456589(uVar7,uVar8,uStack_58,uStack_54,uStack_50,0,0,iStack_10);
          if (iVar6 == 1) {
            FUN_00493a9d();
            return;
          }
        }
        iStack_10 = *(int *)(iStack_10 + 0xc);
      }
      FUN_00493a9d();
      return;
    }
    iVar6 = (int)*(char *)(DAT_013b71e8 + local_14 + 0xef6);
    if (iVar6 != -1) {
      if ((((iVar6 == 5) || (iVar6 == 0x35)) || (iVar6 == 0x36)) ||
         (((iVar6 == 0x4a || (iVar6 == 0x4c)) || ((iVar6 == 0x4d || (iVar6 == 0x4f)))))) {
        local_44 = (uint)*(byte *)(DAT_013b71e8 + 0x745) * *(int *)(&DAT_0092ff24 + iVar6 * 0x60) *
                   100;
      }
      else if (((iVar6 == 9) || (iVar6 == 0xb)) ||
              ((iVar6 == 0xd ||
               ((((iVar6 == 0xf || (iVar6 == 0x25)) || (iVar6 == 0x56)) || (iVar6 == 0x57)))))) {
        local_44 = (uint)*(byte *)(DAT_013b71e8 + 0x746) * *(int *)(&DAT_0092ff24 + iVar6 * 0x60) *
                   100;
      }
      else if ((((((iVar6 == 0x29) || (iVar6 == 0x2b)) ||
                 ((iVar6 == 0x2c || ((iVar6 == 0x2d || (iVar6 == 0x2e)))))) || (iVar6 == 0x40)) ||
               (((iVar6 == 0x42 || (iVar6 == 0x44)) || (iVar6 == 0x46)))) ||
              ((iVar6 == 0x47 || (iVar6 == 0x59)))) {
        local_44 = (uint)*(byte *)(DAT_013b71e8 + 0x747) * *(int *)(&DAT_0092ff24 + iVar6 * 0x60) *
                   100;
      }
      else {
        if ((((iVar6 != 0x38) && (iVar6 != 0x39)) &&
            ((iVar6 != 0x3a && (((iVar6 != 0x3b && (iVar6 != 0x3c)) && (iVar6 != 0x3d)))))) &&
           ((iVar6 != 0x3e && (iVar6 != 0x3f)))) goto LAB_004937c9;
        local_44 = 80000;
      }
      if ((uint)(*(int *)(&DAT_0092eaf8 + iVar6 * 4) + local_44) <= uVar5) {
        FUN_013c0207();
        return;
      }
    }
LAB_004937c9:
    local_14 = local_14 + 1;
  } while( true );
}

