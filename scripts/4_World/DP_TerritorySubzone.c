// Territory Subzone System
class DP_TerritorySubzone
{
    string RenterID;
    vector Position;
    float Radius;
    int ExpiryTimestamp;
    int DailyPrice;
    int CreatedTimestamp;
    
    void DP_TerritorySubzone(string renterID = "", vector pos = "0 0 0", float radius = 15.0, int durationDays = 7, int price = 0)
    {
        RenterID = renterID;
        Position = pos;
        Radius = radius;
        DailyPrice = price;
        CreatedTimestamp = GetGame().GetTime();
        ExpiryTimestamp = CreatedTimestamp + (durationDays * 86400); // 86400 seconds per day
    }
    
    bool IsExpired()
    {
        return GetGame().GetTime() >= ExpiryTimestamp;
    }
    
    bool IsPlayerInside(vector playerPos)
    {
        float distSq = vector.DistanceSq(playerPos, Position);
        return distSq <= (Radius * Radius);
    }
    
    void ExtendDuration(int days)
    {
        ExpiryTimestamp += (days * 86400);
    }
    
    int GetDaysRemaining()
    {
        int currentTime = GetGame().GetTime();
        if (currentTime >= ExpiryTimestamp) return 0;
        return Math.Ceil((ExpiryTimestamp - currentTime) / 86400.0);
    }
}
