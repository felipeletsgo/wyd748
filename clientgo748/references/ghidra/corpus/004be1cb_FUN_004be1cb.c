// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004be1cb | Name: FUN_004be1cb


void __fastcall FUN_004be1cb(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = (undefined4 *)(param_1 + 0x288);
  for (iVar1 = 0x11; register0x00000010 = (BADSPACEBASE *)((int)register0x00000010 + 4), iVar1 != 0;
      iVar1 = iVar1 + -1) {
    *puVar2 = *(undefined4 *)register0x00000010;
    puVar2 = puVar2 + 1;
  }
  return;
}

