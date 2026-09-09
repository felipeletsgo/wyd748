// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cbbfa | Name: FUN_004cbbfa


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004cbbfa(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((((DAT_0092e660 != 1) || (param_1[0x41] == 4)) &&
      ((param_1[0x3c] == 0 || (_DAT_005a439c <= (float)param_1[0x44])))) &&
     (iVar1 = (**(code **)(*param_1 + 0x38))(), iVar1 == 1)) {
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,param_1 + 0xb);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    if (param_1[0x1b] == 1) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,4);
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,2);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x142);
    uVar2 = FUN_004b925c(param_1[0x3f],360000);
    (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar2);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
              (DAT_005ccf9c[0xa8f9],6,2,param_1 + 0x20,0x18);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
    if (param_1[0x1b] == 1) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    }
  }
  return 1;
}

