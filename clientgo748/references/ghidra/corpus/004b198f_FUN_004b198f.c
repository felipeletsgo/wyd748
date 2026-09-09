// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b198f | Name: FUN_004b198f


void FUN_004b198f(int param_1,int param_2,int param_3,int param_4)

{
  undefined1 uVar1;
  int iVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  local_8 = 0;
  for (local_c = 0; (local_8 < param_2 && (local_c < param_4)); local_c = local_c + 1) {
    if (*(char *)(param_3 + local_c) == ' ') {
      *(undefined1 *)(param_1 + local_8) = 0x2b;
    }
    else if ((((*(char *)(param_3 + local_c) == '-') || (*(char *)(param_3 + local_c) == '@')) ||
             (*(char *)(param_3 + local_c) == '*')) ||
            ((*(char *)(param_3 + local_c) == '_' || (*(char *)(param_3 + local_c) == '.')))) {
      *(undefined1 *)(param_1 + local_8) = *(undefined1 *)(param_3 + local_c);
    }
    else {
      iVar2 = FUN_005912d5((int)*(char *)(param_3 + local_c));
      if (iVar2 == 0) {
        *(undefined1 *)(param_1 + local_8) = 0x25;
        uVar1 = FUN_004b1acf(*(char *)(param_3 + local_c) >> 4 & 0xf);
        *(undefined1 *)(param_1 + local_8 + 1) = uVar1;
        local_8 = local_8 + 2;
        uVar1 = FUN_004b1acf(*(byte *)(param_3 + local_c) & 0xf);
        *(undefined1 *)(param_1 + local_8) = uVar1;
      }
      else {
        *(undefined1 *)(param_1 + local_8) = *(undefined1 *)(param_3 + local_c);
      }
    }
    local_8 = local_8 + 1;
  }
  return;
}

