// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00442ff7 | Name: FUN_00442ff7


int __fastcall FUN_00442ff7(undefined4 param_1)

{
  short sVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int local_10;
  int local_c;
  
  iVar4 = DAT_013b71e8;
  local_10 = FUN_0054cd07(DAT_013b71e8 + 0x778,2,param_1);
  local_c = FUN_0054cd07(iVar4 + 0x780,2);
  sVar1 = *(short *)(&DAT_00d449d4 + *(short *)(iVar4 + 0x778) * 0x8c);
  sVar2 = *(short *)(&DAT_00d449d4 + *(short *)(iVar4 + 0x780) * 0x8c);
  if ((sVar1 == 0x2f) && (sVar2 == 0x2d)) {
    local_c = 0;
  }
  if (local_c < local_10) {
    local_10 = local_10 + local_c / 3;
  }
  else {
    local_10 = local_c + local_10 / 3;
  }
  iVar5 = (int)*(short *)(iVar4 + 0x778);
  if ((((-1 < iVar5) || (iVar5 < 0x1964)) &&
      ((*(short *)(&DAT_00d449d6 + iVar5 * 0x8c) == 0x40 ||
       (*(short *)(&DAT_00d449d6 + iVar5 * 0x8c) == 0xc0)))) && ((sVar1 != 0x2c && (sVar2 != 0x2f)))
     ) {
    iVar3 = FUN_0054e06c(iVar4 + 0x778);
    if ((8 < iVar3) &&
       ((*(short *)(&DAT_00d449d4 + iVar5 * 0x8c) != 0x2f &&
        (*(short *)(&DAT_00d449d4 + iVar5 * 0x8c) != 0x2c)))) {
      local_10 = local_10 + 0x28;
    }
  }
  iVar5 = (int)*(short *)(iVar4 + 0x780);
  if ((((-1 < iVar5) || (iVar5 < 0x1964)) &&
      ((*(short *)(&DAT_00d449d6 + iVar5 * 0x8c) == 0x40 ||
       (*(short *)(&DAT_00d449d6 + iVar5 * 0x8c) == 0xc0)))) && ((sVar1 != 0x2c && (sVar2 != 0x2f)))
     ) {
    iVar4 = FUN_0054e06c(iVar4 + 0x780);
    if ((8 < iVar4) &&
       ((*(short *)(&DAT_00d449d4 + iVar5 * 0x8c) != 0x2f &&
        (*(short *)(&DAT_00d449d4 + iVar5 * 0x8c) != 0x2c)))) {
      local_10 = local_10 + 0x28;
    }
  }
  return local_10;
}

