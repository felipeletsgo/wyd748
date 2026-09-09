// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042261f | Name: FUN_0042261f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0042261f(int param_1,int param_2,int param_3,int param_4)

{
  size_t sVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 local_14;
  undefined4 local_10;
  
  if (*(int *)(param_1 + 0x1a0) != -1) {
    param_2 = *(int *)(param_1 + 0x1a0);
  }
  if (*(int *)(param_1 + 0x1a4) != -1) {
    param_3 = *(int *)(param_1 + 0x1a4);
  }
  if (*(int *)(param_1 + 0x19c) == 0) {
    FUN_00421b8f(param_1 + 0x18,*(undefined4 *)(param_1 + 8),0);
  }
  sVar1 = _strlen((char *)(param_1 + 0x18));
  if (((int)sVar1 < 1) || (0xff < (int)sVar1)) {
    uVar2 = 0;
  }
  else {
    (**(code **)(*DAT_005ccf9c + 0x40))(2);
    local_10 = 0;
    for (local_14 = 0; local_14 < *(int *)(param_1 + 0x14); local_14 = local_14 + 1) {
      iVar3 = (int)*(short *)(param_1 + 0x1a8 + local_14 * 2);
      if (*(int *)(param_1 + 0xc) == 1) {
        if (param_4 != 0) {
          FUN_0042fc6b(0,(float)local_14 * (float)(DAT_005b4914 + 1),(float)iVar3,
                       (float)DAT_005b4914,(float)(param_2 + 1),
                       (float)(param_3 + 2) + (float)(local_14 * DAT_005b4914),
                       *(undefined4 *)(param_1 + 0x19c),*(undefined4 *)(param_1 + 4),0x3f800000,
                       0x3f800000);
        }
        FUN_0042fc6b(0,(float)local_14 * (float)(DAT_005b4914 + 1),(float)iVar3,(float)DAT_005b4914,
                     (float)param_2,(float)(param_3 + 1) + (float)(local_14 * DAT_005b4914),
                     *(undefined4 *)(param_1 + 0x19c),*(undefined4 *)(param_1 + 8),0x3f800000,
                     0x3f800000);
      }
      else {
        if (param_4 != 0) {
          FUN_0042fc6b(0,(float)local_14 * (float)(DAT_005b4914 + 1),(float)iVar3,
                       (float)DAT_005b4914,(float)param_2 + (float)local_10 + _DAT_005a3660,
                       (float)(param_3 + 2),*(undefined4 *)(param_1 + 0x19c),
                       *(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 0x10),
                       *(undefined4 *)(param_1 + 0x10));
        }
        FUN_0042fc6b(0,(float)local_14 * (float)(DAT_005b4914 + 1),(float)iVar3,(float)DAT_005b4914,
                     (float)param_2 + (float)local_10,(float)(param_3 + 1),
                     *(undefined4 *)(param_1 + 0x19c),*(undefined4 *)(param_1 + 8),
                     *(undefined4 *)(param_1 + 0x10),*(undefined4 *)(param_1 + 0x10));
      }
      local_10 = local_10 + iVar3;
    }
    uVar2 = 1;
  }
  return uVar2;
}

