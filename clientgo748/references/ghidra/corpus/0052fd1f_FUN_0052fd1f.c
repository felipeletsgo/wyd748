// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052fd1f | Name: FUN_0052fd1f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0052fd1f(int param_1)

{
  float fVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  float10 fVar5;
  undefined4 uVar6;
  
  if ((((*(int *)(param_1 + 0x18) == -1) || (*(int *)(param_1 + 0x18) == DAT_005bdef0)) ||
      (*(int *)(param_1 + 0x18) == DAT_005bdef4)) ||
     ((*(int *)(param_1 + 0x18) == DAT_005bdef8 || (*(int *)(param_1 + 0x18) == DAT_005bdefc)))) {
    uVar3 = (**(code **)(*DAT_0092e654 + 8))();
    if (*(int *)(param_1 + 0x48) == 1) {
      fVar5 = (float10)FUN_00423b00(((float)(uVar3 % 20000) * _DAT_005a4148) / _DAT_005a420c);
      fVar1 = (float)fVar5;
      if ((*(float *)(param_1 + 0x34) <= _DAT_005a4738) ||
         (_DAT_005a45d4 <= *(float *)(param_1 + 0x34))) {
        if ((*(float *)(param_1 + 0x34) <= _DAT_005a56f0) ||
           ((float)_DAT_005a56e8 <= *(float *)(param_1 + 0x34))) {
          if ((*(float *)(param_1 + 0x34) <= _DAT_005a56e0) ||
             ((float)_DAT_005a56d8 <= *(float *)(param_1 + 0x34))) {
            *(float *)(param_1 + 0x28) = _DAT_005a40f0 * fVar1 + *(float *)(param_1 + 0x160);
          }
          else {
            *(float *)(param_1 + 0x2c) = _DAT_005a40f0 * fVar1 + *(float *)(param_1 + 0x164);
          }
        }
        else {
          *(float *)(param_1 + 0x2c) = _DAT_005a40f0 * fVar1 + *(float *)(param_1 + 0x164);
        }
      }
      else {
        *(float *)(param_1 + 0x2c) = _DAT_005a40f0 * fVar1 + *(float *)(param_1 + 0x164);
      }
      if (fVar1 < _DAT_005a4214) {
        fVar5 = (float10)FUN_004d0150(param_1 + 0x28);
        if (((float)fVar5 < _DAT_005a3834) && (DAT_005ccf98 != 0)) {
          iVar4 = FUN_00429a6d(9);
          if (iVar4 != 0) {
            uVar6 = 0;
            uVar2 = 0;
            FUN_00429a6d(9);
            FUN_0042ad2b(uVar2,uVar6);
          }
        }
      }
    }
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

