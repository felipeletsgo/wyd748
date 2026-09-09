// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058f0ca | Name: operator_new


/* Library Function - Single Match
    void * __cdecl operator new(unsigned int)
   
   Library: Visual Studio 2003 Release */

void * __cdecl operator_new(uint param_1)

{
  void *pvVar1;
  
  pvVar1 = __nh_malloc(param_1,1);
  return pvVar1;
}

