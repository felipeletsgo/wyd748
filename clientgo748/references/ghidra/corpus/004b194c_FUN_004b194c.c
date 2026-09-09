// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b194c | Name: FUN_004b194c


void FUN_004b194c(int param_1,char param_2,undefined1 param_3)

{
  undefined4 local_8;
  
  for (local_8 = 0; *(char *)(param_1 + local_8) != '\0'; local_8 = local_8 + 1) {
    if (*(char *)(param_1 + local_8) == param_2) {
      *(undefined1 *)(param_1 + local_8) = param_3;
    }
  }
  return;
}

