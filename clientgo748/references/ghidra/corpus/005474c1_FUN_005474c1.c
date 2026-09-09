// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005474c1 | Name: FUN_005474c1


undefined4 __fastcall FUN_005474c1(int param_1)

{
  undefined4 uVar1;
  
  if (DAT_005d03f0 == 1) {
    uVar1 = 0;
  }
  else if ((((*(int *)(param_1 + 0x18) == -1) || (*(int *)(param_1 + 0x18) == DAT_005bdef0)) ||
           (*(int *)(param_1 + 0x18) == DAT_005bdef4)) ||
          ((*(int *)(param_1 + 0x18) == DAT_005bdef8 || (*(int *)(param_1 + 0x18) == DAT_005bdefc)))
          ) {
    FUN_0053fa66();
    if ((*(int *)(param_1 + 0x60) != 0) && (*(int *)(param_1 + 0x48) == 1)) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,0);
      FUN_004be1ef(0,0x3f800000,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

