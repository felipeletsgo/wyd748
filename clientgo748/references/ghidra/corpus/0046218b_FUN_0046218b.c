// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0046218b | Name: FUN_0046218b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0046218b(int param_1,undefined4 param_2,undefined4 param_3,uint param_4)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_10 [8];
  uint local_8;
  
  if (((((*(int *)(*(int *)(param_1 + 0x4c) + 0x23c) != 1) &&
        (*(int *)(param_1 + 0x27474) + 1000U <= param_4)) &&
       ((uint)*(ushort *)(param_1 + 0x275ae) != *(uint *)(*(int *)(param_1 + 0x4c) + 0x20))) &&
      (((((*(char *)(*(int *)(param_1 + 0x4c) + 0x79c) != '\0' ||
          (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == -1)) ||
         ((*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0 ||
          ((*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 1 ||
           (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 2)))))) ||
        (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 3)) ||
       (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) == 0xe)))) &&
     (((*(int *)(*(int *)(param_1 + 0x4c) + 0x768) == 0 &&
       (*(int *)(*(int *)(param_1 + 0x4c) + 0x76c) == 0)) ||
      (*(int *)(param_1 + 0x274d4) + 1000U <= param_4)))) {
    if ((3 < *(int *)(*(int *)(param_1 + 0x4c) + 0x740)) &&
       (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) < 10)) {
      local_8 = *(uint *)(&DAT_0065a4a8 +
                         *(int *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x60) + 0x2d0) * 4 +
                         *(int *)(*(int *)(param_1 + 0x4c) + 0x5c) * 900);
      if (2 < local_8) {
        local_8 = local_8 - 2;
      }
      if ((*(int *)(DAT_005d0504 + 0x1c) != 0) &&
         (param_4 < *(int *)(*(int *)(param_1 + 0x4c) + 0x600) +
                    *(int *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x60) + 0x2e4) * local_8 * 4)) {
        return 1;
      }
    }
    if (*(int *)(param_1 + 0x40) != 0) {
      FUN_004fa8a1(*(undefined4 *)(param_1 + 0x26e7c));
      iVar1 = __ftol();
      iVar2 = __ftol();
      if (iVar1 == iVar2) {
        iVar1 = __ftol();
        iVar2 = __ftol();
        if (iVar1 == iVar2) {
          return 1;
        }
      }
      FUN_00493c60(local_10,*(int *)(param_1 + 0x4c) + 0x28);
      uVar3 = __ftol();
      *(undefined4 *)(param_1 + 0x275b0) = uVar3;
      uVar3 = __ftol();
      *(undefined4 *)(param_1 + 0x275b4) = uVar3;
      if ((_DAT_005a40f4 <= *(float *)(*(int *)(param_1 + 0x4c) + 0x230)) ||
         (*(float *)(*(int *)(param_1 + 0x4c) + 0x230) == _DAT_005a3534)) {
        FUN_00520216(*(undefined4 *)(param_1 + 0x275b0),*(undefined4 *)(param_1 + 0x275b4),0,0);
      }
      *(undefined4 *)(param_1 + 0x26e90) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x76c) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x768) = 0;
      *(undefined4 *)(param_1 + 0x7c) = 0;
      *(undefined4 *)(param_1 + 0x26e98) = 0;
    }
  }
  return 1;
}

