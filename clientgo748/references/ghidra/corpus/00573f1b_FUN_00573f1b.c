// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573f1b | Name: FUN_00573f1b


void FUN_00573f1b(int *param_1,undefined1 *param_2)

{
  undefined1 uVar1;
  int iVar2;
  
  if (*(char *)((int)param_1 + 9) == '\x10') {
    for (iVar2 = (uint)*(byte *)((int)param_1 + 10) * *param_1; iVar2 != 0; iVar2 = iVar2 + -1) {
      uVar1 = *param_2;
      *param_2 = param_2[1];
      param_2[1] = uVar1;
      param_2 = param_2 + 2;
    }
  }
  return;
}

