// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004173cf | Name: FUN_004173cf


int FUN_004173cf(int param_1)

{
  bool bVar1;
  int iVar2;
  uint local_10;
  
  iVar2 = FUN_0054e06c(param_1);
  if (iVar2 < 10) {
    return 0;
  }
  local_10 = 0;
  if (*(char *)(param_1 + 2) == '+') {
LAB_00417440:
    bVar1 = true;
  }
  else {
    if ((*(char *)(param_1 + 2) < 's') || ('~' < *(char *)(param_1 + 2))) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) goto LAB_00417440;
    bVar1 = false;
  }
  if (!bVar1) {
    if (*(char *)(param_1 + 4) == '+') {
LAB_00417494:
      bVar1 = true;
    }
    else {
      if ((*(char *)(param_1 + 4) < 's') || ('~' < *(char *)(param_1 + 4))) {
        bVar1 = false;
      }
      else {
        bVar1 = true;
      }
      if (bVar1) goto LAB_00417494;
      bVar1 = false;
    }
    if (!bVar1) {
      if (*(char *)(param_1 + 6) == '+') {
LAB_004174e4:
        bVar1 = true;
      }
      else {
        if ((*(char *)(param_1 + 6) < 's') || ('~' < *(char *)(param_1 + 6))) {
          bVar1 = false;
        }
        else {
          bVar1 = true;
        }
        if (bVar1) goto LAB_004174e4;
        bVar1 = false;
      }
      if (!bVar1) goto LAB_004175b4;
    }
  }
  if (*(char *)(param_1 + 2) == '+') {
LAB_00417538:
    bVar1 = true;
  }
  else {
    if ((*(char *)(param_1 + 2) < 's') || ('~' < *(char *)(param_1 + 2))) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) goto LAB_00417538;
    bVar1 = false;
  }
  if (bVar1) {
    local_10 = (uint)*(byte *)(param_1 + 3);
    goto LAB_004175b4;
  }
  if (*(char *)(param_1 + 4) == '+') {
LAB_00417593:
    bVar1 = true;
  }
  else {
    if ((*(char *)(param_1 + 4) < 's') || ('~' < *(char *)(param_1 + 4))) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) goto LAB_00417593;
    bVar1 = false;
  }
  if (bVar1) {
    local_10 = (uint)*(byte *)(param_1 + 5);
  }
  else {
    local_10 = (uint)*(byte *)(param_1 + 7);
  }
LAB_004175b4:
  return (local_10 - 0xe6) % 4 + 5;
}

