// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6350 | Name: operator=


/* Library Function - Single Match
    public: class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
   & __thiscall std::basic_string<char,struct std::char_traits<char>,class std::allocator<char>
   >::operator=(char const *)
   
   Libraries: Visual Studio 2003 Debug, Visual Studio 2005 Debug */

basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> * __thiscall
std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::operator=
          (basic_string<char,std::char_traits<char>,std::allocator<char>_> *this,char *param_1)

{
  basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> *pbVar1;
  
  pbVar1 = assign(this,param_1);
  return pbVar1;
}

