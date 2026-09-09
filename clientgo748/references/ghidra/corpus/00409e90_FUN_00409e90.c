// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409e90 | Name: FUN_00409e90


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_00409e90(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            )

{
  undefined4 *puVar1;
  float *pfVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined1 local_28 [8];
  undefined1 local_20 [8];
  undefined1 local_18 [8];
  undefined1 local_10 [8];
  uint local_8;
  
  *(float *)(param_1 + 0x68) = *(float *)(param_1 + 0x4c) - _DAT_005a3534;
  *(undefined4 *)(param_1 + 0x6c) = *(undefined4 *)(param_1 + 0x50);
  if (DAT_005ccec8 == 2) {
    *(undefined4 *)(param_1 + 0x68) = 0xc2c80000;
    *(undefined4 *)(param_1 + 0x6c) = 0xc2c80000;
  }
  *(undefined4 *)(param_1 + 0x70) = *(undefined4 *)(param_1 + 0x54);
  *(undefined4 *)(param_1 + 0x74) = *(undefined4 *)(param_1 + 0x58);
  *(undefined4 *)(param_1 + 0x1ac) = param_5;
  *(undefined4 *)(param_1 + 0x24) = 1;
  if (*(int *)(param_1 + 0x1e4) == 2) {
    if (*(int *)(param_1 + 0x1e8) == 0) {
      *(undefined4 *)(param_1 + 0x1e4) = 0;
    }
    else {
      puVar1 = (undefined4 *)(**(code **)(**(int **)(param_1 + 0x1e8) + 0x84))();
      puVar4 = (undefined4 *)(param_1 + 0x1ec);
      for (iVar3 = 0x5b; iVar3 != 0; iVar3 = iVar3 + -1) {
        *puVar4 = *puVar1;
        puVar1 = puVar1 + 1;
        puVar4 = puVar4 + 1;
      }
      if (*(int *)(param_1 + 0x1ec) == 5) {
        pfVar2 = (float *)FUN_0040c110(local_10);
        *(float *)(param_1 + 0x1f0) = *(float *)(param_1 + 0x4c) + *pfVar2;
        iVar3 = FUN_0040c110(local_18);
        *(float *)(param_1 + 500) = *(float *)(param_1 + 0x50) + *(float *)(iVar3 + 4);
        *(undefined4 *)(param_1 + 0x334) = *(undefined4 *)(param_1 + 0x1ac);
        *(undefined4 *)(param_1 + 0x208) = 0x3fc00000;
        local_8 = (**(code **)(*DAT_0092e654 + 8))();
        local_8 = local_8 % 3000;
        *(float *)(param_1 + 0x204) = ((float)local_8 * _DAT_005a35c4) / _DAT_005a35c0;
      }
      else {
        pfVar2 = (float *)FUN_0040c110(local_20);
        *(float *)(param_1 + 0x1f0) = (*(float *)(param_1 + 0x4c) + *pfVar2) - _DAT_005a3e9c;
        iVar3 = FUN_0040c110(local_28);
        *(float *)(param_1 + 500) =
             (*(float *)(param_1 + 0x50) + *(float *)(iVar3 + 4)) - _DAT_005a3e9c;
        *(undefined4 *)(param_1 + 0x334) = *(undefined4 *)(param_1 + 0x1ac);
      }
      FUN_0040c26d(param_2,param_1 + 0x1ec,param_5);
      FUN_0040c26d(param_2,param_1 + 100,param_5);
    }
  }
  else {
    FUN_0040c26d(param_2,param_1 + 100,param_5);
  }
  return;
}

