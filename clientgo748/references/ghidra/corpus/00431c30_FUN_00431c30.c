// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00431c30 | Name: FUN_00431c30


undefined4 __fastcall FUN_00431c30(int param_1)

{
  int iVar1;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  local_c = 0;
  if (*(int *)(param_1 + 0x118) != 0) {
    do {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x118) + 0x20))
                        (*(undefined4 *)(param_1 + 0x118),&local_8,&local_14,&local_10,0);
      if (iVar1 < 0) {
        return 0;
      }
      local_c = (**(code **)(**(int **)(param_1 + 0x118) + 0x30))
                          (*(undefined4 *)(param_1 + 0x118),local_8,local_14,local_10);
    } while (local_8 != 1);
    FUN_0043199b(param_1,0,0);
  }
  return 1;
}

