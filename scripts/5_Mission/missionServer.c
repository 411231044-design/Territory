modded class MissionServer
{
    ref DP_TerritoryCache m_TerritoryCache;
    ref DP_TerritoryBonusManager m_BonusManager;
    ref DP_TerritoryMaintenance m_Maintenance;
    
    override void OnInit()
    {
        super.OnInit();
        
        Print("[DP_Territory] ------------------------------------------------");
        Print("[DP_Territory] ЗАГРУЗКА КОНФИГУРАЦИИ ТЕРРИТОРИИ...");
        DP_TerritoryConfig.Get(); 
        Print("[DP_Territory] Конфиг загружен успешно.");
        
        // Initialize new systems
        m_TerritoryCache = new DP_TerritoryCache();
        Print("[DP_Territory] Cache system initialized.");
        
        m_BonusManager = new DP_TerritoryBonusManager();
        Print("[DP_Territory] Bonus manager initialized.");
        
        m_Maintenance = new DP_TerritoryMaintenance();
        Print("[DP_Territory] Maintenance system initialized.");
        
        Print("[DP_Territory] ------------------------------------------------");
    }
    
    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        
        if (m_TerritoryCache)
        {
            m_TerritoryCache.OnUpdate(timeslice);
        }
        
        if (m_BonusManager)
        {
            m_BonusManager.OnUpdate(timeslice);
        }
        
        if (m_Maintenance)
        {
            m_Maintenance.OnUpdate(timeslice);
        }
    }
    
    void ~MissionServer()
    {
        // Cleanup systems
        if (m_TerritoryCache)
        {
            Print("[DP_Territory] Cleaning up cache");
            delete m_TerritoryCache;
            m_TerritoryCache = null;
        }
        
        if (m_BonusManager)
        {
            Print("[DP_Territory] Cleaning up bonus manager");
            delete m_BonusManager;
            m_BonusManager = null;
        }
        
        if (m_Maintenance)
        {
            Print("[DP_Territory] Cleaning up maintenance");
            delete m_Maintenance;
            m_Maintenance = null;
        }
    }
}