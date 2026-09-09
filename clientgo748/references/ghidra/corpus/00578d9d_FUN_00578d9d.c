// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578d9d | Name: FUN_00578d9d


void FUN_00578d9d(int *param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  
  param_1[1] = 0;
  uVar1 = FUN_005840ee(param_1);
  puVar2 = (undefined4 *)FUN_005840ca(param_1,0x50);
  if (puVar2 == (undefined4 *)0x0) {
    FUN_0058ea01(param_1);
    *(undefined4 *)(*param_1 + 0x14) = 0x35;
    *(undefined4 *)(*param_1 + 0x18) = 0;
    (**(code **)*param_1)(param_1);
  }
  else {
    *puVar2 = FUN_0057843d;
    puVar2[1] = FUN_00578539;
    puVar2[2] = FUN_005785c7;
    puVar2[3] = FUN_00578659;
    puVar2[4] = FUN_005786f7;
    puVar2[5] = FUN_00578758;
    puVar2[6] = FUN_005787b9;
    puVar2[7] = FUN_00578a4b;
    puVar2[8] = FUN_00578b69;
    puVar2[9] = FUN_00578c8c;
    puVar2[10] = FUN_00578d6d;
    puVar2[0xb] = uVar1;
    puVar3 = puVar2 + 0xf;
    iVar4 = 2;
    do {
      puVar3[-2] = 0;
      *puVar3 = 0;
      puVar3 = puVar3 + -1;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
    puVar2[0x10] = 0;
    puVar2[0x11] = 0;
    puVar2[0x12] = 0x50;
    param_1[1] = (int)puVar2;
  }
  return;
}

