// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058f83f | Name: FUN_0058f83f


int FUN_0058f83f(uint param_1)

{
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  
  if ((param_1 < DAT_013b8720) &&
     ((*(byte *)((&DAT_013b8620)[(int)param_1 >> 5] + 4 + (param_1 & 0x1f) * 0x24) & 1) != 0)) {
    FUN_00594e8f(param_1);
    iVar1 = FUN_00595131(param_1,0,1);
    iVar3 = -1;
    if (iVar1 != -1) {
      iVar3 = FUN_00595131(param_1,0,2);
      if (iVar1 != iVar3) {
        FUN_00595131(param_1,iVar1,0);
      }
    }
    FUN_00594eee(param_1);
    return iVar3;
  }
  puVar2 = (undefined4 *)FUN_00594c1d();
  *puVar2 = 9;
  puVar2 = (undefined4 *)FUN_00594c26();
  *puVar2 = 0;
  return -1;
}

