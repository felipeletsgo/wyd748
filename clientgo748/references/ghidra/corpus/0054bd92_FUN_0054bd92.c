// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054bd92 | Name: FUN_0054bd92


bool FUN_0054bd92(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  short sVar2;
  short sVar3;
  int iVar4;
  uint uVar5;
  
  uVar5 = (uint)*(byte *)(param_1 + 0x14);
  iVar4 = FUN_00559ef5(param_2);
  if (uVar5 < 4) {
    uVar1 = *(undefined4 *)(&DAT_005be108 + uVar5 * 0x18);
    sVar3 = (*(short *)(param_1 + 0x3a) - (short)*(undefined4 *)(&DAT_005be104 + uVar5 * 0x18)) * 2;
    sVar2 = *(short *)(param_1 + 0x24) *
            (short)*(undefined4 *)(&DAT_005be2fc + (uVar5 + iVar4 * 4) * 4);
    *(short *)(param_1 + 0x2c) = (short)uVar1 + sVar3 + sVar2;
    *(short *)(param_1 + 0x48) = (short)uVar1 + sVar3 + sVar2;
    uVar1 = *(undefined4 *)(&DAT_005be10c + uVar5 * 0x18);
    sVar3 = (*(short *)(param_1 + 0x36) - (short)*(undefined4 *)(&DAT_005be0fc + uVar5 * 0x18)) * 2;
    sVar2 = *(short *)(param_1 + 0x24) *
            (short)*(undefined4 *)(&DAT_005be31c + (uVar5 + iVar4 * 4) * 4);
    *(short *)(param_1 + 0x2e) = (short)uVar1 + sVar3 + sVar2;
    *(short *)(param_1 + 0x4a) = (short)uVar1 + sVar3 + sVar2;
  }
  return uVar5 < 4;
}

