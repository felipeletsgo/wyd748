// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00431b5d | Name: FUN_00431b5d


int __fastcall FUN_00431b5d(int param_1)

{
  int local_c;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x11c) == 0) {
    local_8 = 0;
  }
  else {
    local_8 = (**(code **)(**(int **)(param_1 + 0x11c) + 0x8c))
                        (*(undefined4 *)(param_1 + 0x11c),&local_c);
    if ((-1 < local_8) && (local_c == 0)) {
      (**(code **)(**(int **)(param_1 + 0x11c) + 0x80))
                (*(undefined4 *)(param_1 + 0x11c),&DAT_005ccfb0);
      (**(code **)(**(int **)(param_1 + 0x11c) + 0x7c))
                (*(undefined4 *)(param_1 + 0x11c),*(undefined4 *)(DAT_013b71e0 + 4));
      local_c = -1;
      local_8 = (**(code **)(**(int **)(param_1 + 0x11c) + 0x90))
                          (*(undefined4 *)(param_1 + 0x11c),0xffffffff);
    }
  }
  return local_8;
}

