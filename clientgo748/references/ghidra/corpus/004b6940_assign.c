// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6940 | Name: assign


/* Library Function - Single Match
    public: class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
   & __thiscall std::basic_string<char,struct std::char_traits<char>,class std::allocator<char>
   >::assign(char const *)
   
   Libraries: Visual Studio 2003 Debug, Visual Studio 2005 Debug */

basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> * __thiscall
std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::assign
          (basic_string<char,std::char_traits<char>,std::allocator<char>_> *this,char *param_1)

{
  undefined4 uVar1;
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *pbVar2;
  
  uVar1 = length(param_1,this);
  pbVar2 = (basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *)
           FUN_004b6be0(param_1,uVar1);
  return pbVar2;
}

