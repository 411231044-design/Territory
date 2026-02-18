modded class TerritoryFlag
{
    static const int RPC_CLAIM_TERRITORY = 35466; 
    static const int RPC_SYNC_DATA       = 35467; 
    static const int RPC_REQ_DATA        = 35468;
    static const int RPC_DELETE_TERRITORY = 35469; 
    static const int RPC_ADD_MEMBER       = 35470;
    static const int RPC_REM_MEMBER       = 35471;
    static const int RPC_UPGRADE          = 35472;
    static const int RPC_CHANGE_ROLE      = 35473;
    static const int RPC_CREATE_SUBZONE   = 35474;
    static const int RPC_DELETE_SUBZONE   = 35475;
    static const int RPC_EXTEND_SUBZONE   = 35476;

    protected string m_OwnerID; 
    protected bool m_IsOwned;
    protected int m_Level;
    protected int m_PreservationLevel; 
    ref array<ref DP_TerritoryMember> m_Members; 
    ref array<ref DP_TerritorySubzone> m_Subzones;
    ref array<ref DP_CostItem> m_ClientNextLevelCost; 

    void TerritoryFlag()
    {
        if (!m_OwnerID) m_OwnerID = "";
        m_IsOwned = false;
        m_Level = 1;
        m_PreservationLevel = 0;
        m_Members = new array<ref DP_TerritoryMember>();
        m_Subzones = new array<ref DP_TerritorySubzone>();
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
        if (m_Members) 
        { 
            for (int i = 0; i < m_Members.Count(); i++)
            {
                DP_TerritoryMember member = m_Members.Get(i);
                if (member && member.PlayerID == playerID) return true;
            }
        }
        
        // Check subzone permissions
        return HasSubzonePermissions(playerID);
    }
    
    // New role-based methods
    array<ref DP_TerritoryMember> GetMembers()
    {
        return m_Members;
    }
    
    array<ref DP_TerritorySubzone> GetSubzones()
    {
        return m_Subzones;
    }
    
    int GetPlayerRole(string playerID)
    {
        if (playerID == m_OwnerID) return DP_TerritoryRole.OWNER;
        
        if (m_Members)
        {
            for (int i = 0; i < m_Members.Count(); i++)
            {
                DP_TerritoryMember member = m_Members.Get(i);
                if (member && member.PlayerID == playerID)
                {
                    return member.Role;
                }
            }
        }
        
        return -1; // Not a member
    }
    
    bool CanBuild(string playerID)
    {
        int role = GetPlayerRole(playerID);
        if (role == -1) return HasSubzonePermissions(playerID); // Check subzone
        return DP_TerritoryRole.CanBuild(role);
    }
    
    bool CanManageMembers(string playerID)
    {
        int role = GetPlayerRole(playerID);
        if (role == -1) return false;
        return DP_TerritoryRole.CanManageMembers(role);
    }
    
    bool CanManageSubzones(string playerID)
    {
        int role = GetPlayerRole(playerID);
        if (role == -1) return false;
        return DP_TerritoryRole.CanManageSubzones(role);
    }
    
    bool ChangeRole(string playerID, int newRole)
    {
        if (!m_Members) return false;
        
        for (int i = 0; i < m_Members.Count(); i++)
        {
            DP_TerritoryMember member = m_Members.Get(i);
            if (member && member.PlayerID == playerID)
            {
                member.Role = newRole;
                return true;
            }
        }
        
        return false;
    }
    
    // Subzone methods
    bool HasSubzonePermissions(string playerID)
    {
        if (!m_Subzones) return false;
        
        for (int i = 0; i < m_Subzones.Count(); i++)
        {
            DP_TerritorySubzone subzone = m_Subzones.Get(i);
            if (subzone && subzone.RenterID == playerID && !subzone.IsExpired())
            {
                return true;
            }
        }
        
        return false;
    }
    
    DP_TerritorySubzone GetPlayerSubzone(string playerID)
    {
        if (!m_Subzones) return null;
        
        for (int i = 0; i < m_Subzones.Count(); i++)
        {
            DP_TerritorySubzone subzone = m_Subzones.Get(i);
            if (subzone && subzone.RenterID == playerID && !subzone.IsExpired())
            {
                return subzone;
            }
        }
        
        return null;
    }
    
    bool CreateSubzone(string renterID, vector pos, float radius, int days, int price)
    {
        if (m_Level < 2) return false; // Must be level 2+
        if (!m_Subzones) m_Subzones = new array<ref DP_TerritorySubzone>();
        
        DP_TerritorySubzone subzone = new DP_TerritorySubzone(renterID, pos, radius, days, price);
        m_Subzones.Insert(subzone);
        
        return true;
    }
    
    bool RemoveSubzone(string renterID)
    {
        if (!m_Subzones) return false;
        
        for (int i = 0; i < m_Subzones.Count(); i++)
        {
            DP_TerritorySubzone subzone = m_Subzones.Get(i);
            if (subzone && subzone.RenterID == renterID)
            {
                m_Subzones.Remove(i);
                return true;
            }
        }
        
        return false;
    }
    
    bool ExtendSubzone(string renterID, int additionalDays)
    {
        if (!m_Subzones) return false;
        
        for (int i = 0; i < m_Subzones.Count(); i++)
        {
            DP_TerritorySubzone subzone = m_Subzones.Get(i);
            if (subzone && subzone.RenterID == renterID)
            {
                subzone.ExtendDuration(additionalDays);
                return true;
            }
        }
        
        return false;
    }
    
    void TransferOwnership(string newOwnerID)
    {
        // Remove from members if they were a member
        if (m_Members)
        {
            for (int i = m_Members.Count() - 1; i >= 0; i--)
            {
                DP_TerritoryMember member = m_Members.Get(i);
                if (member && member.PlayerID == newOwnerID)
                {
                    m_Members.Remove(i);
                    break;
                }
            }
        }
        
        // Change ownership
        string oldOwnerID = m_OwnerID;
        m_OwnerID = newOwnerID;
        
        // Optionally add old owner as admin
        if (oldOwnerID != "")
        {
            DP_TerritoryMember oldOwnerMember = new DP_TerritoryMember(oldOwnerID, DP_TerritoryRole.ADMIN);
            m_Members.Insert(oldOwnerMember);
        }
        
        SetSynchDirty();
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
                DP_TerritoryMember member = m_Members.Get(i);
                if (!member) continue;
                
                string memberID = member.PlayerID;
                PlayerBase memberPlayer = GetPlayerByUID(memberID);
                if (memberPlayer && memberPlayer.GetTerjeSkills())
                {
                    float memberBonus = 0;
                    memberPlayer.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_Foreman", memberBonus);
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
    void RequestAddMember(string id, int role = DP_TerritoryRole.BUILDER) 
    { 
        if (GetGame().IsClient() && IsValidPlayerId(id)) 
        {
            GetGame().RPCSingleParam(this, RPC_ADD_MEMBER, new Param2<string, int>(id, role), true); 
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
                
                // Log the claim
                string playerName = player.GetIdentity().GetName();
                DP_TerritoryLogger.LogClaim(senderID, playerName, this.GetPosition(), m_Level);
                
                UpdatePerkCache(player); 
                SendSyncToClient(sender);
                return;
            }

            if (rpc_type == RPC_ADD_MEMBER)
            {
                Param2<string, int> pAdd; 
                if (ctx.Read(pAdd) && senderID == m_OwnerID) 
                { 
                    // Validate the player ID
                    if (!IsValidPlayerId(pAdd.param1))
                    {
                        player.MessageImportant("⛔ Недопустимый ID игрока!");
                        Print(string.Format("[DP_Territory SECURITY] Invalid player ID attempt: %1", pAdd.param1));
                        DP_TerritoryLogger.LogSecurity(string.Format("Invalid player ID attempt by %1: %2", senderID, pAdd.param1));
                        return;
                    }
                    
                    string targetID = pAdd.param1;
                    int role = pAdd.param2;
                    
                    // Default to BUILDER if invalid role
                    if (role < DP_TerritoryRole.ADMIN || role > DP_TerritoryRole.GUEST)
                    {
                        role = DP_TerritoryRole.BUILDER;
                    }
                    
                    // Check if already a member
                    bool alreadyMember = false;
                    if (m_Members)
                    {
                        for (int i = 0; i < m_Members.Count(); i++)
                        {
                            DP_TerritoryMember member = m_Members.Get(i);
                            if (member && member.PlayerID == targetID)
                            {
                                alreadyMember = true;
                                break;
                            }
                        }
                    }
                    
                    if (!alreadyMember) 
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

                        DP_TerritoryMember newMember = new DP_TerritoryMember(targetID, role);
                        m_Members.Insert(newMember); 
                        SetSynchDirty(); SendSyncToClient(sender); 
                        player.MessageImportant("✅ Добавлен (" + m_Members.Count() + "/" + totalLimit + ") - " + DP_TerritoryRole.GetRoleName(role));
                        
                        // Log the action
                        string playerName = player.GetIdentity().GetName();
                        DP_TerritoryLogger.LogAddMember(senderID, playerName, targetID, role, this.GetPosition());
                        
                        Print(string.Format("[DP_Territory] Player %1 added member %2 with role %3", senderID, targetID, role));
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
                
                int oldLevel = m_Level;
                m_Level = nextLevelInt; 
                SetSynchDirty(); SendSyncToClient(sender);
                player.MessageImportant("✅ Уровень " + m_Level + "! (Скидка " + (int)discountPercent + "%)");
                
                // Log the upgrade
                string playerName = player.GetIdentity().GetName();
                DP_TerritoryLogger.LogUpgrade(senderID, playerName, this.GetPosition(), oldLevel, m_Level);
                
                return;
            }

            if (rpc_type == RPC_DELETE_TERRITORY) 
            { 
                if (senderID == m_OwnerID) 
                { 
                    // Log the deletion
                    string playerName = player.GetIdentity().GetName();
                    DP_TerritoryLogger.LogDelete(senderID, playerName, this.GetPosition());
                    
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
                        DP_TerritoryLogger.LogSecurity(string.Format("Invalid player ID in remove by %1: %2", senderID, pRem.param1));
                        return;
                    }
                    
                    bool found = false;
                    if (m_Members)
                    {
                        for (int i = m_Members.Count() - 1; i >= 0; i--)
                        {
                            DP_TerritoryMember member = m_Members.Get(i);
                            if (member && member.PlayerID == pRem.param1)
                            {
                                m_Members.Remove(i);
                                found = true;
                                break;
                            }
                        }
                    }
                    
                    if (found) 
                    { 
                        SetSynchDirty(); 
                        SendSyncToClient(sender);
                        player.MessageImportant("✅ Игрок исключен!");
                        
                        // Log the action
                        string playerName = player.GetIdentity().GetName();
                        DP_TerritoryLogger.LogRemoveMember(senderID, playerName, pRem.param1, this.GetPosition());
                        
                        Print(string.Format("[DP_Territory] Player %1 removed member %2", senderID, pRem.param1));
                    }
                    else
                    {
                        player.MessageImportant("⛔ Игрок не найден в списке!");
                    }
                } 
                return; 
            }
            
            if (rpc_type == RPC_CHANGE_ROLE)
            {
                Param2<string, int> pRole;
                if (ctx.Read(pRole) && CanManageMembers(senderID))
                {
                    string targetID = pRole.param1;
                    int newRole = pRole.param2;
                    
                    // Validate role
                    if (newRole < DP_TerritoryRole.ADMIN || newRole > DP_TerritoryRole.GUEST)
                    {
                        player.MessageImportant("⛔ Недопустимая роль!");
                        return;
                    }
                    
                    // Get old role
                    int oldRole = GetPlayerRole(targetID);
                    
                    if (ChangeRole(targetID, newRole))
                    {
                        SetSynchDirty();
                        SendSyncToClient(sender);
                        player.MessageImportant("✅ Роль изменена: " + DP_TerritoryRole.GetRoleName(newRole));
                        
                        // Log the action
                        string playerName = player.GetIdentity().GetName();
                        DP_TerritoryLogger.LogChangeRole(senderID, playerName, targetID, oldRole, newRole, this.GetPosition());
                    }
                    else
                    {
                        player.MessageImportant("⛔ Игрок не найден!");
                    }
                }
                return;
            }
            
            if (rpc_type == RPC_CREATE_SUBZONE)
            {
                Param4<string, float, int, int> pSubzone;
                if (ctx.Read(pSubzone) && senderID == m_OwnerID)
                {
                    if (m_Level < 2)
                    {
                        player.MessageImportant("⛔ Нужен уровень 2+!");
                        return;
                    }
                    
                    string renterID = pSubzone.param1;
                    float radius = pSubzone.param2;
                    int days = pSubzone.param3;
                    int price = pSubzone.param4;
                    
                    // Validate parameters
                    if (radius < 10.0 || radius > 20.0)
                    {
                        player.MessageImportant("⛔ Радиус должен быть 10-20м!");
                        return;
                    }
                    
                    vector subzonePos = player.GetPosition();
                    
                    if (CreateSubzone(renterID, subzonePos, radius, days, price))
                    {
                        SetSynchDirty();
                        player.MessageImportant("✅ Субзона создана!");
                        
                        // Log the action
                        string playerName = player.GetIdentity().GetName();
                        DP_TerritoryLogger.LogCreateSubzone(senderID, playerName, renterID, subzonePos, radius, days, this.GetPosition());
                    }
                    else
                    {
                        player.MessageImportant("⛔ Ошибка создания!");
                    }
                }
                return;
            }
            
            if (rpc_type == RPC_DELETE_SUBZONE)
            {
                Param1<string> pDelSubzone;
                if (ctx.Read(pDelSubzone) && CanManageSubzones(senderID))
                {
                    string renterID = pDelSubzone.param1;
                    
                    if (RemoveSubzone(renterID))
                    {
                        SetSynchDirty();
                        player.MessageImportant("✅ Субзона удалена!");
                        
                        // Log the action
                        string playerName = player.GetIdentity().GetName();
                        DP_TerritoryLogger.LogDeleteSubzone(senderID, playerName, renterID, this.GetPosition());
                    }
                    else
                    {
                        player.MessageImportant("⛔ Субзона не найдена!");
                    }
                }
                return;
            }
            
            if (rpc_type == RPC_EXTEND_SUBZONE)
            {
                Param2<string, int> pExtSubzone;
                if (ctx.Read(pExtSubzone) && senderID == m_OwnerID)
                {
                    string renterID = pExtSubzone.param1;
                    int additionalDays = pExtSubzone.param2;
                    
                    if (ExtendSubzone(renterID, additionalDays))
                    {
                        SetSynchDirty();
                        player.MessageImportant("✅ Субзона продлена на " + additionalDays + " дней!");
                    }
                    else
                    {
                        player.MessageImportant("⛔ Субзона не найдена!");
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
        ctx.Write(m_Subzones);
        ctx.Write(m_Level); 
        ctx.Write(m_PreservationLevel); 
    }
    
    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    { 
        if (!super.OnStoreLoad(ctx, version)) return false; 
        if (!ctx.Read(m_OwnerID)) return false; 
        if (!ctx.Read(m_IsOwned)) return false; 
        if (!ctx.Read(m_Members)) 
        {
            // Migration: Try to read old format (array<string>)
            array<string> oldMembers;
            if (ctx.Read(oldMembers))
            {
                // Convert old members to new format
                m_Members = new array<ref DP_TerritoryMember>();
                for (int i = 0; i < oldMembers.Count(); i++)
                {
                    DP_TerritoryMember member = new DP_TerritoryMember(oldMembers.Get(i), DP_TerritoryRole.BUILDER);
                    m_Members.Insert(member);
                }
                Print("[DP_Territory] Migrated old member format to new format");
            }
            else
            {
                m_Members = new array<ref DP_TerritoryMember>();
            }
        }
        if (!ctx.Read(m_Subzones)) m_Subzones = new array<ref DP_TerritorySubzone>();
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