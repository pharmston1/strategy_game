package com.clivingston.strat150;

public class Equipment extends StratActor {

	public Equipment(Player owner, String equipType, String equipID, RulesEntry entry) {
		super(owner, equipType, equipID, entry);
		// TODO Auto-generated constructor stub
		
		
	}
	
	/**
	 * Gets the parent unit of this equipment.
	 * @return The parent unit, or null if none.
	 */
	public Unit getParentUnit()
	{
		StratActor a = parent;
		
		while(a != null)
		{
			if(a instanceof Unit)
				return (Unit) a;
			
			a = a.parent;
		}
		
		return null;
	}

}
