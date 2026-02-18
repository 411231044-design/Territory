// ОПЫТ ЗА РАСПИЛ БРЕВЕН
modded class ActionSawPlanks
{
    override void OnFinishProgressServer(ActionData action_data)
    {
        super.OnFinishProgressServer(action_data);
        if (GetGame().IsServer() && action_data.m_Player)
        {
            if (action_data.m_Player.GetTerjeSkills()) 
            {
                // ИСПРАВЛЕНО: AddExperience -> AddSkillExperience
                action_data.m_Player.GetTerjeSkills().AddSkillExperience("Skill_Architect", 25);
            }
        }
    }
}

// ОПЫТ ЗА СТРОИТЕЛЬСТВО
modded class ActionBuildPart
{
    override void OnFinishProgressServer(ActionData action_data)
    {
        super.OnFinishProgressServer(action_data);
        if (GetGame().IsServer() && action_data.m_Player)
        {
            if (action_data.m_Player.GetTerjeSkills()) 
            {
                // ИСПРАВЛЕНО: AddExperience -> AddSkillExperience
                action_data.m_Player.GetTerjeSkills().AddSkillExperience("Skill_Architect", 100);
            }
        }
    }
}

// ПЕРК: РЕСТАВРАТОР (ActionRepairPart)
modded class ActionRepairPart
{
    override void OnFinishProgressServer(ActionData action_data)
    {
        super.OnFinishProgressServer(action_data);
        
        PlayerBase player = action_data.m_Player;
        if (player)
        {
            float perkVal = 0;
            if (player.GetTerjeSkills()) 
            {
                player.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_Restorer", perkVal);
            }
            int perkLvl = (int)perkVal;

            if (perkLvl > 0)
            {
                float chance = 0.3 * perkLvl; 
                if (Math.RandomFloat01() < chance)
                {
                     ItemBase tool = action_data.m_MainItem;
                     if (tool && !tool.IsRuined())
                     {
                         tool.AddHealth(10); 
                     }
                }
            }
            
            if (player.GetTerjeSkills()) 
            {
                // ИСПРАВЛЕНО: AddExperience -> AddSkillExperience
                player.GetTerjeSkills().AddSkillExperience("Skill_Architect", 20);
            }
        }
    }
}