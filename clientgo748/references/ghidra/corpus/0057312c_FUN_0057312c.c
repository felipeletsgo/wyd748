// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057312c | Name: FUN_0057312c


void FUN_0057312c(int param_1)

{
  int *piVar1;
  
  *(undefined4 *)(param_1 + 0xc4) = 0;
  *(undefined4 *)(param_1 + 0x7c) = 0;
  *(undefined4 *)(param_1 + 0x178) = 0;
  piVar1 = (int *)(param_1 + 400);
  *(undefined1 *)(*piVar1 + 0x50) = 0;
  *(undefined1 *)(*piVar1 + 0x51) = 0;
  *(undefined4 *)(*piVar1 + 0x58) = 0;
  return;
}

