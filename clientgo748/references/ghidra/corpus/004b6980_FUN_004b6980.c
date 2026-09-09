// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6980 | Name: FUN_004b6980


void __thiscall FUN_004b6980(int param_1,char param_2)

{
  char *pcVar1;
  
  if ((param_2 != '\0') && (*(int *)(param_1 + 4) != 0)) {
    pcVar1 = (char *)FUN_004b6eb0(*(undefined4 *)(param_1 + 4));
    if ((*pcVar1 == '\0') ||
       (pcVar1 = (char *)FUN_004b6eb0(*(undefined4 *)(param_1 + 4)), *pcVar1 == -1)) {
      FUN_004b6b10(*(int *)(param_1 + 4) + -1,*(int *)(param_1 + 0xc) + 2);
    }
    else {
      pcVar1 = (char *)FUN_004b6eb0(*(undefined4 *)(param_1 + 4));
      *pcVar1 = *pcVar1 + -1;
    }
  }
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 8) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  return;
}

