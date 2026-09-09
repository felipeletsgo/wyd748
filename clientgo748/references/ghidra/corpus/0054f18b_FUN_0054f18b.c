// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054f18b | Name: FUN_0054f18b


void FUN_0054f18b(int param_1,int param_2)

{
  int iVar1;
  int local_1c;
  int local_14;
  
  iVar1 = DAT_005b8924 + param_2;
  param_2 = DAT_005b8928 + param_2;
  for (local_14 = DAT_005b8928; local_14 < param_2; local_14 = local_14 + 1) {
    for (local_1c = DAT_005b8924; local_1c < iVar1; local_1c = local_1c + 1) {
      if (((&DAT_00934178)[(local_1c >> 2 & 0x3ffU) + (local_14 >> 2 & 0x3ffU) * 0x400] & 2) != 0) {
        *(undefined1 *)
         (param_1 + (((local_14 - DAT_005b8928) * DAT_005b891c + local_1c) - DAT_005b8924)) = 0x7f;
      }
    }
  }
  return;
}

