// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056c61e | Name: FUN_0056c61e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056c61e(int param_1,uint param_2,uint param_3,int param_4)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  float fVar5;
  int iVar6;
  int iVar7;
  ushort *extraout_EDX;
  ushort *puVar8;
  uint uVar9;
  
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
  }
  iVar6 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  uVar9 = 0;
  if (*(int *)(param_1 + 0x1058) != 0) {
    iVar7 = 0;
    puVar8 = extraout_EDX;
    do {
      fVar5 = *(float *)(iVar6 + ((param_3 & 3) + (param_2 & 3) * 8) * 4 + (uVar9 & 3) * 4);
      pfVar1 = (float *)(iVar7 + param_4);
      iVar2 = iVar7 + 4;
      iVar3 = iVar7 + 8;
      iVar4 = iVar7 + 0xc;
      iVar7 = iVar7 + 0x10;
      *puVar8 = (ushort)((((int)ROUND(*(float *)(iVar4 + param_4) * _DAT_005a696c + fVar5) << 3 |
                          (int)ROUND(*pfVar1 * _DAT_005a697c + fVar5)) << 3 |
                         (int)ROUND(*(float *)(iVar2 + param_4) * _DAT_005a697c + fVar5)) << 2) |
                (ushort)(int)ROUND(*(float *)(iVar3 + param_4) * _DAT_005a6800 + fVar5);
      puVar8 = puVar8 + 1;
      uVar9 = uVar9 + 1;
    } while (uVar9 < *(uint *)(param_1 + 0x1058));
  }
  return;
}

