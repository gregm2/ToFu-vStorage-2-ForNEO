#ifdef GAMELABS
class VST_NEO_CFCloud_UnlockBarrelsRadius extends GameLabsContextAction {
        void VST_NEO_CFCloud_UnlockBarrelsRadius() {
            this.actionCode = "VST_NEO_CFCloud_UnlockBarrelsRadius";
            this.actionName = "Unlock locked barrels in radius (inifinite height)";
            this.actionIcon = "database";
            this.actionColour = "warning";
            this.actionContext = "world";

            this.parameters.Insert("vector", GameLabsActionParameter("Coordinates", "World coordinates", "vector"));
		    GameLabsActionParameter radius_gap = new GameLabsActionParameter("Radius", "Radius to unlock barrels in (must be more than 0)", "float");
			radius_gap.valueFloat = 0.1;
            this.parameters.Insert("radius", radius_gap);

        }

        override bool Execute(GameLabsActionContext context) 
		{
			vector position = context.parameters.Get("vector").GetVector();
			float radius = context.parameters.Get("radius").GetFloat();
		
            GetGameLabs().GetLogger().Warn(string.Format("[UnlockBarrelsRadius] Unlocking %1m at %2", radius, position));

			if (radius <= 0) 
			{
				GetGameLabs().GetLogger().Warn("[UnlockBarrelsRadius] radius less than or equal to zero");
				return false;
			}
    
			int unlockedcount = 0;
			unlockedcount = Barrel_ColorBase.vst_neo_unlock_in_radius(position, radius);
			GetGameLabs().GetLogger().Warn(string.Format("[UnlockBarrelsRadius] Unlocked %1 items", unlockedcount));
		
            return true;
        }
};

class VST_NEO_CFCloud_UnlockAllBarrels extends GameLabsContextAction {
        void VST_NEO_CFCloud_UnlockAllBarrels() {
            this.actionCode = "VST_NEO_CFCloud_UnlockAllBarrels";
            this.actionName = "Unlock ALL locked barrels";
            this.actionIcon = "database";
            this.actionColour = "danger";
            this.actionContext = "world";
        }

        override bool Execute(GameLabsActionContext context) 
		{
            GetGameLabs().GetLogger().Warn("[UnlockBarrelsAll] Unlocking ALL barrels");

			int unlockedcount = 0;
			unlockedcount = Barrel_ColorBase.vst_neo_unlock_all();
			GetGameLabs().GetLogger().Warn(string.Format("[UnlockBarrelsAll] Unlocked %1 items", unlockedcount));		
            return true;
        }
};

#endif
