// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005711ab | Name: FUN_005711ab


/* WARNING: Removing unreachable block (ram,0x005711ca) */
/* WARNING: Removing unreachable block (ram,0x005711bc) */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

undefined8 __fastcall FUN_005711ab(undefined4 param_1,undefined4 param_2)

{
  uint *puVar1;
  int iVar2;
  undefined4 local_8;
  
  local_8 = 0;
  puVar1 = (uint *)cpuid(0x80000000);
  if (0x80000000 < *puVar1) {
    iVar2 = cpuid(0x80000001);
    local_8 = (uint)((*(uint *)(iVar2 + 8) & 0x80000000) != 0);
  }
  return CONCAT44(param_2,local_8);
}

