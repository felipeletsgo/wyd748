// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056d958 | Name: FUN_0056d958


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056d958(int param_1,uint param_2,uint param_3,int param_4)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  float fVar4;
  int iVar5;
  float fVar6;
  int iVar7;
  uint *puVar8;
  uint local_c;
  
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
  }
  puVar8 = (uint *)(*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   *(int *)(param_1 + 0x18));
  iVar5 = *(int *)(param_1 + 0x2c);
  FUN_0056b8e3();
  local_c = 0;
  if (*(int *)(param_1 + 0x1058) != 0) {
    iVar7 = 0;
    do {
      fVar4 = *(float *)(iVar5 + ((param_3 & 3) + (param_2 & 3) * 8) * 4 + (local_c & 3) * 4);
      uVar1 = (uint)ROUND(*(float *)(iVar7 + param_4) * _DAT_005a6984 + fVar4);
      uVar2 = (uint)ROUND(*(float *)(iVar7 + 4 + param_4) * _DAT_005a6984 + fVar4);
      uVar3 = (uint)ROUND(*(float *)(iVar7 + 8 + param_4) * _DAT_005a6984 + fVar4);
      fVar6 = *(float *)(iVar7 + 0xc + param_4) * _DAT_005a6984;
      *puVar8 = uVar2 << 0x10 | uVar3;
      puVar8[1] = (((int)ROUND(fVar6 + fVar4) << 0x10 | uVar1) >> 0x10 | (int)uVar2 >> 0x1f) << 0x10
                  | (uVar1 << 0x10 | uVar2) >> 0x10 | (int)uVar3 >> 0x1f;
      puVar8 = puVar8 + 2;
      local_c = local_c + 1;
      iVar7 = iVar7 + 0x10;
    } while (local_c < *(uint *)(param_1 + 0x1058));
  }
  return;
}

