// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cd174 | Name: FUN_004cd174


undefined4 __fastcall FUN_004cd174(int param_1)

{
  undefined4 uVar1;
  
  if (DAT_0092e660 == 1) {
    uVar1 = 1;
  }
  else if (*(int *)(param_1 + 0x28) == 0) {
    uVar1 = 0;
  }
  else {
    FUN_00430fc0(param_1 + 0x2c);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,param_1 + 0x2c);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    if (*(int *)(param_1 + 0x6c) == 1) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,4);
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x142);
    uVar1 = FUN_004b925c(*(undefined4 *)(param_1 + 0x150),360000);
    (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
              (DAT_005ccf9c[0xa8f9],6,2,param_1 + 0x88,0x18);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
              (DAT_005ccf9c[0xa8f9],6,2,param_1 + 0xe8,0x18);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
    if (*(int *)(param_1 + 0x6c) == 1) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
    }
    uVar1 = 1;
  }
  return uVar1;
}

