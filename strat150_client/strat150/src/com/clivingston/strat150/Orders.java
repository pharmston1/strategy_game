package com.clivingston.strat150;

import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.badlogic.gdx.graphics.Color;


//Master orders container
public class Orders {
	
	Set<Message> messages;
	Set<Drop> drops;
	Set<BuyOrder> buyOrders;
	Set<UnitOrders> unitOrders;
	
	Player player;
	
	boolean updated;
	
	public Orders(Player p)
	{
		messages = new HashSet<Message>();
		drops = new HashSet<Drop>();
		buyOrders = new HashSet<BuyOrder>();
		unitOrders = new HashSet<UnitOrders>();
		
		player = p;
		updated = true;
	}
	
	public void addDrop(String tag, int qty, int x, int y)
	{
		if(x < 0 || y < 0)
			return;
		
		Drop drop = new Drop();
		
		drop.qty = qty;
		drop.unitID = tag;
		drop.xpos = x;
		drop.ypos = y;
		
		updated = true;
		drops.add(drop);
		
	}
	
	//Internal method that generates the info block
	private String serializeInfoBlock()
	{
		String s = "\"Info\" : { ";
		
		s += "\"userID\" : \"" + player.getUserID() + "\"," +
		     "\"Nickname\" : \"" + Base64Coder.encodeString(player.getNick()) + "\"," +
		     "\"Faction\" : \"" + player.getFaction() + "\"," +
		     "\"Color\" : [" + player.color.r + "," + player.color.g + "," + player.color.b + "]";
		
		s += "},";
		
		return s;
	}
	
	//Internal method to generate unit orders
	private String serializeUnitOrders()
	{
		if(unitOrders.size() == 0)
			return "";
		
		StringBuilder buf = new StringBuilder();

		buf.append(" "); //This space might or might not get deleted
	
		
		for(UnitOrders order : unitOrders)
		if(order.updated())
		{
			buf.append(order.serialize());
			buf.append(",");
		}
		
		//try{
		buf.deleteCharAt(buf.length()-1); //Get rid of that comma
		//}catch(Exception e){}
		
		
		return buf.toString();
	}
	
	//Internal method to generate purchase orders
	private String serializeBuyOrders()
	{
		if(buyOrders.size() == 0)
			return "";
		
		StringBuilder buf = new StringBuilder();

		buf.append("\"Buy\" : { "); //This space might or might not get deleted
		
		boolean skip = true;
		for(BuyOrder order : buyOrders)
		if(order.updated())
		{
			buf.append(order.serialize());
			buf.append(",");
			skip = false;
		}
		
		if(skip) //All buy orders are bunk
			return "";
		
		//try{
		buf.deleteCharAt(buf.length()-1); //Get rid of that comma
		//}catch(Exception e){}
		
		
		buf.append("},");
		
		return buf.toString();
	}	
	
	//internal method to generate unit drops
	private String serializeDrops()
	{
		if(drops.size() == 0)
			return "";
		
		StringBuilder buf = new StringBuilder();
		
		buf.append("\"Drop\" : { "); //This space might or might not get deleted
		
		
		for(Drop drop : drops)
		{
			buf.append(drop.serialize());
			buf.append(",");
		}
		
		//try{
		buf.deleteCharAt(buf.length()-1); //Get rid of that comma
		//}catch(Exception e){}
		
		
		buf.append("},");
		
		drops.clear();
				
		return buf.toString();
	}
	
	
	/**
	 * Generates a fresh buy order.
	 * @return The new buy order.
	 */
	public BuyOrder buy()
	{
		BuyOrder bo = new BuyOrder();
		
		if(buyOrders == null)
			buyOrders = new HashSet<BuyOrder>();
		
		buyOrders.add(bo);
		
		updated = true;
		return bo;
	}
	
	/**
	 * Attempts to add unit orders to the object. 
	 * @param order Unit orders to add.
	 * @return True if units have been updated, false if they have not been updated.
	 */
	public boolean addUnitOrder(UnitOrders order)
	{
		if(!order.updated())
			return false;
		
		unitOrders.add(order);
		
		updated  = true;
		return true;
	}
	
	
	/**
	 * Serializes the entire orders class as JSON, ready to send to the server.
	 * @return Serialized orders, or NULL if nothing has changed.
	 */
	public String serialize()
	{
		if(!updated)
			return null;
		
		StringBuilder buf = new StringBuilder();
		
		buf.append("{");
		
		buf.append(serializeInfoBlock());
		buf.append(serializeBuyOrders());
		buf.append(serializeDrops());
		buf.append(serializeUnitOrders());
		
		if(buf.charAt(buf.length() - 1) == ',')
			buf.deleteCharAt(buf.length() - 1);
		
		buf.append("}");
		
		updated = false;
		return buf.toString();
	}


	//Drop orders type nested class
	private class Drop{
		public String unitType;
		public String unitID;
		public int qty;
		public int xpos, ypos;
		
		public Drop()
		{
			//Initialize to null drop
			qty = -1;
			xpos = -1;
			ypos = -1;
			unitType = null;
			unitID = null;
		}
		
		
		/**
		 * Serializes to JSON.
		 * @return Serialized string, NULL if invalid.
		 * */
		public String serialize()
		{
			if(unitType == null && unitID == null)
				return null;
			
			if(qty < 1 || xpos < 0 || ypos < 0)
				return null;
			
			String s = null;
			
			if(unitID != null)
				s = "\""+unitID+"\" : ["+qty+", "+xpos+", "+ypos+"]";
			else
				s = "\""+unitType+"\" : ["+qty+", "+xpos+", "+ypos+"]";
			
			return s;
			
		}
		
		
	}
	

}
