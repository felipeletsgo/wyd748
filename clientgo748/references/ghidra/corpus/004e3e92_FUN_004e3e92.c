// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e3e92 | Name: FUN_004e3e92


void __fastcall FUN_004e3e92(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x9c) < 2) {
    iVar1 = FUN_004b14fd(*(undefined4 *)(param_1 + 0xa8),1,180000);
    if (iVar1 != 0) {
      *(undefined4 *)(param_1 + 0x70) = *(undefined4 *)(iVar1 + 0x340);
    }
  }
  else {
    iVar1 = FUN_004b14fd(*(undefined4 *)(param_1 + 0xa8),0,180000);
    if (iVar1 != 0) {
      *(undefined4 *)(param_1 + 0x70) = *(undefined4 *)(iVar1 + 0x340);
    }
  }
  FUN_004c8daa();
  return;
}

