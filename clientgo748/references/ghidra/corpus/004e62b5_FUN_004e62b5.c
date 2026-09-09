// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e62b5 | Name: FUN_004e62b5


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e62b5(int *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  float10 fVar3;
  undefined1 *puVar4;
  int iVar5;
  undefined4 uVar6;
  undefined1 local_5c [12];
  undefined1 local_50 [12];
  undefined1 local_44 [12];
  uint local_38;
  int local_34;
  undefined1 local_30 [16];
  uint local_20;
  int local_18;
  int local_14;
  undefined4 local_10 [2];
  undefined4 local_8;
  
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  local_14 = FUN_004b14fd(0x2c4,1,180000);
  if (local_14 != 0) {
    (**(code **)(**(int **)(local_14 + 0x318) + 0x34))(*(undefined4 *)(local_14 + 0x318),local_30);
    (**(code **)(**(int **)(local_14 + 0x318) + 0x2c))
              (*(undefined4 *)(local_14 + 0x318),0,0,&local_18,0);
    local_38 = local_20 / 0x18;
    for (local_34 = 0; local_34 < (int)local_38; local_34 = local_34 + 1) {
      *(undefined4 *)(local_18 + 0xc + local_34 * 0x18) = 0xffffffff;
    }
    (**(code **)(**(int **)(local_14 + 0x318) + 0x30))(*(undefined4 *)(local_14 + 0x318));
    *(undefined4 *)(local_14 + 0x298) = 0x13;
  }
  (**(code **)(*param_1 + 0x38))();
  iVar5 = param_1[0x2a];
  uVar6 = 0;
  param_1[0x29] = (int)((float)(uint)(iVar1 - param_1[0x20]) / (float)iVar5);
  if ((_DAT_005a34a0 < (float)param_1[0x29]) && (param_1[7] != 0)) {
    param_1[7] = 0;
  }
  if (param_1[7] != 0) {
    param_1[0x24] = *(int *)(param_1[7] + 0x28);
    param_1[0x26] = *(int *)(param_1[7] + 0x2c);
    param_1[0x25] = (int)(*(float *)(param_1[7] + 0x30) + _DAT_005a3660);
  }
  FUN_00493db0(local_10,param_1 + 0x21);
  fVar3 = (float10)FUN_00493c30(local_10[0],local_8,iVar5,uVar6);
  param_1[0x28] = (int)(float)(fVar3 - (float10)_DAT_005a4380);
  if (_DAT_005a3660 <= (float)param_1[0x29]) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  else {
    uVar6 = FUN_004c8c60(local_50,param_1[0x29]);
    puVar4 = local_5c;
    FUN_004c8c60(local_44,_DAT_005a3660 - (float)param_1[0x29]);
    uVar6 = FUN_00493d50(puVar4,uVar6);
    FUN_004310a0(uVar6);
    if (param_1[0x2b] != 0) {
      puVar2 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
      (**(code **)(*(int *)param_1[0x2b] + 0x3c))(*puVar2,puVar2[1]);
    }
  }
  return 1;
}

