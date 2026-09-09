// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409b3d | Name: FUN_00409b3d


void __thiscall FUN_00409b3d(int *param_1,int param_2)

{
  param_1[0x79] = param_2;
  (**(code **)(*param_1 + 0x54))();
  return;
}

