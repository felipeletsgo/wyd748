// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6b30 | Name: FUN_004b6b30


int __thiscall FUN_004b6b30(int param_1,undefined4 param_2,uint param_3,uint param_4)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  
  uVar2 = FUN_004b6430();
  if (uVar2 < param_3) {
    FUN_0059e4cc();
  }
  iVar3 = FUN_004b6430();
  if (iVar3 - param_3 < param_4) {
    param_4 = iVar3 - param_3;
  }
  if ((uint)(DAT_005a467c - *(int *)(param_1 + 8)) <= param_4) {
    FUN_0059e2a0();
  }
  if (param_4 != 0) {
    iVar3 = *(int *)(param_1 + 8) + param_4;
    cVar1 = FUN_004b6d70(iVar3,0);
    if (cVar1 != '\0') {
      iVar4 = FUN_004b6400(param_4);
      FUN_004b66f0(*(int *)(param_1 + 4) + *(int *)(param_1 + 8),iVar4 + param_3);
      FUN_004b6d20(iVar3);
    }
  }
  return param_1;
}

