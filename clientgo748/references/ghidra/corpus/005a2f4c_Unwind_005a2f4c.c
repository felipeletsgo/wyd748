// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005a2f4c | Name: Unwind@005a2f4c


void Unwind_005a2f4c(void)

{
  int unaff_EBP;
  
  exception::~exception(*(exception **)(unaff_EBP + -0x14));
  return;
}

