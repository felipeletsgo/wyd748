// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00406275 | Name: FUN_00406275


void __thiscall FUN_00406275(int *param_1,int param_2)

{
  param_1[0x1c] = param_2;
  (**(code **)(*param_1 + 0x54))();
  return;
}

