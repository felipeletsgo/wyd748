// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004af484 | Name: FUN_004af484


void __thiscall FUN_004af484(int param_1,int param_2)

{
  if ((*(int *)(param_1 + 0x34 + param_2 * 4) == 0) && (DAT_013b71e8 != (int *)0x0)) {
    *(undefined4 *)(param_1 + 0x34 + param_2 * 4) = 1;
    (**(code **)(*DAT_013b71e8 + 0x10))(param_2);
  }
  return;
}

