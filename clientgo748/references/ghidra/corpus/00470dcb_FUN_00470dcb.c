// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00470dcb | Name: FUN_00470dcb


void __fastcall FUN_00470dcb(int param_1)

{
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x289a4) != 0) {
    for (local_8 = 0; local_8 < 100; local_8 = local_8 + 1) {
      *(undefined4 *)(*(int *)(param_1 + 0x289a8 + local_8 * 4) + 0x1e8) = 0;
      (**(code **)(**(int **)(param_1 + 0x289a8 + local_8 * 4) + 0x54))();
    }
  }
  return;
}

