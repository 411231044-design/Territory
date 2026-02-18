// Territory Role System
class DP_TerritoryRole
{
    static const int OWNER = 0;
    static const int ADMIN = 1;
    static const int BUILDER = 2;
    static const int GUEST = 3;
    
    static string GetRoleName(int role)
    {
        switch (role)
        {
            case OWNER: return "OWNER";
            case ADMIN: return "ADMIN";
            case BUILDER: return "BUILDER";
            case GUEST: return "GUEST";
        }
        return "UNKNOWN";
    }
    
    static bool CanBuild(int role)
    {
        return (role == OWNER || role == ADMIN || role == BUILDER);
    }
    
    static bool CanManageMembers(int role)
    {
        return (role == OWNER || role == ADMIN);
    }
    
    static bool CanDeleteTerritory(int role)
    {
        return (role == OWNER);
    }
    
    static bool CanChangeOwner(int role)
    {
        return (role == OWNER);
    }
    
    static bool CanManageSubzones(int role)
    {
        return (role == OWNER || role == ADMIN);
    }
}

class DP_TerritoryMember
{
    string PlayerID;
    int Role;
    int JoinedTimestamp;
    int LastSeenTimestamp;
    
    void DP_TerritoryMember(string id = "", int role = DP_TerritoryRole.BUILDER)
    {
        PlayerID = id;
        Role = role;
        JoinedTimestamp = GetGame().GetTime();
        LastSeenTimestamp = GetGame().GetTime();
    }
    
    void UpdateLastSeen()
    {
        LastSeenTimestamp = GetGame().GetTime();
    }
}
