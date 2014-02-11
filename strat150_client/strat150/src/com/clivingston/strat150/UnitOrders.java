package com.clivingston.strat150;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import com.badlogic.gdx.Gdx;

public class UnitOrders {
	
	public static final String STANCE_DEFAULT = "Default";
	public static final String STANCE_ABORT = "Abort";
	public static final String STANCE_OFFENSIVE = "Offensive";
	public static final String STANCE_DEFENSIVE = "Defensive";
	
	public static final String STANCE_PATROL = "Patrol";
	public static final String STANCE_SCOUT = "Scout";
	public static final String STANCE_HUNT = "Hunt";
	
	ArrayList<SubOrder> subOrders;
	Map<String, SubOrder> targets;  //Associative map of targets
	
	private boolean bChanged;
	private String stance;
	private String unitID;
	
 	protected void log(String str)
	{Gdx.app.log("Orders ["+unitID+"]", str);}
 	
 	/**
 	 * 
 	 * @return Unit orders serialized in JSON format.
 	 */
 	public String serialize()
 	{
 		StringBuffer r = new StringBuffer();
 		
 		r.append("\"");
 		r.append(unitID);
 		r.append("\"");
 		r.append(":");
 		r.append("{");
 		
 		StringBuffer moves = new StringBuffer();
 		StringBuffer targets = new StringBuffer();
 		
 		int moveCount = 0;
 		int targetCount = 0;
 		
 		moves.append("\"Move\" : [");
 		targets.append("\"Target\" : [");
 		
 		//Cycle through each sub-order
 		for(SubOrder order : subOrders)
 		{
 			if(order.hasTarget())
 			{
 				if(targetCount > 0)
 					targets.append(", ");
 				
 				targets.append("\"");		
 				targets.append(order.targetID);
 				targets.append("\"");
 				targetCount++;
 			}
 			
 			if(order.isMove())
 			{
 				if(moveCount > 0)
 					moves.append(", ");
 				
 				moves.append("\"");		
 				moves.append(order.moveX);
 				moves.append(" ");
 				moves.append(order.moveY);
 				moves.append("\"");
 				moveCount++;
 			}	
 		}
 		
 		moves.append("],");
 		targets.append("],");
 		
 		if(moveCount > 0)
 			r.append(moves);
 		if(targetCount > 0)
 			r.append(targets);
 		
 		//Now to append the stance
 		r.append("\"Stance\" : \"");
 		r.append(stance);
 		r.append("\"");
 		
 		r.append("}");
 		
 		log(r.toString());
 		
 		bChanged = false;
 		return r.toString();
 	}
 	
	public UnitOrders(String unitID)
	{
		subOrders = new ArrayList<SubOrder>();
		targets = new HashMap<String, SubOrder>();
		stance = STANCE_DEFAULT;
		this.unitID = unitID;
	}
	
	
	/**
	 * Clears all orders and resets stance
	 */
	public void clearOrders()
	{
		subOrders.clear();
		stance = UnitOrders.STANCE_DEFAULT;
		this.bChanged = true;
	}
	
	/**
	 * Adds a move coordinate to the orders sequence.
	 * @param x Move X coord
	 * @param y Move Y coord
	 */
	public void appendOrder(int x, int y)
	{
		appendOrder(x, y, null);
	}
	
	/**
	 * Adds a move coordinate/target to the orders sequence
	 * @param x Move X coord
	 * @param y Move Y coord
	 * @param targetID Target ID or null
	 */
	public void appendOrder(int x, int y, String targetID)
	{
		SubOrder sub = new SubOrder();
		
		if(x >= 0 && y >= 0)
		{
			sub.moveX = x;
			sub.moveY = y;
		}
		
		if(targetID != null)
		{
			sub.targetID = targetID;
			targets.put(targetID, sub);
		}
		
		this.subOrders.add(sub);
		
		this.bChanged = true;
	}
	
	/**
	 * Adds a single target without move coordinates.
	 * @param targetID Target ID
	 */
	public void appendTarget(String targetID)
	{
		if(targetID == null)
			return;
		
		appendOrder(-1, -1, targetID);
	}
	
	/**
	 * Checks if this set of orders has changed since it was last serialized.
	 * @return True if changed, False if unchanged.
	 */
	public boolean updated()
	{
		return bChanged;
	}
	
	/**
	 * Sets the unit stance.
	 * @param stance The new unit stance.
	 */
	public void setStance(String stance)
	{
		this.stance = stance;
		bChanged = true;
	}	
	
	/**
	 * Updates the order with finished move coordinates, updating waypoints as appropriate.
	 * @param x New X-coordinate
	 * @param y New Y-coordinate
	 */
	public void doneMove(int x, int y)
	{
		//Cannot update if we have no orders
		if(subOrders.size() == 0)
			return;
		
		//Get the first move order
		int i = 0;
		SubOrder currentOrder = subOrders.get(i);
		
		while(i < subOrders.size() && !currentOrder.isMove())
			currentOrder = subOrders.get(i++);
		
		//Doubles as a valid/invalid move order
		if(currentOrder.moveX == x && currentOrder.moveY == y)
		{
			//If this is a patrol loop
			if(this.stance.equals(STANCE_PATROL))
			{
				subOrders.add(currentOrder); //Append to end
				subOrders.remove(i);		//Remove old order
				log("Reached patrol point at ["+x+", "+y+"]");
				bChanged = true;
			}
			else
			{
				log("Reached move point at ["+x+", "+y+"]");
				subOrders.remove(i);		//Remove old order
				bChanged = true;
			}
			
			
		}
		
	}
	
	/**
	 * Notifies this set of orders that the given targetID is dead/destroyed.
	 * @param targetID 
	 */
	public void deadTarget(String targetID)
	{
		//Cannot update if we have no orders
		if(subOrders.size() == 0)
			return;		
		
		SubOrder o = targets.get(targetID);
		
		if(o != null)
			subOrders.remove(o);
	}
}
