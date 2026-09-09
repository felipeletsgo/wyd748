// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005558ab | Name: FUN_005558ab


/* WARNING: Removing unreachable block (ram,0x005558d3) */

byte FUN_005558ab(int param_1,int param_2)

{
  byte bVar1;
  
  bVar1 = *(byte *)(param_1 + param_2 + 0x18);
  if (199 < bVar1) {
    bVar1 = 200;
  }
  return bVar1;
}

