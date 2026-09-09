// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0045379f | Name: FUN_0045379f


undefined4 __thiscall FUN_0045379f(undefined4 param_1,char param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((param_2 == 's') || (param_2 == 'S')) {
    iVar1 = FUN_0040c0f0(param_1);
    if (iVar1 == 0) {
      iVar1 = FUN_0040c0f0(param_1);
      if (iVar1 == 1) {
        uVar2 = 1;
      }
      else {
        FUN_0044c53f();
        uVar2 = 1;
      }
    }
    else {
      uVar2 = 1;
    }
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

