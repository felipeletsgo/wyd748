// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053f6b0 | Name: FUN_0053f6b0


undefined4 FUN_0053f6b0(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int extraout_var;
  
  iVar1 = __ftol();
  iVar2 = (int)(iVar1 + (iVar1 >> 0x1f & 3U)) >> 2;
  iVar1 = extraout_var;
  iVar3 = __ftol();
  iVar3 = (int)(iVar3 + (iVar3 >> 0x1f & 3U)) >> 2;
  __ftol();
  iVar4 = __ftol();
  iVar4 = (int)(iVar4 + (iVar4 >> 0x1f & 0x7fU)) >> 7;
  if ((((iVar3 < 0x400) && (iVar2 < 0x400)) && (-1 < iVar3)) && (-1 < iVar2)) {
    if (((&DAT_00934178)[iVar2 + iVar3 * 0x400] & 1) == 0) {
      uVar6 = 0;
    }
    else {
      uVar6 = 1;
    }
  }
  else {
    FUN_00431159(s_Wrong_Position__X__d_Y__d__005bdf14);
    iVar2 = (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
    uVar6 = *(undefined4 *)(iVar2 + 0x28);
    uVar5 = *(undefined4 *)(iVar2 + 0x2c);
    iVar2 = __ftol(iVar1,uVar6,uVar5);
    iVar2 = (int)(iVar2 + (iVar2 >> 0x1f & 0x7fU)) >> 7;
    iVar3 = __ftol();
    iVar3 = (int)(iVar3 + (iVar3 >> 0x1f & 0x7fU)) >> 7;
    if (((((iVar2 == 0xd) && (iVar3 == 0x1f)) || ((iVar2 == 0xe && (iVar3 == 0x1e)))) ||
        ((iVar2 == 0xf && (iVar3 == 0x1f)))) || ((iVar2 == 0x12 && (iVar3 == 0x1e)))) {
      *(undefined4 *)(DAT_0067cf38 + 0x54) = 0;
    }
    else {
      if (*(int *)(DAT_0067cf38 + 0x54) == 0) {
        uVar6 = __ftol(iVar1,uVar6,uVar5,iVar3,iVar2,iVar4);
        uVar5 = __ftol(uVar6);
        FUN_0049ed58(6,*(undefined4 *)(iVar1 + 0x20),*(undefined4 *)(iVar1 + 0x24),uVar5,uVar6);
      }
      *(undefined4 *)(DAT_0067cf38 + 0x54) = 1;
    }
    uVar6 = 0;
  }
  return uVar6;
}

