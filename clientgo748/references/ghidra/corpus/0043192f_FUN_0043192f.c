// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0043192f | Name: FUN_0043192f


undefined4 __thiscall FUN_0043192f(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  FUN_0058f078(param_1 + 4,param_2);
  *(undefined4 *)(param_1 + 0x10c) = 0;
  iVar1 = FUN_004313c5(param_1 + 4);
  if (iVar1 < 0) {
    FUN_0043199b();
    uVar2 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x108) == 1) {
      FUN_00431b5d();
    }
    uVar2 = 1;
  }
  return uVar2;
}

