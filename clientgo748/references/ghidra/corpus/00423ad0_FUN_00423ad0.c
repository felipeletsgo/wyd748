// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423ad0 | Name: FUN_00423ad0


COleResizeBar * __thiscall FUN_00423ad0(COleResizeBar *param_1,uint param_2)

{
  COleResizeBar::~COleResizeBar(param_1);
  if ((param_2 & 1) != 0) {
    FUN_0058ea3a(param_1);
  }
  return param_1;
}

