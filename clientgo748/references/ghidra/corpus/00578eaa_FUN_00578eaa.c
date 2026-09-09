// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00578eaa | Name: FUN_00578eaa


void FUN_00578eaa(int param_1)

{
  int iVar1;
  
  iVar1 = (*(code *)**(undefined4 **)(param_1 + 4))(param_1,0,0x112);
  *(undefined1 *)(iVar1 + 0x111) = 0;
  return;
}

