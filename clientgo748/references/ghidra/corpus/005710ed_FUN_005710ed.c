// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005710ed | Name: FUN_005710ed


/* WARNING: Removing unreachable block (ram,0x005710f9) */

undefined8 __fastcall FUN_005710ed(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = cpuid_Version_info(1);
  return CONCAT44(param_2,(uint)((*(uint *)(iVar1 + 8) & 0x800000) != 0));
}

