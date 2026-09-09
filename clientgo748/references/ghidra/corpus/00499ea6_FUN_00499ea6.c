// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00499ea6 | Name: FUN_00499ea6


void __fastcall FUN_00499ea6(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  if (((*(int *)(param_1 + 0x54) != 1) && (*(int *)(param_1 + 0x4c) != 0)) &&
     (*(int *)(param_1 + 0x40) != 0)) {
    uVar1 = __ftol();
    uVar2 = __ftol();
    FUN_00499f0e(uVar1,uVar2);
  }
  return;
}

