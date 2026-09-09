// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056dbe6 | Name: FUN_0056dbe6


/* WARNING: Removing unreachable block (ram,0x0056dc49) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056dbe6(int param_1,int param_2,int param_3,float *param_4)

{
  uint *puVar1;
  float fVar2;
  uint *puVar3;
  
  fVar2 = _DAT_005a68b0;
  puVar3 = (uint *)(*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   *(int *)(param_1 + 0x18));
  puVar1 = puVar3 + *(int *)(param_1 + 0x1058);
  for (; puVar3 < puVar1; puVar3 = puVar3 + 1) {
    *param_4 = (float)*(byte *)((int)puVar3 + 2) * fVar2;
    param_4[1] = (float)*(byte *)((int)puVar3 + 1) * fVar2;
    param_4[2] = (float)(*puVar3 & 0xff) * fVar2;
    param_4[3] = 1.0;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

