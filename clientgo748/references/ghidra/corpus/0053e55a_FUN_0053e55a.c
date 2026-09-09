// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053e55a | Name: FUN_0053e55a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0053e55a(int param_1,int param_2)

{
  double dVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int local_14c;
  int local_148;
  int local_11c;
  int local_110;
  char local_10c [256];
  int local_c;
  int local_8;
  
  if (param_2 == 0) {
    uVar2 = 0;
  }
  else {
    _memset(local_10c,0,0x100);
    dVar1 = _DAT_005a5a80 - (double)(*(float *)(param_1 + 0x34) * (float)_DAT_005a5a88);
    for (local_110 = 0; local_110 < 0x10; local_110 = local_110 + 1) {
      for (local_11c = 0; local_11c < 0x10; local_11c = local_11c + 1) {
        FUN_00591014(dVar1);
        FUN_0058fe04(dVar1);
        FUN_0058fe04(dVar1);
        FUN_00591014(dVar1);
        iVar3 = __ftol();
        iVar4 = __ftol();
        if ((((-1 < iVar3) && (-1 < iVar4)) && (iVar3 < 0x10)) && (iVar4 < 0x10)) {
          local_10c[local_11c + local_110 * 0x10] =
               (&DAT_005da288)[iVar3 + *(int *)(param_1 + 0x40) * 0x100 + iVar4 * 0x10];
        }
      }
    }
    local_c = __ftol();
    iVar3 = __ftol();
    local_c = local_c + -7;
    local_8 = iVar3 + -7;
    for (local_110 = 0; local_110 < 0x10; local_110 = local_110 + 1) {
      for (local_148 = 0; local_148 < 0x10; local_148 = local_148 + 1) {
        if (((-1 < local_c + local_148) && (local_c + local_148 < 0x81)) &&
           ((-1 < local_8 + local_110 &&
            ((local_8 + local_110 < 0x81 && (local_10c[local_148 + local_110 * 0x10] != '\0')))))) {
          local_14c = __ftol();
          if (_DAT_005a3534 < *(float *)(param_1 + 0x30)) {
            local_14c = local_14c + 1;
          }
          if (0x7f < local_14c) {
            local_14c = 0x7f;
          }
          if (*(char *)(param_2 + 0xc0cc + (local_8 + local_110) * 0x80 + local_c + local_148) <
              (char)local_14c) {
            *(char *)(param_2 + 0xc0cc + (local_8 + local_110) * 0x80 + local_c + local_148) =
                 (char)local_14c;
          }
        }
      }
    }
    uVar2 = 1;
  }
  return uVar2;
}

