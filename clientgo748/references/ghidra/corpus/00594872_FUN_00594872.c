// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594872 | Name: FUN_00594872


undefined8 FUN_00594872(int *param_1)

{
  *param_1 = *param_1 + 8;
  return *(undefined8 *)(*param_1 + -8);
}

