// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e01d4 | Name: FUN_004e01d4


void __fastcall FUN_004e01d4(int param_1)

{
  undefined4 uVar1;
  undefined1 local_44 [64];
  
  if ((*(int *)(param_1 + 0x88) != 0) && (*(int *)(param_1 + 0x84) != 0)) {
    FUN_00430f10();
    FUN_00430fc0(local_44);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_44);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    if (DAT_005ccf9c[0xa8e5] == 1) {
      if (DAT_005ccf9c[0xa944] == 0x20) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xff000000);
      }
      else {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xf000);
      }
    }
    else if (DAT_005ccf9c[0xa944] == 0x20) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xdd);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xd);
    }
    if (*(int *)(param_1 + 0x6c) == 1) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,4);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x8c,3);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,DAT_005ccf9c[0xac0e]);
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x142);
    uVar1 = FUN_004b925c(*(undefined4 *)(param_1 + 0x80),360000);
    (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x124))
              (DAT_005ccf9c[0xa8f9],4,0,*(undefined4 *)(param_1 + 0x8c),
               *(int *)(param_1 + 0x94) * *(int *)(param_1 + 0x94) * 2,
               *(undefined4 *)(param_1 + 0x88),0x65,*(undefined4 *)(param_1 + 0x84),0x18);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
    (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
  }
  return;
}

