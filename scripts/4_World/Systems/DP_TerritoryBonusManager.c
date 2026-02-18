// Territory Bonus Manager
class DP_TerritoryBonusManager
{
    const float UPDATE_INTERVAL = 1.0; // Update every 1 second
    
    float m_Timer;
    
    void DP_TerritoryBonusManager()
    {
        m_Timer = 0;
    }
    
    void OnUpdate(float timeslice)
    {
        m_Timer += timeslice;
        
        if (m_Timer >= UPDATE_INTERVAL)
        {
            ApplyBonusesToPlayers();
            m_Timer = 0;
        }
    }
    
    void ApplyBonusesToPlayers()
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.IsAlive()) continue;
            
            ApplyBonusToPlayer(player);
        }
    }
    
    void ApplyBonusToPlayer(PlayerBase player)
    {
        if (!player) return;
        
        vector playerPos = player.GetPosition();
        DP_TerritoryManager tm = DP_TerritoryManager.TM_GetInstance();
        if (!tm) return;
        
        float maxRadius = DP_TerritoryConfig.Get().GetMaxPossibleRadius();
        Object flagObj = tm.TM_GetNearestFlag(playerPos, maxRadius);
        
        if (!flagObj) return;
        
        TerritoryFlag flag = TerritoryFlag.Cast(flagObj);
        if (!flag) return;
        
        string playerID = player.GetIdentity().GetId();
        string ownerID = flag.GetOwnerID();
        int territoryLevel = flag.GetTerritoryLevel();
        
        bool isOwner = (playerID == ownerID);
        bool isMember = flag.HasPermissions(playerID);
        
        // Apply bonuses if player is owner or member
        if (isOwner || isMember)
        {
            ApplyFriendlyBonuses(player, territoryLevel, flag);
        }
        else
        {
            // Apply debuffs if player is enemy
            ApplyEnemyDebuffs(player);
        }
    }
    
    void ApplyFriendlyBonuses(PlayerBase player, int level, TerritoryFlag flag)
    {
        if (!player) return;
        
        // Level 1+: Slower hunger/thirst depletion
        if (level >= 1)
        {
            PlayerStat energy = player.GetStatEnergy();  // Hunger
            PlayerStat water = player.GetStatWater();    // Thirst
            
            if (energy)
            {
                energy.Add(0.5); // Slower depletion
            }
            
            if (water)
            {
                water.Add(0.5); // Slower depletion
            }
        }
        
        // Level 2+: Health regeneration + crafting speed bonuses
        if (level >= 2)
        {
            float regenAmount = 0.1; // Base regeneration for level 2
            
            // Level 3: Enhanced regeneration
            if (level >= 3)
            {
                regenAmount = 0.15;
            }
            
            float currentHealth = player.GetHealth("", "Health");
            float maxHealth = player.GetMaxHealth("", "Health");
            
            if (currentHealth < maxHealth)
            {
                player.AddHealth("", "Health", regenAmount);
            }
        }
        
        // Level 3: Zombie invisibility
        if (level >= 3)
        {
            // Zombie invisibility within 20m of flag
            vector flagPos = flag.GetPosition();
            float distToFlag = vector.Distance(player.GetPosition(), flagPos);
            
            if (distToFlag <= 20.0)
            {
                // Make player invisible to zombies
                player.SetInvisibility(true);
            }
            else
            {
                player.SetInvisibility(false);
            }
        }
    }
    
    void ApplyEnemyDebuffs(PlayerBase player)
    {
        if (!player) return;
        
        // Stamina penalty
        PlayerStat stamina = player.GetStatStamina();
        if (stamina)
        {
            stamina.Add(-1.0); // -1/sec
        }
        
        // No invisibility for enemies
        player.SetInvisibility(false);
    }
}
