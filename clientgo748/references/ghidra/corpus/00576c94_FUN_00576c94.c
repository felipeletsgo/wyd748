// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00576c94 | Name: FUN_00576c94


/* WARNING: Removing unreachable block (ram,0x00576ccd) */
/* WARNING: Removing unreachable block (ram,0x00576cfc) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_00576c94(int param_1,uint *param_2)

{
  float fVar1;
  int iVar2;
  float *pfVar3;
  uint uVar4;
  uint uVar5;
  
  iVar2 = FUN_00576b56(param_1,param_2 + 2);
  fVar1 = _DAT_005a69a8;
  if (-1 < iVar2) {
    uVar4 = *param_2;
    iVar2 = 8;
    pfVar3 = (float *)(param_1 + 0xc);
    do {
      uVar5 = uVar4 & 0xf;
      uVar4 = uVar4 >> 4;
      *pfVar3 = (float)uVar5 * fVar1;
      pfVar3 = pfVar3 + 4;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
    uVar4 = param_2[1];
    iVar2 = 8;
    pfVar3 = (float *)(param_1 + 0x8c);
    do {
      uVar5 = uVar4 & 0xf;
      uVar4 = uVar4 >> 4;
      *pfVar3 = (float)uVar5 * fVar1;
      pfVar3 = pfVar3 + 4;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
    iVar2 = 0;
  }
  return iVar2;
}

