// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057843d | Name: FUN_0057843d


int FUN_0057843d(int *param_1,int param_2,undefined4 *param_3)

{
  int iVar1;
  int *piVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  uint uVar7;
  uint uVar8;
  
  iVar4 = param_1[1];
  if (0x3b9ac9f0 < param_3) {
    FUN_00578428();
  }
  uVar7 = (uint)param_3;
  if (((uint)param_3 & 7) != 0) {
    uVar7 = (int)param_3 + (8 - ((uint)param_3 & 7));
  }
  if ((param_2 < 0) || (1 < param_2)) {
    *(undefined4 *)(*param_1 + 0x14) = 0xc;
    *(int *)(*param_1 + 0x18) = param_2;
    (**(code **)*param_1)(param_1);
  }
  param_3 = (undefined4 *)0x0;
  puVar3 = (undefined4 *)(iVar4 + 0x30 + param_2 * 4);
  puVar6 = (undefined4 *)*puVar3;
  if (puVar6 != (undefined4 *)0x0) {
    do {
      puVar5 = puVar6;
      puVar6 = puVar5;
      if (uVar7 <= (uint)puVar5[2]) break;
      puVar6 = (undefined4 *)*puVar5;
      param_3 = puVar5;
    } while (puVar6 != (undefined4 *)0x0);
    if (puVar6 != (undefined4 *)0x0) goto LAB_00578523;
  }
  iVar1 = uVar7 + 0x10;
  if (param_3 == (undefined4 *)0x0) {
    uVar8 = *(uint *)(&DAT_005a8a10 + param_2 * 4);
  }
  else {
    uVar8 = *(uint *)(&DAT_005a8a18 + param_2 * 4);
  }
  if (1000000000U - iVar1 < uVar8) {
    uVar8 = 1000000000U - iVar1;
  }
  while( true ) {
    puVar6 = (undefined4 *)FUN_005840ca(param_1,uVar8 + iVar1);
    if (puVar6 != (undefined4 *)0x0) break;
    uVar8 = uVar8 >> 1;
    if (uVar8 < 0x32) {
      FUN_00578428();
    }
  }
  piVar2 = (int *)(iVar4 + 0x48);
  *piVar2 = *piVar2 + uVar8 + iVar1;
  *puVar6 = 0;
  puVar6[1] = 0;
  puVar6[2] = uVar8 + uVar7;
  if (param_3 == (undefined4 *)0x0) {
    param_3 = puVar3;
  }
  *param_3 = puVar6;
LAB_00578523:
  iVar4 = puVar6[1];
  puVar6[2] = puVar6[2] - uVar7;
  puVar6[1] = iVar4 + uVar7;
  return iVar4 + 0x10 + (int)puVar6;
}

