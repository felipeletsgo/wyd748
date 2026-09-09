// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005554cc | Name: FUN_005554cc


undefined4
FUN_005554cc(int param_1,undefined4 param_2,undefined4 param_3,int param_4,int param_5,int param_6,
            int param_7,undefined1 param_8)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int local_10;
  int local_c;
  
  if ((param_1 < 10) && (-1 < param_1)) {
    for (local_c = 0; local_c < 6; local_c = local_c + 1) {
      for (local_10 = 0; local_10 < 6; local_10 = local_10 + 1) {
        iVar1 = param_4 + -2 + local_10;
        iVar2 = param_5 + -2 + local_c;
        if ((((iVar1 - DAT_005b8924 < 1) || (iVar2 - DAT_005b8928 < 1)) ||
            (DAT_005b891c + -2 < iVar1 - DAT_005b8924)) ||
           (DAT_005b8920 + -2 < iVar2 - DAT_005b8928)) break;
        if (*(int *)(&DAT_005bed50 +
                    local_10 * 4 + local_c * 0x18 + param_7 * 0x90 + param_1 * 0x240) != 0) {
          *(undefined1 *)
           (param_6 + (((iVar2 - DAT_005b8928) * DAT_005b891c + iVar1) - DAT_005b8924)) = param_8;
        }
      }
    }
    uVar3 = 1;
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}

