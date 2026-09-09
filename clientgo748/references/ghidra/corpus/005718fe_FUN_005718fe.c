// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005718fe | Name: FUN_005718fe


uint FUN_005718fe(void)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int unaff_ESI;
  
  if (*(int *)(unaff_ESI + 0x10) != 0xcc) {
    (*(code *)**(undefined4 **)(unaff_ESI + 0x17c))();
    *(undefined4 *)(unaff_ESI + 0x78) = 0;
    *(undefined4 *)(unaff_ESI + 0x10) = 0xcc;
  }
  if (*(char *)(*(int *)(unaff_ESI + 0x17c) + 8) != '\0') {
    puVar1 = (uint *)(unaff_ESI + 0x78);
    do {
      uVar3 = *puVar1;
      if (uVar3 < *(uint *)(unaff_ESI + 0x60)) {
        do {
          if (*(int *)(unaff_ESI + 8) != 0) {
            *(uint *)(*(int *)(unaff_ESI + 8) + 4) = uVar3;
            *(undefined4 *)(*(int *)(unaff_ESI + 8) + 8) = *(undefined4 *)(unaff_ESI + 0x60);
            (*(code *)**(undefined4 **)(unaff_ESI + 8))();
          }
          uVar2 = *puVar1;
          (**(code **)(*(int *)(unaff_ESI + 0x180) + 4))();
          uVar3 = *puVar1;
          if (uVar3 == uVar2) {
            return uVar3 & 0xffffff00;
          }
        } while (uVar3 < *(uint *)(unaff_ESI + 0x60));
      }
      (**(code **)(*(int *)(unaff_ESI + 0x17c) + 4))();
      (*(code *)**(undefined4 **)(unaff_ESI + 0x17c))();
      *puVar1 = 0;
    } while (*(char *)(*(int *)(unaff_ESI + 0x17c) + 8) != '\0');
  }
  iVar4 = (*(char *)(unaff_ESI + 0x41) != '\0') + 0xcd;
  *(int *)(unaff_ESI + 0x10) = iVar4;
  return CONCAT31((int3)((uint)iVar4 >> 8),1);
}

