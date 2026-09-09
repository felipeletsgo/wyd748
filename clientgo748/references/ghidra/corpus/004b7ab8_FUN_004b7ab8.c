// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7ab8 | Name: FUN_004b7ab8


void __fastcall FUN_004b7ab8(undefined4 *param_1)

{
  *param_1 = &PTR_FUN_005a4684;
  FUN_004b7ca2();
  FUN_004b8bde();
  if (param_1[0x3a0ca] != 0) {
    FUN_0058ea3a(param_1[0x3a0ca]);
    param_1[0x3a0ca] = 0;
  }
  if (param_1[0x3a0cb] != 0) {
    FUN_0058ea3a(param_1[0x3a0cb]);
    param_1[0x3a0cb] = 0;
  }
  if (param_1[0x3a0cc] != 0) {
    FUN_0058ea3a(param_1[0x3a0cc]);
    param_1[0x3a0cc] = 0;
  }
  if (param_1[0x3a0cd] != 0) {
    FUN_0058ea3a(param_1[0x3a0cd]);
    param_1[0x3a0cd] = 0;
  }
  if (param_1[0x3a0ce] != 0) {
    FUN_0058ea3a(param_1[0x3a0ce]);
    param_1[0x3a0ce] = 0;
  }
  FUN_004bab41();
  DAT_0092e640 = 0;
  return;
}

