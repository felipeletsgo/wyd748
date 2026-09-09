// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00592792 | Name: FUN_00592792


void FUN_00592792(undefined4 *param_1,undefined4 *param_2)

{
  for (; param_1 < param_2; param_1 = param_1 + 1) {
    if ((code *)*param_1 != (code *)0x0) {
      (*(code *)*param_1)();
    }
  }
  return;
}

