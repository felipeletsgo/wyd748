// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a483 | Name: FUN_0055a483


void __thiscall FUN_0055a483(undefined4 param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  
  FUN_0055a709(&DAT_00e37d70,param_3,0x40,param_1);
  FUN_0055a626(&DAT_00e37d70,&DAT_00e37d70,&DAT_005a5e8c,0x38);
  for (iVar1 = 0; iVar1 < 0x10; iVar1 = iVar1 + 1) {
    FUN_0055a6ba(PTR_DAT_005c490c,0x1c,(int)(char)(&DAT_005a5ef4)[iVar1]);
    FUN_0055a6ba(PTR_DAT_005c4910,0x1c,(int)(char)(&DAT_005a5ef4)[iVar1]);
    FUN_0055a626(param_2 + iVar1 * 0x30,&DAT_00e37d70,&DAT_005a5ec4,0x30);
  }
  return;
}

