// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005a2fb0 | Name: Unwind@005a2fb0


void Unwind_005a2fb0(void)

{
  int unaff_EBP;
  
  exception::~exception(*(exception **)(unaff_EBP + -0x10));
  return;
}

