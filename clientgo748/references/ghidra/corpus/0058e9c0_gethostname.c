// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e9c0 | Name: gethostname


int gethostname(char *name,int namelen)

{
  int iVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e9c0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  iVar1 = gethostname(name,namelen);
  return iVar1;
}

