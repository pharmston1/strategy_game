package com.clivingston.strat150;

/**
 * Client-side representation of a game unit.
 * @author chris
 *
 */
public class Unit extends StratActor {

	double hitpoints;
	double speed;
	
	public float xpos, ypos; //On-screen tweening coords
	float tweenTime;
	float tTimer;
	
	int xActual, yActual; //Actual coordinates
	
	public UnitOrders orders;
	private float xRate;
	private float yRate;
	
	public Unit(Player owner, String type, String id, RulesEntry entry) {
		super(owner, type, id, entry);
		// TODO Auto-generated constructor stub
		
		xActual = -1;
		yActual = -1;
		
		xpos = -1;
		ypos = -1;
		
		tweenTime = 1.0f; //Default tween time
	}
	
	public boolean isVisible()
	{
		//log(xActual+" "+yActual);
		return !(this.isDestroyed() || xActual < 0 || yActual < 0);
	}
	
	
	
	/**
	 * Initializes orders for this unit.
	 */
	public void initOrders()
	{
		orders = new UnitOrders(id);
	}
	
	public void setNewPos(int x, int y)
	{
		if(orders != null)
			orders.doneMove(x, y);
		
		//Make the unit appear if it's not visible
		if(xpos < 0 || ypos < 0)
		{
			xpos = x;
			ypos = y;
		}
		
		xActual = x;
		yActual = y;
		tTimer = 0;
		
		xRate = -(xpos - xActual)/tweenTime;
		yRate = -(ypos - yActual)/tweenTime;
		
		//log("Y-Rate: "+yRate);
	}
	
	/**
	 * Sets the time for the client to slide the unit between points on-screen.
	 * @param t
	 */
	public void setTweenTime(float t)
	{
		tweenTime = t;
	}
	
	public void update(float deltaTime)
	{
		if(tTimer < tweenTime)		//Animate unit to its position
		{
			tTimer += deltaTime;
			
			xpos += deltaTime*xRate;
			ypos += deltaTime*yRate;
			
			//log(deltaTime+" "+1/deltaTime);
			
		}
		else
		{
			xpos = xActual;
			ypos = yActual;
		}
		
	}
	
}
