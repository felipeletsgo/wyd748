// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056ccd7 | Name: FUN_0056ccd7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056ccd7(int param_1,uint param_2,uint param_3,int param_4)

{
  int iVar1;
  float fVar2;
  int iVar3;
  float *pfVar4;
  ushort *extraout_EDX;
  ushort *puVar5;
  int iVar6;
  uint uVar7;
  
  uVar7 = 0;
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
  }
  iVar3 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  if (*(int *)(param_1 + 0x1058) != 0) {
    iVar6 = 0;
    puVar5 = extraout_EDX;
    do {
      fVar2 = *(float *)(iVar3 + ((param_3 & 3) + (param_2 & 3) * 8) * 4 + (uVar7 & 3) * 4);
      pfVar4 = (float *)(param_4 + iVar6);
      iVar1 = iVar6 + 0xc;
      iVar6 = iVar6 + 0x10;
      *puVar5 = (ushort)(((int)ROUND(*(float *)(iVar1 + param_4) * _DAT_005a696c + fVar2) & 0xffU)
                        << 8) |
                (ushort)(int)ROUND((*pfVar4 * _DAT_005a67e0 +
                                   pfVar4[1] * _DAT_005a67e4 + pfVar4[2] * _DAT_005a67e8) *
                                   _DAT_005a696c + fVar2);
      puVar5 = puVar5 + 1;
      uVar7 = uVar7 + 1;
    } while (uVar7 < *(uint *)(param_1 + 0x1058));
  }
  return;
}

