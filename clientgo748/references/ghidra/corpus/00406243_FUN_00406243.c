// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00406243 | Name: FUN_00406243


void __thiscall FUN_00406243(int *param_1,int param_2)

{
  param_1[0x1b] = param_2;
  (**(code **)(*param_1 + 0x54))();
  return;
}

