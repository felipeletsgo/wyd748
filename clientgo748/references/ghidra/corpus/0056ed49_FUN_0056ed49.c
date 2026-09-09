// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056ed49 | Name: FUN_0056ed49


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0056ed49(int param_1)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  ushort *local_10;
  uint local_c;
  
  if ((*(int *)(param_1 + 0x108c) != 0) && (*(int *)(param_1 + 0x1090) != 0)) {
    local_c = *(uint *)(param_1 + 0x1070);
    local_10 = (ushort *)
               (*(int *)(param_1 + 0x1080) * *(int *)(param_1 + 0x1054) +
                *(int *)(param_1 + 0x1074) * *(int *)(param_1 + 0x1050) + local_c * 2 +
               *(int *)(param_1 + 0x18));
    pfVar1 = *(float **)(param_1 + 0x106c);
    if (local_c < *(uint *)(param_1 + 0x1078)) {
      do {
        iVar2 = (int)ROUND(pfVar1[2] * _DAT_005a6a08 +
                           pfVar1[1] * _DAT_005a6a04 + *pfVar1 * _DAT_005a6a0c + _DAT_005a67d4) +
                0x10;
        iVar4 = (int)ROUND(pfVar1[5] * _DAT_005a6a04 +
                           pfVar1[6] * _DAT_005a6a08 + pfVar1[4] * _DAT_005a6a0c + _DAT_005a67d4) +
                0x10;
        iVar3 = (int)ROUND(((pfVar1[2] * _DAT_005a6a00 - pfVar1[1] * _DAT_005a69fc) -
                           *pfVar1 * _DAT_005a69f8) + _DAT_005a67d4) + 0x80;
        iVar5 = (int)ROUND((*pfVar1 * _DAT_005a6a00 -
                           (pfVar1[2] * _DAT_005a69f0 + pfVar1[1] * _DAT_005a69f4)) + _DAT_005a67d4)
                + 0x80;
        if (iVar2 < 0) {
          iVar2 = 0;
        }
        else if (0xff < iVar2) {
          iVar2 = 0xff;
        }
        if (iVar4 < 0) {
          iVar4 = 0;
        }
        else if (0xff < iVar4) {
          iVar4 = 0xff;
        }
        if (iVar3 < 0) {
          iVar3 = 0;
        }
        else if (0xff < iVar3) {
          iVar3 = 0xff;
        }
        if (iVar5 < 0) {
          iVar5 = 0;
        }
        else if (0xff < iVar5) {
          iVar5 = 0xff;
        }
        local_c = local_c + 2;
        pfVar1 = pfVar1 + 8;
        *local_10 = (ushort)(iVar2 << ((byte)*(undefined4 *)(param_1 + 0x1094) & 0x1f)) |
                    (ushort)(iVar3 << ((byte)*(undefined4 *)(param_1 + 0x1098) & 0x1f));
        local_10[1] = (ushort)(iVar4 << ((byte)*(undefined4 *)(param_1 + 0x1094) & 0x1f)) |
                      (ushort)(iVar5 << ((byte)*(undefined4 *)(param_1 + 0x1098) & 0x1f));
        local_10 = local_10 + 2;
      } while (local_c < *(uint *)(param_1 + 0x1078));
    }
    *(undefined4 *)(param_1 + 0x108c) = 0;
  }
  return 0;
}

