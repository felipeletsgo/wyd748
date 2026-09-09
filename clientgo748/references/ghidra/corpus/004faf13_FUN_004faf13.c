// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004faf13 | Name: FUN_004faf13


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004faf13(int param_1,short param_2)

{
  undefined4 uVar1;
  short local_c [4];
  
  if (*(int *)(param_1 + 0x214) == 0) {
    _memset(local_c,0,8);
    local_c[0] = param_2;
    uVar1 = FUN_0054cd07(local_c,0x12);
    *(undefined4 *)(param_1 + 0x24c) = uVar1;
    uVar1 = FUN_004fab52(*(undefined4 *)(param_1 + 0x24c));
    *(undefined4 *)(param_1 + 0x5c) = uVar1;
    if (param_2 == 0x19) {
      *(float *)(param_1 + 0x38) = *(float *)(param_1 + 0x38) * _DAT_005a55f4;
    }
  }
  return;
}

