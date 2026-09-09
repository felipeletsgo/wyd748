// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058f730 | Name: FUN_0058f730


/* WARNING: Unable to track spacebase fully for stack */

void FUN_0058f730(void)

{
  uint in_EAX;
  undefined1 *puVar1;
  undefined4 unaff_retaddr;
  
  puVar1 = &stack0x00000004;
  for (; 0xfff < in_EAX; in_EAX = in_EAX - 0x1000) {
    puVar1 = puVar1 + -0x1000;
  }
  *(undefined4 *)(puVar1 + (-4 - in_EAX)) = unaff_retaddr;
  return;
}

