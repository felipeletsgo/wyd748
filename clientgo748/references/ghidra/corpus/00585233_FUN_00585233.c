// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00585233 | Name: FUN_00585233


void FUN_00585233(int param_1)

{
  int iVar1;
  char cVar2;
  
  iVar1 = *(int *)(param_1 + 0x184);
  if (*(int *)(iVar1 + 0x10) != 0) {
    if (*(char *)(param_1 + 0x49) != '\0') {
      cVar2 = FUN_00584c33();
      if (cVar2 != '\0') {
        *(undefined1 **)(iVar1 + 0xc) = &LAB_00584d16;
        goto LAB_00585264;
      }
    }
    *(code **)(iVar1 + 0xc) = FUN_00584acd;
  }
LAB_00585264:
  *(undefined4 *)(param_1 + 0x88) = 0;
  return;
}

