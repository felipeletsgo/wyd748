// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005782d2 | Name: FUN_005782d2


void FUN_005782d2(int param_1)

{
  FUN_005780ca();
  FUN_00578240();
  (*(code *)**(undefined4 **)(param_1 + 0x194))(param_1);
  (*(code *)**(undefined4 **)(param_1 + 0x184))(param_1);
  **(undefined4 **)(param_1 + 0x18c) = *(undefined4 *)(*(int *)(param_1 + 0x184) + 4);
  return;
}

