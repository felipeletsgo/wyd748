// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004aaedc | Name: FUN_004aaedc


undefined4 __thiscall FUN_004aaedc(int param_1,undefined4 param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 local_c;
  undefined4 local_8;
  
  FUN_0049a73a(param_2);
  iVar2 = (**(code **)(*DAT_0092e654 + 8))();
  if (*(int *)(DAT_005ccf9c + 0x2b04c) == 1) {
    (**(code **)(*DAT_005ccec0 + 0x60))(1);
  }
  if (*(char *)(param_1 + 0x27d24) == '\0') {
    FUN_004ab2bb(*(undefined4 *)(param_1 + 0x2009c),iVar2,12000,0);
  }
  else if (*(char *)(param_1 + 0x27d24) == '\x01') {
    FUN_004ab7cd(*(undefined4 *)(param_1 + 0x27d28),iVar2,1000,0);
  }
  else if (*(char *)(param_1 + 0x27d24) == '\x02') {
    FUN_004ab2bb(*(undefined4 *)(param_1 + 0x27d28),iVar2,1000,0);
  }
  for (local_8 = 0; local_8 < 0x32; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x26f0c + local_8 * 4) != 0) {
      *(undefined4 *)(*(int *)(param_1 + 0x26f0c + local_8 * 4) + 0x228) = 0xc0800000;
    }
  }
  if (DAT_005c4958 == 1) {
    if ((5000 < (uint)(iVar2 - *(int *)(param_1 + 0x2009c))) &&
       (*(char *)(param_1 + 0x26f08) == '\0')) {
      if (*(int *)(param_1 + 0x27d30) == 0) {
        FUN_004ac81e(0);
        *(undefined1 *)(param_1 + 0x26f08) = 1;
      }
      else if (*(int *)(param_1 + 0x27d30) == 1) {
        for (local_c = 0; local_c < 0x32; local_c = local_c + 1) {
          FUN_004ac81e(local_c);
        }
        if (DAT_005ccf98 != 0) {
          iVar3 = FUN_00429a6d(0xf);
          if (iVar3 != 0) {
            FUN_00429a6d(0xf);
            iVar3 = FUN_0042afd0();
            if (iVar3 == 0) {
              uVar5 = 0;
              uVar4 = 0;
              FUN_00429a6d(0xf);
              FUN_0042ad2b(uVar4,uVar5);
            }
          }
        }
        *(undefined1 *)(param_1 + 0x26f08) = 1;
      }
    }
    if ((7000 < (uint)(iVar2 - *(int *)(param_1 + 0x2009c))) &&
       (*(char *)(param_1 + 0x26f08) == '\x01')) {
      if (*(int *)(param_1 + 0x27d30) == 0) {
        FUN_004ac81e(1);
        *(undefined1 *)(param_1 + 0x26f08) = 2;
      }
      else if (*(int *)(param_1 + 0x27d30) == 1) {
        if (DAT_005ccf98 != 0) {
          iVar3 = FUN_00429a6d(0x10);
          if (iVar3 != 0) {
            FUN_00429a6d(0x10);
            iVar3 = FUN_0042afd0();
            if (iVar3 == 0) {
              uVar5 = 0;
              uVar4 = 0;
              FUN_00429a6d(0x10);
              FUN_0042ad2b(uVar4,uVar5);
            }
          }
        }
        *(undefined1 *)(param_1 + 0x26f08) = 2;
      }
    }
    if (((*(int *)(param_1 + 0x27d30) == 1) && (14000 < (uint)(iVar2 - *(int *)(param_1 + 0x2009c)))
        ) && (*(int *)(param_1 + 0x26ef8) == 0)) {
      FUN_004ac8c5();
      *(undefined4 *)(param_1 + 0x26ef8) = 1;
    }
    if (((*(int *)(param_1 + 0x27d30) == 3) && (12000 < (uint)(iVar2 - *(int *)(param_1 + 0x2009c)))
        ) && (*(char *)(param_1 + 0x26f08) == '\0')) {
      if (*(int *)(param_1 + 0x26f0c) != 0) {
        FUN_0051dba4(0xe,0);
      }
      *(undefined1 *)(param_1 + 0x26f08) = 1;
    }
    if (((*(int *)(param_1 + 0x27d30) == 3) && (20000 < (uint)(iVar2 - *(int *)(param_1 + 0x2009c)))
        ) && (*(char *)(param_1 + 0x26f08) == '\x01')) {
      if (*(int *)(param_1 + 0x26f0c) != 0) {
        FUN_0051dba4(0xe,0);
      }
      *(undefined1 *)(param_1 + 0x26f08) = 2;
    }
  }
  if (6000 < (uint)(iVar2 - *(int *)(param_1 + 0x27d2c))) {
    piVar1 = *(int **)(param_1 + 0x27d20);
    (**(code **)(**(int **)(param_1 + 0x26e60) + 100))(1);
    (**(code **)(*piVar1 + 100))(1);
  }
  return 1;
}

