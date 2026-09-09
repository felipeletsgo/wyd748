// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00472651 | Name: FUN_00472651


/* WARNING: Removing unreachable block (ram,0x00472674) */
/* WARNING: Removing unreachable block (ram,0x00472685) */

void __fastcall FUN_00472651(int param_1)

{
  int local_c;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x28b44) != 0) {
    local_c = 0;
    local_8 = (**(code **)(**(int **)(param_1 + 0x28b68) + 0x88))();
    FUN_005909d2(local_8,&DAT_005b8260,&local_c);
    (**(code **)(**(int **)(param_1 + 0x28b68) + 0x90))(&DAT_005d0484);
    (**(code **)(**(int **)(param_1 + 0x28b60) + 0x90))(&DAT_005d0488);
    (**(code **)(**(int **)(param_1 + 0x28b64) + 0x90))(&DAT_005d048c);
    if ((DAT_00e37d28 < local_c) || (local_c < 1)) {
      *(undefined4 *)(param_1 + 0x28b40) = 0;
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x28b54) + 0x80))(&DAT_00d42700 + local_c * 0x60,0);
      (**(code **)(**(int **)(param_1 + 0x28b58) + 0x80))(&DAT_00d42720 + local_c * 0x60,0);
      (**(code **)(**(int **)(param_1 + 0x28b5c) + 0x80))(&DAT_00d42740 + local_c * 0x60,0);
      *(int *)(param_1 + 0x28b40) = local_c;
    }
  }
  return;
}

