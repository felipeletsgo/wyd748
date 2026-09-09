// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00520fa4 | Name: FUN_00520fa4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00520fa4(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  float10 fVar4;
  undefined1 local_18 [8];
  float local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    iVar1 = __ftol();
    iVar2 = __ftol();
    if (iVar1 == iVar2) {
      iVar1 = __ftol();
      iVar2 = __ftol();
      if (iVar1 == iVar2) {
        return;
      }
    }
    FUN_00493c60(&local_c,param_1 + 0x28);
    FUN_0040bd30(&stack0x00000004);
    fVar4 = (float10)FUN_00493c30(local_c,local_8);
    *(float *)(param_1 + 0x22c) = (float)(fVar4 + (float10)_DAT_005a4380);
    if (_DAT_005a3534 <= *(float *)(param_1 + 0x34)) {
      if (_DAT_005a4290 < *(float *)(param_1 + 0x34)) {
        *(float *)(param_1 + 0x34) = *(float *)(param_1 + 0x34) - _DAT_005a4290;
      }
    }
    else {
      *(float *)(param_1 + 0x34) = *(float *)(param_1 + 0x34) + _DAT_005a4290;
    }
    *(undefined4 *)(param_1 + 0x224) = *(undefined4 *)(param_1 + 0x34);
    if ((_DAT_005a3534 <= *(float *)(param_1 + 0x22c) - *(float *)(param_1 + 0x224)) ||
       (_DAT_005a46d4 <= *(float *)(param_1 + 0x22c) - *(float *)(param_1 + 0x224))) {
      if ((_DAT_005a3534 < *(float *)(param_1 + 0x22c) - *(float *)(param_1 + 0x224)) &&
         (_DAT_005a4148 < *(float *)(param_1 + 0x22c) - *(float *)(param_1 + 0x224))) {
        *(float *)(param_1 + 0x22c) = *(float *)(param_1 + 0x22c) - _DAT_005a4290;
      }
    }
    else {
      *(float *)(param_1 + 0x22c) = *(float *)(param_1 + 0x22c) + _DAT_005a4290;
    }
    FUN_0040bd30(param_1 + 0x28);
    uVar3 = FUN_00493c60(local_18,param_1 + 0x28);
    FUN_0040bd30(uVar3);
    uVar3 = FUN_00430f20(0,0);
    fVar4 = (float10)FUN_004d0150(uVar3);
    local_10 = (float)fVar4;
    uVar3 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x5f0) = uVar3;
  }
  return;
}

