// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6c30 | Name: FUN_004b6c30


int __thiscall FUN_004b6c30(int param_1,uint param_2,uint param_3)

{
  char cVar1;
  int iVar2;
  
  if (*(uint *)(param_1 + 8) < param_2) {
    FUN_0059e4cc();
  }
  FUN_004b72b0();
  if (*(int *)(param_1 + 8) - param_2 < param_3) {
    param_3 = *(int *)(param_1 + 8) - param_2;
  }
  if (param_3 != 0) {
    FUN_004b6ce0(*(int *)(param_1 + 4) + param_2,*(int *)(param_1 + 4) + param_2 + param_3,
                 (*(int *)(param_1 + 8) - param_2) - param_3);
    iVar2 = *(int *)(param_1 + 8) - param_3;
    cVar1 = FUN_004b6d70(iVar2,0);
    if (cVar1 != '\0') {
      FUN_004b6d20(iVar2);
    }
  }
  return param_1;
}

