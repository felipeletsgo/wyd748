// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b72b0 | Name: FUN_004b72b0


void __fastcall
FUN_004b72b0(basic_string<char,std::char_traits<char>,std::allocator<char>_> *param_1)

{
  char *pcVar1;
  
  if (*(int *)(param_1 + 4) != 0) {
    pcVar1 = (char *)FUN_004b6eb0(*(undefined4 *)(param_1 + 4));
    if (*pcVar1 != '\0') {
      pcVar1 = (char *)FUN_004b6eb0(*(undefined4 *)(param_1 + 4));
      if (*pcVar1 != -1) {
        pcVar1 = *(char **)(param_1 + 4);
        FUN_004b6980(1);
        std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::assign(param_1,pcVar1)
        ;
      }
    }
  }
  return;
}

