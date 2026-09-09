// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053e324 | Name: FUN_0053e324


void __fastcall FUN_0053e324(int param_1)

{
  undefined4 uVar1;
  undefined4 local_8;
  
  *(undefined4 *)(param_1 + 0x20) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x44) = 1;
  *(undefined4 *)(param_1 + 0x24) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x60) = 0;
  *(undefined4 *)(param_1 + 0x48) = 0;
  *(undefined4 *)(param_1 + 0x4c) = 0;
  *(undefined4 *)(param_1 + 0x34) = 0;
  *(undefined4 *)(param_1 + 0x38) = 0x3f800000;
  uVar1 = FUN_00430f50(0,0,0);
  FUN_004310a0(uVar1);
  *(undefined4 *)(param_1 + 0x3c) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x40) = 0xffffffff;
  for (local_8 = 0; local_8 < 10; local_8 = local_8 + 1) {
    uVar1 = FUN_00430f50(0,0,0);
    FUN_004310a0(uVar1);
    uVar1 = FUN_00430f50(0,0,0);
    FUN_004310a0(uVar1);
  }
  *(undefined4 *)(param_1 + 0x158) = 0;
  *(undefined4 *)(param_1 + 0x154) = 0;
  *(undefined4 *)(param_1 + 0x15c) = 0;
  return;
}

