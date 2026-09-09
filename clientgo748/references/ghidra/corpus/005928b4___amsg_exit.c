// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005928b4 | Name: __amsg_exit


/* Library Function - Single Match
    __amsg_exit
   
   Library: Visual Studio 2003 Release */

void __cdecl __amsg_exit(int param_1)

{
  if (DAT_013b8030 == 1) {
    FUN_0059a01b();
  }
  FUN_0059a054(param_1);
  (*(code *)PTR___exit_005c9c00)(0xff);
  return;
}

