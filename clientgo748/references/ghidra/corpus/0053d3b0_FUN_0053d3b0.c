// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053d3b0 | Name: FUN_0053d3b0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0053d3b0(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  float10 fVar4;
  float local_24;
  float local_20;
  float local_1c;
  undefined4 local_18;
  float local_14;
  undefined1 local_10 [8];
  float local_8;
  
  if (((((*(int *)(param_1 + 0x24) == 0x138) && (iVar2 = __ftol(), iVar2 == 0xe30)) &&
       (iVar2 = __ftol(), iVar2 == 0xc22)) ||
      ((*(int *)(param_1 + 0x18) == -1 || (*(int *)(param_1 + 0x18) == DAT_005bdef0)))) ||
     ((*(int *)(param_1 + 0x18) == DAT_005bdef4 ||
      ((*(int *)(param_1 + 0x18) == DAT_005bdef8 || (*(int *)(param_1 + 0x18) == DAT_005bdefc))))))
  {
    FUN_00430f20(*(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x48),
                 *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x50));
    fVar4 = (float10)FUN_0053d6c0(local_10);
    local_14 = (float)fVar4;
    local_8 = *(float *)(DAT_005ccf9c + 0x2b044) * *(float *)(DAT_005ccf9c + 0x2b048);
    if (local_14 <= _DAT_005a59bc) {
      uVar3 = (**(code **)(*DAT_0092e654 + 8))();
      if (*(int *)(param_1 + 0x60) != 0) {
        *(undefined4 *)(param_1 + 0x48) = 1;
        iVar2 = FUN_0049c70f(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c));
        if (iVar2 < 0x7f) {
          *(float *)(param_1 + 0x30) = (float)iVar2 * _DAT_005a4214;
        }
        FUN_0049cb58(&local_24,*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c));
        local_24 = local_24 * _DAT_005a40ec + _DAT_005a34a0;
        local_20 = local_20 * _DAT_005a40ec + _DAT_005a34a0;
        local_1c = local_1c * _DAT_005a40ec + _DAT_005a34a0;
        iVar2 = *(int *)(param_1 + 0x60);
        *(float *)(iVar2 + 0x288) = local_24;
        *(float *)(iVar2 + 0x28c) = local_20;
        *(float *)(iVar2 + 0x290) = local_1c;
        *(undefined4 *)(iVar2 + 0x294) = local_18;
        iVar2 = *(int *)(param_1 + 0x60);
        iVar1 = *(int *)(param_1 + 0x60);
        *(undefined4 *)(iVar1 + 0x2a8) = *(undefined4 *)(iVar2 + 0x288);
        *(undefined4 *)(iVar1 + 0x2ac) = *(undefined4 *)(iVar2 + 0x28c);
        *(undefined4 *)(iVar1 + 0x2b0) = *(undefined4 *)(iVar2 + 0x290);
        *(undefined4 *)(iVar1 + 0x2b4) = *(undefined4 *)(iVar2 + 0x294);
        FUN_004bda22(uVar3);
      }
      uVar3 = 1;
    }
    else {
      uVar3 = 1;
    }
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}

