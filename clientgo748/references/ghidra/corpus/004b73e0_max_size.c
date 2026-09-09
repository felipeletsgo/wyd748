// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b73e0 | Name: max_size


/* WARNING: Removing unreachable block (ram,0x004b73fe) */
/* Library Function - Single Match
    public: unsigned int __thiscall std::allocator<char>::max_size(void)const 
   
   Library: Visual Studio 2003 Debug */

uint __thiscall std::allocator<char>::max_size(allocator<char> *this)

{
  return 0xffffffff;
}

