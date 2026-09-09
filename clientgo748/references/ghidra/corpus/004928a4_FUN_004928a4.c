// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004928a4 | Name: FUN_004928a4


undefined4 __thiscall FUN_004928a4(int param_1,int param_2)

{
  *(undefined1 *)(param_2 + 0x6b) = 0;
  *(undefined1 *)(param_2 + 0x8b) = 0;
  *(undefined1 *)(param_2 + 0xab) = 0;
  *(undefined1 *)(param_2 + 0xcb) = 0;
  *(undefined1 *)(param_2 + 0xeb) = 0;
  if (*(int *)(param_1 + 0x28614) != 0) {
    (**(code **)(**(int **)(param_1 + 0x28614) + 0x80))(param_2 + 0xc,0);
  }
  if (*(int *)(param_1 + 0x28618) != 0) {
    (**(code **)(**(int **)(param_1 + 0x28618) + 0x88))(param_2 + 0x6c);
  }
  if (*(int *)(param_1 + 0x2861c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x2861c) + 0x88))(param_2 + 0x8c);
  }
  if (*(int *)(param_1 + 0x28620) != 0) {
    (**(code **)(**(int **)(param_1 + 0x28620) + 0x88))(param_2 + 0xac);
  }
  if (*(int *)(param_1 + 0x28624) != 0) {
    (**(code **)(**(int **)(param_1 + 0x28624) + 0x88))(param_2 + 0xcc);
  }
  if (*(int *)(param_1 + 0x28610) != 0) {
    (**(code **)(**(int **)(param_1 + 0x28610) + 0x60))(1);
  }
  return 1;
}

