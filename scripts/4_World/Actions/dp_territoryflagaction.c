class DP_TerritoryFlagAction: ActionInteractBase
{
    void DP_TerritoryFlagAction()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
        m_HUDCursorIcon = CursorIcons.CloseHood;
    }

    override string GetText()
    {
        return "Управление территорией";
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        Object targetObject = target.GetObject();
        if (!targetObject) return false;

        TerritoryFlag flag = TerritoryFlag.Cast(targetObject);
        if (!flag) return false;

        return true;
    }

    override void OnStartClient(ActionData action_data)
    {
        Object targetObject = action_data.m_Target.GetObject();
        TerritoryFlag flag = TerritoryFlag.Cast(targetObject);
        if (!flag) return;

        flag.RequestSyncDataAll(); 
        DP_TerritoryManager.TM_GetInstance().m_ClientPendingTarget = flag;
        GetGame().GetUIManager().EnterScriptedMenu(DP_TerritoryConstants.MENU_ID_TERRITORY_MANAGEMENT, null);
    }
}