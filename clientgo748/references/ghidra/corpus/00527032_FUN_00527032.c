// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00527032 | Name: FUN_00527032


void __thiscall FUN_00527032(int param_1,byte *param_2)

{
  char cVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    if (*(byte *)(param_1 + 0x1f2) < 10) {
      *(byte *)(param_1 + 0x202) = *param_2;
    }
    else {
      *(byte *)(param_1 + 0x202) = *param_2 & 0xf;
      if (*(char *)(param_1 + 0x202) != '\0') {
        *(char *)(param_1 + 0x202) = *(char *)(param_1 + 0x202) + 's';
      }
    }
    if (*(byte *)(param_1 + 499) < 10) {
      *(byte *)(param_1 + 0x203) = param_2[1];
    }
    else {
      *(byte *)(param_1 + 0x203) = param_2[1] & 0xf;
      if (*(char *)(param_1 + 0x203) != '\0') {
        *(char *)(param_1 + 0x203) = *(char *)(param_1 + 0x203) + 's';
      }
    }
    if (*(byte *)(param_1 + 500) < 10) {
      *(byte *)(param_1 + 0x204) = param_2[2];
    }
    else {
      *(byte *)(param_1 + 0x204) = param_2[2] & 0xf;
      if (*(char *)(param_1 + 0x204) != '\0') {
        *(char *)(param_1 + 0x204) = *(char *)(param_1 + 0x204) + 's';
      }
    }
    if (*(byte *)(param_1 + 0x1f5) < 10) {
      *(byte *)(param_1 + 0x205) = param_2[3];
    }
    else {
      *(byte *)(param_1 + 0x205) = param_2[3] & 0xf;
      if (*(char *)(param_1 + 0x205) != '\0') {
        *(char *)(param_1 + 0x205) = *(char *)(param_1 + 0x205) + 's';
      }
    }
    if (*(byte *)(param_1 + 0x1f6) < 10) {
      *(byte *)(param_1 + 0x206) = param_2[4];
    }
    else {
      *(byte *)(param_1 + 0x206) = param_2[4] & 0xf;
      if (*(char *)(param_1 + 0x206) != '\0') {
        *(char *)(param_1 + 0x206) = *(char *)(param_1 + 0x206) + 's';
      }
    }
    if (*(byte *)(param_1 + 0x1f7) < 10) {
      *(byte *)(param_1 + 0x207) = param_2[5];
    }
    else {
      *(byte *)(param_1 + 0x207) = param_2[5] & 0xf;
      if (*(char *)(param_1 + 0x207) != '\0') {
        *(char *)(param_1 + 0x207) = *(char *)(param_1 + 0x207) + 's';
      }
    }
    if (*(byte *)(param_1 + 0x1f9) < 10) {
      *(byte *)(param_1 + 0x209) = param_2[6];
    }
    else {
      *(byte *)(param_1 + 0x209) = param_2[6] & 0xf;
      if (*(char *)(param_1 + 0x209) != '\0') {
        *(char *)(param_1 + 0x209) = *(char *)(param_1 + 0x209) + 's';
      }
    }
    if (*(byte *)(param_1 + 0x1f8) < 10) {
      *(byte *)(param_1 + 0x208) = param_2[7];
    }
    else {
      *(byte *)(param_1 + 0x208) = param_2[7] & 0xf;
      if (*(char *)(param_1 + 0x208) != '\0') {
        *(char *)(param_1 + 0x208) = *(char *)(param_1 + 0x208) + 's';
      }
    }
    iVar2 = (int)(uint)*param_2 >> 4;
    if (((iVar2 == 0) || (4 < *(byte *)(param_1 + 0x1fa))) || (*(byte *)(param_1 + 0x1f2) < 10)) {
      if (((iVar2 != 0) && (*(char *)(param_1 + 0x1fa) == '\x04')) &&
         (9 < *(byte *)(param_1 + 0x1f2))) {
        *(char *)(param_1 + 0x1fa) = (char)iVar2 + '\x04';
      }
    }
    else {
      *(char *)(param_1 + 0x1fa) = (char)iVar2 + '\b';
    }
    iVar2 = (int)(uint)param_2[1] >> 4;
    if (((iVar2 == 0) || (4 < *(byte *)(param_1 + 0x1fb))) || (*(byte *)(param_1 + 499) < 10)) {
      if (((iVar2 != 0) && (*(char *)(param_1 + 0x1fb) == '\x04')) && (9 < *(byte *)(param_1 + 499))
         ) {
        *(char *)(param_1 + 0x1fb) = (char)iVar2 + '\x04';
      }
    }
    else {
      *(char *)(param_1 + 0x1fb) = (char)iVar2 + '\b';
    }
    iVar2 = (int)(uint)param_2[2] >> 4;
    if (((iVar2 == 0) || (4 < *(byte *)(param_1 + 0x1fc))) || (*(byte *)(param_1 + 500) < 10)) {
      if (((iVar2 != 0) && (*(char *)(param_1 + 0x1fc) == '\x04')) && (9 < *(byte *)(param_1 + 500))
         ) {
        *(char *)(param_1 + 0x1fc) = (char)iVar2 + '\x04';
      }
    }
    else {
      *(char *)(param_1 + 0x1fc) = (char)iVar2 + '\b';
    }
    iVar2 = (int)(uint)param_2[3] >> 4;
    if (((iVar2 == 0) || (4 < *(byte *)(param_1 + 0x1fd))) || (*(byte *)(param_1 + 0x1f5) < 10)) {
      if (((iVar2 != 0) && (*(char *)(param_1 + 0x1fd) == '\x04')) &&
         (9 < *(byte *)(param_1 + 0x1f5))) {
        *(char *)(param_1 + 0x1fd) = (char)iVar2 + '\x04';
      }
    }
    else {
      *(char *)(param_1 + 0x1fd) = (char)iVar2 + '\b';
    }
    iVar2 = (int)(uint)param_2[4] >> 4;
    if (((iVar2 == 0) || (4 < *(byte *)(param_1 + 0x1fe))) || (*(byte *)(param_1 + 0x1f6) < 10)) {
      if (((iVar2 != 0) && (*(char *)(param_1 + 0x1fe) == '\x04')) &&
         (9 < *(byte *)(param_1 + 0x1f6))) {
        *(char *)(param_1 + 0x1fe) = (char)iVar2 + '\x04';
      }
    }
    else {
      *(char *)(param_1 + 0x1fe) = (char)iVar2 + '\b';
    }
    iVar2 = (int)(uint)param_2[5] >> 4;
    if (((iVar2 == 0) || (4 < *(byte *)(param_1 + 0x1ff))) || (*(byte *)(param_1 + 0x1f7) < 10)) {
      if (((iVar2 != 0) && (*(char *)(param_1 + 0x1ff) == '\x04')) &&
         (9 < *(byte *)(param_1 + 0x1f7))) {
        *(char *)(param_1 + 0x1ff) = (char)iVar2 + '\x04';
      }
    }
    else {
      *(char *)(param_1 + 0x1ff) = (char)iVar2 + '\b';
    }
    cVar1 = (char)((int)(uint)param_2[6] >> 4);
    if ((*(byte *)(param_1 + 0x201) < 5) && (9 < *(byte *)(param_1 + 0x1f9))) {
      *(char *)(param_1 + 0x201) = cVar1 + '\b';
    }
    else if (((int)(uint)param_2[6] >> 4 != 0) &&
            ((*(char *)(param_1 + 0x201) == '\x04' && (9 < *(byte *)(param_1 + 0x1f9))))) {
      *(char *)(param_1 + 0x201) = cVar1 + '\x04';
    }
    cVar1 = (char)((int)(uint)param_2[7] >> 4);
    if ((*(byte *)(param_1 + 0x200) < 5) && (9 < *(byte *)(param_1 + 0x1f8))) {
      *(char *)(param_1 + 0x200) = cVar1 + '\b';
    }
    else if (((int)(uint)param_2[7] >> 4 != 0) &&
            ((*(char *)(param_1 + 0x200) == '\x04' && (9 < *(byte *)(param_1 + 0x1f8))))) {
      *(char *)(param_1 + 0x200) = *(char *)(param_1 + 0x200) + '\x04' + cVar1;
    }
  }
  return;
}

