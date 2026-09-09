// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005755d9 | Name: FUN_005755d9


void FUN_005755d9(int param_1,uint *param_2,uint param_3,uint param_4,char param_5,byte param_6,
                 undefined1 param_7,undefined1 param_8,undefined1 param_9)

{
  byte bVar1;
  
  if ((param_1 != 0) && (param_2 != (uint *)0x0)) {
    *(undefined1 *)((int)param_2 + 0x1a) = param_8;
    param_2[1] = param_4;
    *(undefined1 *)((int)param_2 + 0x1b) = param_9;
    *param_2 = param_3;
    *(char *)(param_2 + 6) = param_5;
    *(byte *)((int)param_2 + 0x19) = param_6;
    *(undefined1 *)(param_2 + 7) = param_7;
    if ((param_6 == 3) || ((param_6 & 2) == 0)) {
      *(undefined1 *)((int)param_2 + 0x1d) = 1;
    }
    else {
      *(undefined1 *)((int)param_2 + 0x1d) = 3;
    }
    if ((param_6 & 4) != 0) {
      *(char *)((int)param_2 + 0x1d) = *(char *)((int)param_2 + 0x1d) + '\x01';
    }
    bVar1 = *(char *)((int)param_2 + 0x1d) * param_5;
    *(byte *)((int)param_2 + 0x1e) = bVar1;
    if ((uint)(0x7fffffff / (ulonglong)(uint)((int)(bVar1 + 7) >> 3)) < param_3) {
      FUN_005733ef(param_1,"Width too large to process image data; rowbytes will overflow.");
      param_2[3] = 0;
    }
    else {
      param_2[3] = bVar1 * param_3 + 7 >> 3;
    }
  }
  return;
}

