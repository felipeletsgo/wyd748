// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00585c3e | Name: FUN_00585c3e


void FUN_00585c3e(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  
  puVar1 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x58);
  *(undefined4 **)(param_1 + 0x194) = puVar1;
  iVar2 = 4;
  *puVar1 = FUN_00585b0f;
  puVar1[1] = FUN_00585742;
  puVar1 = puVar1 + 0xd;
  do {
    puVar1[4] = 0;
    *puVar1 = 0;
    puVar1 = puVar1 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  return;
}

