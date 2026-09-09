// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e526 | Name: FUN_0059e526


void FUN_0059e526(void)

{
  exception *this;
  int unaff_EBP;
  undefined4 *unaff_FS_OFFSET;
  
  FUN_00592900();
  *(exception **)(unaff_EBP + -0x10) = this;
  *(undefined ***)this = &PTR_FUN_005aa85c;
  *(undefined4 *)(unaff_EBP + -4) = 0;
  FUN_004b6980(1);
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  exception::~exception(this);
  *unaff_FS_OFFSET = *(undefined4 *)(unaff_EBP + -0xc);
  return;
}

