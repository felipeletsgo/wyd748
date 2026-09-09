// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c8daa | Name: FUN_004c8daa


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004c8daa(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  float local_14;
  float local_10;
  
  iVar1 = FUN_00494dcf();
  if (iVar1 == 30000) {
    if (*(int *)(param_1 + 0x24) == 0) {
      *(undefined4 *)(param_1 + 0x28) = 0;
      uVar2 = 0;
    }
    else {
      iVar1 = *(int *)(DAT_013b71e8 + 0x1b084);
      if (*(int *)(iVar1 + 0x94) == 0) {
        if ((((*(float *)(iVar1 + 0x98) < *(float *)(param_1 + 0x74)) &&
             (*(float *)(param_1 + 0x74) < *(float *)(iVar1 + 0xa0))) &&
            (*(float *)(iVar1 + 0x9c) < *(float *)(param_1 + 0x7c))) &&
           (*(float *)(param_1 + 0x7c) < *(float *)(iVar1 + 0xa4))) {
          *(undefined4 *)(param_1 + 0x28) = 1;
          return 1;
        }
      }
      else {
        if (*(float *)(iVar1 + 0xa8) - *(float *)(param_1 + 0x74) <= _DAT_005a3534) {
          local_10 = -(*(float *)(iVar1 + 0xa8) - *(float *)(param_1 + 0x74));
        }
        else {
          local_10 = *(float *)(iVar1 + 0xa8) - *(float *)(param_1 + 0x74);
        }
        if (*(float *)(iVar1 + 0xac) - *(float *)(param_1 + 0x7c) <= _DAT_005a3534) {
          local_14 = -(*(float *)(iVar1 + 0xac) - *(float *)(param_1 + 0x7c));
        }
        else {
          local_14 = *(float *)(iVar1 + 0xac) - *(float *)(param_1 + 0x7c);
        }
        if (local_10 + local_14 < _DAT_005a4204) {
          *(undefined4 *)(param_1 + 0x28) = 1;
          return 1;
        }
      }
      *(undefined4 *)(param_1 + 0x28) = 1;
      uVar2 = 1;
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x28) = 1;
    uVar2 = 1;
  }
  return uVar2;
}

