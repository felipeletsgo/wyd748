// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058fe9e | Name: _malloc


/* Library Function - Single Match
    _malloc
   
   Library: Visual Studio 2003 Release */

void * __cdecl _malloc(size_t _Size)

{
  void *pvVar1;
  
  pvVar1 = __nh_malloc(_Size,DAT_013b804c);
  return pvVar1;
}

