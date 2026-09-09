// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059adea | Name: FUN_0059adea


undefined4 FUN_0059adea(undefined4 param_1,undefined4 param_2)

{
  undefined4 uVar1;
  bool bVar2;
  
  InterlockedIncrement((LONG *)&DAT_013b8728);
  bVar2 = DAT_013b8724 != 0;
  if (bVar2) {
    InterlockedDecrement((LONG *)&DAT_013b8728);
    FUN_00596aad(0x13);
  }
  uVar1 = FUN_0059ae43(param_1,param_2);
  if (bVar2) {
    FUN_00596b0e(0x13);
  }
  else {
    InterlockedDecrement((LONG *)&DAT_013b8728);
  }
  return uVar1;
}

