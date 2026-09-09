// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004318e9 | Name: FUN_004318e9


void __fastcall FUN_004318e9(int param_1)

{
  undefined1 local_c [8];
  
  if ((*(int *)(param_1 + 0x11c) != 0) && (*(int *)(param_1 + 0x120) != 0)) {
    (**(code **)(**(int **)(param_1 + 0x11c) + 0x50))(*(undefined4 *)(param_1 + 0x11c),local_c);
  }
  return;
}

