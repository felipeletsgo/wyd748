// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056c575 | Name: FUN_0056c575


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056c575(int param_1,uint param_2,uint param_3,int param_4)

{
  int iVar1;
  uint uVar2;
  undefined1 *extraout_EDX;
  undefined1 *puVar3;
  int iVar4;
  
  iVar4 = 0;
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
  }
  iVar1 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  uVar2 = 0;
  puVar3 = extraout_EDX;
  if (*(int *)(param_1 + 0x1058) != 0) {
    do {
      *puVar3 = (char)(int)ROUND(*(float *)(iVar4 + 0xc + param_4) * _DAT_005a696c +
                                 *(float *)(iVar1 + ((param_3 & 3) + (param_2 & 3) * 8) * 4 +
                                           (uVar2 & 3) * 4));
      puVar3 = puVar3 + 1;
      uVar2 = uVar2 + 1;
      iVar4 = iVar4 + 0x10;
    } while (uVar2 < *(uint *)(param_1 + 0x1058));
  }
  return;
}

