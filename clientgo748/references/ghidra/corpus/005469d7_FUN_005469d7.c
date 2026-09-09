// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005469d7 | Name: FUN_005469d7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_005469d7(int param_1)

{
  float fVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  float10 fVar5;
  int local_1c;
  int local_10;
  int local_c;
  uint local_8;
  
  local_8 = local_8 & 0xffffff00;
  iVar2 = __ftol();
  if (iVar2 >> 7 != 0x1c) {
    iVar2 = __ftol();
    if (iVar2 >> 7 != 0x1d) goto LAB_00546a34;
  }
  iVar2 = __ftol();
  if (iVar2 >> 7 != 0x17) {
    iVar2 = __ftol();
    if (iVar2 >> 7 != 0x16) goto LAB_00546a34;
  }
  local_8 = CONCAT31(local_8._1_3_,1);
LAB_00546a34:
  uVar3 = (**(code **)(*DAT_0092e654 + 8))();
  *(uint *)(param_1 + 0x178) = uVar3;
  if (*(int *)(param_1 + 0x17c) != 0) {
    local_c = 0;
    (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x318) + 0x2c))
              (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x318),0,0,&local_c,0);
    uVar4 = uVar3 % 12000;
    for (local_10 = 0; local_10 < *(int *)(param_1 + 0x164) + 1; local_10 = local_10 + 1) {
      for (local_1c = 0; local_1c < *(int *)(param_1 + 0x160) + 1; local_1c = local_1c + 1) {
        if ((local_8 & 0xff) == 1) {
          *(float *)(local_c + 0x14 + (local_1c + local_10 * (*(int *)(param_1 + 0x160) + 1)) * 0x20
                    ) = (float)(uVar3 % 18000) / _DAT_005a5b7c + (float)local_10 / _DAT_005a5b80;
          *(float *)(local_c + 0x10 + (local_1c + local_10 * (*(int *)(param_1 + 0x160) + 1)) * 0x20
                    ) = (float)local_1c / _DAT_005a5b80;
          *(float *)(local_c + 0x1c + (local_1c + local_10 * (*(int *)(param_1 + 0x160) + 1)) * 0x20
                    ) = (float)local_10 / _DAT_005a46ec;
          *(float *)(local_c + 0x18 + (local_1c + local_10 * (*(int *)(param_1 + 0x160) + 1)) * 0x20
                    ) = (float)(uVar3 % 18000) / _DAT_005a5b7c + (float)local_1c / _DAT_005a46ec;
          fVar5 = (float10)FUN_00423b00((float)uVar4 / _DAT_005a5b78 + (float)uVar4 / _DAT_005a5b78
                                        + ((float)local_1c * _DAT_005a4148) / _DAT_005a4cec);
          *(float *)(local_c + 4 + (local_1c + local_10 * (*(int *)(param_1 + 0x160) + 1)) * 0x20) =
               (float)(fVar5 * (float10)_DAT_005a40f4 - (float10)_DAT_005a4214);
        }
        else {
          if (((DAT_005ccfa0 == 0) || (DAT_005ccfa0 == 3)) || (DAT_005ccfa0 == 4)) {
            *(float *)(local_c + 0x1c +
                      (local_1c + local_10 * (*(int *)(param_1 + 0x160) + 1)) * 0x20) =
                 (float)uVar4 / _DAT_005a5b74 + (float)local_10 / _DAT_005a40f0;
          }
          else {
            *(float *)(local_c + 0x14 +
                      (local_1c + local_10 * (*(int *)(param_1 + 0x160) + 1)) * 0x20) =
                 (float)uVar4 / _DAT_005a5b74 + (float)local_10 / _DAT_005a4678;
            *(float *)(local_c + 0x10 +
                      (local_1c + local_10 * (*(int *)(param_1 + 0x160) + 1)) * 0x20) =
                 (float)local_1c / _DAT_005a4418;
          }
          if (DAT_005ccfa0 == 2) {
            fVar1 = ((float)uVar4 / _DAT_005a5b78) * _DAT_005a4148;
            fVar5 = (float10)FUN_00423b00(fVar1 + fVar1 +
                                          ((float)local_1c * _DAT_005a4148) / _DAT_005a365c);
            *(float *)(local_c + 4 + (local_1c + local_10 * (*(int *)(param_1 + 0x160) + 1)) * 0x20)
                 = (float)(fVar5 * (float10)_DAT_005a439c - (float10)_DAT_005a439c);
          }
          else {
            fVar1 = ((float)uVar4 / _DAT_005a5b78) * _DAT_005a4148;
            fVar5 = (float10)FUN_00423b00(fVar1 + fVar1 +
                                          ((float)local_1c * _DAT_005a4148) / _DAT_005a365c);
            *(float *)(local_c + 4 + (local_1c + local_10 * (*(int *)(param_1 + 0x160) + 1)) * 0x20)
                 = (float)(fVar5 * (float10)_DAT_005a439c - (float10)_DAT_005a4214);
          }
        }
      }
    }
    (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x318) + 0x30))
              (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x318));
  }
  return 1;
}

