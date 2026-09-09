// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004032e8 | Name: FUN_004032e8


undefined4 __thiscall
FUN_004032e8(int *param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  if (param_1[0xf] == 0) {
    return 0;
  }
  iVar1 = FUN_00401000(param_4,param_5,param_1[0x13],param_1[0x14],param_1[0x15],param_1[0x16]);
  param_1[0xd] = iVar1;
  if (param_1[0x7c] != 0) {
    if (param_1[0xd] == 1) {
      iVar1 = FUN_0040c0f0();
      if (iVar1 == 0) {
        (**(code **)(*(int *)param_1[0x7c] + 0x60))(1);
      }
      if (DAT_005b892c == 2) {
        if (((param_1[0x28] < 3) && (-3 < param_1[0x28])) && (param_1[0x19] = 6, param_1[0x6c] != 0)
           ) {
          *(undefined4 *)(param_1[0x6c] + 4) = 0x66ffffff;
        }
      }
      else if (((param_1[0x28] < 3) && (-3 < param_1[0x28])) &&
              (param_1[0x19] = 6, param_1[0x6c] != 0)) {
        *(undefined4 *)(param_1[0x6c] + 4) = 0x66ffffff;
      }
    }
    else {
      (**(code **)(*(int *)param_1[0x7c] + 0x60))(0);
      if (DAT_005b892c == 2) {
        param_1[0x19] = 4;
        if (param_1[0x6c] != 0) {
          *(undefined4 *)(param_1[0x6c] + 4) = 0xff000000;
        }
      }
      else {
        param_1[0x19] = 4;
        if (param_1[0x6c] != 0) {
          *(undefined4 *)(param_1[0x6c] + 4) = 0xff000000;
        }
      }
    }
  }
  if (param_2 == 0x200) {
    if ((param_1[0x79] == 1) && (param_1[0x17] != 0)) {
      (*(code *)**(undefined4 **)param_1[0x17])(param_1[0x11],2);
    }
  }
  else if (param_2 == 0x201) {
    if (param_1[10] == 0) {
      return 1;
    }
    if (param_1[0xd] == 1) {
      param_1[0x7d] = 1;
      param_1[0xc] = 1;
      (**(code **)(*param_1 + 0x54))();
      return 1;
    }
  }
  else {
    if (param_2 != 0x202) {
      return 0;
    }
    if (param_1[10] == 0) {
      return 1;
    }
    if ((((param_1[0xb] == 1) && (param_1[0xd] == 1)) && (param_1[0x7d] == 1)) &&
       (param_1[0x17] != 0)) {
      uVar2 = (**(code **)(*DAT_0092e654 + 8))();
      if (param_1[0x81] + 500U < uVar2) {
        if (((param_1[0x80] == 1) && (DAT_005ccf98 != 0)) &&
           (iVar1 = FUN_00429a6d(0x35), iVar1 != 0)) {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(0x35);
          FUN_0042ad2b(uVar3,uVar4);
        }
        (**(code **)(*param_1 + 0x54))();
        (*(code *)**(undefined4 **)param_1[0x17])(param_1[0x11],0);
        iVar1 = (**(code **)(*DAT_0092e654 + 8))();
        param_1[0x81] = iVar1;
        param_1[0x7d] = 0;
      }
      return 1;
    }
    param_1[0x7d] = 0;
  }
  if (((char)param_1[0x7b] != '\0') && (iVar1 = FUN_0040c0f0(), iVar1 == 0)) {
    (**(code **)(*(int *)param_1[0x7c] + 0x60))(1);
  }
  (**(code **)(*param_1 + 0x54))();
  return 0;
}

