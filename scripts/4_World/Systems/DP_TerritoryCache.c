// Territory Cache System for Performance
class DP_TerritoryCacheEntry
{
    int WallCount;
    int TentCount;
    int FurnitureCount;
    ref map<string, int> CustomItemCounts; // For tracking custom limits
    int LastUpdateTime;
    
    void DP_TerritoryCacheEntry()
    {
        WallCount = 0;
        TentCount = 0;
        FurnitureCount = 0;
        CustomItemCounts = new map<string, int>();
        LastUpdateTime = 0;
    }
}

class DP_TerritoryCache
{
    const float CACHE_UPDATE_INTERVAL = 5.0; // Update every 5 seconds
    const float CACHE_CLEANUP_INTERVAL = 60.0; // Cleanup stale entries every 60 seconds
    
    ref map<TerritoryFlag, ref DP_TerritoryCacheEntry> m_Cache;
    float m_UpdateTimer;
    float m_CleanupTimer;
    
    void DP_TerritoryCache()
    {
        m_Cache = new map<TerritoryFlag, ref DP_TerritoryCacheEntry>();
        m_UpdateTimer = 0;
        m_CleanupTimer = 0;
    }
    
    void OnUpdate(float timeslice)
    {
        m_UpdateTimer += timeslice;
        m_CleanupTimer += timeslice;
        
        // Update cache entries
        if (m_UpdateTimer >= CACHE_UPDATE_INTERVAL)
        {
            UpdateAllCaches();
            m_UpdateTimer = 0;
        }
        
        // Cleanup stale entries
        if (m_CleanupTimer >= CACHE_CLEANUP_INTERVAL)
        {
            CleanupStaleEntries();
            m_CleanupTimer = 0;
        }
    }
    
    void UpdateAllCaches()
    {
        DP_TerritoryManager tm = DP_TerritoryManager.TM_GetInstance();
        if (!tm) return;
        
        array<ref TM_TerritoryOwnership> ownerships = tm.GetOwnerships();
        if (!ownerships) return;
        
        for (int i = 0; i < ownerships.Count(); i++)
        {
            TM_TerritoryOwnership ownership = ownerships.Get(i);
            if (!ownership || !ownership.flagObj) continue;
            
            TerritoryFlag flag = TerritoryFlag.Cast(ownership.flagObj);
            if (!flag) continue;
            
            UpdateCacheForFlag(flag);
        }
    }
    
    void UpdateCacheForFlag(TerritoryFlag flag)
    {
        if (!flag) return;
        
        DP_TerritoryCacheEntry entry;
        if (!m_Cache.Find(flag, entry))
        {
            entry = new DP_TerritoryCacheEntry();
            m_Cache.Insert(flag, entry);
        }
        
        // Get objects in territory
        float radius = flag.GetCurrentRadius();
        array<Object> objects = new array<Object>;
        array<CargoBase> proxyCargos = new array<CargoBase>;
        GetGame().GetObjectsAtPosition(flag.GetPosition(), radius, objects, proxyCargos);
        
        // Reset counts
        entry.WallCount = 0;
        entry.TentCount = 0;
        entry.FurnitureCount = 0;
        entry.CustomItemCounts.Clear();
        
        // Count objects
        for (int i = 0; i < objects.Count(); i++)
        {
            Object obj = objects.Get(i);
            if (!obj || obj == flag) continue;
            if (!DP_TerritoryManager.TM_GetInstance().IsPlacedObject(obj)) continue;
            
            string objType = obj.GetType();
            int category = DP_TerritoryConfig.Get().GetItemCategory(objType);
            
            if (category == 1) entry.WallCount++;
            else if (category == 2) entry.TentCount++;
            else if (category == 3) entry.FurnitureCount++;
            
            // Track custom limits
            int level = flag.GetTerritoryLevel();
            DP_ItemLimit customLimit = DP_TerritoryConfig.Get().GetCustomLimit(objType, level);
            if (customLimit)
            {
                int currentCount = 0;
                if (entry.CustomItemCounts.Find(customLimit.ClassName, currentCount))
                {
                    entry.CustomItemCounts.Set(customLimit.ClassName, currentCount + 1);
                }
                else
                {
                    entry.CustomItemCounts.Insert(customLimit.ClassName, 1);
                }
            }
        }
        
        entry.LastUpdateTime = GetGame().GetTime();
    }
    
    DP_TerritoryCacheEntry GetCacheEntry(TerritoryFlag flag)
    {
        if (!flag) return null;
        
        DP_TerritoryCacheEntry entry;
        if (m_Cache.Find(flag, entry))
        {
            return entry;
        }
        
        // If not cached yet, create and update
        UpdateCacheForFlag(flag);
        if (m_Cache.Find(flag, entry))
        {
            return entry;
        }
        
        return null;
    }
    
    void InvalidateCache(TerritoryFlag flag)
    {
        if (!flag) return;
        UpdateCacheForFlag(flag);
    }
    
    void CleanupStaleEntries()
    {
        array<TerritoryFlag> keysToRemove = new array<TerritoryFlag>();
        
        for (int i = 0; i < m_Cache.Count(); i++)
        {
            TerritoryFlag flag = m_Cache.GetKey(i);
            
            // Check if flag still exists
            if (!flag || flag.IsDeleted())
            {
                keysToRemove.Insert(flag);
            }
        }
        
        // Remove stale entries
        for (int j = 0; j < keysToRemove.Count(); j++)
        {
            m_Cache.Remove(keysToRemove.Get(j));
        }
        
        if (keysToRemove.Count() > 0)
        {
            Print(string.Format("[DP_TerritoryCache] Cleaned up %1 stale entries", keysToRemove.Count()));
        }
    }
    
    // Get instance from mission server
    static DP_TerritoryCache GetInstance()
    {
        MissionServer mission = MissionServer.Cast(GetGame().GetMission());
        if (mission)
        {
            return mission.m_TerritoryCache;
        }
        return null;
    }
}
