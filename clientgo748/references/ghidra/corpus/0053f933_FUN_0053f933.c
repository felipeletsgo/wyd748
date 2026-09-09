// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053f933 | Name: FUN_0053f933


undefined4 FUN_0053f933(void)

{
  int iVar1;
  int iVar2;
  undefined4 local_14;
  
  iVar1 = __ftol();
  iVar2 = __ftol();
  if ((iVar1 >> 7 == 8) && (iVar2 >> 7 == 0xd)) {
    local_14 = 1;
  }
  else {
    local_14 = 0;
  }
  return local_14;
}

