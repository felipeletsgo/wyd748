// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004055c1 | Name: FUN_004055c1


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_004055c1(int param_1,undefined4 param_2,float param_3,float param_4,undefined4 param_5,
            undefined4 param_6)

{
  int iVar1;
  undefined4 uVar2;
  int local_18;
  int local_10;
  
  FUN_00401637(param_2,param_3,param_4,param_5,param_6);
  if (*(int *)(param_1 + 0x1e4) == 1) {
    *(float *)(param_1 + 0x1fc) = param_3 + *(float *)(param_1 + 0x4c) + _DAT_005a365c;
    *(float *)(param_1 + 0x200) = param_4 + *(float *)(param_1 + 0x50) + _DAT_005a365c;
    *(undefined4 *)(param_1 + 0x204) = *(undefined4 *)(param_1 + 0x1f0);
    *(float *)(param_1 + 0x208) = *(float *)(param_1 + 0x58) - _DAT_005a3834;
    *(undefined4 *)(param_1 + 0x340) = param_5;
  }
  else if (*(int *)(param_1 + 0x1e4) == 2) {
    local_10 = *(int *)(param_1 + 0x234);
    if (local_10 < -2) {
      local_10 = -*(int *)(param_1 + 0x234);
    }
    iVar1 = FUN_004b8c45(local_10);
    if (iVar1 != 0) {
      uVar2 = __ftol();
      *(undefined4 *)(*(int *)(iVar1 + 4) + 0xc + *(int *)(param_1 + 0x238) * 0x1c) = uVar2;
    }
    *(float *)(param_1 + 0x1fc) = param_3 + *(float *)(param_1 + 0x4c) + _DAT_005a365c;
    *(float *)(param_1 + 0x200) = param_4 + *(float *)(param_1 + 0x50) + _DAT_005a365c;
    *(undefined4 *)(param_1 + 0x204) = *(undefined4 *)(param_1 + 0x1f0);
    *(float *)(param_1 + 0x208) = *(float *)(param_1 + 0x58) - _DAT_005a3834;
    *(undefined4 *)(param_1 + 0x340) = param_5;
  }
  else {
    local_18 = *(int *)(param_1 + 0x234);
    if (local_18 < -2) {
      local_18 = -*(int *)(param_1 + 0x234);
    }
    iVar1 = FUN_004b8c45(local_18);
    if (iVar1 != 0) {
      uVar2 = __ftol();
      *(undefined4 *)(*(int *)(iVar1 + 4) + 0x10 + *(int *)(param_1 + 0x238) * 0x1c) = uVar2;
      uVar2 = __ftol();
      *(undefined4 *)(*(int *)(iVar1 + 4) + 8 + *(int *)(param_1 + 0x238) * 0x1c) = uVar2;
    }
    *(float *)(param_1 + 0x1fc) = param_3 + *(float *)(param_1 + 0x4c);
    *(float *)(param_1 + 0x200) =
         (param_4 + *(float *)(param_1 + 0x50) + *(float *)(param_1 + 0x58)) -
         *(float *)(param_1 + 500);
    *(undefined4 *)(param_1 + 0x204) = *(undefined4 *)(param_1 + 0x54);
    *(undefined4 *)(param_1 + 0x208) = *(undefined4 *)(param_1 + 500);
    *(undefined4 *)(param_1 + 0x340) = param_5;
  }
  FUN_0040c26d(param_2,param_1 + 0x1f8,param_5);
  return;
}

