// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058feb0 | Name: __nh_malloc


/* Library Function - Single Match
    __nh_malloc
   
   Library: Visual Studio 2003 Release */

void * __cdecl __nh_malloc(size_t _Size,int _NhFlag)

{
  void *pvVar1;
  int iVar2;
  
  if (_Size < 0xffffffe1) {
    do {
      pvVar1 = (void *)FUN_0058fedc(_Size);
      if (pvVar1 != (void *)0x0) {
        return pvVar1;
      }
      if (_NhFlag == 0) {
        return (void *)0x0;
      }
      iVar2 = FUN_00595683(_Size);
    } while (iVar2 != 0);
  }
  return (void *)0x0;
}

