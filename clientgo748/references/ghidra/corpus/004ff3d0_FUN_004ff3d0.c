// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ff3d0 | Name: FUN_004ff3d0


undefined4 __fastcall FUN_004ff3d0(int param_1)

{
  undefined4 uVar1;
  
  if ((*(int *)(param_1 + 0x24c) == 0x38) && (*(short *)(param_1 + 0x1d2) == 0)) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

