// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573a8b | Name: FUN_00573a8b


void FUN_00573a8b(int param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 local_8;
  
  local_8 = FUN_00573eb5(param_1);
  iVar1 = *(int *)(param_1 + 0xbc);
  *(int *)(param_1 + 0xc0) = iVar1;
  iVar2 = iVar1;
  puVar3 = param_2;
  if (0 < local_8) {
    do {
      for (; iVar2 != 0; iVar2 = iVar2 + -1) {
        FUN_005736e4(param_1,*puVar3,0);
        puVar3 = puVar3 + 1;
      }
      local_8 = local_8 + -1;
      iVar2 = iVar1;
      puVar3 = param_2;
    } while (local_8 != 0);
  }
  return;
}

