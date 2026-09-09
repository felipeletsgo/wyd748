// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005312c5 | Name: FUN_005312c5


undefined4 __fastcall FUN_005312c5(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  if (DAT_005d03f0 == 1) {
    uVar1 = 0;
  }
  else {
    if (param_1[0x18] != 0) {
      iVar2 = (**(code **)(*param_1 + 0x4c))();
      if (iVar2 == 1) {
        if (DAT_005ccf9c[0xa944] == 0x20) {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xaa);
        }
        else {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xff);
        }
        FUN_004be1ef(0,0x3f800000,0);
      }
    }
    uVar1 = 1;
  }
  return uVar1;
}

