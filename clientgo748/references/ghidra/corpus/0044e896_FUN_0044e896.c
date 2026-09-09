// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044e896 | Name: FUN_0044e896


void __fastcall FUN_0044e896(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(DAT_013b71e8 + 0x1b084);
  *(undefined4 *)(iVar1 + 0x24) = 0;
  *(undefined4 *)(iVar1 + 0x2c) = 0xbf490fdb;
  *(undefined4 *)(iVar1 + 0x34) = *(undefined4 *)(iVar1 + 0xc0);
  *(undefined4 *)(iVar1 + 0x38) = *(undefined4 *)(iVar1 + 0xc0);
  *(undefined4 *)(iVar1 + 0x3c) = 0x3ea8f5c3;
  *(undefined4 *)(iVar1 + 0x28) = 0x3f490fdb;
  *(undefined4 *)(DAT_005ccf9c + 0x2b038) = 1;
  *(undefined4 *)(*(int *)(param_1 + 0x44) + 0x48) = 0;
  *(undefined4 *)(param_1 + 0x274e8) = 0;
  *(undefined4 *)(param_1 + 0x275e4) = 0;
  FUN_004efb89(10);
  return;
}

