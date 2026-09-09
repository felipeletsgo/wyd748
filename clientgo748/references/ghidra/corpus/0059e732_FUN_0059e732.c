// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e732 | Name: FUN_0059e732


exception * __thiscall FUN_0059e732(exception *param_1,byte param_2)

{
  exception::~exception(param_1);
  if ((param_2 & 1) != 0) {
    FUN_0058ea3a(param_1);
  }
  return param_1;
}

