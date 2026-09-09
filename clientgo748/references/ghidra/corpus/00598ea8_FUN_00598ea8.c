// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00598ea8 | Name: FUN_00598ea8


void FUN_00598ea8(int param_1,int param_2,uint param_3,int param_4,int param_5,int param_6,
                 int param_7,int param_8,int param_9,int param_10,int param_11)

{
  int iVar1;
  int iVar2;
  
  if (param_2 == 1) {
    if ((param_3 & 3) == 0) {
      iVar1 = *(int *)(&DAT_005cc71c + param_4 * 4);
    }
    else {
      iVar1 = *(int *)(&DAT_005cc750 + param_4 * 4);
    }
    iVar2 = (int)(param_3 * 0x16d + -0x63db + iVar1 + 1 + ((int)(param_3 - 1) >> 2)) % 7;
    if (param_6 < iVar2) {
      iVar1 = iVar1 + 1 + (param_5 * 7 - iVar2) + param_6;
    }
    else {
      iVar1 = iVar1 + -6 + (param_5 * 7 - iVar2) + param_6;
    }
    if (param_5 == 5) {
      if ((param_3 & 3) == 0) {
        iVar2 = *(int *)(&DAT_005cc720 + param_4 * 4);
      }
      else {
        iVar2 = *(int *)(&DAT_005cc754 + param_4 * 4);
      }
      if (iVar2 < iVar1) {
        iVar1 = iVar1 + -7;
      }
    }
  }
  else {
    if ((param_3 & 3) == 0) {
      iVar1 = *(int *)(&DAT_005cc71c + param_4 * 4);
    }
    else {
      iVar1 = *(int *)(&DAT_005cc750 + param_4 * 4);
    }
    iVar1 = iVar1 + param_7;
  }
  if (param_1 == 1) {
    DAT_005cc4e0 = param_3;
    DAT_005cc4e8 = ((param_8 * 0x3c + param_9) * 0x3c + param_10) * 1000 + param_11;
    DAT_005cc4e4 = iVar1;
  }
  else {
    DAT_005cc4f8 = ((param_8 * 0x3c + param_9) * 0x3c + DAT_005cc450 + param_10) * 1000 + param_11;
    if (DAT_005cc4f8 < 0) {
      DAT_005cc4f8 = DAT_005cc4f8 + 86400000;
      DAT_005cc4f4 = iVar1 + -1;
    }
    else {
      DAT_005cc4f4 = iVar1;
      if (86399999 < DAT_005cc4f8) {
        DAT_005cc4f8 = DAT_005cc4f8 + -86400000;
        DAT_005cc4f4 = iVar1 + 1;
      }
    }
    DAT_005cc4f0 = param_3;
  }
  return;
}

