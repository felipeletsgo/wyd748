// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059d496 | Name: __mbsnbicoll


/* Library Function - Single Match
    __mbsnbicoll
   
   Library: Visual Studio 2003 Release */

int __cdecl __mbsnbicoll(uchar *_Str1,uchar *_Str2,size_t _MaxCount)

{
  int iVar1;
  
  if (_MaxCount == 0) {
    return 0;
  }
  iVar1 = FUN_0059d7df(DAT_013b85e4,1,_Str1,_MaxCount,_Str2,_MaxCount,DAT_013b83b4);
  if (iVar1 == 0) {
    return 0x7fffffff;
  }
  return iVar1 + -2;
}

