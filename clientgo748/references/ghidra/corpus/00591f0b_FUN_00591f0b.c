// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591f0b | Name: FUN_00591f0b


undefined4 FUN_00591f0b(undefined4 param_1)

{
  uint uVar1;
  int iVar2;
  
  FUN_00592780();
  uVar1 = FUN_005994a2(DAT_013b873c);
  if (uVar1 < (uint)((int)DAT_013b8738 + (4 - DAT_013b873c))) {
    iVar2 = FUN_005994a2(DAT_013b873c);
    iVar2 = FUN_00599173(DAT_013b873c,iVar2 + 0x10);
    if (iVar2 == 0) {
      param_1 = 0;
      goto LAB_00591f80;
    }
    DAT_013b8738 = (undefined4 *)(iVar2 + ((int)DAT_013b8738 - DAT_013b873c >> 2) * 4);
    DAT_013b873c = iVar2;
  }
  *DAT_013b8738 = param_1;
  DAT_013b8738 = DAT_013b8738 + 1;
LAB_00591f80:
  FUN_00592789();
  return param_1;
}

