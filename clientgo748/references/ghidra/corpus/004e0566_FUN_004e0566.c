// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e0566 | Name: FUN_004e0566


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e0566(int param_1)

{
  float fVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  float10 fVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  int local_28;
  int local_24;
  int local_14;
  
  if (*(int *)(param_1 + 0x84) == 0) {
    uVar2 = 0;
  }
  else {
    iVar3 = (**(code **)(*DAT_0092e654 + 8))();
    if ((*(int *)(param_1 + 0xa0) != 0) &&
       (*(uint *)(param_1 + 0xa0) < (uint)(iVar3 - *(int *)(param_1 + 0x9c)))) {
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
    }
    if (*(int *)(param_1 + 0xa0) != 0) {
      fVar1 = (float)(uint)(iVar3 - *(int *)(param_1 + 0x9c)) / (float)*(int *)(param_1 + 0xa0);
      if (*(int *)(param_1 + 0xb4) == 1) {
        fVar7 = (float10)FUN_00423b00(fVar1 * _DAT_005a4148);
        if (fVar7 <= (float10)_DAT_005a3534) {
          FUN_00423b00(fVar1 * _DAT_005a4148);
        }
        else {
          FUN_00423b00(fVar1 * _DAT_005a4148);
        }
      }
      else {
        fVar7 = (float10)FUN_00493bf0((fVar1 * _DAT_005a4148) / _DAT_005a365c);
        if (fVar7 <= (float10)_DAT_005a3534) {
          FUN_00493bf0((fVar1 * _DAT_005a4148) / _DAT_005a365c);
        }
        else {
          FUN_00493bf0((fVar1 * _DAT_005a4148) / _DAT_005a365c);
        }
      }
      uVar2 = *(undefined4 *)(param_1 + 0xa4);
      uVar12 = 0;
      iVar3 = __ftol();
      uVar10 = *(undefined4 *)(param_1 + 0xa8);
      uVar11 = 0;
      iVar4 = __ftol();
      uVar8 = *(undefined4 *)(param_1 + 0xac);
      uVar9 = 0;
      iVar5 = __ftol();
      uVar6 = __ftol(*(undefined4 *)(param_1 + 0xb0),0,uVar8,uVar9,uVar10,uVar11,uVar2,uVar12);
      for (local_14 = 0; local_14 < *(int *)(param_1 + 0x8c); local_14 = local_14 + 1) {
        if (*(int *)(param_1 + 0xb8) == 0) {
          *(uint *)(*(int *)(param_1 + 0x84) + 0xc + local_14 * 0x18) =
               iVar3 << 0x18 | iVar4 << 0x10 | iVar5 << 8 | uVar6;
        }
        else {
          *(uint *)(*(int *)(param_1 + 0x84) + 0xc + local_14 * 0x18) =
               iVar3 << 0x18 | *(int *)(param_1 + 0xa8) << 0x10 | *(int *)(param_1 + 0xac) << 8 |
               *(uint *)(param_1 + 0xb0);
        }
      }
      if (2 < *(int *)(param_1 + 0x94)) {
        *(undefined4 *)(*(int *)(param_1 + 0x84) + 0xc + (*(int *)(param_1 + 0x8c) + -1) * 0x18) = 0
        ;
        *(undefined4 *)(*(int *)(param_1 + 0x84) + 0xc + (*(int *)(param_1 + 0x8c) + -2) * 0x18) = 0
        ;
        *(undefined4 *)
         (*(int *)(param_1 + 0x84) + 0xc +
         ((*(int *)(param_1 + 0x8c) + -1) - (*(int *)(param_1 + 0x94) + 1)) * 0x18) = 0;
      }
    }
    if ((*(int *)(param_1 + 0xa0) == 0) && (*(int *)(param_1 + 0x6c) == 0)) {
      if (DAT_005ccfa0 == 3) {
        for (local_24 = 0; local_24 < *(int *)(param_1 + 0x8c); local_24 = local_24 + 1) {
          *(undefined4 *)(*(int *)(param_1 + 0x84) + 0xc + local_24 * 0x18) = 0xa0a0a0a0;
        }
      }
      else {
        for (local_28 = 0; local_28 < *(int *)(param_1 + 0x8c); local_28 = local_28 + 1) {
          *(undefined4 *)(*(int *)(param_1 + 0x84) + 0xc + local_28 * 0x18) = 0xffffffff;
        }
      }
    }
    uVar2 = 1;
  }
  return uVar2;
}

