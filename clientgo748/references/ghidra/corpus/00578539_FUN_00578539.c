// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578539 | Name: FUN_00578539


undefined4 * FUN_00578539(int *param_1,int param_2,uint param_3)

{
  int *piVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  
  iVar3 = param_1[1];
  if (0x3b9ac9f0 < param_3) {
    FUN_00578428();
  }
  if ((param_3 & 7) != 0) {
    param_3 = param_3 + (8 - (param_3 & 7));
  }
  if ((param_2 < 0) || (1 < param_2)) {
    *(undefined4 *)(*param_1 + 0x14) = 0xc;
    *(int *)(*param_1 + 0x18) = param_2;
    (**(code **)*param_1)(param_1);
  }
  puVar5 = (undefined4 *)FUN_005840ca(param_1,param_3 + 0x10);
  if (puVar5 == (undefined4 *)0x0) {
    FUN_00578428();
  }
  piVar1 = (int *)(iVar3 + 0x48);
  *piVar1 = *piVar1 + param_3 + 0x10;
  puVar2 = (undefined4 *)(iVar3 + 0x38 + param_2 * 4);
  uVar4 = *puVar2;
  puVar5[2] = 0;
  puVar5[1] = param_3;
  *puVar5 = uVar4;
  *puVar2 = puVar5;
  return puVar5 + 4;
}

