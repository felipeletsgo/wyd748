// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00403d9b | Name: FUN_00403d9b


void FUN_00403d9b(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  FUN_004015dd(param_1);
  if (((param_1 == 1) && (param_2 == 1)) && (DAT_005ccf98 != 0)) {
    iVar1 = FUN_00429a6d(0x21);
    if (iVar1 != 0) {
      uVar3 = 0;
      uVar2 = 0;
      FUN_00429a6d(0x21);
      FUN_0042ad2b(uVar2,uVar3);
    }
  }
  return;
}

