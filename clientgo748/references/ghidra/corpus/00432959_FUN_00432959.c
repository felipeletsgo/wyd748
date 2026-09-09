// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00432959 | Name: FUN_00432959


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00432959(int param_1,uint param_2)

{
  int iVar1;
  int local_10;
  int local_c;
  
  FUN_0049a73a(param_2);
  if (DAT_005c4958 == 1) {
    for (local_c = 0; local_c < 0x32; local_c = local_c + 1) {
      for (local_10 = 0; local_10 < 0x10; local_10 = local_10 + 1) {
        if (((uint)(*(int *)(param_1 + 0x27858 + local_c * 0x100 + local_10 * 0x10) +
                   *(int *)(param_1 + 0x2009c)) < param_2) &&
           (*(char *)(param_1 + 0x2aa58 + local_10 + local_c * 0x10) == '\0')) {
          *(undefined1 *)(param_1 + 0x2aa58 + local_10 + local_c * 0x10) = 1;
          if (*(char *)(param_1 + local_c * 0x100 + 0x2785c + local_10 * 0x10) == 'M') {
            if (*(int *)(param_1 + 0x26e30 + local_c * 4) != 0) {
              *(float *)(*(int *)(param_1 + 0x26e30 + local_c * 4) + 0x220) =
                   (float)(int)*(short *)(param_1 + local_c * 0x100 + 0x2785e + local_10 * 0x10);
              iVar1 = param_1 + 0x27858 + local_c * 0x100;
              FUN_00520216(*(undefined4 *)(iVar1 + 8 + local_10 * 0x10),
                           *(undefined4 *)(iVar1 + 0xc + local_10 * 0x10),0x20,0);
            }
          }
          else if ((*(char *)(param_1 + local_c * 0x100 + 0x2785c + local_10 * 0x10) == 'A') &&
                  (*(int *)(param_1 + 0x26e30 + local_c * 4) != 0)) {
            FUN_0051dba4((int)*(short *)(param_1 + local_c * 0x100 + 0x2785e + local_10 * 0x10),1);
          }
          break;
        }
      }
    }
    param_2 = param_2 - *(int *)(param_1 + 0x2009c);
    if (param_2 < 0x90880) {
      (**(code **)(**(int **)(param_1 + 0x3a778) + 0x70))
                (0,_DAT_005a4284 - (float)param_2 / _DAT_005a4288);
    }
    if ((1000 < param_2) && (*(int *)(*(int *)(param_1 + 0x3a778) + 0x28) == 0)) {
      (**(code **)(**(int **)(param_1 + 0x3a778) + 0x60))(1);
    }
    if (((40000 < param_2) && (param_2 < 80000)) &&
       (*(int *)(*(int *)(param_1 + 0x44) + 0x164) == 0)) {
      FUN_00549bbf(0xc);
    }
    if (((80000 < param_2) && (param_2 < 120000)) &&
       (*(int *)(*(int *)(param_1 + 0x44) + 0x164) == 2)) {
      FUN_00549bbf(0xd);
    }
    if (((120000 < param_2) && (param_2 < 160000)) &&
       (*(int *)(*(int *)(param_1 + 0x44) + 0x164) == 3)) {
      FUN_00549bbf(10);
    }
    if (((160000 < param_2) && (param_2 < 200000)) &&
       (*(int *)(*(int *)(param_1 + 0x44) + 0x164) == 0)) {
      FUN_00549bbf(0xb);
    }
    if (((200000 < param_2) && (param_2 < 240000)) &&
       (*(int *)(*(int *)(param_1 + 0x44) + 0x164) == 1)) {
      FUN_00549bbf(10);
    }
    if (((240000 < param_2) && (param_2 < 280000)) &&
       (*(int *)(*(int *)(param_1 + 0x44) + 0x164) == 0)) {
      FUN_00549bbf(0xc);
      *(undefined4 *)(DAT_005ccec0 + 0x28) = 0;
    }
    if (((0x45d30 < param_2) && (*(int *)(DAT_013b71e0 + 0xec) != 0)) &&
       (*(int *)(param_1 + 0x26e1c) != 0)) {
      FUN_00429480();
      *(undefined4 *)(param_1 + 0x26e1c) = 0;
      FUN_00428ba0(0xe);
      (**(code **)(**(int **)(param_1 + 0x3a77c) + 0x60))(1);
    }
    if (0x98198 < param_2) {
      PostMessageA(*(HWND *)(DAT_013b71e0 + 4),0x10,0,0);
    }
  }
  return 1;
}

