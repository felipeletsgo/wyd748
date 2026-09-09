// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6ed0 | Name: FUN_004b6ed0


void __thiscall FUN_004b6ed0(int param_1,int param_2,uint param_3,undefined4 param_4)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 local_18;
  
  if ((uint)((*(int *)(param_1 + 0xc) - *(int *)(param_1 + 8)) / 0x28) < param_3) {
    uVar1 = FUN_004b6560();
    if (param_3 < uVar1) {
      local_18 = FUN_004b6560();
    }
    else {
      local_18 = param_3;
    }
    iVar2 = FUN_004b6560();
    iVar3 = FUN_004b73a0(iVar2 + local_18,0);
    iVar4 = FUN_004b7320(*(undefined4 *)(param_1 + 4),param_2,iVar3);
    FUN_004b7360(iVar4,param_3,param_4);
    FUN_004b7320(param_2,*(undefined4 *)(param_1 + 8),iVar4 + param_3 * 0x28);
    FUN_004b6ae0(*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 8));
    FUN_004b6b10(*(undefined4 *)(param_1 + 4),
                 (*(int *)(param_1 + 0xc) - *(int *)(param_1 + 4)) / 0x28);
    *(uint *)(param_1 + 0xc) = iVar3 + (iVar2 + local_18) * 0x28;
    iVar2 = FUN_004b6560();
    *(uint *)(param_1 + 8) = iVar3 + iVar2 * 0x28 + param_3 * 0x28;
    *(int *)(param_1 + 4) = iVar3;
  }
  else if ((uint)((*(int *)(param_1 + 8) - param_2) / 0x28) < param_3) {
    FUN_004b7320(param_2,*(undefined4 *)(param_1 + 8),param_2 + param_3 * 0x28);
    FUN_004b7360(*(undefined4 *)(param_1 + 8),param_3 - (*(int *)(param_1 + 8) - param_2) / 0x28,
                 param_4);
    _Destroy_range<>(param_2,*(undefined4 *)(param_1 + 8),param_4);
    *(uint *)(param_1 + 8) = *(int *)(param_1 + 8) + param_3 * 0x28;
  }
  else if (param_3 != 0) {
    FUN_004b7320(*(int *)(param_1 + 8) + param_3 * -0x28,*(undefined4 *)(param_1 + 8),
                 *(undefined4 *)(param_1 + 8));
    FUN_004b7530(param_2,*(int *)(param_1 + 8) + param_3 * -0x28,*(undefined4 *)(param_1 + 8));
    _Destroy_range<>(param_2,param_2 + param_3 * 0x28,param_4);
    *(uint *)(param_1 + 8) = *(int *)(param_1 + 8) + param_3 * 0x28;
  }
  return;
}

