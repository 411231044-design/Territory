// Territory Maintenance System
class DP_TerritoryMaintenance
{
    const float CHECK_INTERVAL = 3600.0; // 1 hour
    const int INACTIVITY_WARNING_DAYS = 30;
    const int INACTIVITY_DELETE_DAYS = 60;
    
    float m_Timer;
    
    void DP_TerritoryMaintenance()
    {
        m_Timer = 0;
    }
    
    void OnUpdate(float timeslice)
    {
        m_Timer += timeslice;
        
        if (m_Timer >= CHECK_INTERVAL)
        {
            RunMaintenanceCheck();
            m_Timer = 0;
        }
    }
    
    void RunMaintenanceCheck()
    {
        Print("[DP_Maintenance] Starting maintenance cycle...");
        
        DP_TerritoryManager tm = DP_TerritoryManager.TM_GetInstance();
        if (!tm)
        {
            Print("[DP_Maintenance] Territory manager not initialized");
            return;
        }
        
        array<ref TM_TerritoryOwnership> ownerships = tm.GetOwnerships();
        if (!ownerships)
        {
            Print("[DP_Maintenance] Ownerships array not initialized");
            return;
        }
        
        int territoriesChecked = 0;
        int territoriesDeleted = 0;
        int ownershipsTransferred = 0;
        
        // Create a copy to iterate over (to avoid modification during iteration)
        array<TerritoryFlag> flagsToCheck = new array<TerritoryFlag>();
        
        for (int i = 0; i < ownerships.Count(); i++)
        {
            TM_TerritoryOwnership ownership = ownerships.Get(i);
            if (!ownership || !ownership.flagObj) continue;
            
            TerritoryFlag flag = TerritoryFlag.Cast(ownership.flagObj);
            if (!flag) continue;
            
            flagsToCheck.Insert(flag);
        }
        
        // Process each territory
        for (int j = 0; j < flagsToCheck.Count(); j++)
        {
            TerritoryFlag flag = flagsToCheck.Get(j);
            if (!flag) continue;
            
            territoriesChecked++;
            
            // Check owner validity
            if (!CheckOwnerExists(flag))
            {
                // Owner doesn't exist, transfer to senior member
                if (TransferOwnershipToMember(flag))
                {
                    ownershipsTransferred++;
                    DP_TerritoryLogger.Log("MAINTENANCE", string.Format("Transferred ownership for territory at <%1, %2, %3> - owner account deleted", 
                        (int)flag.GetPosition()[0], (int)flag.GetPosition()[1], (int)flag.GetPosition()[2]));
                }
                else
                {
                    // No members to transfer to, delete territory
                    DeleteTerritory(flag);
                    territoriesDeleted++;
                    DP_TerritoryLogger.Log("MAINTENANCE", string.Format("Deleted territory at <%1, %2, %3> - owner account deleted, no members", 
                        (int)flag.GetPosition()[0], (int)flag.GetPosition()[1], (int)flag.GetPosition()[2]));
                    continue;
                }
            }
            
            // Check inactivity
            int daysInactive = GetDaysInactive(flag);
            
            if (daysInactive >= INACTIVITY_DELETE_DAYS)
            {
                DeleteTerritory(flag);
                territoriesDeleted++;
                DP_TerritoryLogger.Log("MAINTENANCE", string.Format("Deleted territory at <%1, %2, %3> - inactive for %4 days", 
                    (int)flag.GetPosition()[0], (int)flag.GetPosition()[1], (int)flag.GetPosition()[2], daysInactive));
            }
            else if (daysInactive >= INACTIVITY_WARNING_DAYS)
            {
                DP_TerritoryLogger.Log("MAINTENANCE", string.Format("Warning: Territory at <%1, %2, %3> inactive for %4 days (will be deleted at %5 days)", 
                    (int)flag.GetPosition()[0], (int)flag.GetPosition()[1], (int)flag.GetPosition()[2], daysInactive, INACTIVITY_DELETE_DAYS));
            }
            
            // Cleanup expired subzones
            CleanupExpiredSubzones(flag);
        }
        
        Print(string.Format("[DP_Maintenance] Maintenance complete. Checked: %1, Deleted: %2, Transferred: %3", 
            territoriesChecked, territoriesDeleted, ownershipsTransferred));
    }
    
    bool CheckOwnerExists(TerritoryFlag flag)
    {
        if (!flag) return false;
        
        string ownerID = flag.GetOwnerID();
        if (!ownerID || ownerID == "") return false;
        
        // In a real implementation, this would check against a database
        // For now, we assume the owner exists if they have a valid ID
        // This is a placeholder for future implementation
        return true;
    }
    
    int GetDaysInactive(TerritoryFlag flag)
    {
        if (!flag) return 999;
        
        // Check if any member has been active recently
        // This is a simplified version - in production, you'd track last seen timestamps
        // For now, we return 0 (always active) as placeholder
        // Future implementation should check against player activity database
        return 0;
    }
    
    bool TransferOwnershipToMember(TerritoryFlag flag)
    {
        if (!flag) return false;
        
        array<ref DP_TerritoryMember> members = flag.GetMembers();
        if (!members || members.Count() == 0) return false;
        
        // Find the member with admin role or the oldest member
        DP_TerritoryMember newOwner = null;
        
        // First try to find an admin
        for (int i = 0; i < members.Count(); i++)
        {
            DP_TerritoryMember member = members.Get(i);
            if (member && member.Role == DP_TerritoryRole.ADMIN)
            {
                newOwner = member;
                break;
            }
        }
        
        // If no admin, take the first member
        if (!newOwner && members.Count() > 0)
        {
            newOwner = members.Get(0);
        }
        
        if (!newOwner) return false;
        
        // Transfer ownership
        flag.TransferOwnership(newOwner.PlayerID);
        
        return true;
    }
    
    void DeleteTerritory(TerritoryFlag flag)
    {
        if (!flag) return;
        
        string ownerID = flag.GetOwnerID();
        DP_TerritoryManager.TM_GetInstance().TM_UnregisterByOwnerId(ownerID);
        GetGame().ObjectDelete(flag);
    }
    
    void CleanupExpiredSubzones(TerritoryFlag flag)
    {
        if (!flag) return;
        
        array<ref DP_TerritorySubzone> subzones = flag.GetSubzones();
        if (!subzones) return;
        
        int removedCount = 0;
        
        for (int i = subzones.Count() - 1; i >= 0; i--)
        {
            DP_TerritorySubzone subzone = subzones.Get(i);
            if (subzone && subzone.IsExpired())
            {
                subzones.Remove(i);
                removedCount++;
                DP_TerritoryLogger.Log("MAINTENANCE", string.Format("Removed expired subzone for renter %1 at territory <%2, %3, %4>", 
                    subzone.RenterID, (int)flag.GetPosition()[0], (int)flag.GetPosition()[1], (int)flag.GetPosition()[2]));
            }
        }
        
        if (removedCount > 0)
        {
            flag.SetSynchDirty();
        }
    }
}
