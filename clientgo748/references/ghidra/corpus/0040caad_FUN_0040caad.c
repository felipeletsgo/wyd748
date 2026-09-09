// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040caad | Name: FUN_0040caad


void __thiscall FUN_0040caad(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int local_1c;
  int local_14;
  int local_8;
  
  iVar2 = FUN_0058f716(param_2,&DAT_005b1100);
  if ((iVar2 != 0) && (iVar1 = *(int *)(param_1 + 0x28), local_8 = iVar1, iVar1 != 0)) {
    do {
      if (*(char *)(local_8 + 0x14) == '\0') {
        if (*(int *)(local_8 + 0x40) == 0xc) {
          for (local_14 = 0; local_14 < 0x800; local_14 = local_14 + 1) {
            if (*(int *)(DAT_013b71e8 + 0xf78 + local_14 * 0x34) == *(int *)(local_8 + 0x44)) {
              uVar3 = FUN_0042260e();
              FUN_0058f6a9(iVar2,s__s__s_005b1104,DAT_013b71e8 + 0xf48 + local_14 * 0x34,uVar3);
              break;
            }
          }
        }
        if (*(int *)(local_8 + 0x40) == 2) {
          for (local_1c = 0; local_1c < 0x800; local_1c = local_1c + 1) {
            if (*(int *)(DAT_013b71e8 + 0xf78 + local_1c * 0x34) == *(int *)(local_8 + 0x44)) {
              FUN_0058f6a9(iVar2,s__s__s_005b110c,DAT_013b71e8 + 0xf48 + local_1c * 0x34,
                           local_8 + 0xaa);
              break;
            }
          }
        }
        if (*(int *)(local_8 + 0x10) == 0) goto LAB_0040cc02;
        local_8 = *(int *)(local_8 + 0x10);
      }
      else {
LAB_0040cc02:
        while (*(int *)(local_8 + 0xc) == 0) {
          local_8 = *(int *)(local_8 + 4);
          if ((local_8 == iVar1) || (local_8 == 0)) goto LAB_0040cc2d;
        }
        local_8 = *(int *)(local_8 + 0xc);
      }
LAB_0040cc2d:
    } while ((local_8 != iVar1) && (local_8 != 0));
    FUN_0058f62c(iVar2);
  }
  return;
}

