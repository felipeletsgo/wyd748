// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c75d4 | Name: FUN_004c75d4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004c75d4(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  float local_14;
  float local_10;
  
  iVar1 = FUN_00494dcf();
  if (iVar1 == 30000) {
    iVar1 = *(int *)(DAT_013b71e8 + 0x1b084);
    if (*(int *)(iVar1 + 0x94) == 0) {
      if ((((*(float *)(iVar1 + 0x98) < *(float *)(param_1 + 0x40)) &&
           (*(float *)(param_1 + 0x40) < *(float *)(iVar1 + 0xa0))) &&
          (*(float *)(iVar1 + 0x9c) < *(float *)(param_1 + 0x48))) &&
         (*(float *)(param_1 + 0x48) < *(float *)(iVar1 + 0xa4))) {
        uVar2 = FUN_004c7320();
        *(undefined4 *)(param_1 + 0x58) = uVar2;
        return *(undefined4 *)(param_1 + 0x58);
      }
    }
    else {
      if (*(float *)(iVar1 + 0xa8) - *(float *)(param_1 + 0x40) <= _DAT_005a3534) {
        local_10 = -(*(float *)(iVar1 + 0xa8) - *(float *)(param_1 + 0x40));
      }
      else {
        local_10 = *(float *)(iVar1 + 0xa8) - *(float *)(param_1 + 0x40);
      }
      if (*(float *)(iVar1 + 0xac) - *(float *)(param_1 + 0x48) <= _DAT_005a3534) {
        local_14 = -(*(float *)(iVar1 + 0xac) - *(float *)(param_1 + 0x48));
      }
      else {
        local_14 = *(float *)(iVar1 + 0xac) - *(float *)(param_1 + 0x48);
      }
      if (local_10 + local_14 < _DAT_005a4204) {
        uVar2 = FUN_004c7320();
        *(undefined4 *)(param_1 + 0x58) = uVar2;
        return *(undefined4 *)(param_1 + 0x58);
      }
    }
    *(undefined4 *)(param_1 + 0x58) = 0;
    uVar2 = 0;
  }
  else {
    *(undefined4 *)(param_1 + 0x58) = 1;
    uVar2 = 1;
  }
  return uVar2;
}

