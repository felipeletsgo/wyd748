// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056eb55 | Name: FUN_0056eb55


/* WARNING: Removing unreachable block (ram,0x0056ebd4) */
/* WARNING: Removing unreachable block (ram,0x0056ebb2) */
/* WARNING: Removing unreachable block (ram,0x0056ebed) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056eb55(int param_1,int param_2,int param_3,float *param_4)

{
  uint *puVar1;
  float fVar2;
  ushort uVar3;
  uint *puVar4;
  
  puVar4 = (uint *)(*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * param_3 +
                   *(int *)(param_1 + 0x18));
  puVar1 = puVar4 + *(int *)(param_1 + 0x1058) * 2;
  for (; puVar4 < puVar1; puVar4 = puVar4 + 2) {
    uVar3 = __aullshr();
    fVar2 = _DAT_005a69b4;
    *param_4 = (float)uVar3 * _DAT_005a69b4;
    param_4[1] = (float)(*puVar4 >> 0x10) * fVar2;
    param_4[2] = (float)(*puVar4 & 0xffff) * fVar2;
    param_4[3] = (float)*(ushort *)((int)puVar4 + 6) * fVar2;
    param_4 = param_4 + 4;
  }
  if (*(int *)(param_1 + 0x10) != 0) {
    FUN_0056bcb5(param_4 + *(int *)(param_1 + 0x1058) * -4);
  }
  return;
}

