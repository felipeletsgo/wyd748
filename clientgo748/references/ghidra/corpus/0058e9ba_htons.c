// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058e9ba | Name: htons


u_short htons(u_short hostshort)

{
  u_short uVar1;
  
                    /* WARNING: Could not recover jumptable at 0x0058e9ba. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar1 = htons(hostshort);
  return uVar1;
}

