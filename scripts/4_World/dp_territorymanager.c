class TM_TerritoryOwnership
{
    string ownerId;
    Object flagObj;
    void TM_TerritoryOwnership() {}
}

class DP_TerritoryManager
{
    protected static ref DP_TerritoryManager s_Instance;
    protected ref array<ref TM_TerritoryOwnership> m_Ownerships;
    
    Object m_ClientPendingTarget;
    string m_LastReceivedOwnerID;

    void DP_TerritoryManager()
    {
        m_Ownerships = new array<ref TM_TerritoryOwnership>();
    }

    static DP_TerritoryManager TM_GetInstance()
    {
        if (!s_Instance) s_Instance = new DP_TerritoryManager();
        return s_Instance;
    }
    
    // Public getter for m_Ownerships
    array<ref TM_TerritoryOwnership> GetOwnerships()
    {
        return m_Ownerships;
    }

    Object TM_GetNearestFlag(vector position, float radius)
    {
        float minDistSq = radius * radius;
        for (int i = 0; i < m_Ownerships.Count(); ++i)
        {
            TM_TerritoryOwnership to = m_Ownerships.Get(i);
            if (!to || !to.flagObj) continue;
            
            float searchR = radius; 
            TerritoryFlag flag = TerritoryFlag.Cast(to.flagObj);
            if (flag) searchR = flag.GetCurrentRadius();
            
            float distSq = vector.DistanceSq(position, to.flagObj.GetPosition());
            if (distSq < (searchR * searchR)) return to.flagObj;
        }
        return null;
    }

    bool TM_IsTerritoryNearby(vector position, float safeDistance)
    {
        float safeDistSq = safeDistance * safeDistance;
        for (int i = 0; i < m_Ownerships.Count(); ++i)
        {
            TM_TerritoryOwnership to = m_Ownerships.Get(i);
            if (!to || !to.flagObj) continue;
            float distSq = vector.DistanceSq(position, to.flagObj.GetPosition());
            if (distSq < safeDistSq) return true; 
        }
        return false;
    }

    bool TM_HasTerritory(string ownerId)
    {
        if (!ownerId) return false;
        for (int i = 0; i < m_Ownerships.Count(); ++i)
        {
            TM_TerritoryOwnership to = m_Ownerships.Get(i);
            if (to && to.ownerId == ownerId) return true;
        }
        return false;
    }
    
    bool IsPlacedObject(Object obj)
    {
        if (!obj) return false;
        EntityAI entity = EntityAI.Cast(obj);
        if (!entity) return false;
        if (entity.IsHologram()) return false;
        if (entity.GetHierarchyParent()) return false; 
        TentBase tent = TentBase.Cast(obj);
        if (tent) { if (tent.GetState() == 0) return false; }
        return true;
    }

   bool TM_CheckLimits(PlayerBase player, string itemClassname)
    {
        if (!player) return false;
        float maxR = DP_TerritoryConfig.Get().GetMaxPossibleRadius();
        Object flagObj = TM_GetNearestFlag(player.GetPosition(), maxR);
        if (!flagObj) return true; 
        
        TerritoryFlag flag = TerritoryFlag.Cast(flagObj);
        if (!flag) return true;
        
        int lvl = flag.GetTerritoryLevel();
        DP_LevelDefinition levelDef = DP_TerritoryConfig.Get().GetLevelConfig(lvl);
        if (!levelDef) return true;

        // Проверяем индивидуальный лимит
        DP_ItemLimit customLimit = DP_TerritoryConfig.Get().GetCustomLimit(itemClassname, lvl);
        
        // ВСЕ переменные объявляем ОДИН РАЗ в начале метода
        float currentR = flag.GetCurrentRadius();
        array<Object> objects = new array<Object>;
        array<CargoBase> proxyCargos = new array<CargoBase>;
        int currentCount = 0;
        float totalBonusPercent = 0.0;
        int bonusSlots = 0;  // ОБЪЯВЛЯЕМ ЗДЕСЬ
        int maxLimit = 0;    // ОБЪЯВЛЯЕМ ЗДЕСЬ
        int i;
        Object obj;
        string limitMessage = "";
        int category = 0;
        int baseLimit = 999999;
        
        GetGame().GetObjectsAtPosition(flag.GetPosition(), currentR, objects, proxyCargos);
        
        if (customLimit)
        {
            // Предмет имеет индивидуальный лимит
            currentCount = 0;
            
            // Считаем только этот конкретный предмет
            for (i = 0; i < objects.Count(); i++)
            {
                obj = objects.Get(i);
                if (!obj) continue;
                if (!IsPlacedObject(obj)) continue;
                
                if (GetGame().IsKindOf(obj.GetType(), customLimit.ClassName))
                {
                    currentCount++;
                }
            }
            
            maxLimit = customLimit.MaxCount;
            limitMessage = "";
            
            // Проверяем, нужно ли применять бонусы
            if (customLimit.ApplyPerkBonus)
            {
                // Применяем бонусы от перков
                totalBonusPercent = flag.GetTotalForemanBonus();
                bonusSlots = Math.Floor(customLimit.MaxCount * totalBonusPercent / 100.0);
                maxLimit = customLimit.MaxCount + bonusSlots;
                limitMessage = "⛔ Лимит \"" + customLimit.DisplayName + "\" (" + lvl + " ур): " + maxLimit + " [База: " + customLimit.MaxCount + " +" + (int)totalBonusPercent + "%]";
            }
            else
            {
                // Жесткий лимит без бонусов
                maxLimit = customLimit.MaxCount;
                limitMessage = "⛔ Лимит \"" + customLimit.DisplayName + "\" (" + lvl + " ур): " + maxLimit + " [Фикс. лимит]";
            }
            
            if ((currentCount + 1) > maxLimit)
            {
                player.MessageImportant(limitMessage);
                return false;
            }
            
            return true;
        }
        
        // Если нет индивидуального лимита, используем категорийную проверку
        category = DP_TerritoryConfig.Get().GetItemCategory(itemClassname);
        if (category == 0) return true; 

        currentCount = 0;
        
        if (category == 1) baseLimit = levelDef.MaxStructures;
        if (category == 2) baseLimit = levelDef.MaxTents;
        if (category == 3) baseLimit = levelDef.MaxFurniture;

        // Получаем суммарный бонус всех участников в процентах
        totalBonusPercent = flag.GetTotalForemanBonus();
        
        // Вычисляем итоговый лимит: базовый + (базовый * процент / 100)
        bonusSlots = Math.Floor(baseLimit * totalBonusPercent / 100.0);
        maxLimit = baseLimit + bonusSlots;

        for (i = 0; i < objects.Count(); i++)
        {
            obj = objects.Get(i);
            if (!obj) continue;
            if (!IsPlacedObject(obj)) continue;
            if (DP_TerritoryConfig.Get().GetItemCategory(obj.GetType()) == category) currentCount++;
        }
        
        if ((currentCount + 1) > maxLimit)
        {
            player.MessageImportant("⛔ Лимит (" + lvl + " ур): " + maxLimit + " [База: " + baseLimit + " +" + (int)totalBonusPercent + "%]");
            return false;
        }
        return true;
    }

    bool TM_RegisterOwner_Unique(string ownerId, Object flagObj)
    {
        if (!ownerId || ownerId == "" || !flagObj) 
        {
            Print("[DP_Territory ERROR] Invalid parameters for TM_RegisterOwner_Unique");
            return false;
        }
        
        TM_UnregisterByOwnerId(ownerId);
        TM_TerritoryOwnership to = new TM_TerritoryOwnership();
        to.ownerId = ownerId;
        to.flagObj = flagObj;
        m_Ownerships.Insert(to);
        
        Print(string.Format("[DP_Territory] Registered territory for owner %1", ownerId));
        return true;
    }

    void TM_UnregisterByOwnerId(string ownerId)
    {
        if (!ownerId || ownerId == "") 
        {
            Print("[DP_Territory WARNING] Empty ownerId in TM_UnregisterByOwnerId");
            return;
        }
        
        int removedCount = 0;
        for (int i = m_Ownerships.Count() - 1; i >= 0; i--)
        {
            TM_TerritoryOwnership to = m_Ownerships.Get(i);
            if (to && to.ownerId == ownerId) 
            {
                m_Ownerships.RemoveOrdered(i);
                removedCount++;
            }
        }
        
        // Always log the result for debugging
        if (removedCount > 0)
        {
            Print(string.Format("[DP_Territory] Unregistered %1 territory(ies) for owner %2", removedCount, ownerId));
        }
        else
        {
            Print(string.Format("[DP_Territory] No territories found for owner %1", ownerId));
        }
    }
    
    bool TM_CanBuildOrDismantle(PlayerBase player)
    {
        if (!player) return false;
        float searchR = DP_TerritoryConfig.Get().GetMaxPossibleRadius();
        Object flagObj = TM_GetNearestFlag(player.GetPosition(), searchR); 
        if (!flagObj) return true; 
        
        TerritoryFlag flag = TerritoryFlag.Cast(flagObj);
        if (flag)
        {
            string uid = player.GetIdentity().GetId();
            if (flag.HasPermissions(uid)) return true;
            return false; 
        }
        return true;
    }
}