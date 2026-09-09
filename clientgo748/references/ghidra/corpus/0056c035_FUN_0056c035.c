// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056c035 | Name: FUN_0056c035


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056c035(int param_1,uint param_2,uint param_3,int param_4)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  ushort *extraout_EDX;
  ushort *puVar7;
  uint uVar8;
  
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
  }
  iVar5 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  uVar8 = 0;
  if (*(int *)(param_1 + 0x1058) != 0) {
    iVar6 = 0;
    puVar7 = extraout_EDX;
    do {
      fVar4 = *(float *)(iVar5 + ((param_3 & 3) + (param_2 & 3) * 8) * 4 + (uVar8 & 3) * 4);
      pfVar1 = (float *)(iVar6 + param_4);
      iVar2 = iVar6 + 4;
      iVar3 = iVar6 + 8;
      iVar6 = iVar6 + 0x10;
      *puVar7 = (ushort)(((int)ROUND(*pfVar1 * _DAT_005a6974 + fVar4) << 6 |
                         (int)ROUND(*(float *)(iVar2 + param_4) * _DAT_005a6970 + fVar4)) << 5) |
                (ushort)(int)ROUND(*(float *)(iVar3 + param_4) * _DAT_005a6974 + fVar4);
      puVar7 = puVar7 + 1;
      uVar8 = uVar8 + 1;
    } while (uVar8 < *(uint *)(param_1 + 0x1058));
  }
  return;
}

