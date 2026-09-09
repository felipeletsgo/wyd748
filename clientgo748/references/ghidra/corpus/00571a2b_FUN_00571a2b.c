// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00571a2b | Name: FUN_00571a2b


undefined4 FUN_00571a2b(int *param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  
  if (param_1[4] == 0xca) {
    FUN_00579490(param_1);
    if ((char)param_1[0x10] != '\0') {
      param_1[4] = 0xcf;
      return 1;
    }
    param_1[4] = 0xcb;
  }
  if (param_1[4] == 0xcb) {
    if (*(char *)(param_1[99] + 0x10) != '\0') {
      while( true ) {
        if ((undefined4 *)param_1[2] != (undefined4 *)0x0) {
          (**(code **)param_1[2])(param_1);
        }
        iVar2 = (**(code **)param_1[99])(param_1);
        if (iVar2 == 0) {
          return 0;
        }
        if (iVar2 == 2) break;
        if ((param_1[2] != 0) && ((iVar2 == 3 || (iVar2 == 1)))) {
          piVar1 = (int *)(param_1[2] + 4);
          *piVar1 = *piVar1 + 1;
          iVar2 = param_1[2];
          if (*(int *)(iVar2 + 8) <= *(int *)(iVar2 + 4)) {
            *(int *)(iVar2 + 8) = param_1[0x46] + *(int *)(iVar2 + 8);
          }
        }
      }
    }
    param_1[0x21] = param_1[0x1f];
  }
  else if (param_1[4] != 0xcc) {
    *(undefined4 *)(*param_1 + 0x14) = 0x12;
    *(int *)(*param_1 + 0x18) = param_1[4];
    (**(code **)*param_1)(param_1);
  }
  uVar3 = FUN_005718fe();
  return uVar3;
}

