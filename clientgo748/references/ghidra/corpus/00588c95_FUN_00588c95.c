// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00588c95 | Name: FUN_00588c95


void FUN_00588c95(void)

{
  int in_EAX;
  int iVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  int *piVar5;
  
  iVar4 = *(int *)(in_EAX + 0x1a4);
  iVar1 = (*(code *)**(undefined4 **)(in_EAX + 4))();
  piVar2 = (int *)(iVar1 + 0x3fc);
  *(int **)(iVar4 + 0x28) = piVar2;
  iVar4 = 0;
  uVar3 = 0;
  iVar1 = 0;
  piVar5 = piVar2;
  do {
    piVar2[uVar3] = iVar4;
    *piVar5 = iVar1;
    uVar3 = uVar3 + 1;
    piVar5 = piVar5 + -1;
    iVar4 = iVar4 + 1;
    iVar1 = iVar1 + -1;
  } while ((int)uVar3 < 0x10);
  if ((int)uVar3 < 0x30) {
    piVar5 = piVar2 + -uVar3;
    do {
      piVar2[uVar3] = iVar4;
      *piVar5 = -iVar4;
      uVar3 = uVar3 + 1;
      piVar5 = piVar5 + -1;
      iVar4 = iVar4 + (~uVar3 & 1);
    } while ((int)uVar3 < 0x30);
  }
  if ((int)uVar3 < 0x100) {
    piVar5 = piVar2 + -uVar3;
    do {
      piVar2[uVar3] = iVar4;
      *piVar5 = -iVar4;
      uVar3 = uVar3 + 1;
      piVar5 = piVar5 + -1;
    } while ((int)uVar3 < 0x100);
  }
  return;
}

