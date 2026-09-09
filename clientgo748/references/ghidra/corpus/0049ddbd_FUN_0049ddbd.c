// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049ddbd | Name: FUN_0049ddbd


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0049ddbd(int param_1)

{
  int iVar1;
  int iVar2;
  float fVar3;
  int iVar4;
  uint uVar5;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  float local_1c;
  int local_c;
  
  if (*(int *)(param_1 + 0x2009c) != 0) {
    iVar4 = (**(code **)(*DAT_0092e654 + 8))();
    iVar1 = *(int *)(param_1 + 0x2009c);
    iVar2 = *(int *)(DAT_013b71e8 + 0x1b084);
    *(undefined4 *)(iVar2 + 0x34) = 0x41300000;
    uVar5 = (iVar4 - iVar1) + 100;
    for (local_c = 0; local_c < *(int *)(param_1 + 0x200b0) + -1; local_c = local_c + 1) {
      if ((*(uint *)(param_1 + 0x200b4 + local_c * 0x1c) < uVar5) &&
         (uVar5 < *(uint *)(param_1 + 0x200b4 + (local_c + 1) * 0x1c))) {
        fVar3 = (float)(uVar5 - *(int *)(param_1 + 0x200b4 + local_c * 0x1c)) /
                (float)(*(int *)(param_1 + 0x200b4 + (local_c + 1) * 0x1c) -
                       *(int *)(param_1 + 0x200b4 + local_c * 0x1c));
        *(float *)(iVar2 + 0x28) =
             fVar3 * *(float *)(param_1 + 0x200c8 + (local_c + 1) * 0x1c) +
             (_DAT_005a3660 - fVar3) * *(float *)(param_1 + 0x200c8 + local_c * 0x1c);
        *(undefined4 *)(iVar2 + 0x70) = *(undefined4 *)(iVar2 + 0x28);
        *(float *)(iVar2 + 0x2c) =
             fVar3 * *(float *)(param_1 + 0x200cc + (local_c + 1) * 0x1c) +
             (_DAT_005a3660 - fVar3) * *(float *)(param_1 + 0x200cc + local_c * 0x1c);
        *(undefined4 *)(iVar2 + 0x74) = *(undefined4 *)(iVar2 + 0x2c);
        FUN_00430f50(0,0,0);
        FUN_00430f50(0,0,0);
        if ((*(short *)(param_1 + 0x200b8 + local_c * 0x1c) == 1) && (*(int *)(param_1 + 0x4c) != 0)
           ) {
          local_24 = *(float *)(*(int *)(param_1 + 0x4c) + 0x28);
          local_20 = *(float *)(*(int *)(param_1 + 0x4c) + 0x30);
          local_1c = *(float *)(*(int *)(param_1 + 0x4c) + 0x2c);
        }
        if ((*(short *)(param_1 + 0x200b8 + (local_c + 1) * 0x1c) == 1) &&
           (*(int *)(param_1 + 0x4c) != 0)) {
          local_30 = *(float *)(*(int *)(param_1 + 0x4c) + 0x28);
          local_2c = *(float *)(*(int *)(param_1 + 0x4c) + 0x30);
          local_28 = *(float *)(*(int *)(param_1 + 0x4c) + 0x2c);
        }
        *(float *)(iVar2 + 0x48) =
             (local_30 + *(float *)(param_1 + 0x200bc + (local_c + 1) * 0x1c)) * fVar3 +
             (_DAT_005a3660 - fVar3) * (local_24 + *(float *)(param_1 + 0x200bc + local_c * 0x1c));
        *(float *)(iVar2 + 0x4c) =
             (local_2c + *(float *)(param_1 + 0x200c0 + (local_c + 1) * 0x1c)) * fVar3 +
             (_DAT_005a3660 - fVar3) * (local_20 + *(float *)(param_1 + 0x200c0 + local_c * 0x1c));
        *(float *)(iVar2 + 0x50) =
             (local_28 + *(float *)(param_1 + 0x200c4 + (local_c + 1) * 0x1c)) * fVar3 +
             (_DAT_005a3660 - fVar3) * (local_1c + *(float *)(param_1 + 0x200c4 + local_c * 0x1c));
        return;
      }
    }
    if (*(uint *)(param_1 + 0x200b4 + (*(int *)(param_1 + 0x200b0) + -1) * 0x1c) < uVar5) {
      *(undefined4 *)(iVar2 + 0x28) =
           *(undefined4 *)(param_1 + 0x200c8 + (*(int *)(param_1 + 0x200b0) + -1) * 0x1c);
      *(undefined4 *)(iVar2 + 0x2c) =
           *(undefined4 *)(param_1 + 0x200cc + (*(int *)(param_1 + 0x200b0) + -1) * 0x1c);
      FUN_00430f50(0,0,0);
      if ((*(int *)(param_1 + 0x4c) != 0) &&
         (*(short *)(param_1 + 0x200b8 + (*(int *)(param_1 + 0x200b0) + -1) * 0x1c) == 1)) {
        local_3c = *(float *)(*(int *)(param_1 + 0x4c) + 0x28);
        local_38 = *(float *)(*(int *)(param_1 + 0x4c) + 0x30);
        local_34 = *(float *)(*(int *)(param_1 + 0x4c) + 0x2c);
      }
      *(float *)(iVar2 + 0x48) =
           local_3c + *(float *)(param_1 + 0x200bc + (*(int *)(param_1 + 0x200b0) + -1) * 0x1c);
      *(float *)(iVar2 + 0x4c) =
           local_38 + *(float *)(param_1 + 0x200c0 + (*(int *)(param_1 + 0x200b0) + -1) * 0x1c);
      *(float *)(iVar2 + 0x50) =
           local_34 + *(float *)(param_1 + 0x200c4 + (*(int *)(param_1 + 0x200b0) + -1) * 0x1c);
    }
  }
  return;
}

