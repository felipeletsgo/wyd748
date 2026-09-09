// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00492bd6 | Name: FUN_00492bd6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00492bd6(int param_1,int param_2)

{
  undefined1 local_88 [128];
  int local_8;
  
  local_8 = param_2;
  if (*(int *)(param_2 + 0xc) == 0) {
    (**(code **)(**(int **)(param_1 + 0x278dc) + 0x60))(0);
  }
  else {
    *(undefined4 *)(param_1 + 0x28e48) = *(undefined4 *)(param_2 + 0xc);
    FUN_0058f078(local_88,s_0____d_005b890c,*(undefined4 *)(param_1 + 0x28e48));
    if (DAT_005b892c == 2) {
      (**(code **)(**(int **)(param_1 + 0x278dc) + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) - _DAT_005a4434,
                 _DAT_005a40fc * DAT_005b4910);
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x278dc) + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) / DAT_005b490c - _DAT_005a4434,0x41f00000)
      ;
    }
    (**(code **)(**(int **)(param_1 + 0x278dc) + 0x80))(local_88,0);
    (**(code **)(**(int **)(param_1 + 0x278dc) + 0x60))(1);
  }
  return 1;
}

