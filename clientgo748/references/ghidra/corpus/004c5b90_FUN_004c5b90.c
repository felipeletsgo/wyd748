// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c5b90 | Name: FUN_004c5b90


undefined4 * __thiscall FUN_004c5b90(undefined4 param_1,undefined4 *param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 local_44 [16];
  
  FUN_00430f10();
  thunk_FUN_00561a5b(local_44,param_1,param_3);
  puVar2 = local_44;
  puVar3 = param_2;
  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  return param_2;
}

