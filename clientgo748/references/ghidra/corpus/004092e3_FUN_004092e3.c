// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004092e3 | Name: FUN_004092e3


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_004092e3(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  undefined4 uVar1;
  int iVar2;
  int local_8;
  
  if ((*(int *)(param_1 + 0x3c) == 0) && (*(char *)(param_1 + 0x1e9) == '\0')) {
    uVar1 = 0;
  }
  else {
    if (*(char *)(param_1 + 0x1e8) == '\0') {
      local_8 = FUN_00401000(param_4,param_5,*(undefined4 *)(param_1 + 0x4c),
                             *(undefined4 *)(param_1 + 0x50),*(undefined4 *)(param_1 + 0x200),
                             *(undefined4 *)(param_1 + 0x204));
    }
    else {
      local_8 = FUN_00401000(param_4,param_5,*(undefined4 *)(param_1 + 0x4c),
                             *(undefined4 *)(param_1 + 0x50),
                             *(float *)(param_1 + 0x200) - _DAT_005a3658,
                             *(undefined4 *)(param_1 + 0x204));
    }
    if ((((local_8 == 1) && (param_2 == 0x202)) && (*(char *)(param_1 + 0x1e9) == '\x01')) &&
       (*(int *)(param_1 + 0x208) != 0)) {
      (**(code **)(**(int **)(DAT_0067cf38 + 0x28) + 0x40))(*(undefined4 *)(param_1 + 0x208));
    }
    if ((local_8 == 1) && ((param_2 == 0x201 || (param_2 == 0x204)))) {
      uVar1 = 1;
    }
    else {
      if ((local_8 == 1) && (param_2 == 0x202)) {
        iVar2 = __ftol();
        if ((iVar2 + *(int *)(param_1 + 0x1f8) < *(int *)(param_1 + 0x210)) &&
           (*(int *)(param_1 + 0x1fc) = iVar2 + *(int *)(param_1 + 0x1f8),
           *(int *)(param_1 + 0x5c) != 0)) {
          *(undefined4 *)(param_1 + 0x1ec) = 0;
          (**(code **)**(undefined4 **)(param_1 + 0x5c))
                    (*(undefined4 *)(param_1 + 0x44),*(undefined4 *)(param_1 + 0x1fc));
        }
        if (*(int *)(param_1 + 0x3c) == 1) {
          return 1;
        }
      }
      if ((local_8 == 1) && (param_2 == 0x205)) {
        iVar2 = __ftol();
        if ((iVar2 + *(int *)(param_1 + 0x1f8) < *(int *)(param_1 + 0x210)) &&
           (*(int *)(param_1 + 0x1fc) = iVar2 + *(int *)(param_1 + 0x1f8),
           *(int *)(param_1 + 0x5c) != 0)) {
          *(undefined4 *)(param_1 + 0x1ec) = 1;
          (**(code **)**(undefined4 **)(param_1 + 0x5c))
                    (*(undefined4 *)(param_1 + 0x44),*(undefined4 *)(param_1 + 0x1fc));
        }
        if (*(int *)(param_1 + 0x3c) == 1) {
          return 1;
        }
      }
      uVar1 = 0;
    }
  }
  return uVar1;
}

