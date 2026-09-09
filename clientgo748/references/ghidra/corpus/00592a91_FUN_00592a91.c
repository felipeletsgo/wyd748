// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00592a91 | Name: FUN_00592a91


uint FUN_00592a91(uint param_1,uint param_2)

{
  uint uVar1;
  
  uVar1 = FUN_00592adc();
  uVar1 = uVar1 & ~param_2 | param_1 & param_2;
  FUN_00592b6e(uVar1);
  return uVar1;
}

