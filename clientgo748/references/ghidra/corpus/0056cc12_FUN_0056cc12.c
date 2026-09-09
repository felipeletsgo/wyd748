// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056cc12 | Name: FUN_0056cc12


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056cc12(int param_1,uint param_2,uint param_3,int param_4)

{
  int iVar1;
  float *pfVar2;
  uint uVar3;
  undefined1 *extraout_EDX;
  undefined1 *puVar4;
  int iVar5;
  
  iVar5 = 0;
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
  }
  iVar1 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  uVar3 = 0;
  puVar4 = extraout_EDX;
  if (*(int *)(param_1 + 0x1058) != 0) {
    do {
      pfVar2 = (float *)(param_4 + iVar5);
      *puVar4 = (char)(int)ROUND(*(float *)(iVar1 + ((param_3 & 3) + (param_2 & 3) * 8) * 4 +
                                           (uVar3 & 3) * 4) +
                                 (*pfVar2 * _DAT_005a67e0 +
                                 pfVar2[1] * _DAT_005a67e4 + pfVar2[2] * _DAT_005a67e8) *
                                 _DAT_005a696c);
      puVar4 = puVar4 + 1;
      uVar3 = uVar3 + 1;
      iVar5 = iVar5 + 0x10;
    } while (uVar3 < *(uint *)(param_1 + 0x1058));
  }
  return;
}

