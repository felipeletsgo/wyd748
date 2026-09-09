// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00531dfa | Name: FUN_00531dfa


undefined4 __fastcall FUN_00531dfa(int param_1)

{
  undefined4 uVar1;
  
  if (DAT_005d03f0 == 1) {
    uVar1 = 0;
  }
  else if ((((*(int *)(param_1 + 0x18) == -1) || (*(int *)(param_1 + 0x18) == DAT_005bdef0)) ||
           (*(int *)(param_1 + 0x18) == DAT_005bdef4)) ||
          ((*(int *)(param_1 + 0x18) == DAT_005bdef8 || (*(int *)(param_1 + 0x18) == DAT_005bdefc)))
          ) {
    if ((*(int *)(param_1 + 0x60) != 0) && (*(int *)(param_1 + 0x48) == 1)) {
      if (DAT_005ccf9c[0xa944] == 0x20) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xaa);
      }
      else {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xff);
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x8c,3);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,DAT_005ccf9c[0xac0e]);
      FUN_004be1ef(0,0x3f800000,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

