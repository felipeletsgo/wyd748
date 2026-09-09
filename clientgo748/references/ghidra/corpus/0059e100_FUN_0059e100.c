// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e100 | Name: FUN_0059e100


void FUN_0059e100(HWND param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  
  puVar1 = (undefined4 *)GetWindowLongA(param_1,-0x15);
  piVar4 = (int *)&param_1;
  piVar3 = (int *)&DAT_005aa908;
  iVar2 = (*(code *)**(undefined4 **)*puVar1)((undefined4 *)*puVar1);
  if (iVar2 == 0) {
    (**(code **)(*piVar3 + 0x68))(piVar3);
    (**(code **)(*piVar4 + 0x70))(piVar4);
    (**(code **)(*piVar3 + 8))(piVar3);
  }
  return;
}

