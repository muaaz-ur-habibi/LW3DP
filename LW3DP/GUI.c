#include "headers/GUI.h"


HMENU CreateMenuBar(HWND hwnd, HMENU *sub_menus, char** sub_menu_names, int n_menu_items)
{
    HMENU hMenu = CreateMenu();
    for (size_t i = 0; i < n_menu_items; i++)
    {
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)sub_menus[i], sub_menu_names[i]);
    }
    
    SetMenu(hwnd, hMenu);

    return hMenu;
}

HMENU CreateSubMenu(char **menu_items, UINT_PTR *menu_items_ids, int n_menu_items)
{
    HMENU hMenu = CreatePopupMenu();

    for (size_t i = 0; i < n_menu_items; i++)
    {
        AppendMenuA(hMenu, MF_STRING, menu_items_ids[i], menu_items[i]);
    }
    
    return hMenu;
}

INITCOMMONCONTROLSEX ccInit()
{
    INITCOMMONCONTROLSEX ccex;
    ccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    ccex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&ccex);

    return ccex;
}

void CreateGUIComponents()
{

}