// Territory Logging System
class DP_TerritoryLogger
{
    static const string LOG_DIR = "$profile:DarkProjectConfig/TerritoryMod/Logs";
    static const string LOG_FILE = "$profile:DarkProjectConfig/TerritoryMod/Logs/territory_log.txt";
    
    static void Init()
    {
        if (!FileExist("$profile:DarkProjectConfig"))
        {
            MakeDirectory("$profile:DarkProjectConfig");
        }
        
        if (!FileExist("$profile:DarkProjectConfig/TerritoryMod"))
        {
            MakeDirectory("$profile:DarkProjectConfig/TerritoryMod");
        }
        
        if (!FileExist(LOG_DIR))
        {
            MakeDirectory(LOG_DIR);
        }
    }
    
    static void Log(string actionType, string message)
    {
        Init();
        
        FileHandle file = OpenFile(LOG_FILE, FileMode.APPEND);
        if (file)
        {
            int year, month, day, hour, minute, second;
            GetYearMonthDay(year, month, day);
            GetHourMinuteSecond(hour, minute, second);
            
            string timestamp = string.Format("[%1-%2-%3 %4:%5:%6]", 
                year, 
                month.ToStringLen(2), 
                day.ToStringLen(2), 
                hour.ToStringLen(2), 
                minute.ToStringLen(2), 
                second.ToStringLen(2)
            );
            
            string logEntry = string.Format("%1 %2 | %3\n", timestamp, actionType, message);
            FPrintln(file, logEntry);
            CloseFile(file);
        }
    }
    
    static void LogClaim(string playerID, string playerName, vector territoryPos, int level)
    {
        string msg = string.Format("Player: %1 (%2) | Territory Pos: <%3, %4, %5> | Level: %6", 
            playerName, playerID, (int)territoryPos[0], (int)territoryPos[1], (int)territoryPos[2], level);
        Log("CLAIM_TERRITORY", msg);
    }
    
    static void LogUpgrade(string playerID, string playerName, vector territoryPos, int oldLevel, int newLevel)
    {
        string msg = string.Format("Player: %1 (%2) | Territory Pos: <%3, %4, %5> | Level: %6 -> %7", 
            playerName, playerID, (int)territoryPos[0], (int)territoryPos[1], (int)territoryPos[2], oldLevel, newLevel);
        Log("UPGRADE_TERRITORY", msg);
    }
    
    static void LogAddMember(string ownerID, string ownerName, string targetID, int role, vector territoryPos)
    {
        string msg = string.Format("Owner: %1 (%2) | Target: %3 | Role: %4 | Territory Pos: <%5, %6, %7>", 
            ownerName, ownerID, targetID, DP_TerritoryRole.GetRoleName(role), (int)territoryPos[0], (int)territoryPos[1], (int)territoryPos[2]);
        Log("ADD_MEMBER", msg);
    }
    
    static void LogRemoveMember(string ownerID, string ownerName, string targetID, vector territoryPos)
    {
        string msg = string.Format("Owner: %1 (%2) | Target: %3 | Territory Pos: <%4, %5, %6>", 
            ownerName, ownerID, targetID, (int)territoryPos[0], (int)territoryPos[1], (int)territoryPos[2]);
        Log("REMOVE_MEMBER", msg);
    }
    
    static void LogChangeRole(string executorID, string executorName, string targetID, int oldRole, int newRole, vector territoryPos)
    {
        string msg = string.Format("Executor: %1 (%2) | Target: %3 | Role: %4 -> %5 | Territory Pos: <%6, %7, %8>", 
            executorName, executorID, targetID, DP_TerritoryRole.GetRoleName(oldRole), DP_TerritoryRole.GetRoleName(newRole), 
            (int)territoryPos[0], (int)territoryPos[1], (int)territoryPos[2]);
        Log("CHANGE_ROLE", msg);
    }
    
    static void LogDelete(string playerID, string playerName, vector territoryPos)
    {
        string msg = string.Format("Player: %1 (%2) | Territory Pos: <%3, %4, %5>", 
            playerName, playerID, (int)territoryPos[0], (int)territoryPos[1], (int)territoryPos[2]);
        Log("DELETE_TERRITORY", msg);
    }
    
    static void LogCreateSubzone(string ownerID, string ownerName, string renterID, vector subzonePos, float radius, int days, vector territoryPos)
    {
        string msg = string.Format("Owner: %1 (%2) | Renter: %3 | Subzone Pos: <%4, %5, %6> | Radius: %7 | Days: %8 | Territory Pos: <%9, %10, %11>", 
            ownerName, ownerID, renterID, (int)subzonePos[0], (int)subzonePos[1], (int)subzonePos[2], radius, days,
            (int)territoryPos[0], (int)territoryPos[1], (int)territoryPos[2]);
        Log("CREATE_SUBZONE", msg);
    }
    
    static void LogDeleteSubzone(string executorID, string executorName, string renterID, vector territoryPos)
    {
        string msg = string.Format("Executor: %1 (%2) | Renter: %3 | Territory Pos: <%4, %5, %6>", 
            executorName, executorID, renterID, (int)territoryPos[0], (int)territoryPos[1], (int)territoryPos[2]);
        Log("DELETE_SUBZONE", msg);
    }
    
    static void LogSecurity(string message)
    {
        Log("SECURITY", message);
    }
}
