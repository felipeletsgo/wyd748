// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6670 | Name: FUN_004b6670


int __thiscall FUN_004b6670(int param_1,undefined4 param_2,uint param_3)

{
  char cVar1;
  int iVar2;
  
  if ((uint)(DAT_005a467c - *(int *)(param_1 + 8)) <= param_3) {
    FUN_0059e2a0();
  }
  if (param_3 != 0) {
    iVar2 = *(int *)(param_1 + 8) + param_3;
    cVar1 = FUN_004b6d70(iVar2,0);
    if (cVar1 != '\0') {
      FUN_004b66f0(*(int *)(param_1 + 4) + *(int *)(param_1 + 8),param_2,param_3);
      FUN_004b6d20(iVar2);
    }
  }
  return param_1;
}

