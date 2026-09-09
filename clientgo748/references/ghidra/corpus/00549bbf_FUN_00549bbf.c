// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00549bbf | Name: FUN_00549bbf


void __thiscall FUN_00549bbf(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_004b14fd(*(undefined4 *)(param_1 + 0x24),1,180000);
  if (iVar1 != 0) {
    if (param_2 / 10 == 0) {
      *(int *)(iVar1 + 0x298) = param_2 + 0x43;
      *(uint *)(DAT_005ccf9c + 0x2b034) =
           (uint)*(byte *)(param_1 + 0x2168 + *(int *)(iVar1 + 0x298) * 4) << 0x10 |
           (uint)*(byte *)(param_1 + 0x2178 + *(int *)(iVar1 + 0x298) * 4) << 8 |
           (uint)*(byte *)(param_1 + 0x2188 + *(int *)(iVar1 + 0x298) * 4);
    }
    else if (param_2 / 10 == 1) {
      *(int *)(iVar1 + 0x298) = *(int *)(param_1 + 0x164) % 10 + 0x43;
      *(int *)(param_1 + 0x160) = param_2 % 10 + 0x43;
    }
    *(int *)(param_1 + 0x164) = param_2;
    uVar2 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x168) = uVar2;
  }
  return;
}

