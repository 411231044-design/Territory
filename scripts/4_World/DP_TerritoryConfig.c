class DP_CostItem
{
    string ClassName;
    string DisplayName;
    int Count;
    void DP_CostItem(string name, string disp, int c) { ClassName = name; DisplayName = disp; Count = c; }
}

// ОБНОВЛЕННЫЙ КЛАСС для индивидуальных лимитов предметов
class DP_ItemLimit
{
    string ClassName;      // Название класса предмета (например, "KnittingTable")
    string DisplayName;    // Отображаемое название (например, "Вязальный станок")
    int MaxCount;          // Максимальное количество
    bool ApplyPerkBonus;   // НОВОЕ ПОЛЕ: Применять ли бонус от перков (true/false)
    
    void DP_ItemLimit(string cls, string disp, int max, bool applyBonus = false)
    {
        ClassName = cls;
        DisplayName = disp;
        MaxCount = max;
        ApplyPerkBonus = applyBonus;  // По умолчанию бонусы НЕ применяются
    }
}

class DP_LevelDefinition
{
    int Level;
    float Radius;
    int MaxStructures;
    int MaxTents;
    int MaxFurniture;
    ref array<ref DP_CostItem> UpgradeCost;
    ref array<ref DP_ItemLimit> CustomLimits;

    void DP_LevelDefinition(int l, float r, int ms, int mt, int mf)
    {
        Level = l; Radius = r; MaxStructures = ms; MaxTents = mt; MaxFurniture = mf;
        UpgradeCost = new array<ref DP_CostItem>;
        CustomLimits = new array<ref DP_ItemLimit>;
    }
}

class DP_TerritoryConfig
{
    ref array<string> StructureItems; 
    ref array<string> TentItems;      
    ref array<string> FurnitureItems; 
    ref array<ref DP_LevelDefinition> Levels;

    [NonSerialized()]
    static const string CONFIG_ROOT = "$profile:DarkProjectConfig";
    [NonSerialized()]
    static const string CONFIG_DIR  = "$profile:DarkProjectConfig/TerritoryMod";
    [NonSerialized()]
    static const string CONFIG_PATH = "$profile:DarkProjectConfig/TerritoryMod/DP_Territory_Config.json";
    
    [NonSerialized()]
    static ref DP_TerritoryConfig s_Instance;

    void DP_TerritoryConfig()
    {
        StructureItems = new array<string>;
        TentItems = new array<string>;
        FurnitureItems = new array<string>;
        Levels = new array<ref DP_LevelDefinition>;
    }
    
    static DP_TerritoryConfig Get()
    {
        if (!s_Instance) { s_Instance = new DP_TerritoryConfig(); s_Instance.Load(); }
        return s_Instance;
    }

    void Load()
    {
        if (!FileExist(CONFIG_ROOT)) MakeDirectory(CONFIG_ROOT);
        if (!FileExist(CONFIG_DIR)) MakeDirectory(CONFIG_DIR);
        if (FileExist(CONFIG_PATH)) { JsonFileLoader<DP_TerritoryConfig>.JsonLoadFile(CONFIG_PATH, this); }
        else { CreateDefault(); Save(); }
    }

    void Save()
    {
        if (!FileExist(CONFIG_ROOT)) MakeDirectory(CONFIG_ROOT);
        if (!FileExist(CONFIG_DIR)) MakeDirectory(CONFIG_DIR);
        JsonFileLoader<DP_TerritoryConfig>.JsonSaveFile(CONFIG_PATH, this);
    }

    void CreateDefault()
    {
        // Structure items (walls, watchtowers, etc.)
        StructureItems.Insert("Fence"); 
        StructureItems.Insert("Watchtower"); 
        StructureItems.Insert("FenceKit"); 
        StructureItems.Insert("WatchtowerKit");
        
        // Tent items
        TentItems.Insert("MediumTent"); 
        TentItems.Insert("LargeTent"); 
        TentItems.Insert("CarTent"); 
        TentItems.Insert("PartyTent");
        
        // Furniture items (storage containers)
        FurnitureItems.Insert("Barrel_ColorBase"); 
        FurnitureItems.Insert("SeaChest"); 
        FurnitureItems.Insert("WoodenCrate"); 
        FurnitureItems.Insert("BarrelHoles_ColorBase");
        
        // Level 1: Starter base
        DP_LevelDefinition l1 = new DP_LevelDefinition(1, 50.0, 10, 1, 5);
        // Индивидуальные лимиты для уровня 1
        // false = бонусы от перков НЕ применяются (жесткий лимит)
        l1.CustomLimits.Insert(new DP_ItemLimit("TanningSt", "Вязальный станок", 1, false));
        l1.CustomLimits.Insert(new DP_ItemLimit("Fireplace", "Костер", 3, false));
        // true = бонусы от перков ПРИМЕНЯЮТСЯ
        l1.CustomLimits.Insert(new DP_ItemLimit("GardenPlot", "Грядка", 5, true));
        Levels.Insert(l1);

        // Level 2: Small base
        DP_LevelDefinition l2 = new DP_LevelDefinition(2, 100.0, 20, 2, 10);
        l2.UpgradeCost.Insert(new DP_CostItem("Nail", "Гвозди (шт)", 50)); 
        l2.UpgradeCost.Insert(new DP_CostItem("WoodenPlank", "Доски", 20));
        // Индивидуальные лимиты для уровня 2
        l2.CustomLimits.Insert(new DP_ItemLimit("TanningSt", "Вязальный станок", 2, false));
        l2.CustomLimits.Insert(new DP_ItemLimit("Fireplace", "Костер", 5, false));
        l2.CustomLimits.Insert(new DP_ItemLimit("GardenPlot", "Грядка", 10, true));
        l2.CustomLimits.Insert(new DP_ItemLimit("BarrelHoles_ColorBase", "Бочка", 10, true));
        Levels.Insert(l2);
        
        // Level 3: Large base
        DP_LevelDefinition l3 = new DP_LevelDefinition(3, 150.0, 40, 5, 20);
        l3.UpgradeCost.Insert(new DP_CostItem("Nail", "Гвозди (шт)", 99)); 
        l3.UpgradeCost.Insert(new DP_CostItem("MetalPlate", "Листы железа", 10)); 
        l3.UpgradeCost.Insert(new DP_CostItem("WoodenLog", "Бревна", 5));
        // Индивидуальные лимиты для уровня 3
        l3.CustomLimits.Insert(new DP_ItemLimit("TanningSt", "Вязальный станок", 5, false));
        l3.CustomLimits.Insert(new DP_ItemLimit("Fireplace", "Костер", 10, false));
        l3.CustomLimits.Insert(new DP_ItemLimit("GardenPlot", "Грядка", 20, true));
        l3.CustomLimits.Insert(new DP_ItemLimit("BarrelHoles_ColorBase", "Бочка", 20, true));
        Levels.Insert(l3);
    }
    
    DP_LevelDefinition GetLevelConfig(int level)
    {
        if (level < 1)
        {
            Print(string.Format("[DP_Territory WARNING] Invalid level requested: %1, returning level 1", level));
            level = 1;
        }
        
        for (int i = 0; i < Levels.Count(); i++)
        {
            DP_LevelDefinition levelDef = Levels.Get(i);
            if (levelDef && levelDef.Level == level) 
            {
                return levelDef;
            }
        }
        
        if (Levels.Count() > 0) 
        {
            DP_LevelDefinition firstLevel = Levels.Get(0);
            if (firstLevel)
            {
                Print(string.Format("[DP_Territory WARNING] Level %1 not found, returning level 1", level));
                return firstLevel;
            }
        }
        
        Print("[DP_Territory ERROR] No levels defined in config!");
        return null;
    }
    
    float GetMaxPossibleRadius()
    {
        if (!Levels || Levels.Count() == 0)
        {
            Print("[DP_Territory WARNING] No levels defined, using default radius");
            return DP_TerritoryConstants.DEFAULT_RADIUS * DP_TerritoryConstants.DEFAULT_MAX_RADIUS_MULTIPLIER;
        }
        
        float maxR = 0;
        foreach(DP_LevelDefinition l : Levels) 
        { 
            if (l && l.Radius > maxR) 
            {
                maxR = l.Radius; 
            }
        }
        
        if (maxR == 0) 
        {
            Print("[DP_Territory WARNING] No valid radius in config, using default");
            return DP_TerritoryConstants.DEFAULT_RADIUS * DP_TerritoryConstants.DEFAULT_MAX_RADIUS_MULTIPLIER;
        }
        
        return maxR;
    }
    
    int GetItemCategory(string classname)
    {
        if (IsTypeInList(classname, StructureItems)) return 1;
        if (IsTypeInList(classname, TentItems)) return 2;
        if (IsTypeInList(classname, FurnitureItems)) return 3;
        return 0;
    }
    
    DP_ItemLimit GetCustomLimit(string classname, int territoryLevel)
    {
        DP_LevelDefinition levelDef = GetLevelConfig(territoryLevel);
        if (!levelDef || !levelDef.CustomLimits) return null;
        
        foreach(DP_ItemLimit limit : levelDef.CustomLimits)
        {
            if (limit && GetGame().IsKindOf(classname, limit.ClassName))
            {
                return limit;
            }
        }
        
        return null;
    }
    
    bool IsTypeInList(string cls, array<string> list)
    {
        if (!list || !cls || cls == "") return false;
        
        foreach(string item : list) 
        { 
            if (GetGame().IsKindOf(cls, item)) 
            {
                return true; 
            }
        }
        
        return false;
    }
}