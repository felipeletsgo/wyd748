// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053d8dd | Name: FUN_0053d8dd


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0053d8dd(int param_1,float param_2,undefined4 param_3,float param_4)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_18;
  
  *(float *)(param_1 + 0x68) = param_2;
  *(undefined4 *)(param_1 + 0x6c) = param_3;
  *(float *)(param_1 + 0x70) = param_4;
  *(float *)(param_1 + 0x9c) = param_2;
  *(float *)(param_1 + 0xa0) = param_4;
  iVar1 = __ftol();
  iVar2 = __ftol();
  uVar3 = __ftol();
  iVar4 = __ftol();
  for (local_18 = 0; local_18 < 5; local_18 = local_18 + 1) {
    FUN_00430f20((param_2 - _DAT_005a3660) + (float)local_18,
                 (param_4 - _DAT_005a40f0) + (float)local_18);
    FUN_0049da58(local_24,local_20,iVar4 << 0x18 | iVar1 << 0x10 | iVar2 << 8 | uVar3);
  }
  return;
}

