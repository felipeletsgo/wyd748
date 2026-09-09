// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00455b5b | Name: FUN_00455b5b


undefined4 __thiscall FUN_00455b5b(int param_1,char param_2)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 0x28b44) == 0) && (iVar1 = FUN_0040c0f0(), iVar1 == 0)) {
    return 0;
  }
  if (param_2 == '\t') {
    iVar1 = FUN_00493c10();
    if (iVar1 != 0) {
      (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(*(undefined4 *)(param_1 + 0x28b60));
      return 1;
    }
    iVar1 = FUN_00493c10();
    if (iVar1 != 0) {
      (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(*(undefined4 *)(param_1 + 0x28b64));
      return 1;
    }
    iVar1 = FUN_00493c10();
    if (iVar1 != 0) {
      (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(*(undefined4 *)(param_1 + 0x28b68));
      return 1;
    }
  }
  return 0;
}

