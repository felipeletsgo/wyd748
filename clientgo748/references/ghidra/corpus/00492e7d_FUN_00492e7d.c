// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00492e7d | Name: FUN_00492e7d


undefined4 FUN_00492e7d(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  
  iVar1 = FUN_0049889a(param_1,param_2);
  if (iVar1 == 1) {
    uVar2 = 1;
  }
  else if ((*(short *)(param_2 + 4) == 0x364) || (*(short *)(param_2 + 4) == 0x363)) {
    uVar2 = FUN_004829f2(param_2);
  }
  else if ((*(short *)(param_2 + 4) == 0x366) || (*(short *)(param_2 + 4) == 0x367)) {
    uVar2 = FUN_00482903(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x1c1) {
    uVar2 = FUN_004927dd(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x333) {
    uVar2 = FUN_00481dd6(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x334) {
    uVar2 = FUN_00481ff3(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x7b1) {
    uVar2 = FUN_004825fc(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3b2) {
    uVar2 = FUN_00482885(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3b3) {
    uVar2 = FUN_004828d8(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3a3) {
    uVar2 = FUN_0048297f(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x116) {
    uVar2 = FUN_00484c8a(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x52a) {
    uVar2 = FUN_00484d44(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x10e) {
    uVar2 = FUN_004851bc(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x114) {
    uVar2 = FUN_0048529b(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x39b) {
    uVar2 = FUN_004854ed(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x339) {
    uVar2 = FUN_00485664(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x18b) {
    uVar2 = FUN_0048569a(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x26e) {
    uVar2 = FUN_004856c3(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x175) {
    uVar2 = FUN_00485d43(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x171) {
    uVar2 = FUN_00486009(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x374) {
    uVar2 = FUN_004862b6(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x16f) {
    uVar2 = FUN_004863e2(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x397) {
    uVar2 = FUN_00486424(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x376) {
    uVar2 = FUN_00486808(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x17c) {
    uVar2 = FUN_004875c0(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x379) {
    uVar2 = FUN_00487b92(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x37a) {
    uVar2 = FUN_00487e23(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x338) {
    uVar2 = FUN_00488267(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x37f) {
    uVar2 = FUN_004883bf(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x37d) {
    uVar2 = FUN_00488879(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x37e) {
    uVar2 = FUN_00488b52(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x292) {
    uVar2 = FUN_00488dd6(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x388) {
    uVar2 = FUN_00488ea9(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x387) {
    uVar2 = FUN_00488f09(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x18d) {
    uVar2 = FUN_00488f69(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x196) {
    uVar2 = FUN_00488fff(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x378) {
    uVar2 = FUN_00489023(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x19c) {
    uVar2 = FUN_004890d5(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3a7) {
    uVar2 = FUN_004890f9(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3ac) {
    uVar2 = FUN_00489145(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3a1) {
    uVar2 = FUN_00489192(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3b0) {
    uVar2 = FUN_00489260(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3a8) {
    uVar2 = FUN_00489300(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3a4) {
    uVar2 = FUN_00489407(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3a5) {
    uVar2 = FUN_00489470(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3a2) {
    uVar2 = FUN_00489618(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3bb) {
    uVar2 = FUN_004897ba(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x1bf) {
    uVar2 = FUN_00489882(param_2);
  }
  else if (((*(short *)(param_2 + 4) == 0x36c) || (*(short *)(param_2 + 4) == 0x39d)) ||
          (*(short *)(param_2 + 4) == 0x39e)) {
    uVar2 = FUN_00489a3e(param_2,extraout_var);
  }
  else if (*(short *)(param_2 + 4) == 0x1c5) {
    uVar2 = FUN_00492892(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x1c6) {
    uVar2 = FUN_004928a4(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x2c8) {
    uVar2 = FUN_004929fe(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x1c2) {
    uVar2 = FUN_00492aa4(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0xdc3) {
    uVar2 = FUN_00492ad6(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3cf) {
    uVar2 = FUN_00492bd6(param_2);
  }
  else if (*(short *)(param_2 + 4) == 0x3d0) {
    uVar2 = FUN_00492d55(param_2);
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

