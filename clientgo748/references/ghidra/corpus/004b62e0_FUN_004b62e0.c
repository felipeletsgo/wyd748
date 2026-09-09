// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b62e0 | Name: FUN_004b62e0


basic_string<char,std::char_traits<char>,std::allocator<char>_> * __thiscall
FUN_004b62e0(basic_string<char,std::char_traits<char>,std::allocator<char>_> *param_1,char *param_2,
            basic_string<char,std::char_traits<char>,std::allocator<char>_> *param_3)

{
  *param_1 = *param_3;
  FUN_004b6980(0);
  std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::assign(param_1,param_2);
  return param_1;
}

