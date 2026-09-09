// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005a2fc2 | Name: FUN_005a2fc2


void FUN_005a2fc2(void)

{
  int unaff_EBP;
  int unaff_retaddr;
  
  if (unaff_retaddr == 0x48fa0d) {
    (*(code *)0x48fa11)(0);
    return;
  }
  if (unaff_retaddr == 0x491aaa) {
                    /* WARNING: Could not recover jumptable at 0x0058a7b5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (*(code *)(*(int *)(unaff_EBP + -0x898) + 3))();
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x005a2ffa. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)(unaff_retaddr + 3))();
  return;
}

