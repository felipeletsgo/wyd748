// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0052e6c6 | Name: FUN_0052e6c6


undefined4 __thiscall FUN_0052e6c6(int param_1,int param_2)

{
  if (*(int *)(param_2 + 0xc) == 4) {
    *(undefined4 *)(*(int *)(param_1 + 0x418) + 0xd10) = 0x5500aa00;
    *(undefined1 *)(*(int *)(param_1 + 0x418) + 0xcdc) = 1;
    *(undefined1 *)(param_1 + 0x249) = 1;
  }
  else {
    *(undefined1 *)(param_1 + 0x249) = 0;
  }
  return 1;
}

