// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cdb91 | Name: FUN_004cdb91


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004cdb91(int param_1)

{
  undefined4 uVar1;
  
  if ((DAT_0092e660 != 1) &&
     (((*(int *)(param_1 + 0xf8) == 0 || (_DAT_005a439c <= *(float *)(param_1 + 0x110))) &&
      (*(int *)(param_1 + 0x28) == 1)))) {
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
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x144);
    uVar1 = FUN_004b925c(*(int *)(param_1 + 0x10c) + *(int *)(param_1 + 0x114),360000);
    (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
              (DAT_005ccf9c[0xa8f9],6,2,param_1 + 0x80,0x1c);
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
  }
  return 1;
}

