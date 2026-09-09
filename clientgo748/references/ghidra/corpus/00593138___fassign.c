// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593138 | Name: __fassign


/* Library Function - Single Match
    __fassign
   
   Library: Visual Studio 2003 Release */

void __cdecl __fassign(int flag,char *argument,char *number)

{
  undefined4 local_c;
  undefined4 local_8;
  
  if (flag != 0) {
    FUN_0059aac6(&local_c);
    *(undefined4 *)argument = local_c;
    *(undefined4 *)(argument + 4) = local_8;
    return;
  }
  FUN_0059aaf3(&number,number);
  *(char **)argument = number;
  return;
}

