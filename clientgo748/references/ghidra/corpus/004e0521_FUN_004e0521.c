// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e0521 | Name: FUN_004e0521


undefined4 __fastcall FUN_004e0521(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  if (DAT_005d03f0 == 1) {
    uVar1 = 0;
  }
  else if (DAT_0092e660 == 1) {
    uVar1 = 0;
  }
  else {
    iVar2 = (**(code **)(*param_1 + 0x38))();
    if (iVar2 == 1) {
      (**(code **)(*param_1 + 0x40))();
    }
    uVar1 = 1;
  }
  return uVar1;
}

