// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056ceb8 | Name: FUN_0056ceb8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056ceb8(int param_1,uint param_2,uint param_3,int param_4)

{
  float *pfVar1;
  int iVar2;
  float fVar3;
  int iVar4;
  uint uVar5;
  undefined2 *extraout_EDX;
  undefined2 *puVar6;
  int iVar7;
  
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
  }
  iVar4 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  uVar5 = 0;
  if (*(int *)(param_1 + 0x1058) != 0) {
    iVar7 = 0;
    puVar6 = extraout_EDX;
    do {
      fVar3 = *(float *)(iVar4 + ((param_3 & 3) + (param_2 & 3) * 8) * 4 + (uVar5 & 3) * 4);
      pfVar1 = (float *)(iVar7 + param_4);
      iVar2 = iVar7 + 4;
      iVar7 = iVar7 + 0x10;
      *puVar6 = CONCAT11((char)(int)ROUND(*(float *)(iVar2 + param_4) * _DAT_005a698c + fVar3),
                         (char)(int)ROUND(*pfVar1 * _DAT_005a698c + fVar3));
      puVar6 = puVar6 + 1;
      uVar5 = uVar5 + 1;
    } while (uVar5 < *(uint *)(param_1 + 0x1058));
  }
  return;
}

