// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058ed2c | Name: __global_unwind2


/* Library Function - Single Match
    __global_unwind2
   
   Libraries: Visual Studio 1998 Debug, Visual Studio 1998 Release, Visual Studio 2003 Debug, Visual
   Studio 2003 Release */

void __global_unwind2(PVOID param_1)

{
  RtlUnwind(param_1,(PVOID)0x58ed44,(PEXCEPTION_RECORD)0x0,(PVOID)0x0);
  return;
}

