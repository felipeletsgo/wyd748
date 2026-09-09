// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005954cc | Name: FUN_005954cc


undefined4 FUN_005954cc(void)

{
  uint in_EAX;
  
  if ((in_EAX & 0x80000) != 0) {
    return 7;
  }
  return 1;
}

