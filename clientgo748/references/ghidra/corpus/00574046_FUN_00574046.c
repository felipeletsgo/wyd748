// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00574046 | Name: FUN_00574046


void FUN_00574046(int param_1,int *param_2)

{
  byte bVar1;
  
  if ((*(byte *)(param_1 + 0x61) & 0x10) == 0) goto LAB_00574092;
  if (*(byte *)((int)param_2 + 0x19) == 3) {
    *(char *)((int)param_2 + 0x19) = (*(short *)(param_1 + 0x10a) != 0) * '\x04' + '\x02';
LAB_0057408a:
    *(undefined1 *)(param_2 + 6) = 8;
  }
  else {
    if (*(short *)(param_1 + 0x10a) != 0) {
      *(byte *)((int)param_2 + 0x19) = *(byte *)((int)param_2 + 0x19) | 4;
    }
    if (*(byte *)(param_2 + 6) < 8) goto LAB_0057408a;
  }
  *(undefined2 *)((int)param_2 + 0x16) = 0;
LAB_00574092:
  if ((*(byte *)(param_1 + 0x61) & 0x20) != 0) {
    param_2[10] = *(int *)(param_1 + 0x130);
  }
  if (((*(byte *)(param_1 + 0x61) & 4) != 0) && ((char)param_2[6] == '\x10')) {
    *(undefined1 *)(param_2 + 6) = 8;
  }
  if (((*(byte *)(param_1 + 0x60) & 0x40) != 0) &&
     ((((*(char *)((int)param_2 + 0x19) == '\x02' || (*(char *)((int)param_2 + 0x19) == '\x06')) &&
       (*(int *)(param_1 + 0x174) != 0)) && ((char)param_2[6] == '\b')))) {
    *(undefined1 *)((int)param_2 + 0x19) = 3;
  }
  if (((*(byte *)(param_1 + 0x60) & 4) != 0) && (*(byte *)(param_2 + 6) < 8)) {
    *(undefined1 *)(param_2 + 6) = 8;
  }
  bVar1 = *(byte *)((int)param_2 + 0x19);
  if ((bVar1 == 3) || ((bVar1 & 2) == 0)) {
    *(undefined1 *)((int)param_2 + 0x1d) = 1;
  }
  else {
    *(undefined1 *)((int)param_2 + 0x1d) = 3;
  }
  if ((bVar1 & 4) != 0) {
    *(char *)((int)param_2 + 0x1d) = *(char *)((int)param_2 + 0x1d) + '\x01';
  }
  if (((*(byte *)(param_1 + 0x61) & 0x80) != 0) && ((bVar1 == 2 || (bVar1 == 0)))) {
    *(char *)((int)param_2 + 0x1d) = *(char *)((int)param_2 + 0x1d) + '\x01';
  }
  bVar1 = (char)param_2[6] * *(char *)((int)param_2 + 0x1d);
  *(byte *)((int)param_2 + 0x1e) = bVar1;
  param_2[3] = (uint)bVar1 * *param_2 + 7 >> 3;
  return;
}

