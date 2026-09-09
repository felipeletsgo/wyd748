// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00574edc | Name: FUN_00574edc


void FUN_00574edc(int *param_1,byte *param_2,int param_3,int param_4)

{
  char cVar1;
  int iVar2;
  byte *pbVar3;
  int iVar4;
  
  cVar1 = (char)param_1[2];
  iVar4 = *param_1;
  if (((cVar1 == '\x02') && (param_3 != 0)) &&
     (iVar2 = iVar4, pbVar3 = param_2, *(char *)((int)param_1 + 9) == '\b')) {
    for (; iVar2 != 0; iVar2 = iVar2 + -1) {
      *param_2 = *(byte *)((((*pbVar3 & 0xf8) << 5 | pbVar3[1] & 0xf8) << 2 |
                           (int)(uint)pbVar3[2] >> 3) + param_3);
      param_2 = param_2 + 1;
      pbVar3 = pbVar3 + 3;
    }
  }
  else {
    if (((cVar1 != '\x06') || (param_3 == 0)) ||
       (iVar2 = iVar4, pbVar3 = param_2, *(char *)((int)param_1 + 9) != '\b')) {
      if (cVar1 != '\x03') {
        return;
      }
      if (param_4 == 0) {
        return;
      }
      if (*(char *)((int)param_1 + 9) != '\b') {
        return;
      }
      for (; iVar4 != 0; iVar4 = iVar4 + -1) {
        *param_2 = *(byte *)((uint)*param_2 + param_4);
        param_2 = param_2 + 1;
      }
      return;
    }
    for (; iVar2 != 0; iVar2 = iVar2 + -1) {
      *param_2 = *(byte *)((((*pbVar3 & 0xf8) << 5 | pbVar3[1] & 0xf8) << 2 |
                           (int)(uint)pbVar3[2] >> 3) + param_3);
      param_2 = param_2 + 1;
      pbVar3 = pbVar3 + 4;
    }
  }
  *(byte *)((int)param_1 + 0xb) = *(byte *)((int)param_1 + 9);
  *(undefined1 *)(param_1 + 2) = 3;
  *(undefined1 *)((int)param_1 + 10) = 1;
  param_1[1] = (uint)*(byte *)((int)param_1 + 9) * iVar4 + 7 >> 3;
  return;
}

