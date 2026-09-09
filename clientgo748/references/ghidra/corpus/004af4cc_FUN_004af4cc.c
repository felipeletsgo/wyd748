// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004af4cc | Name: FUN_004af4cc


void __thiscall FUN_004af4cc(int param_1,int param_2)

{
  if ((*(int *)(param_1 + 0x34 + param_2 * 4) != 0) && (DAT_013b71e8 != (int *)0x0)) {
    *(undefined4 *)(param_1 + 0x34 + param_2 * 4) = 0;
    (**(code **)(*DAT_013b71e8 + 0x14))(param_2);
  }
  return;
}

