// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0045e762 | Name: FUN_0045e762


undefined4 __thiscall FUN_0045e762(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  
  if (*(int *)(*(int *)(param_1 + 0x27b38) + 0x28) != 0) {
    return 1;
  }
  if ((param_2 != 0) && (*(int *)(param_2 + 0x238) == 1)) {
    iVar2 = __ftol();
    if ((iVar2 >> 7 < 2) ||
       ((iVar2 = __ftol(), 10 < iVar2 >> 7 || (iVar2 = __ftol(), 4 < iVar2 >> 7)))) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) {
      return 1;
    }
    if (*(char *)(param_2 + 0x724) != '\0') {
      return 1;
    }
    if ((((*(uint *)(param_2 + 0x20) < 1000) || (*(short *)(param_2 + 0x250) != 0x33)) ||
        ((iVar2 = __ftol(), iVar2 >> 7 != 0xd && (iVar2 = __ftol(), iVar2 >> 7 != 0xe)))) ||
       (iVar2 = __ftol(), iVar2 >> 7 != 0x1c)) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) {
      return 1;
    }
    if ((999 < *(uint *)(param_2 + 0x20)) && ((*(byte *)(param_2 + 0x44a) & 0xf) == 1)) {
      return 1;
    }
    if ((999 < *(uint *)(param_2 + 0x20)) && ((*(byte *)(param_2 + 0x44a) & 0xf) == 2)) {
      return 1;
    }
    if ((999 < *(uint *)(param_2 + 0x20)) && ((*(byte *)(param_2 + 0x44a) & 0xf) == 3)) {
      return 1;
    }
    if ((((999 < *(uint *)(param_2 + 0x20)) && (*(short *)(param_2 + 0x250) == 0x43)) &&
        (*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) == 0xd)) &&
       (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) == 0xd)) {
      return 1;
    }
    if (((999 < *(uint *)(param_2 + 0x20)) && (*(short *)(param_2 + 0x250) == 0x43)) &&
       ((*(int *)(*(int *)(param_1 + 0x40) + 0x20a20) == 0x1c &&
        (*(int *)(*(int *)(param_1 + 0x40) + 0x20a24) == 0x18)))) {
      return 1;
    }
    if ((999 < *(uint *)(param_2 + 0x20)) && (*(short *)(param_2 + 0x250) == 0x36)) {
      return 1;
    }
    if ((999 < *(uint *)(param_2 + 0x20)) && (*(short *)(param_2 + 0x250) == 0x37)) {
      return 1;
    }
    if ((999 < *(uint *)(param_2 + 0x20)) && (*(short *)(param_2 + 0x250) == 0x38)) {
      return 1;
    }
    if ((999 < *(uint *)(param_2 + 0x20)) && (*(short *)(param_2 + 0x250) == 0x44)) {
      return 1;
    }
    if ((999 < *(uint *)(param_2 + 0x20)) && (*(short *)(param_2 + 0x250) == 0x39)) {
      return 1;
    }
    if (((999 < *(uint *)(param_2 + 0x20)) && ((*(byte *)(param_2 + 0x44a) & 0xf) == 4)) ||
       ((7 < (*(byte *)(param_2 + 0x44a) & 0xf) && ((*(byte *)(param_2 + 0x44a) & 0xf) < 0x10)))) {
      if (((*(byte *)(param_2 + 0x44a) & 0xf) == 0xf) &&
         (iVar2 = (**(code **)(**(int **)(param_1 + 0x4c) + 0x50))(), iVar2 == 0)) {
        if (('\0' < *(char *)(param_2 + 0x79b)) &&
           ((('\0' < *(char *)(*(int *)(param_1 + 0x4c) + 0x79b) &&
             (*(char *)(param_2 + 0x79b) != *(char *)(*(int *)(param_1 + 0x4c) + 0x79b))) &&
            (*(char *)(*(int *)(param_1 + 0x4c) + 0x79b) != '\x03')))) {
          return 1;
        }
        if ((*(int *)(*(int *)(param_1 + 0x4c) + 0x198) != 0) &&
           ((*(ushort *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x198) + 0x24a) < 2 ||
            ((7 < *(ushort *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x198) + 0x24a) &&
             (*(ushort *)(*(int *)(*(int *)(param_1 + 0x4c) + 0x198) + 0x24a) < 0xf)))))) {
          if ((*(short *)(DAT_013b71e8 + 0x798) == 0x6ce) &&
             ((*(short *)(DAT_013b71e8 + 0x7a0) < 0x6e0 ||
              (0x6e3 < *(short *)(DAT_013b71e8 + 0x7a0))))) {
            return 1;
          }
          if (((*(short *)(DAT_013b71e8 + 0x798) != 0x6ce) ||
              (*(short *)(DAT_013b71e8 + 0x7a0) < 0x6e0)) ||
             (0x6e3 < *(short *)(DAT_013b71e8 + 0x7a0))) {
            return 1;
          }
        }
      }
      if ((*(byte *)(param_2 + 0x44a) & 0xf) == 0xd) {
        return 1;
      }
      if ((*(byte *)(param_2 + 0x44a) & 0xf) == 0xe) {
        return 1;
      }
      return 1;
    }
    if (((999 < *(uint *)(param_2 + 0x20)) && (5 < (*(byte *)(param_2 + 0x44a) & 0xf))) &&
       ((*(byte *)(param_2 + 0x44a) & 0xf) < 9)) {
      return 1;
    }
  }
  return 0;
}

