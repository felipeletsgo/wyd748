// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ddd46 | Name: FUN_004ddd46


undefined4 __fastcall FUN_004ddd46(int param_1)

{
  undefined4 uVar1;
  int local_8;
  
  (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
  (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
  (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
  (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
  (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
  (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
  (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
  (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
  (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
  uVar1 = FUN_004b925c(9,360000);
  (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
  for (local_8 = 0; local_8 < 10; local_8 = local_8 + 1) {
    thunk_FUN_0056239d(param_1 + 0x2c,*(undefined4 *)(param_1 + 0xb4 + local_8 * 0xc),
                       *(undefined4 *)(param_1 + 0xb8 + local_8 * 0xc),
                       *(undefined4 *)(param_1 + 0xbc + local_8 * 0xc));
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,param_1 + 0x2c);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x142);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))(DAT_005ccf9c[0xa8f9],6,2,param_1 + 300,0x18)
    ;
  }
  (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
  (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
  (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
  (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
  (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
  (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
  (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
  return 1;
}

