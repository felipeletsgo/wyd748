// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005873cf | Name: FUN_005873cf


void FUN_005873cf(int param_1,int param_2,int param_3,int *param_4,int param_5)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  int iVar6;
  int local_8;
  
  iVar2 = *(int *)(param_1 + 0x20);
  iVar3 = *(int *)(param_1 + 0x5c);
  if (-1 < param_5 + -1) {
    param_3 = param_3 << 2;
    local_8 = param_5;
    do {
      iVar6 = 0;
      if (0 < iVar2) {
        do {
          puVar5 = *(undefined1 **)(param_3 + *(int *)(param_2 + iVar6 * 4));
          puVar4 = (undefined1 *)(*param_4 + iVar6);
          for (iVar1 = iVar3; iVar1 != 0; iVar1 = iVar1 + -1) {
            *puVar4 = *puVar5;
            puVar4 = puVar4 + iVar2;
            puVar5 = puVar5 + 1;
          }
          iVar6 = iVar6 + 1;
        } while (iVar6 < iVar2);
      }
      param_3 = param_3 + 4;
      param_4 = param_4 + 1;
      local_8 = local_8 + -1;
    } while (local_8 != 0);
  }
  return;
}

