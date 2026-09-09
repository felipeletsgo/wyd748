// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e388 | Name: FUN_0059e388


void FUN_0059e388(void)

{
  exception *this;
  int unaff_EBP;
  
  FUN_00592900();
  *(exception **)(unaff_EBP + -0x10) = this;
  *(undefined ***)this = &PTR_FUN_005aa85c;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  FUN_004b6980(1);
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  exception::~exception(this);
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return;
}

