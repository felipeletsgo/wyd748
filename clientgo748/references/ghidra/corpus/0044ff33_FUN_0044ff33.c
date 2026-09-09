// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044ff33 | Name: FUN_0044ff33


undefined4 FUN_0044ff33(short param_1,short param_2)

{
  undefined4 uVar1;
  
  if (((param_1 == 0x91a) || (param_1 == 0x938)) && (param_2 == 0x974)) {
    uVar1 = 1;
  }
  else if (((param_1 == 0x91b) || (param_1 == 0x939)) && ((param_2 == 0x975 || (param_2 == 0xd28))))
  {
    uVar1 = 1;
  }
  else if (((param_1 == 0x91c) || (param_1 == 0x93a)) && ((param_2 == 0x976 || (param_2 == 0xd29))))
  {
    uVar1 = 1;
  }
  else if (((param_1 == 0x91d) || (param_1 == 0x93b)) && ((param_2 == 0x977 || (param_2 == 0xd2a))))
  {
    uVar1 = 1;
  }
  else if (((param_1 == 0x91e) || (param_1 == 0x93c)) && ((param_2 == 0x978 || (param_2 == 0xd2b))))
  {
    uVar1 = 1;
  }
  else if (((param_1 == 0x91f) || (param_1 == 0x93d)) && ((param_2 == 0x979 || (param_2 == 0xd2c))))
  {
    uVar1 = 1;
  }
  else if (((((param_1 < 0x920) || (0x929 < param_1)) && ((param_1 < 0x93e || (0x947 < param_1))))
           && ((param_1 < 0xb90 || (2999 < param_1)))) || ((param_2 != 0x97a && (param_2 != 0xd2d)))
          ) {
    if (((param_1 == 0x92a) || (param_1 == 0x948)) && ((param_2 == 0x984 || (param_2 == 0xd37)))) {
      uVar1 = 1;
    }
    else if (((param_1 == 0x92b) || (param_1 == 0x949)) &&
            ((param_2 == 0x985 || (param_2 == 0xd38)))) {
      uVar1 = 1;
    }
    else if (((param_1 == 0x92c) || (param_1 == 0x94a)) &&
            ((param_2 == 0x986 || (param_2 == 0xd39)))) {
      uVar1 = 1;
    }
    else if (((param_1 == 0x92d) || (param_1 == 0x94b)) &&
            ((param_2 == 0x97b || (param_2 == 0xd2e)))) {
      uVar1 = 1;
    }
    else if (((param_1 == 0x92e) || (param_1 == 0x94c)) &&
            ((param_2 == 0x97c || (param_2 == 0xd2f)))) {
      uVar1 = 1;
    }
    else if ((((param_1 < 0x92f) || (0x931 < param_1)) && ((param_1 < 0x94d || (0x94f < param_1))))
            || ((param_2 != 0x97d && (param_2 != 0xd30)))) {
      if ((((param_1 < 0x932) || (0x934 < param_1)) && ((param_1 < 0x950 || (0x952 < param_1)))) ||
         ((param_2 != 0x97e && (param_2 != 0xd31)))) {
        uVar1 = 0;
      }
      else {
        uVar1 = 1;
      }
    }
    else {
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

