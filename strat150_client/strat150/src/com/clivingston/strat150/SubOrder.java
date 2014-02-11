package com.clivingston.strat150;


/**
 * Lists a single order instance given to a particular unit.
 * 
 * @author chris
 *
 */
public class SubOrder {
	
	public int moveX, moveY;	//Coordinates to move to
	public String targetID;		//Target ID - could be enemy or friendly
	
	public SubOrder()
	{
		//Default values indicate lack of orders
		
		targetID = null;
	}
	
	
	/**
	 * Whether or not this suborder has valid coordinates. 
	 *
	 * @return      True when coordinates are valid.
	 */
	public boolean isMove()
	{
		return moveX >= 0 && moveY >= 0;
	}
		
	/**
	 * Whether or not this suborder has a target. 
	 *
	 * @return      True when this order has a target element.
	 */
	public boolean hasTarget()
	{
		return targetID != null;
	}
	
}
