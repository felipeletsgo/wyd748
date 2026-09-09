// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00461ed8 | Name: FUN_00461ed8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_00461ed8(int param_1,undefined4 param_2,float param_3,undefined4 param_4,uint param_5)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  undefined1 local_18 [16];
  uint local_8;
  
  if (*(int *)(*(int *)(param_1 + 0x4c) + 0x23c) == 1) {
    uVar1 = 0;
  }
  else if (param_5 < *(int *)(param_1 + 0x27474) + 1000U) {
    uVar1 = 0;
  }
  else if (param_5 < *(int *)(*(int *)(param_1 + 0x4c) + 0x5ec) + 200U) {
    uVar1 = 0;
  }
  else if ((uint)*(ushort *)(param_1 + 0x275ae) == *(uint *)(*(int *)(param_1 + 0x4c) + 0x20)) {
    uVar1 = 0;
  }
  else if (((*(int *)(*(int *)(param_1 + 0x4c) + 0x768) == 0) &&
           (*(int *)(*(int *)(param_1 + 0x4c) + 0x76c) == 0)) ||
          (*(int *)(param_1 + 0x274d4) + 1000U <= param_5)) {
    if ((3 < *(int *)(*(int *)(param_1 + 0x4c) + 0x740)) &&
       (*(int *)(*(int *)(param_1 + 0x4c) + 0x740) < 10)) {
      local_8 = *(uint *)(&DAT_0065a4a8 +
                         *(int *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x60) + 0x2d0) * 4 +
                         *(int *)(*(int *)(param_1 + 0x4c) + 0x5c) * 900);
      if (2 < local_8) {
        local_8 = local_8 - 2;
      }
      if ((*(int *)(DAT_005d0504 + 0x1c) != 0) &&
         (param_5 < *(int *)(*(int *)(param_1 + 0x4c) + 0x600) +
                    *(int *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x60) + 0x2e4) * local_8 * 4)) {
        return 0;
      }
    }
    if (*(int *)(param_1 + 0x40) != 0) {
      if (param_3 < _DAT_005a4384) {
        return 0;
      }
      FUN_004fa8a1(*(undefined4 *)(param_1 + 0x26e7c));
      iVar2 = __ftol();
      iVar3 = __ftol();
      if (iVar2 == iVar3) {
        iVar2 = __ftol();
        iVar3 = __ftol();
        if (iVar2 == iVar3) {
          return 1;
        }
      }
      FUN_00430f20(param_2,param_4);
      FUN_00493c60(local_18,*(int *)(param_1 + 0x4c) + 0x28);
      uVar1 = __ftol();
      *(undefined4 *)(param_1 + 0x275b0) = uVar1;
      uVar1 = __ftol();
      *(undefined4 *)(param_1 + 0x275b4) = uVar1;
      if ((_DAT_005a40f4 <= *(float *)(*(int *)(param_1 + 0x4c) + 0x230)) ||
         (*(float *)(*(int *)(param_1 + 0x4c) + 0x230) == _DAT_005a3534)) {
        FUN_00520216(*(undefined4 *)(param_1 + 0x275b0),*(undefined4 *)(param_1 + 0x275b4),0,0);
      }
      *(undefined4 *)(param_1 + 0x26e90) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x768) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x76c) = 0;
      *(undefined4 *)(param_1 + 0x7c) = 0;
      *(undefined4 *)(param_1 + 0x26e98) = 0;
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

