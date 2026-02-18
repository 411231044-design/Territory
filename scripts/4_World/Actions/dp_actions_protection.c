modded class ActionConstructor
{
    override void RegisterActions(TTypenameArray actions) 
    { 
        super.RegisterActions(actions); 
        actions.Insert(DP_TerritoryFlagAction); 
    }
}

modded class ActionDeployObject
{
    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!super.ActionCondition(player, target, item)) return false;
        
        if (GetGame().IsServer() && item)
        {
            DP_TerritoryManager tm = DP_TerritoryManager.TM_GetInstance();
            if (!tm) return true; 

            if (!tm.TM_CanBuildOrDismantle(player))
            {
                player.MessageImportant("⛔ Чужая территория!");
                return false;
            }

            if (item.IsInherited(TerritoryFlag) || item.IsKindOf("TerritoryFlagKit")) 
            {
                float safeDist = DP_TerritoryConfig.Get().GetMaxPossibleRadius() * 2.0;
                if (tm.TM_IsTerritoryNearby(player.GetPosition(), safeDist))
                {
                    player.MessageImportant("⛔ Слишком близко к другой базе! Нужно " + safeDist + "м.");
                    return false;
                }
            }

            if (!tm.TM_CheckLimits(player, item.GetType()))
            {
                return false;
            }
        }
        return true;
    }
}

modded class ActionContinuousBase
{
    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!super.ActionCondition(player, target, item)) return false;
        
        if (item && GetGame().IsServer())
        {
            if (item.IsInherited(Shovel) || item.IsInherited(Hammer) || item.IsInherited(Pickaxe) || item.IsInherited(Pliers) || item.IsInherited(GardenLime) || item.IsInherited(Crowbar))
            {
                 DP_TerritoryManager tm = DP_TerritoryManager.TM_GetInstance();
                 if (tm && !tm.TM_CanBuildOrDismantle(player))
                 {
                    return false;
                 }
            }
        }
        return true;
    }
}