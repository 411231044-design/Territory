class DP_ManagementMenu : UIScriptedMenu
{
    protected TextWidget m_OwnerText; protected TextWidget m_LevelText; protected TextWidget m_UpgradeInfo; protected TextWidget m_LimitsInfo;
    protected TextListboxWidget m_MembersList; protected TextListboxWidget m_NearbyList;
    protected ButtonWidget m_BtnClose; protected ButtonWidget m_BtnAdd; protected ButtonWidget m_BtnKick; protected ButtonWidget m_BtnDelete; protected ButtonWidget m_BtnUpgrade;
    protected EditBoxWidget m_InputName;
    protected TerritoryFlag m_TargetFlag; protected float m_UpdateTimer; 

    override Widget Init()
    {
        m_UpdateTimer = 0.0; // Initialize update timer
        
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("DP_TerritoryMod/gui/layouts/dp_management.layout");
        if (!layoutRoot) return null;
        m_OwnerText = TextWidget.Cast(layoutRoot.FindAnyWidget("OwnerTextLabel"));
        m_LevelText = TextWidget.Cast(layoutRoot.FindAnyWidget("LevelTextLabel"));
        m_UpgradeInfo = TextWidget.Cast(layoutRoot.FindAnyWidget("UpgradeInfoText"));
        m_LimitsInfo = TextWidget.Cast(layoutRoot.FindAnyWidget("LimitsInfoText"));
        m_MembersList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("MembersList"));
        m_NearbyList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("NearbyPlayersList"));
        m_BtnClose = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ButtonClose"));
        m_BtnAdd = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ButtonAddMember"));
        m_BtnKick = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ButtonKickMember"));
        m_BtnDelete = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ButtonDeleteTerritory"));
        m_BtnUpgrade = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ButtonUpgrade"));
        m_InputName = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("InputPlayerName"));
        
        Object obj = DP_TerritoryManager.TM_GetInstance().m_ClientPendingTarget;
        if (obj) 
        { 
            m_TargetFlag = TerritoryFlag.Cast(obj); 
            m_TargetFlag.RequestSyncDataAll(); 
            UpdateInfo(); 
            UpdateNearbyPlayers(); 
        }
        
        return layoutRoot;
    }

    void UpdateInfo()
    {
        if (!m_TargetFlag) return;
        
        // Validate player and identity
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.GetIdentity())
        {
            Print("[DP_Territory ERROR] Player or identity is null in UpdateInfo");
            return;
        }
        
        string ownerID = m_TargetFlag.GetOwnerID(); 
        string myID = player.GetIdentity().GetId(); 
        int currentLvl = m_TargetFlag.GetTerritoryLevel();
        
        if (m_LevelText) 
        {
            m_LevelText.SetText("Уровень: " + currentLvl);
        }
        
        if (m_LimitsInfo) 
        {
            UpdateLimits(currentLvl);
        }
        
        bool isOwnedByVar = m_TargetFlag.HasOwner(); 
        
        // If owned but owner ID not yet synced
        if (isOwnedByVar && ownerID == "") 
        { 
            m_OwnerText.SetText("Владелец: [Загрузка...]"); 
            m_BtnAdd.Show(false); 
            m_BtnKick.Show(false); 
            m_BtnDelete.Show(false); 
            m_BtnUpgrade.Show(false); 
            return; 
        }
        
        bool amIOwner = (ownerID == myID); 
        bool isFree = (!isOwnedByVar);
        
        // Territory is free - show claim option
        if (isFree) 
        { 
            m_OwnerText.SetText("Владелец: Нет (Свободно)"); 
            m_BtnAdd.Show(true); 
            m_BtnAdd.SetText("ЗАХВАТИТЬ"); 
            m_BtnAdd.SetColor(0xFF00FF00); 
            m_BtnKick.Show(false); 
            m_BtnDelete.Show(false); 
            m_InputName.Show(false); 
            m_MembersList.Show(false); 
            m_NearbyList.Show(false); 
            
            if (m_BtnUpgrade) 
            {
                m_BtnUpgrade.Show(false); 
            }
            
            if (m_UpgradeInfo) 
            {
                m_UpgradeInfo.SetText(""); 
            }
            
            if (m_LimitsInfo) 
            {
                m_LimitsInfo.SetText("Захватите территорию,\nчтобы увидеть лимиты."); 
            }
        }
        // I am the owner - show full management
        else if (amIOwner) 
        { 
            m_OwnerText.SetText("Владелец: ВЫ"); 
            m_BtnAdd.Show(true); 
            m_BtnAdd.SetText("ДОБАВИТЬ"); 
            m_BtnAdd.SetColor(0xFFFFFFFF); 
            m_BtnKick.Show(true); 
            m_BtnDelete.Show(true); 
            m_InputName.Show(true); 
            m_MembersList.Show(true); 
            m_NearbyList.Show(true); 
            
            if (m_BtnUpgrade) 
            { 
                m_BtnUpgrade.Show(true); 
                UpdateUpgradeCost(currentLvl); 
            } 
        }
        // Someone else owns it - limited view
        else 
        { 
            m_OwnerText.SetText("Владелец: " + ownerID); 
            m_BtnAdd.Show(false); 
            m_BtnKick.Show(false); 
            m_BtnDelete.Show(false); 
            m_InputName.Show(false); 
            m_MembersList.Show(true); 
            m_NearbyList.Show(false); 
            
            if (m_BtnUpgrade) 
            {
                m_BtnUpgrade.Show(false); 
            }
            
            if (m_UpgradeInfo) 
            {
                m_UpgradeInfo.SetText("Доступ запрещен"); 
            }
        }
        
        // Update members list
        if (m_TargetFlag.m_Members) 
        { 
            m_MembersList.ClearItems(); 
            for (int i = 0; i < m_TargetFlag.m_Members.Count(); i++) 
            {
                DP_TerritoryMember member = m_TargetFlag.m_Members.Get(i);
                if (member)
                {
                    string displayText = member.PlayerID + " [" + DP_TerritoryRole.GetRoleName(member.Role) + "]";
                    m_MembersList.AddItem(displayText, NULL, 0); 
                }
            }
        }
    }

    void UpdateLimits(int lvl)
    {
        DP_LevelDefinition lvlDef = DP_TerritoryConfig.Get().GetLevelConfig(lvl);
        if (!lvlDef) return;

        int walls = 0; int tents = 0; int furn = 0;
        
        // Try to use cache first for performance
        DP_TerritoryCache cache = DP_TerritoryCache.GetInstance();
        if (cache)
        {
            DP_TerritoryCacheEntry entry = cache.GetCacheEntry(m_TargetFlag);
            if (entry)
            {
                walls = entry.WallCount;
                tents = entry.TentCount;
                furn = entry.FurnitureCount;
            }
        }
        else
        {
            // Fallback: Count manually if cache not available
            float currentR = m_TargetFlag.GetCurrentRadius();
            array<Object> objects = new array<Object>;
            array<CargoBase> proxyCargos = new array<CargoBase>;
            GetGame().GetObjectsAtPosition(m_TargetFlag.GetPosition(), currentR, objects, proxyCargos);

            foreach(Object obj : objects)
            {
                if (obj == m_TargetFlag) continue;
                if (!IsPlacedObjectMenu(obj)) continue;
                int cat = DP_TerritoryConfig.Get().GetItemCategory(obj.GetType());
                if (cat == 1) walls++; if (cat == 2) tents++; if (cat == 3) furn++;
            }
        }

        // НОВАЯ ЛОГИКА: Получаем суммарный бонус территории
        float totalBonusPercent = m_TargetFlag.GetTotalForemanBonus();
        
        int bonusStructures = Math.Floor(lvlDef.MaxStructures * totalBonusPercent / 100.0);
        int bonusTents = Math.Floor(lvlDef.MaxTents * totalBonusPercent / 100.0);
        int bonusFurniture = Math.Floor(lvlDef.MaxFurniture * totalBonusPercent / 100.0);

        string text = ""; 
        text += "СТРУКТУРЫ: " + walls + " / " + (lvlDef.MaxStructures + bonusStructures) + "\n"; 
        text += "(База: " + lvlDef.MaxStructures + " +" + (int)totalBonusPercent + "%)\n\n";
        
        text += "ПАЛАТКИ: " + tents + " / " + (lvlDef.MaxTents + bonusTents) + "\n"; 
        text += "(База: " + lvlDef.MaxTents + " +" + (int)totalBonusPercent + "%)\n\n";
        
        text += "МЕБЕЛЬ: " + furn + " / " + (lvlDef.MaxFurniture + bonusFurniture) + "\n";
        text += "(База: " + lvlDef.MaxFurniture + " +" + (int)totalBonusPercent + "%)\n\n";
        
        // Show active bonuses
        text += "=== БОНУСЫ ===\n";
        if (lvl >= 1)
        {
            text += "✓ Медленный голод/жажда\n";
        }
        if (lvl >= 2)
        {
            text += "✓ Регенерация +10%\n";
            text += "✓ Крафт +5%\n";
        }
        if (lvl >= 3)
        {
            text += "✓ Регенерация +15%\n";
            text += "✓ Крафт +10%\n";
            text += "✓ Невидимость от зомби\n";
        }
        
        // Show subzone info
        if (m_TargetFlag.m_Subzones && m_TargetFlag.m_Subzones.Count() > 0)
        {
            text += "\n=== СУБЗОНЫ ===\n";
            text += "Всего: " + m_TargetFlag.m_Subzones.Count() + "\n";
        }

        if (m_LimitsInfo) m_LimitsInfo.SetText(text);
    }
    
    bool IsPlacedObjectMenu(Object obj) 
    { 
        EntityAI entity = EntityAI.Cast(obj); 
        if (!entity) return false; 
        if (entity == m_TargetFlag) return false; 
        if (entity.IsHologram()) return false; 
        if (entity.GetHierarchyParent()) return false; // Если в руках или инвентаре - не считаем
        
        TentBase tent = TentBase.Cast(obj); 
        if (tent) 
        { 
            // 0 = PACKED (Свернута)
            // 1 = PITCHED (Установлена)
            // 2 = PACKED (Собрана в инвентарь?)
            
            // Если состояние 0 - это просто предмет на земле, не считаем.
            if (tent.GetState() == 0) return false; 
        } 
        
        return true; 
    }

    void UpdateUpgradeCost(int currentLvl)
    {
        if (!m_UpgradeInfo) return;
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        
        float allowedLvl = 0; float discountPercent = 0;
        
        if (player && player.GetTerjeSkills()) 
        {
            bool foundLvl = player.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_GrandArchitect", allowedLvl);
            player.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_ResourcefulPlanner", discountPercent);
        }

        array<ref DP_CostItem> nextCost = m_TargetFlag.GetNextLevelCostClient();
        if (!nextCost || nextCost.Count() == 0) { m_UpgradeInfo.SetText("Максимальный уровень"); m_BtnUpgrade.Show(false); return; }

        string info = "ДЛЯ УРОВНЯ " + (currentLvl + 1) + ":\n";
        bool skillOk = true;
        
        if ((currentLvl + 1) > 1 && allowedLvl < (currentLvl + 1 - 0.1)) 
        { 
            info += "⚠ Требуется перк 'Гл. Архитектор'\n\n"; 
            skillOk = false; 
        } 
        else info += "✔ Навык достаточен\n";
        
        if (discountPercent > 0) info += "(Ваша скидка: " + discountPercent + "%)\n\n"; else info += "\n";
        float discountMult = discountPercent / 100.0;
        bool allCollected = true;

        foreach(DP_CostItem req : nextCost)
        {
            int neededCount = Math.Round(req.Count * (1.0 - discountMult));
            if (neededCount < 1) neededCount = 1;
            int currentAmount = GetItemAmountInFlag(req.ClassName);
            string nameToShow = req.DisplayName; if (nameToShow == "") nameToShow = req.ClassName; 
            info += nameToShow + ": " + currentAmount + " / " + neededCount + "\n";
            if (currentAmount < neededCount) allCollected = false;
        }
        
        if (allCollected && skillOk) { m_UpgradeInfo.SetColor(0xFF00FF00); info += "\n[ГОТОВО К УЛУЧШЕНИЮ]"; m_BtnUpgrade.Show(true); }
        else { m_UpgradeInfo.SetColor(0xFFFF0000); if (!skillOk) m_BtnUpgrade.Show(false); else m_BtnUpgrade.Show(true); }
        m_UpgradeInfo.SetText(info);
    }
    
    int GetItemAmountInFlag(string classname) 
    { 
        if (!m_TargetFlag) return 0; 
        GameInventory inv = m_TargetFlag.GetInventory(); 
        if (!inv) return 0; 
        CargoBase cargo = inv.GetCargo(); 
        if (!cargo) return 0; 
        int count = 0; 
        int cCount = cargo.GetItemCount(); 
        for (int i = 0; i < cCount; i++) 
        { 
            EntityAI item = cargo.GetItem(i); 
            if (item && item.IsInherited(ItemBase)) 
            { 
                if (GetGame().IsKindOf(item.GetType(), classname)) 
                { 
                    ItemBase ib = ItemBase.Cast(item); 
                    count += ib.GetQuantity(); 
                    if (ib.GetQuantityMax() == 0) count++; 
                } 
            } 
        } 
        return count; 
    }

    override void OnShow() 
    { 
        super.OnShow(); 
        GetGame().GetUIManager().ShowCursor(true); 
        GetGame().GetInput().ChangeGameFocus(1); 
        GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_INVENTORY); 
        SetFocus(layoutRoot); 
        
        if (m_InputName) 
        {
            m_InputName.SetText(""); 
        }
    }
    
    override void OnHide() 
    { 
        super.OnHide(); 
        GetGame().GetUIManager().ShowCursor(false); 
        GetGame().GetInput().ResetGameFocus(); 
        GetGame().GetMission().PlayerControlEnable(true); 
    }
    
    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_BtnClose) 
        { 
            Close(); 
            return true; 
        }
        
        if (w == m_BtnDelete && m_TargetFlag) 
        { 
            m_TargetFlag.RequestDelete(); 
            Close(); 
            return true; 
        }
        
        if (w == m_BtnUpgrade && m_TargetFlag) 
        { 
            m_TargetFlag.RequestUpgrade(); 
            return true; 
        }
        
        if (w == m_BtnAdd)
        {
            string ownerID = m_TargetFlag.GetOwnerID();
            if (ownerID == "") 
            { 
                GetGame().RPCSingleParam(m_TargetFlag, TerritoryFlag.RPC_CLAIM_TERRITORY, null, true); 
                return true; 
            }
            
            string idToAdd = m_InputName.GetText(); 
            int row = m_NearbyList.GetSelectedRow();
            if (row != -1) 
            { 
                Param1<string> pData; 
                m_NearbyList.GetItemData(row, 0, pData); 
                if (pData) 
                {
                    idToAdd = pData.param1; 
                }
            }
            
            if (idToAdd != "") 
            {
                // Client-side validation for immediate feedback
                if (!m_TargetFlag.IsValidPlayerId(idToAdd))
                {
                    PlayerBase localPlayer = PlayerBase.Cast(GetGame().GetPlayer());
                    if (localPlayer)
                    {
                        localPlayer.MessageImportant("⛔ Недопустимый формат ID игрока!");
                    }
                    return true;
                }
                
                m_TargetFlag.RequestAddMember(idToAdd); 
                m_InputName.SetText(""); 
            } 
            return true;
        }
        
        if (w == m_BtnKick) 
        { 
            int kRow = m_MembersList.GetSelectedRow(); 
            if (kRow != -1) 
            { 
                // Get the member from the array directly
                if (m_TargetFlag.m_Members && kRow < m_TargetFlag.m_Members.Count())
                {
                    DP_TerritoryMember member = m_TargetFlag.m_Members.Get(kRow);
                    if (member)
                    {
                        m_TargetFlag.RequestRemoveMember(member.PlayerID);
                    }
                }
            } 
            return true; 
        }
        
        return false;
    }
    
    void UpdateNearbyPlayers() 
    { 
        if (!m_NearbyList || !m_NearbyList.IsVisible()) return;
        if (!m_TargetFlag)
        {
            Print("[DP_Territory ERROR] m_TargetFlag is null in UpdateNearbyPlayers");
            return;
        }
        
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.GetIdentity())
        {
            return; // Can't update if player is not valid
        }
        
        m_NearbyList.ClearItems(); 
        array<Object> objects = new array<Object>; 
        array<CargoBase> proxyCargos = new array<CargoBase>; 
        float currentR = m_TargetFlag.GetCurrentRadius(); 
        
        GetGame().GetObjectsAtPosition(player.GetPosition(), currentR, objects, proxyCargos); 
        
        string myID = player.GetIdentity().GetId();
        for (int i = 0; i < objects.Count(); i++) 
        { 
            PlayerBase pb = PlayerBase.Cast(objects.Get(i)); 
            if (pb && pb.GetIdentity()) 
            { 
                string pID = pb.GetIdentity().GetId(); 
                if (pID == myID) 
                {
                    continue; // Skip self
                }
                
                m_NearbyList.AddItem(pb.GetIdentity().GetName(), new Param1<string>(pID), 0); 
            } 
        } 
    }
    override void Update(float timeslice) 
    { 
        super.Update(timeslice); 
        m_UpdateTimer += timeslice; 
        if (m_UpdateTimer > DP_TerritoryConstants.UPDATE_INTERVAL) 
        { 
            UpdateInfo(); 
            UpdateNearbyPlayers(); 
            m_UpdateTimer = 0; 
        } 
        
        Input input = GetGame().GetInput(); 
        if (input.LocalPress("UAUIBack", false)) 
        {
            Close(); 
        }
    }
    override bool OnKeyDown(Widget w, int x, int y, int key) 
    { 
        super.OnKeyDown(w, x, y, key); 
        
        if (key == KeyCode.KC_ESCAPE) 
        { 
            Close(); 
            return true; 
        } 
        
        return false; 
    }
}