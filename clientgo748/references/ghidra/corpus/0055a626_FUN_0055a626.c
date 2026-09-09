// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a626 | Name: FUN_0055a626


void FUN_0055a626(undefined4 param_1,int param_2,int param_3,int param_4)

{
  int local_8;
  
  for (local_8 = 0; local_8 < param_4; local_8 = local_8 + 1) {
    (&DAT_00e37e20)[local_8] = *(undefined1 *)(param_2 + -1 + (int)*(char *)(param_3 + local_8));
  }
  FUN_0058f220(param_1,&DAT_00e37e20,param_4);
  return;
}

