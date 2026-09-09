// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005900ce | Name: _rand


/* Library Function - Single Match
    _rand
   
   Libraries: Visual Studio 2003 Release, Visual Studio 2005 Release, Visual Studio 2008 Release */

int __cdecl _rand(void)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = FUN_00593d53();
  uVar2 = *(int *)(iVar1 + 0x14) * 0x343fd + 0x269ec3;
  *(uint *)(iVar1 + 0x14) = uVar2;
  return uVar2 >> 0x10 & 0x7fff;
}

