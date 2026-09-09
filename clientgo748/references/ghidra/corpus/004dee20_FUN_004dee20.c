// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004dee20 | Name: FUN_004dee20


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004dee20(int param_1,int param_2)

{
  float fVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  float10 fVar6;
  float local_18;
  float local_14;
  float local_10;
  int local_c;
  
  if (*(int *)(param_1 + 0x28) != 0) {
    fVar1 = (float)(uint)(param_2 - *(int *)(param_1 + 0x80)) * _DAT_005a41e8;
    FUN_00430f20(0,0);
    iVar3 = (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
    if ((DAT_0067cf38 == 0) || (iVar4 = FUN_00494dcf(), iVar4 != 0x7534)) {
      if (iVar3 == 0) {
        local_18 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x78) + _DAT_005a4bd8;
        local_14 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x7c) + _DAT_005a4bd8;
      }
      else {
        local_18 = *(float *)(iVar3 + 0x28);
        local_14 = *(float *)(iVar3 + 0x2c);
      }
    }
    else {
      local_18 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x48) - _DAT_005a34a0;
      local_14 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x50) + _DAT_005a4cec;
    }
    local_10 = 0.0;
    if ((DAT_0067cf38 == 0) || (iVar4 = FUN_00494dcf(), iVar4 != 0x7534)) {
      if (iVar3 != 0) {
        local_10 = *(float *)(iVar3 + 0x30);
      }
    }
    else {
      local_10 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x4c) - _DAT_005a365c;
    }
    for (local_c = 0; local_c < 200; local_c = local_c + 1) {
      fVar2 = (((float)local_c + (float)local_c) / _DAT_005a431c) * _DAT_005a4148;
      fVar6 = (float10)FUN_00423b00(fVar2 + fVar2);
      if (*(float *)(param_1 + 0x3a8 + local_c * 0xc) <= local_10) {
        if (iVar3 == 0) {
          if ((DAT_0067cf38 != 0) && (iVar4 = FUN_00494dcf(), iVar4 == 0x7534)) {
            fVar2 = local_10 + _DAT_005a3664;
            iVar4 = _rand();
            *(float *)(param_1 + 0x3a8 + local_c * 0xc) =
                 (float)(iVar4 % 10) * _DAT_005a34a0 + fVar2;
            uVar5 = _rand();
            uVar5 = uVar5 & 0x8000001f;
            if ((int)uVar5 < 0) {
              uVar5 = (uVar5 - 1 | 0xffffffe0) + 1;
            }
            *(float *)(param_1 + 0x3a4 + local_c * 0xc) =
                 (_DAT_005a3834 - (float)(int)uVar5 / _DAT_005a40f0) + local_18;
            uVar5 = _rand();
            uVar5 = uVar5 & 0x8000001f;
            if ((int)uVar5 < 0) {
              uVar5 = (uVar5 - 1 | 0xffffffe0) + 1;
            }
            *(float *)(param_1 + 0x3ac + local_c * 0xc) =
                 (_DAT_005a3834 - (float)(int)uVar5 / _DAT_005a40f0) + local_14;
          }
        }
        else {
          fVar2 = *(float *)(iVar3 + 0x30) + _DAT_005a3664;
          iVar4 = _rand();
          *(float *)(param_1 + 0x3a8 + local_c * 0xc) = (float)(iVar4 % 10) * _DAT_005a34a0 + fVar2;
          uVar5 = _rand();
          uVar5 = uVar5 & 0x8000001f;
          if ((int)uVar5 < 0) {
            uVar5 = (uVar5 - 1 | 0xffffffe0) + 1;
          }
          *(float *)(param_1 + 0x3a4 + local_c * 0xc) =
               (_DAT_005a3834 - (float)(int)uVar5 / _DAT_005a40f0) + local_18;
          uVar5 = _rand();
          uVar5 = uVar5 & 0x8000001f;
          if ((int)uVar5 < 0) {
            uVar5 = (uVar5 - 1 | 0xffffffe0) + 1;
          }
          *(float *)(param_1 + 0x3ac + local_c * 0xc) =
               (_DAT_005a3834 - (float)(int)uVar5 / _DAT_005a40f0) + local_14;
        }
      }
      else {
        *(float *)(param_1 + 0x3a8 + local_c * 0xc) =
             *(float *)(param_1 + 0x3a8 + local_c * 0xc) -
             fVar1 * *(float *)(param_1 + 0x84 + local_c * 4);
        *(float *)(param_1 + 0x3a4 + local_c * 0xc) =
             (float)fVar6 * _DAT_005a45d4 + *(float *)(param_1 + 0x3a4 + local_c * 0xc);
      }
    }
    *(int *)(param_1 + 0x80) = param_2;
  }
  return 1;
}

