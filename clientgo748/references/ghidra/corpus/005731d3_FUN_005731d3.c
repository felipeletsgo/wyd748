// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005731d3 | Name: FUN_005731d3


void FUN_005731d3(int *param_1)

{
  int *unaff_retaddr;
  undefined1 auStack_d0 [192];
  undefined4 uStack_10;
  int *piStack_c;
  
  piStack_c = (int *)0x5731de;
  (**(code **)(*param_1 + 8))();
  piStack_c = param_1;
  uStack_10 = 0x5731e4;
  FUN_00578e73();
  piStack_c = (int *)0x5731ed;
  FUN_005926b9();
  (**(code **)(*unaff_retaddr + 0xc))(unaff_retaddr,auStack_d0);
  return;
}

