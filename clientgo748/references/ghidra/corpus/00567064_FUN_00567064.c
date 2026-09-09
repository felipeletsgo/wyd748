// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00567064 | Name: FUN_00567064


void __fastcall FUN_00567064(int *param_1)

{
  if ((undefined4 *)param_1[1] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[1])(1);
  }
  if ((undefined4 *)*param_1 != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)*param_1)(1);
  }
  return;
}

