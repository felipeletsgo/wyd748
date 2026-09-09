// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013d43fb | Name: FUN_013d43fb


void __fastcall FUN_013d43fb(undefined4 param_1,int param_2)

{
  int iVar1;
  int unaff_EBX;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  puVar2 = (undefined4 *)(unaff_EBX + 0x48);
  puVar3 = (undefined4 *)(param_2 + 0x74);
  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  return;
}

