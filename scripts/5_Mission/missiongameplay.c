modded class MissionGameplay
{
    override UIScriptedMenu CreateScriptedMenu(int id)
    {
        UIScriptedMenu menu = super.CreateScriptedMenu(id);
        if (menu) return menu;
        
        if (id == DP_TerritoryConstants.MENU_ID_TERRITORY_MANAGEMENT)
        {
            menu = new DP_ManagementMenu;
            return menu;
        }

        return null;
    }

    override void OnKeyPress(int key)
    {
        super.OnKeyPress(key);

        if (key == KeyCode.KC_ESCAPE)
        {
            UIScriptedMenu menu = GetGame().GetUIManager().GetMenu();
            if (menu)
            {
                if (menu.ClassName() == "DP_ManagementMenu")
                {
                    menu.Close();
                }
            }
        }
    }
}