// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056df28 | Name: FUN_0056df28


/* WARNING: Removing unreachable block (ram,0x0056df8f) */
/* WARNING: Removing unreachable block (ram,0x0056df74) */
/* WARNING: Removing unreachable block (ram,0x0056dfab) */
/* WARNING: Removing unreachable block (ram,0x0056dfc5) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056df28(int param_1,int param_2,int param_3,float *param_4)

{
  uint *puVar1;
  float fVar2;
  uint *puVar3;
  
  fVar2 = _DAT_005a69b0;
  puVar3 = (uint *)(*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   *(int *)(param_1 + 0x18));
  puVar1 = puVar3 + *(int *)(param_1 + 0x1058);
  for (; puVar3 < puVar1; puVar3 = puVar3 + 1) {
    *param_4 = (float)(*puVar3 & 0x3ff) * fVar2;
    param_4[1] = (float)(*puVar3 >> 10 & 0x3ff) * fVar2;
    param_4[2] = (float)(*puVar3 >> 0x14 & 0x3ff) * fVar2;
    param_4[3] = (float)(*puVar3 >> 0x1e) * _DAT_005a69ac;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

