// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7130 | Name: FID_conflict:max_size


/* Library Function - Multiple Matches With Different Base Names
    public: unsigned int __thiscall std::basic_string<char,struct std::char_traits<char>,class
   std::_DebugHeapAllocator<char> >::max_size(void)const 
    public: unsigned int __thiscall std::basic_string<char,struct std::char_traits<char>,class
   std::allocator<char> >::max_size(void)const 
    public: unsigned int __thiscall std::basic_string<unsigned short,struct
   std::char_traits<unsigned short>,class std::allocator<unsigned short> >::max_size(void)const 
    public: unsigned int __thiscall std::basic_string<wchar_t,struct std::char_traits<wchar_t>,class
   std::allocator<wchar_t> >::max_size(void)const 
   
   Library: Visual Studio 2003 Debug */

int __fastcall FID_conflict_max_size(allocator<char> *param_1)

{
  uint uVar1;
  undefined4 local_10;
  
  uVar1 = std::allocator<char>::max_size(param_1);
  if (uVar1 < 3) {
    local_10 = 1;
  }
  else {
    local_10 = uVar1 - 2;
  }
  return local_10;
}

