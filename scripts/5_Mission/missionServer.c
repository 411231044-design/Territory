modded class MissionServer
{
    ref DP_DecayManager m_DecayManager;
    
    override void OnInit()
    {
        super.OnInit();
        
        Print("[DP_Territory] ------------------------------------------------");
        Print("[DP_Territory] ЗАГРУЗКА КОНФИГУРАЦИИ ТЕРРИТОРИИ...");
        DP_TerritoryConfig.Get(); 
        Print("[DP_Territory] Конфиг загружен успешно.");
        
        // Initialize decay manager
        m_DecayManager = new DP_DecayManager();
        Print("[DP_Territory] Менеджер гниения инициализирован.");
        Print("[DP_Territory] ------------------------------------------------");
    }
    
    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        
        if (m_DecayManager)
        {
            m_DecayManager.OnUpdate(timeslice);
        }
    }
    
    void ~MissionServer()
    {
        // Cleanup decay manager
        if (m_DecayManager)
        {
            Print("[DP_Territory] Cleaning up decay manager");
            delete m_DecayManager;
            m_DecayManager = null;
        }
    }
}