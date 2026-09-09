// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594865 | Name: FUN_00594865


undefined4 FUN_00594865(int *param_1)

{
  *param_1 = *param_1 + 4;
  return *(undefined4 *)(*param_1 + -4);
}

