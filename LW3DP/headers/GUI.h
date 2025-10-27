#include <windows.h>
#include <commctrl.h>

HMENU CreateMenuBar(HWND hwnd, HMENU *sub_menus, char** sub_menu_names, int n_menu_items);
HMENU CreateSubMenu(char **menu_items, UINT_PTR *menu_items_ids, int n_menu_items);
INITCOMMONCONTROLSEX ccInit();
