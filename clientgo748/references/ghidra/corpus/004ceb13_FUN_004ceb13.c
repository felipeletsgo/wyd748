// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ceb13 | Name: FUN_004ceb13


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004ceb13(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  float10 fVar7;
  float local_cc;
  float local_88;
  float local_30;
  int local_28;
  undefined1 local_24 [16];
  uint local_14;
  int local_c;
  float local_8;
  
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*param_1 + 0x38))();
  if (param_1[0x21] != 0) {
    param_1[0x27] = (int)((float)(uint)(iVar1 - param_1[0x20]) / (float)param_1[0x21]);
  }
  if (_DAT_005a3660 < (float)param_1[0x27]) {
    if (param_1[0x28] == 2) {
      param_1[0x20] = iVar1;
    }
    else {
      (**(code **)(*DAT_013b71e8 + 100))(param_1);
    }
  }
  else {
    fVar7 = (float10)FUN_00423b00((float)param_1[0x27] * _DAT_005a4148);
    if (fVar7 <= (float10)_DAT_005a3534) {
      fVar7 = (float10)FUN_00423b00((float)param_1[0x27] * _DAT_005a4148);
      fVar7 = -fVar7;
    }
    else {
      fVar7 = (float10)FUN_00423b00((float)param_1[0x27] * _DAT_005a4148);
    }
    local_88 = (float)fVar7;
    local_8 = local_88;
    local_30 = 0.0;
    if (param_1[0x28] == 3) {
      local_30 = local_88;
    }
    if (param_1[7] != 0) {
      if (*(int *)(param_1[7] + 0x24) == 3) {
        if (*(char *)(param_1[7] + 0x79c) == '\x01') {
          FUN_004310a0(param_1[7] + 0x50);
        }
        else {
          uVar2 = FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),*(undefined4 *)(param_1[7] + 0x30)
                               ,*(undefined4 *)(param_1[7] + 0x2c));
          FUN_004310a0(uVar2);
        }
      }
      else {
        uVar2 = FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),*(undefined4 *)(param_1[7] + 0x30),
                             *(undefined4 *)(param_1[7] + 0x2c));
        FUN_004310a0(uVar2);
      }
      param_1[0x1e] = (int)(local_30 + (float)param_1[0x1e]);
    }
    iVar1 = FUN_004b14fd(0x2c0,1,180000);
    (**(code **)(**(int **)(iVar1 + 0x318) + 0x34))(*(undefined4 *)(iVar1 + 0x318),local_24);
    (**(code **)(**(int **)(iVar1 + 0x318) + 0x2c))(*(undefined4 *)(iVar1 + 0x318),0,0,&local_c,0);
    uVar3 = local_14 / 0x18;
    for (local_28 = 0; local_28 < (int)uVar3; local_28 = local_28 + 1) {
      __ftol();
      iVar4 = __ftol();
      iVar5 = __ftol();
      uVar6 = __ftol();
      *(uint *)(local_c + 0xc + local_28 * 0x18) = iVar4 << 0x10 | iVar5 << 8 | uVar6;
    }
    (**(code **)(**(int **)(iVar1 + 0x318) + 0x30))(*(undefined4 *)(iVar1 + 0x318));
    iVar1 = FUN_004b14fd(0x2c1,1,180000);
    if (iVar1 == 0) {
      return 0;
    }
    (**(code **)(**(int **)(iVar1 + 0x318) + 0x34))(*(undefined4 *)(iVar1 + 0x318),local_24);
    (**(code **)(**(int **)(iVar1 + 0x318) + 0x2c))(*(undefined4 *)(iVar1 + 0x318),0,0,&local_c,0);
    for (local_28 = 0; local_28 < (int)(local_14 / 0x18); local_28 = local_28 + 1) {
      __ftol();
      iVar4 = __ftol();
      iVar5 = __ftol();
      uVar3 = __ftol();
      *(uint *)(local_c + 0xc + local_28 * 0x18) = iVar4 << 0x10 | iVar5 << 8 | uVar3;
    }
    (**(code **)(**(int **)(iVar1 + 0x318) + 0x30))(*(undefined4 *)(iVar1 + 0x318));
    if (param_1[0x28] == 0) {
      *(undefined4 *)(iVar1 + 0x298) = 0x39;
    }
    else {
      *(undefined4 *)(iVar1 + 0x298) = 99;
    }
    if (param_1[0x28] == 1) {
      param_1[0x1e] = (int)((float)param_1[0x1e] + (float)param_1[0x27]);
    }
    fVar7 = (float10)FUN_00423b00(((float)param_1[0x27] * _DAT_005a4148) / _DAT_005a365c);
    if (fVar7 <= (float10)_DAT_005a3534) {
      fVar7 = (float10)FUN_00423b00(((float)param_1[0x27] * _DAT_005a4148) / _DAT_005a365c);
      fVar7 = -fVar7;
    }
    else {
      fVar7 = (float10)FUN_00423b00(((float)param_1[0x27] * _DAT_005a4148) / _DAT_005a365c);
    }
    local_cc = (float)fVar7;
    param_1[0x26] =
         (int)((float)((uint)(param_1[0x20] + param_1[7]) % 10) / _DAT_005a3670 +
              local_cc * _DAT_005a4148 * _DAT_005a3794);
  }
  return 1;
}

