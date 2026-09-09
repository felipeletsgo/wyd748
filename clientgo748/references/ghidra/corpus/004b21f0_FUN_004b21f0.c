// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b21f0 | Name: FUN_004b21f0


void __thiscall FUN_004b21f0(int *param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_1 + 0x30))(param_2);
  if (iVar1 != 0) {
    *(undefined1 *)(iVar1 + 0x14) = 1;
    param_1[0x6c23] = 1;
  }
  return;
}

