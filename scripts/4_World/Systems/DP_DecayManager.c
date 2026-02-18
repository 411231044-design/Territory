class DP_DecayManager
{
    const float DECAY_INTERVAL = 3600.0; // 1 Hour
    const float BASE_DAMAGE = 50.0;
    
    float m_Timer;

    void DP_DecayManager() 
    { 
        m_Timer = 0; 
    }

    void OnUpdate(float timeslice)
    {
        m_Timer += timeslice;
        if (m_Timer >= DECAY_INTERVAL) 
        { 
            ProcessDecay(); 
            m_Timer = 0; 
        }
    }

    void ProcessDecay()
    {
        Print("[DP_Decay] Starting decay cycle...");
        
        // Get all TerritoryFlag objects using manager instead of world search
        DP_TerritoryManager tm = DP_TerritoryManager.TM_GetInstance();
        if (!tm)
        {
            Print("[DP_Decay] Territory manager not initialized");
            return;
        }
        
        array<ref TM_TerritoryOwnership> ownerships = tm.GetOwnerships();
        if (!ownerships)
        {
            Print("[DP_Decay] Ownerships array not initialized");
            return;
        }
        
        int flagsProcessed = 0;
        for (int i = 0; i < ownerships.Count(); i++)
        {
            TM_TerritoryOwnership ownership = ownerships.Get(i);
            if (!ownership || !ownership.flagObj) continue;
            
            TerritoryFlag flag = TerritoryFlag.Cast(ownership.flagObj);
            if (!flag) continue;

            int perkLevel = flag.GetPreservationPerkLevel(); 
            float damageMult = 1.0;
            
            // Apply perk-based damage reduction
            if (perkLevel == 1) damageMult = 0.8;
            else if (perkLevel == 2) damageMult = 0.6;
            else if (perkLevel >= 3) damageMult = 0.5;
            
            float finalDamage = BASE_DAMAGE * damageMult;
            float radius = flag.GetCurrentRadius();
            
            array<Object> baseObjects = new array<Object>;
            array<CargoBase> proxy = new array<CargoBase>;
            GetGame().GetObjectsAtPosition(flag.GetPosition(), radius, baseObjects, proxy);
            
            int objectsDecayed = 0;
            foreach(Object baseObj : baseObjects)
            {
                // Only decay fences and watchtowers
                if (baseObj.IsInherited(Fence) || baseObj.IsInherited(Watchtower))
                {
                    EntityAI building = EntityAI.Cast(baseObj);
                    // Only apply damage to buildings that are not already ruined
                    // Buildings near ruin will be processed and may become ruined this cycle
                    if (building && !building.IsRuined()) 
                    {
                        building.AddHealth("", "", -finalDamage);
                        objectsDecayed++;
                    }
                }
            }
            
            flagsProcessed++;
            Print(string.Format("[DP_Decay] Flag %1: Processed %2 objects with %.1f damage (perk level %3)", 
                flagsProcessed, objectsDecayed, finalDamage, perkLevel));
        }
        
        Print(string.Format("[DP_Decay] Decay cycle completed. Processed %1 territories", flagsProcessed));
    }
}