// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00579acf | Name: FUN_00579acf


bool FUN_00579acf(int param_1)

{
  int iVar1;
  int iVar2;
  bool bVar3;
  
  iVar1 = param_1;
  bVar3 = true;
  if ((*(byte *)(param_1 + 0x10c) & 0x20) == 0) {
    if ((*(byte *)(param_1 + 0x5d) & 8) == 0) goto LAB_00579afb;
  }
  else if ((*(uint *)(param_1 + 0x5c) & 0x300) != 0x300) goto LAB_00579afb;
  bVar3 = false;
LAB_00579afb:
  FUN_00575739(param_1,&param_1,4);
  if (bVar3) {
    iVar2 = FUN_00579a92(&param_1);
    bVar3 = iVar2 != *(int *)(iVar1 + 0x100);
  }
  else {
    bVar3 = false;
  }
  return bVar3;
}

