// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00531fbb | Name: FUN_00531fbb


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00531fbb(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  float10 fVar5;
  int local_38;
  float local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  float local_20;
  float local_1c;
  float local_18;
  undefined4 local_14;
  float local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if ((((param_1[6] == -1) || (param_1[6] == DAT_005bdef0)) || (param_1[6] == DAT_005bdef4)) ||
     ((param_1[6] == DAT_005bdef8 || (param_1[6] == DAT_005bdefc)))) {
    iVar3 = (**(code **)(*DAT_0092e654 + 8))();
    if ((param_1[0x18] == 0) || (param_1[9] != 3)) {
      if ((param_1[9] == 5) && (param_1[100] != 0)) {
        FUN_00430f20(param_1[10],param_1[0xb]);
        local_30 = 0.0;
        fVar5 = (float10)FUN_0049d6eb(local_2c,local_28,&local_30);
        if ((float10)_DAT_005a44b0 < fVar5) {
          param_1[0xc] = (int)(local_30 + _DAT_005a586c);
          *(int *)(param_1[100] + 0x78) = param_1[0xc];
        }
      }
    }
    else {
      iVar4 = (**(code **)(*param_1 + 0x4c))();
      if (iVar4 == 1) {
        local_24 = DAT_0067cf38;
        FUN_00430f20(param_1[10],param_1[0xb]);
        local_10 = 0.0;
        fVar5 = (float10)FUN_0049d6eb(local_c,local_8,&local_10);
        if ((float10)_DAT_005a44b0 < fVar5) {
          param_1[0xc] = (int)(local_10 + _DAT_005a40ec);
          (**(code **)(*param_1 + 0x44))(param_1[10],param_1[0xc],param_1[0xb]);
        }
        FUN_0049cb58(&local_20,param_1[10],param_1[0xb]);
        local_20 = local_20 * _DAT_005a40ec + _DAT_005a34a0;
        local_1c = local_1c * _DAT_005a40ec + _DAT_005a34a0;
        local_18 = local_18 * _DAT_005a40ec + _DAT_005a34a0;
        iVar4 = param_1[0x18];
        *(float *)(iVar4 + 0x288) = local_20;
        *(float *)(iVar4 + 0x28c) = local_1c;
        *(float *)(iVar4 + 0x290) = local_18;
        *(undefined4 *)(iVar4 + 0x294) = local_14;
        iVar4 = param_1[0x18];
        iVar1 = param_1[0x18];
        *(undefined4 *)(iVar1 + 0x2a8) = *(undefined4 *)(iVar4 + 0x288);
        *(undefined4 *)(iVar1 + 0x2ac) = *(undefined4 *)(iVar4 + 0x28c);
        *(undefined4 *)(iVar1 + 0x2b0) = *(undefined4 *)(iVar4 + 0x290);
        *(undefined4 *)(iVar1 + 0x2b4) = *(undefined4 *)(iVar4 + 0x294);
        FUN_004bda22(iVar3);
      }
    }
    if (500 < (uint)(iVar3 - param_1[0x66])) {
      for (local_38 = 0; local_38 < 1; local_38 = local_38 + 1) {
        *(int *)(param_1[local_38 + 0x65] + 0x74) = param_1[10];
        *(int *)(param_1[local_38 + 0x65] + 0x7c) = param_1[0xb];
        *(float *)(param_1[local_38 + 0x65] + 0x78) = (float)param_1[0xc] - _DAT_005a4214;
      }
    }
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

