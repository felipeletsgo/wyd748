// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004eb6ae | Name: FUN_004eb6ae


void __fastcall FUN_004eb6ae(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_004b14fd(0x2c4,1,180000);
  if (iVar1 != 0) {
    *(undefined4 *)(param_1 + 0x70) = *(undefined4 *)(iVar1 + 0x340);
  }
  *(undefined4 *)(param_1 + 0x70) = 0x3f800000;
  FUN_004c8daa();
  return;
}

