// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cde40 | Name: FUN_004cde40


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004cde40(uint param_1)

{
  float fVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  float10 fVar6;
  undefined4 local_48;
  undefined4 local_38;
  undefined4 local_1c;
  undefined4 local_c;
  undefined4 local_8;
  
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  if (*(int *)(param_1 + 0xfc) == 0) {
    *(uint *)(param_1 + 0xfc) = uVar2;
  }
  if (uVar2 < *(uint *)(param_1 + 0xfc)) {
    local_8 = *(uint *)(param_1 + 0xf8);
  }
  else {
    local_8 = uVar2 - *(int *)(param_1 + 0xfc);
  }
  if ((*(int *)(param_1 + 0xf8) == 0) || ((int)local_8 < *(int *)(param_1 + 0xf8))) {
    local_c = *(int *)(param_1 + 0x118) * *(int *)(param_1 + 0x11c);
    if (local_c == 0) {
      local_c = 1;
    }
    if (*(int *)(param_1 + 0x11c) == 0) {
      *(undefined4 *)(param_1 + 0x114) = 0;
    }
    else {
      *(int *)(param_1 + 0x114) =
           (int)(((ulonglong)local_8 % (ulonglong)local_c) / (ulonglong)*(uint *)(param_1 + 0x11c));
    }
    if (*(int *)(param_1 + 0xf8) != 0) {
      *(float *)(param_1 + 0x110) = (float)(int)local_8 / (float)*(int *)(param_1 + 0xf8);
    }
    if (*(float *)(param_1 + 0x110) < _DAT_005a3534) {
      *(undefined4 *)(param_1 + 0x110) = 0x3c23d70a;
    }
    if (_DAT_005a3660 < *(float *)(param_1 + 0x110)) {
      *(undefined4 *)(param_1 + 0x110) = 0x3f800000;
    }
    if (*(int *)(param_1 + 0x100) == 1) {
      FUN_00423b00(*(float *)(param_1 + 0x110) * _DAT_005a4148);
      iVar3 = __ftol();
      iVar4 = __ftol();
      iVar5 = __ftol();
      uVar2 = __ftol();
      for (local_1c = 0; local_1c < 4; local_1c = local_1c + 1) {
        *(uint *)(param_1 + 0x90 + local_1c * 0x1c) =
             iVar3 << 0x18 | iVar4 << 0x10 | iVar5 << 8 | uVar2;
      }
    }
    else if (*(int *)(param_1 + 0x100) == 2) {
      fVar1 = (float)((uVar2 + (param_1 % 100) * 200) % 3000) / _DAT_005a35c0;
      fVar6 = (float10)FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      if (fVar6 <= (float10)_DAT_005a3534) {
        FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      }
      else {
        FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      }
      iVar3 = __ftol();
      fVar6 = (float10)FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      if (fVar6 <= (float10)_DAT_005a3534) {
        FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      }
      else {
        FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      }
      iVar4 = __ftol();
      fVar6 = (float10)FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      if (fVar6 <= (float10)_DAT_005a3534) {
        FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      }
      else {
        FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      }
      iVar5 = __ftol();
      fVar6 = (float10)FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      if (fVar6 <= (float10)_DAT_005a3534) {
        FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      }
      else {
        FUN_00423b00(fVar1 * _DAT_005a4148 + fVar1 * _DAT_005a4148);
      }
      uVar2 = __ftol();
      for (local_38 = 0; local_38 < 4; local_38 = local_38 + 1) {
        *(uint *)(param_1 + 0x90 + local_38 * 0x1c) =
             iVar3 << 0x18 | iVar4 << 0x10 | iVar5 << 8 | uVar2;
      }
    }
    else if (*(int *)(param_1 + 0x100) == 3) {
      FUN_00423b00(*(float *)(param_1 + 0x110) * _DAT_005a4148);
      iVar3 = __ftol(*(undefined4 *)(param_1 + 0x128),0);
      for (local_48 = 0; local_48 < 4; local_48 = local_48 + 1) {
        *(uint *)(param_1 + 0x90 + local_48 * 0x1c) =
             iVar3 << 0x18 | *(int *)(param_1 + 300) << 0x10 | *(int *)(param_1 + 0x130) << 8 |
             *(uint *)(param_1 + 0x134);
      }
    }
    *(float *)(param_1 + 0x80) =
         *(float *)(param_1 + 0x120) * *(float *)(param_1 + 0x110) +
         ((float)*(int *)(param_1 + 0x138) -
         *(float *)(param_1 + 0xf0) * *(float *)(param_1 + 0x110) * *(float *)(param_1 + 0x104));
    *(undefined4 *)(param_1 + 0xd4) = *(undefined4 *)(param_1 + 0x80);
    *(float *)(param_1 + 0x9c) =
         *(float *)(param_1 + 0x120) * *(float *)(param_1 + 0x110) +
         *(float *)(param_1 + 0xf0) * *(float *)(param_1 + 0x110) * *(float *)(param_1 + 0x104) +
         (float)*(int *)(param_1 + 0x138);
    *(undefined4 *)(param_1 + 0xb8) = *(undefined4 *)(param_1 + 0x9c);
    *(float *)(param_1 + 0x84) =
         *(float *)(param_1 + 0x124) * *(float *)(param_1 + 0x110) +
         ((float)*(int *)(param_1 + 0x13c) -
         *(float *)(param_1 + 0xf4) * *(float *)(param_1 + 0x110) * *(float *)(param_1 + 0x108));
    *(undefined4 *)(param_1 + 0xa0) = *(undefined4 *)(param_1 + 0x84);
    *(float *)(param_1 + 0xbc) =
         *(float *)(param_1 + 0x124) * *(float *)(param_1 + 0x110) +
         *(float *)(param_1 + 0xf4) * *(float *)(param_1 + 0x110) * *(float *)(param_1 + 0x108) +
         (float)*(int *)(param_1 + 0x13c);
    *(undefined4 *)(param_1 + 0xd8) = *(undefined4 *)(param_1 + 0xbc);
  }
  else {
    *(undefined4 *)(param_1 + 0x28) = 0;
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  return 1;
}

