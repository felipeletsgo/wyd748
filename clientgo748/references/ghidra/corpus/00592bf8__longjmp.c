// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00592bf8 | Name: _longjmp


/* WARNING: Unable to track spacebase fully for stack */
/* Library Function - Single Match
    _longjmp
   
   Libraries: Visual Studio 1998 Debug, Visual Studio 1998 Release, Visual Studio 2003 Debug, Visual
   Studio 2003 Release */

void __cdecl _longjmp(int *_Buf,int _Value)

{
  void *pvVar1;
  int iVar2;
  
  pvVar1 = (void *)_Buf[6];
  if (pvVar1 != ExceptionList) {
    __global_unwind2(pvVar1);
  }
  if (pvVar1 != (void *)0x0) {
    iVar2 = FUN_0059a378(_Buf + 8);
    if ((iVar2 == 0) || (_Buf[8] != 0x56433230)) {
      __local_unwind2(pvVar1,_Buf[7]);
    }
    else if ((code *)_Buf[9] != (code *)0x0) {
      (*(code *)_Buf[9])(_Buf);
    }
  }
  FUN_0058ee02(0);
                    /* WARNING: Could not recover jumptable at 0x00592c6d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)_Buf[5])();
  return;
}

