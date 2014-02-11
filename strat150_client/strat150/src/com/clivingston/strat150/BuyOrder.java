package com.clivingston.strat150;

import java.util.HashMap;
import java.util.Map;

import com.badlogic.gdx.utils.Base64Coder;

public class BuyOrder {
	
	private static int unique = 0; //used to generate labels and such
	
	private int qty;
	private String title;
	
	private String serialized; 
	
	private String unitType; //Type (unit/equipment) that we're purchasing
	private Map<String, String> equipTypes; //Key = label, Value = equipType
	private Map<String, Integer> equipQtys; //Key = label, Value = label quantity 
	private Map<String, String> attachedTo; //Key = label, Value = label Key is attached to
	
	private int dropX, dropY;
	private boolean bUpdated;
	
	/**
	 * Gets a JSON-encoded version of this order for saving later.
	 * @param key User-inputted identifier (will be escaped as Base64).
	 * @return The encoded string.
	 */
	public String getSavedString(String key)
	{
		if(bUpdated)
		{
			serialize();
			bUpdated = false;
		}
		
		return ("{\"" + Base64Coder.encodeString(key) + "\" : { " + serialized+"}");
	}
	
	/**
	 * Shell function...doesn't do anything yet
	 * @param json JSON-encoded buy order to deserialize
	 * @return True on success.
	 */
	public boolean deserialize(String json)
	{
		//TODO:  Impliment this for saving
		return false;
	}
	
	public boolean updated()
	{
		return bUpdated;
	}
	
	public BuyOrder()
	{
		title = "P"+(unique++);
		qty = 1;
		dropX = -1;
		dropY = -1;
		bUpdated = false; //There's nothing here...
	}
	
	public void setTotalQty(int qty)
	{
		this.qty = qty;
		bUpdated = true;
	}
	
	public void setUnitType(String type)
	{
		unitType = type;
		bUpdated = true;
	}
	
	public void setDropCoords(int x, int y)
	{
		dropX = x;
		dropY = y;
		bUpdated = true;
	}
	
	/**
	 * Serializes this object to JSON.
	 * @return Serialzed object, or NULL if invalid.
	 */
	public String serialize()
	{
		if(!bUpdated || qty < 1 || (unitType == null && equipTypes == null)) //If nothing is being bought, bail
			return null;
		
		String s = "";
		
		//s = "\"" + title + "\" : { " + s;
		
		if(qty > 1)
		{
			s += "\"Qty\" : " + qty + ",";
		}
		
		if(unitType != null)
		{
			s += "\"Unit\" : \"" + unitType + "\",";
		}
		
		if(dropX >= 0 && dropY >= 0)
		{
			s += "\"Drop\" : \"" + dropX + " " + dropY + "\",";
		}
		
		if(equipTypes != null)
		{	
			String eq = "\"Equipment\" : [";
			String att = "\"Attach\" : [";;
			
			boolean isAttached = false;
			
			for(String label : equipTypes.keySet())
			{
				String type = equipTypes.get(label);
				Integer eqQty = equipQtys.get(label);
				String attachEq = attachedTo.get(label);
				
				if(attachEq != null)
				{
					att += "\"" + label + " " + attachEq + "\",";
					isAttached = true;
				}
				
				eq += "\"" + type + " " + eqQty + " " + label + "\","; 
				
			}
			
			//Be rid of trailing commas
			eq = eq.substring(0, eq.length() - 1);
			att = att.substring(0, att.length() - 1);
			
			eq += "]";
			att += "]";
			
			s += eq;
			
			if(isAttached)
				s += "," + att;	
		}
		else
		{
			//Get rid of the comma at the end
			s = s.substring(0, s.length() - 1);
		}
		
		s += "}";
		
		bUpdated = false;
		serialized = s;
		s = "\"" + title + "\" : { " + s;
		return s;
	}
	
	
	/**
	 * Adds an equipment type to the buy order attached to the base type.
	 * @param type Equipment type string.
	 * @param qty Total quantity.
	 * @return Generated label.
	 */
	public String addEquip(String type, int qty)
	{
		return addEquip(type, qty, null);
	}
	
	/**
	 * Adds an equipment type to the buy order attached to the specified target.
	 * @param type Equipment type string.
	 * @param qty Total quantity.
	 * @param attachTo Label or existing unitID to attach to.
	 * @return Generated label.
	 */
	public String addEquip(String type, int qty, String attachTo)
	{
		String label = "E"+(unique++);
		
		//Initialize maps, if need be
		if(equipTypes == null)
			equipTypes = new HashMap<String, String>();
		
		if(equipQtys == null)
			equipQtys = new HashMap<String, Integer>();
		
		if(attachedTo == null)
			attachedTo = new HashMap<String, String>();
		
		
		equipQtys.put(label, qty);
		equipTypes.put(label, type);
		
		if(attachedTo != null)
			attachedTo.put(label, attachTo);
		
		bUpdated = true;
		return label;
	}

}
