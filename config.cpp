class CfgPatches
{
    class DP_TerritoryMod
    {
        units[] = {"TerritoryFlag"};
        weapons[] = {};
        requiredVersion = 1.0;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Camping", "TerjeCore", "TerjeSkills"};
    };
};

class CfgMods
{
    class DP_TerritoryMod
    {
        dir = "DP_TerritoryMod";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "DP_TerritoryMod";
        author = "DP";
        version = "1.0";
        type = "mod";
        dependencies[] = {"Game", "World", "Mission"};
        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"DP_TerritoryMod/scripts/3_Game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"DP_TerritoryMod/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"DP_TerritoryMod/scripts/5_Mission"};
            };
        };
    };
};

class CfgVehicles
{
	class BaseBuildingBase; 
	class TerritoryFlag: BaseBuildingBase 
	{	
		class Cargo
		{
			itemsCargoSize[] = {10,100};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
	};
};

class CfgTerjeSkills
{
    class Skill_Architect
    {
        id = "Skill_Architect";
        enabled = 1;
        displayName = "Архитектор";
        description = "Навык строительства и управления базой.";
        
        // Главная иконка навыка
        icon = "DP_TerritoryMod/gui/images/architect_icon_ca.paa"; 
        
        perkPointsPerLevel = 1;
        expLoseOnDeath = 0;
        levels[] = {
            500, 1500, 3000, 5000, 8000, 12000, 17000, 23000, 30000, 40000, 
            55000, 75000, 100000, 130000, 165000, 205000, 250000, 300000, 360000, 430000, 
            500000, 600000, 750000, 1000000, 1500000
        };

        class Perks
        {
            // 1. Главный Архитектор
            class Perk_GrandArchitect
            {
                id = "Perk_GrandArchitect";
                enabled = 1;
                displayName = "Главный Архитектор";
                description = "Доступ к улучшению территории.";
                
                stagesCount = 2;
                
                disabledIcon = "DP_TerritoryMod/gui/images/headacrhitect_icon_ca.paa";
                enabledIcon = "DP_TerritoryMod/gui/images/headacrhitectActive_icon_ca.paa";
                
                requiredSkillLevels[] = {5, 15};
                requiredPerkPoints[] = {1, 1};
                values[] = {2.0, 3.0}; 
            };

            // 2. Гостеприимство
            class Perk_Hospitality
            {
                id = "Perk_Hospitality";
                enabled = 1;
                displayName = "Гостеприимство";
                description = "Увеличивает лимит друзей на территории.";
                
                stagesCount = 4;
                
                disabledIcon = "DP_TerritoryMod/gui/images/guest_icon_ca.paa";
                enabledIcon = "DP_TerritoryMod/gui/images/guestActive_icon_ca.paa";
                
                requiredSkillLevels[] = {2, 6, 10, 20};
                requiredPerkPoints[] = {1, 1, 1, 1};
                values[] = {1.0, 2.0, 3.0, 5.0};
            };

            // 3. Экономный
            class Perk_ResourcefulPlanner
            {
                id = "Perk_ResourcefulPlanner";
                enabled = 1;
                displayName = "Экономный";
                description = "Снижает стоимость улучшения территории (%).";
                
                stagesCount = 3;
                
                disabledIcon = "DP_TerritoryMod/gui/images/economy_icon_ca.paa";
                enabledIcon = "DP_TerritoryMod/gui/images/economyActive_icon_ca.paa";
                
                requiredSkillLevels[] = {3, 12, 25};
                requiredPerkPoints[] = {1, 1, 1};
                values[] = {10.0, 20.0, 30.0};
            };

            // 4. Прораб
            class Perk_Foreman
            {
                id = "Perk_Foreman";
                enabled = 1;
                displayName = "Прораб";
                description = "Увеличивает лимит построек и мебели.";
                
                stagesCount = 3;
                
                disabledIcon = "DP_TerritoryMod/gui/images/prorab_icon_ca.paa";
                enabledIcon = "DP_TerritoryMod/gui/images/prorabActive_icon_ca.paa";
                
                requiredSkillLevels[] = {5, 15, 30};
                requiredPerkPoints[] = {1, 1, 1};
                values[] = {100.0, 200.0, 300.0};
            };
            
            // 5. Консервант
            class Perk_Preservation
            {
                id = "Perk_Preservation";
                enabled = 1;
                displayName = "Консервант";
                description = "Замедляет гниение базы.";
                
                stagesCount = 3;
                
                disabledIcon = "DP_TerritoryMod/gui/images/conservator_icon_ca.paa";
                enabledIcon = "DP_TerritoryMod/gui/images/conservatorActive_icon_ca.paa";
                
                requiredSkillLevels[] = {8, 18, 28};
                requiredPerkPoints[] = {1, 1, 1};
                values[] = {1.0, 2.0, 3.0};
            };

            // 6. Реставратор
            class Perk_Restorer
            {
                id = "Perk_Restorer";
                enabled = 1;
                displayName = "Реставратор";
                description = "Шанс сберечь инструмент при ремонте.";
                
                stagesCount = 2;
                
                disabledIcon = "DP_TerritoryMod/gui/images/rest_icon_ca.paa";
                enabledIcon = "DP_TerritoryMod/gui/images/restActive_icon_ca.paa";
                
                requiredSkillLevels[] = {4, 14};
                requiredPerkPoints[] = {1, 1};
                values[] = {1.0, 2.0};
            };
        };
    };
};