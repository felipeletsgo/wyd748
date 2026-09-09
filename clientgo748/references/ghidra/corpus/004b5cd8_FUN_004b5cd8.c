// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b5cd8 | Name: FUN_004b5cd8


undefined4 FUN_004b5cd8(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  undefined4 uVar2;
  
  cVar1 = FUN_004b5d16(param_1,param_2,param_4);
  if (cVar1 == '\0') {
    uVar2 = FUN_004b5f0a(param_1,param_2,param_3,param_4);
  }
  else {
    uVar2 = 1;
  }
  return uVar2;
}

