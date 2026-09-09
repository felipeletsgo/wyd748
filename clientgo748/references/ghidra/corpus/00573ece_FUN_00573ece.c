// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573ece | Name: FUN_00573ece


void FUN_00573ece(int param_1,byte param_2,int param_3)

{
  *(byte *)(param_1 + 0x61) = *(byte *)(param_1 + 0x61) | 0x80;
  *(ushort *)(param_1 + 0x11e) = (ushort)param_2;
  if (param_3 == 1) {
    *(byte *)(param_1 + 0x5c) = *(byte *)(param_1 + 0x5c) | 0x80;
  }
  else {
    *(byte *)(param_1 + 0x5c) = *(byte *)(param_1 + 0x5c) & 0x7f;
  }
  if (*(char *)(param_1 + 0x116) == '\x02') {
    *(undefined1 *)(param_1 + 0x11b) = 4;
  }
  if ((*(char *)(param_1 + 0x116) == '\0') && (7 < *(byte *)(param_1 + 0x117))) {
    *(undefined1 *)(param_1 + 0x11b) = 2;
  }
  return;
}

