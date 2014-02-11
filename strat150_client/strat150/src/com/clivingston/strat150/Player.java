/**
 * 
 */
package com.clivingston.strat150;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import com.badlogic.gdx.graphics.Color;

/**
 * @author chris
 *
 */
public class Player {
	
	Map<String, Player> knownAllies;
	Map<String, Unit> units;
	Map<String, Equipment> equipment;
	
	String userID;
	String nickname;
	String faction;
	public Color color;
	double balance;
	
	
	public Player(String userID, String nickname, String faction)
	{
		this.userID = userID;
		this.nickname = nickname;
		knownAllies = new HashMap<String, Player>();
		units = new HashMap<String, Unit>();
		equipment = new HashMap<String, Equipment>();
		this.faction = faction;
		color = new Color(Color.WHITE);
	}
	
	
	
	/**
	 * Adds a Unit/Equipment item to this player's inventory.
	 * @param actor
	 */
	public void addItem(StratActor actor)
	{
		
		if(actor instanceof Unit)
			units.put(actor.id, (Unit) actor);
		
		if(actor instanceof Equipment)
			equipment.put(actor.id, (Equipment) actor);
		
		if(actor.owner != this)
		{
			actor.owner.rmItem(actor);
			actor.owner = this;
		}
		
	}
	
	/**
	 * Removes a Unit/Equipment item from this player's inventory.
	 * @param actor
	 */
	public void rmItem(StratActor actor)
	{
		if(actor instanceof Unit)
			units.remove(actor.id);
		
		if(actor instanceof Equipment)
			equipment.remove(actor.id);
		
		actor.owner = null;
		
	}
	
	/**
	 * Sets the player faction.
	 * @param faction
	 */
	public void setFaction(String faction)
	{
		this.faction = faction;
	}
	
	/**
	 * Gets the player faction.
	 * @return
	 */
	public String getFaction()
	{
		return faction;
	}
	
	/**
	 * Sets the player nickname.
	 * @param newNick
	 */
	public void setNickname(String newNick)
	{
		nickname = newNick;
	}
	
	/**
	 * Gets the player nickname
	 * @return
	 */
	public String getNick()
	{
		return nickname;
	}
	
	public String getUserID()
	{
		return userID;
	}
	
	public Set<String> getAllies()
	{
		return knownAllies.keySet();
	}
	
	public void setHostile(Player other)
	{
		knownAllies.remove(other.getUserID());
	}
	
	public void setAlly(Player other)
	{
		knownAllies.put(other.getUserID(), other);
	}
	
	public boolean hasUnit(String unitID)
	{
		return units.containsKey(unitID);
	}
	
	public boolean hasEquipment(String equipID)
	{
		return equipment.containsKey(equipID);
	}
	

}
