// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578e8e | Name: FUN_00578e8e


void FUN_00578e8e(int param_1)

{
  int iVar1;
  
  iVar1 = (*(code *)**(undefined4 **)(param_1 + 4))(param_1,0,0x82);
  *(undefined1 *)(iVar1 + 0x80) = 0;
  return;
}

