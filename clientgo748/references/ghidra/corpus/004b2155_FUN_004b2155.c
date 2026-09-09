// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b2155 | Name: FUN_004b2155


void __fastcall FUN_004b2155(undefined4 *param_1)

{
  *param_1 = &PTR_FUN_005a45fc;
  if (param_1[0x6c1f] != 0) {
    if ((undefined4 *)param_1[0x6c1f] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x6c1f])(1);
    }
    param_1[0x6c1f] = 0;
  }
  DAT_0067cf38 = 0;
  return;
}

