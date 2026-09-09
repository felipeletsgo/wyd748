// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053f852 | Name: FUN_0053f852


undefined4 __fastcall FUN_0053f852(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  iVar1 = __ftol();
  iVar1 = (int)(iVar1 + (iVar1 >> 0x1f & 3U)) >> 2;
  iVar2 = __ftol();
  iVar2 = (int)(iVar2 + (iVar2 >> 0x1f & 3U)) >> 2;
  if ((((iVar2 < 0x400) && (iVar1 < 0x400)) && (-1 < iVar2)) && (-1 < iVar1)) {
    if (((&DAT_00934178)[iVar1 + iVar2 * 0x400] & 0x40) == 0) {
      uVar3 = 0;
    }
    else {
      uVar3 = 1;
    }
  }
  else {
    FUN_00431159(s_Wrong_Position__X__d_Y__d__005bdf34);
    if (*(int *)(DAT_0067cf38 + 0x54) == 0) {
      uVar3 = __ftol();
      uVar4 = __ftol(uVar3);
      FUN_0049ed58(7,*(undefined4 *)(param_1 + 0x20),*(undefined4 *)(param_1 + 0x24),uVar4,uVar3);
    }
    *(undefined4 *)(DAT_0067cf38 + 0x54) = 1;
    uVar3 = 0;
  }
  return uVar3;
}

