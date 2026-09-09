// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00429420 | Name: FUN_00429420


undefined4 __fastcall FUN_00429420(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 8) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(**(int **)(param_1 + 8) + 0x38))
                      (*(undefined4 *)(param_1 + 8),&stack0x00000004,1,&stack0x00000004,0);
  }
  return uVar1;
}

