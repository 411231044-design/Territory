modded class TerritoryFlag
{
    static const int RPC_CLAIM_TERRITORY = 35466; 
    static const int RPC_SYNC_DATA       = 35467; 
    static const int RPC_REQ_DATA        = 35468;
    static const int RPC_DELETE_TERRITORY = 35469; 
    static const int RPC_ADD_MEMBER       = 35470;
    static const int RPC_REM_MEMBER       = 35471;
    static const int RPC_UPGRADE          = 35472;

    protected string m_OwnerID; 
    protected bool m_IsOwned;
    protected int m_Level;
    protected int m_PreservationLevel; 
    ref array<string> m_Members; 
    ref array<ref DP_CostItem> m_ClientNextLevelCost; 

    void TerritoryFlag()
    {
        if (!m_OwnerID) m_OwnerID = "";
        m_IsOwned = false;
        m_Level = 1;
        m_PreservationLevel = 0;
        m_Members = new array<string>();
        m_ClientNextLevelCost = new array<ref DP_CostItem>;
        
        RegisterNetSyncVariableBool("m_IsOwned");
        RegisterNetSyncVariableInt("m_Level");
    }
    
    // --- ИСПРАВЛЕНИЕ РАДИУСА ---
    float GetCurrentRadius()
    {
        DP_LevelDefinition lvlDef = DP_TerritoryConfig.Get().GetLevelConfig(m_Level);
        
        // Если конфиг есть И радиус в нем больше 0 - берем из конфига
        if (lvlDef && lvlDef.Radius > 0) return lvlDef.Radius;
        
        // Если конфига нет или там записан 0 - возвращаем дефолтное значение
        return DP_TerritoryConstants.DEFAULT_RADIUS;
    }
    // ---------------------------
    
    int GetPreservationPerkLevel() 
    { 
        return m_PreservationLevel; 
    }

    void UpdatePerkCache(PlayerBase owner)
    {
        if (!owner) return;
        
        if (owner.GetTerjeSkills()) 
        {
            float val = 0;
            if (owner.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_Preservation", val))
            {
                m_PreservationLevel = (int)val;
            }
            else 
            {
                m_PreservationLevel = 0;
            }
            SetSynchDirty();
        }
    }

    int GetTerritoryLevel() 
    { 
        if (m_Level < 1) return 1;
        return m_Level; 
    }

    bool HasPermissions(string playerID) 
    { 
        if (playerID == m_OwnerID) return true; 
        if (m_Members) { if (m_Members.Find(playerID) != -1) return true; } 
        return false; 
    }
    
    bool HasOwner() { return m_IsOwned; }
    string GetOwnerID() { return m_OwnerID; }

    float GetTotalForemanBonus()
    {
        float totalBonus = 0;
        
        // Получаем бонус владельца
        if (m_OwnerID != "")
        {
            PlayerBase owner = GetPlayerByUID(m_OwnerID);
            if (owner && owner.GetTerjeSkills())
            {
                float ownerBonus = 0;
                owner.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_Foreman", ownerBonus);
                totalBonus += ownerBonus;
            }
        }
        
        // Получаем бонусы всех участников
        if (m_Members)
        {
            for (int i = 0; i < m_Members.Count(); i++)
            {
                string memberID = m_Members.Get(i);
                PlayerBase member = GetPlayerByUID(memberID);
                if (member && member.GetTerjeSkills())
                {
                    float memberBonus = 0;
                    member.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_Foreman", memberBonus);
                    totalBonus += memberBonus;
                }
            }
        }
        
        return totalBonus;
    }

    // Вспомогательная функция для получения игрока по UID
    PlayerBase GetPlayerByUID(string uid)
    {
        if (!uid || uid == "") return null;
        
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player && player.GetIdentity() && player.GetIdentity().GetId() == uid)
            {
                return player;
            }
        }
        
        return null;
    }

    void RequestSyncDataAll() { if (GetGame().IsClient()) GetGame().RPCSingleParam(this, RPC_REQ_DATA, null, true); }
    void RequestClaim() { if (GetGame().IsClient()) GetGame().RPCSingleParam(this, RPC_CLAIM_TERRITORY, null, true); }
    void RequestDelete() { if (GetGame().IsClient()) GetGame().RPCSingleParam(this, RPC_DELETE_TERRITORY, null, true); }
    void RequestUpgrade() { if (GetGame().IsClient()) GetGame().RPCSingleParam(this, RPC_UPGRADE, null, true); }
    void RequestAddMember(string id) 
    { 
        if (GetGame().IsClient() && IsValidPlayerId(id)) 
        {
            GetGame().RPCSingleParam(this, RPC_ADD_MEMBER, new Param1<string>(id), true); 
        }
    }
    
    void RequestRemoveMember(string id) 
    { 
        if (GetGame().IsClient() && IsValidPlayerId(id)) 
        {
            GetGame().RPCSingleParam(this, RPC_REM_MEMBER, new Param1<string>(id), true); 
        }
    }
    
    // Helper function to validate player ID format
    bool IsValidPlayerId(string id)
    {
        if (!id || id == "") return false;
        
        // Player IDs should be reasonable length (typically SteamID64 or similar)
        if (id.Length() < 3 || id.Length() > 64) return false;
        
        // Allowed characters for player IDs
        string allowedChars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
        
        for (int i = 0; i < id.Length(); i++)
        {
            string c = id.Get(i);
            if (allowedChars.IndexOf(c) == -1)
            {
                return false;
            }
        }
        
        return true;
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);
        
        if (GetGame().IsServer())
        {
            string senderID = sender.GetId();
            PlayerBase player = PlayerBase.Cast(sender.GetPlayer());

            if (player && senderID == m_OwnerID) UpdatePerkCache(player);

            if (rpc_type == RPC_CLAIM_TERRITORY)
            {
                if (m_IsOwned) { player.MessageImportant("⛔ Уже занято!"); return; }
                if (DP_TerritoryManager.TM_GetInstance().TM_HasTerritory(senderID)) { player.MessageImportant("⛔ У вас уже есть база!"); return; }
                
                float safeDist = DP_TerritoryConfig.Get().GetMaxPossibleRadius() * 2.0; 
                if (DP_TerritoryManager.TM_GetInstance().TM_IsTerritoryNearby(this.GetPosition(), safeDist)) 
                { 
                    player.MessageImportant("⛔ Слишком близко к другой базе! Нужно " + safeDist + "м."); 
                    return; 
                }
                
                if (!CheckLimitsForClaim(player)) return;

                m_OwnerID = senderID; m_IsOwned = true; m_Level = 1; m_Members.Clear(); SetSynchDirty(); 
                DP_TerritoryManager.TM_GetInstance().TM_RegisterOwner_Unique(senderID, this);
                
                // Исправленное сообщение в чат
                player.MessageImportant("✅ Территория захвачена! Радиус: " + GetCurrentRadius() + "м");
                
                UpdatePerkCache(player); 
                SendSyncToClient(sender);
                return;
            }

            if (rpc_type == RPC_ADD_MEMBER)
            {
                Param1<string> pAdd; 
                if (ctx.Read(pAdd) && senderID == m_OwnerID) 
                { 
                    // Validate the player ID
                    if (!IsValidPlayerId(pAdd.param1))
                    {
                        player.MessageImportant("⛔ Недопустимый ID игрока!");
                        Print(string.Format("[DP_Territory SECURITY] Invalid player ID attempt: %1", pAdd.param1));
                        return;
                    }
                    
                    // Check if already a member
                    if (m_Members.Find(pAdd.param1) == -1) 
                    { 
                        int maxMembers = DP_TerritoryConstants.DEFAULT_MAX_MEMBERS;
                        float bonusMembers = 0;
                        
                        if (player.GetTerjeSkills()) 
                        {
                            player.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_Hospitality", bonusMembers);
                        }
                        
                        int totalLimit = maxMembers + (int)bonusMembers;

                        if (m_Members.Count() >= totalLimit)
                        {
                            player.MessageImportant("⛔ Лимит: " + maxMembers + " (Нужен перк +"+(int)bonusMembers+")");
                            return;
                        }

                        m_Members.Insert(pAdd.param1); 
                        SetSynchDirty(); SendSyncToClient(sender); 
                        player.MessageImportant("✅ Добавлен (" + m_Members.Count() + "/" + totalLimit + ")");
                        Print(string.Format("[DP_Territory] Player %1 added member %2", senderID, pAdd.param1));
                    }
                    else
                    {
                        player.MessageImportant("⛔ Этот игрок уже добавлен!");
                    }
                } 
                return;
            }

            if (rpc_type == RPC_UPGRADE)
            {
                if (senderID != m_OwnerID) { player.MessageImportant("⛔ Только владелец!"); return; }
                
                int nextLevelInt = m_Level + 1;
                DP_LevelDefinition nextLvl = DP_TerritoryConfig.Get().GetLevelConfig(nextLevelInt);
                
                if (!nextLvl || nextLvl.Level <= m_Level) { player.MessageImportant("⛔ Макс. уровень!"); return; }

                if (nextLevelInt > 1)
                {
                    float allowedLvl = 0;
                    
                    if (player.GetTerjeSkills()) 
                    {
                        player.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_GrandArchitect", allowedLvl);
                    }
                    else
                    {
                        Print("[DP ERROR] No TerjeSkills component!");
                    }
                    
                    int requiredPerkLevel = nextLevelInt - 1; 
                    if (allowedLvl < (requiredPerkLevel - 0.1))
                    {
                         player.MessageImportant("⛔ Нужен перк! (У вас: " + allowedLvl + " | Надо: " + requiredPerkLevel + ")");
                         return;
                    }
                }

                float discountPercent = 0;
                if (player.GetTerjeSkills()) player.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_ResourcefulPlanner", discountPercent);
                float discountMult = discountPercent / 100.0; 

                array<ref DP_CostItem> discountedCosts = new array<ref DP_CostItem>;
                if (nextLvl.UpgradeCost)
                {
                    foreach(DP_CostItem originalItem : nextLvl.UpgradeCost)
                    {
                        int newCount = Math.Round(originalItem.Count * (1.0 - discountMult));
                        if (newCount < 1) newCount = 1;
                        discountedCosts.Insert(new DP_CostItem(originalItem.ClassName, originalItem.DisplayName, newCount));
                    }
                }

                string missingRes = CheckResourcesFlag(discountedCosts);
                if (missingRes != "") 
                { 
                    Print("[DP RESOURCE FAIL] Missing: " + missingRes);
                    player.MessageImportant("⛔ Не хватает: " + missingRes); 
                    return; 
                }

                ConsumeResourcesFlag(discountedCosts);
                
                m_Level = nextLevelInt; 
                SetSynchDirty(); SendSyncToClient(sender);
                player.MessageImportant("✅ Уровень " + m_Level + "! (Скидка " + (int)discountPercent + "%)");
                return;
            }

            if (rpc_type == RPC_DELETE_TERRITORY) 
            { 
                if (senderID == m_OwnerID) 
                { 
                    DP_TerritoryManager.TM_GetInstance().TM_UnregisterByOwnerId(m_OwnerID); 
                    GetGame().ObjectDelete(this); 
                } 
                return; 
            }
            
            if (rpc_type == RPC_REM_MEMBER) 
            { 
                Param1<string> pRem; 
                if (ctx.Read(pRem) && senderID == m_OwnerID) 
                {
                    // Validate the player ID
                    if (!IsValidPlayerId(pRem.param1))
                    {
                        player.MessageImportant("⛔ Недопустимый ID игрока!");
                        Print(string.Format("[DP_Territory SECURITY] Invalid player ID in remove: %1", pRem.param1));
                        return;
                    }
                    
                    int idx = m_Members.Find(pRem.param1); 
                    if (idx != -1) 
                    { 
                        m_Members.Remove(idx); 
                        SetSynchDirty(); 
                        SendSyncToClient(sender);
                        player.MessageImportant("✅ Игрок исключен!");
                        Print(string.Format("[DP_Territory] Player %1 removed member %2", senderID, pRem.param1));
                    }
                    else
                    {
                        player.MessageImportant("⛔ Игрок не найден в списке!");
                    }
                } 
                return; 
            }
            
            if (rpc_type == RPC_REQ_DATA) 
            { 
                SendSyncToClient(sender); 
                return; 
            }
        }

        if (GetGame().IsClient())
        {
            if (rpc_type == RPC_SYNC_DATA)
            {
                Param3<string, array<string>, array<ref DP_CostItem>> data;
                if (ctx.Read(data)) 
                { 
                    m_OwnerID = data.param1; 
                    m_Members = data.param2; 
                    m_ClientNextLevelCost = data.param3; 
                    DP_TerritoryManager.TM_GetInstance().m_LastReceivedOwnerID = m_OwnerID; 
                }
            }
        }
    }
    
    bool CheckLimitsForClaim(PlayerBase player)
    {
        DP_LevelDefinition l1 = DP_TerritoryConfig.Get().GetLevelConfig(1);
        if (!l1) return true;
        array<Object> objects = new array<Object>;
        array<CargoBase> proxyCargos = new array<CargoBase>;
        GetGame().GetObjectsAtPosition(this.GetPosition(), l1.Radius, objects, proxyCargos);

        int walls = 0; int tents = 0; int furn  = 0;
        foreach(Object obj : objects)
        {
            if (obj == this) continue;
            if (!DP_TerritoryManager.TM_GetInstance().IsPlacedObject(obj)) continue;
            int cat = DP_TerritoryConfig.Get().GetItemCategory(obj.GetType());
            if (cat == 1) walls++; if (cat == 2) tents++; if (cat == 3) furn++;
        }
        if (walls > l1.MaxStructures) { player.MessageImportant("⛔ Очистите территорию! Стройка: " + walls + "/" + l1.MaxStructures); return false; }
        if (tents > l1.MaxTents) { player.MessageImportant("⛔ Очистите территорию! Палатки: " + tents + "/" + l1.MaxTents); return false; }
        if (furn > l1.MaxFurniture) { player.MessageImportant("⛔ Очистите территорию! Мебель: " + furn + "/" + l1.MaxFurniture); return false; }
        return true;
    }

    void SendSyncToClient(PlayerIdentity target) 
    { 
        array<ref DP_CostItem> nextCost = new array<ref DP_CostItem>;
        DP_LevelDefinition nextLvl = DP_TerritoryConfig.Get().GetLevelConfig(m_Level + 1);
        if (nextLvl && nextLvl.UpgradeCost) nextCost = nextLvl.UpgradeCost;
        GetGame().RPCSingleParam(this, RPC_SYNC_DATA, new Param3<string, array<string>, array<ref DP_CostItem>>(m_OwnerID, m_Members, nextCost), true, target); 
    }

    string CheckResourcesFlag(array<ref DP_CostItem> costs) 
    { 
        if (!costs) return ""; 
        foreach(DP_CostItem cost : costs) 
        { 
            int found = GetItemAmountInEntity(this, cost.ClassName); 
            if (found < cost.Count) return cost.ClassName; 
        } 
        return ""; 
    }
    
    void ConsumeResourcesFlag(array<ref DP_CostItem> costs) 
    { 
        if (!costs) return; 
        foreach(DP_CostItem cost : costs) 
        { 
            RemoveItemAmountInEntity(this, cost.ClassName, cost.Count); 
        } 
    }
    int GetItemAmountInEntity(EntityAI entity, string classname) 
    { 
        if (!entity) return 0; 
        GameInventory inventory = entity.GetInventory(); 
        if (!inventory) return 0; 
        CargoBase cargo = inventory.GetCargo(); 
        if (!cargo) return 0; 
        
        int count = 0; 
        int cargoCount = cargo.GetItemCount(); 
        for (int i = 0; i < cargoCount; i++) 
        { 
            EntityAI item = cargo.GetItem(i); 
            if (item && GetGame().IsKindOf(item.GetType(), classname)) 
            { 
                ItemBase ib = ItemBase.Cast(item); 
                count += ib.GetQuantity(); 
                if (ib.GetQuantityMax() == 0) count++; 
            } 
        } 
        return count; 
    }
    
    void RemoveItemAmountInEntity(EntityAI entity, string classname, int amountToRemove) 
    { 
        if (!entity) return; 
        GameInventory inventory = entity.GetInventory(); 
        if (!inventory) return; 
        CargoBase cargo = inventory.GetCargo(); 
        if (!cargo) return; 
        
        int needed = amountToRemove; 
        for (int i = cargo.GetItemCount() - 1; i >= 0; i--) 
        { 
            if (needed <= 0) break; 
            EntityAI item = cargo.GetItem(i); 
            if (item && GetGame().IsKindOf(item.GetType(), classname)) 
            { 
                ItemBase ib = ItemBase.Cast(item); 
                if (ib.GetQuantityMax() > 0) 
                { 
                    int qty = ib.GetQuantity(); 
                    if (qty > needed) 
                    { 
                        ib.AddQuantity(-needed); 
                        needed = 0; 
                    } 
                    else 
                    { 
                        needed -= qty; 
                        GetGame().ObjectDelete(ib); 
                    } 
                } 
                else 
                { 
                    GetGame().ObjectDelete(ib); 
                    needed--; 
                } 
            } 
        } 
    }
    
    override void OnStoreSave(ParamsWriteContext ctx) 
    { 
        super.OnStoreSave(ctx); 
        ctx.Write(m_OwnerID); 
        ctx.Write(m_IsOwned); 
        ctx.Write(m_Members); 
        ctx.Write(m_Level); 
        ctx.Write(m_PreservationLevel); 
    }
    
    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    { 
        if (!super.OnStoreLoad(ctx, version)) return false; 
        if (!ctx.Read(m_OwnerID)) return false; 
        if (!ctx.Read(m_IsOwned)) return false; 
        if (!ctx.Read(m_Members)) return false; 
        if (!ctx.Read(m_Level)) m_Level = 1; 
        if (!ctx.Read(m_PreservationLevel)) m_PreservationLevel = 0; 
        if (m_IsOwned && m_OwnerID != "") DP_TerritoryManager.TM_GetInstance().TM_RegisterOwner_Unique(m_OwnerID, this); 
        SetSynchDirty(); 
        return true; 
    }

    override void AfterStoreLoad() 
    { 
        super.AfterStoreLoad(); 
        if (m_IsOwned && m_OwnerID != "") 
        {
            DP_TerritoryManager.TM_GetInstance().TM_RegisterOwner_Unique(m_OwnerID, this); 
        }
    }
    
    array<ref DP_CostItem> GetNextLevelCostClient() 
    { 
        return m_ClientNextLevelCost; 
    }
    
    override void SetActions() 
    { 
        super.SetActions(); 
        AddAction(DP_TerritoryFlagAction); 
    }
}