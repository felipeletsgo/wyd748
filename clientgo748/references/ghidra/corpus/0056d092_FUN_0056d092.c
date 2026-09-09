// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056d092 | Name: FUN_0056d092


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056d092(int param_1,uint param_2,uint param_3,int param_4)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  float fVar4;
  int iVar5;
  uint uVar6;
  uint *extraout_EDX;
  uint *puVar7;
  int iVar8;
  undefined1 local_24;
  
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
  }
  iVar5 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  uVar6 = 0;
  if (*(int *)(param_1 + 0x1058) != 0) {
    iVar8 = 0;
    puVar7 = extraout_EDX;
    do {
      fVar4 = *(float *)(iVar5 + ((param_3 & 3) + (param_2 & 3) * 8) * 4 + (uVar6 & 3) * 4);
      pfVar1 = (float *)(iVar8 + param_4);
      iVar2 = iVar8 + 4;
      iVar3 = iVar8 + 0xc;
      iVar8 = iVar8 + 0x10;
      local_24 = (undefined1)(int)ROUND(*(float *)(iVar2 + param_4) * _DAT_005a698c + fVar4);
      *puVar7 = (uint)CONCAT11((char)(int)ROUND(*(float *)(iVar3 + param_4) * _DAT_005a696c + fVar4)
                               ,local_24) << 8 | (int)ROUND(*pfVar1 * _DAT_005a698c + fVar4) & 0xffU
      ;
      puVar7 = puVar7 + 1;
      uVar6 = uVar6 + 1;
    } while (uVar6 < *(uint *)(param_1 + 0x1058));
  }
  return;
}

