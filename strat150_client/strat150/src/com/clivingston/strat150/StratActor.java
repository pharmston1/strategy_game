package com.clivingston.strat150;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.g2d.Sprite;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;


//StratActor - corresponds with simulator actor
public abstract class StratActor {
	
	Player owner;
	String type;
	String id;
	
	int lastTick;
	
	Set<StratActor> attached;
	Set<StratActor> freshAttached; //For attach orders
	//Unit host;
	StratActor parent;
	
	int quantity;
	boolean bApplySingle; //For ammo/consumables
	
	Map<String, Trait> traits;
	Map<String, Double> cachedTraits; //For when calculations have already been done 
	
	private String mapSpriteKey;
	private Sprite mapSprite;
	private String iconSpriteKey;
	private Sprite iconSprite;
	
	private boolean bDestroyed;
	
	private RulesEntry defaults;
	private RulesEntry modified;
	
	private String logtag;
	protected void log(String msg)
	{	
		if(logtag == null)
			logtag = "["+id+"] "+type;
		
		
		Gdx.app.log(logtag, msg);
	}
	
	public StratActor(Player owner, String type, String id, RulesEntry entry)
	{
		
		
		this.defaults = entry;
		this.owner = owner;
		this.type = type;
		this.id = id;
		attached = new HashSet<StratActor>();
		freshAttached = new HashSet<StratActor>();
		traits = new HashMap<String, Trait>();
		cachedTraits = new HashMap<String, Double>();
		modified = new RulesEntry();
		bDestroyed = false;
	}
	
	public boolean use(int qty)
	{
		if(quantity == 0)
			return false;
		
		quantity -= qty;
		
		return true;
	}
	
	public boolean isDestroyed()
	{
		return bDestroyed;
	}
	
	
	/**
	 * Flags this actor and all of its children as destroyed.
	 */
	public void destroy()
	{
		if(bDestroyed)
			return;
		
		for(StratActor c : attached)
			c.destroy();
		
		bDestroyed = true;
	}
	
	//Returns true if the object specified is safe to attach in any way
	protected boolean isSafe(StratActor other)
	{
		if(getNetwork(null).contains(other))
			return false;
			
		return true;
	}
	
	//Recursively builds a set of all items in this set.
	protected Set<StratActor> getNetwork(Set<StratActor> items)
	{
		if(items == null)
			items = new HashSet<StratActor>();
		
		items.add(this);
		
		if(parent != null && !items.contains(parent))
			parent.getNetwork(items);
		
		for(StratActor child : attached)
		{
			if(!items.contains(child))
				child.getNetwork(items);
		}
		
		return items;
	}
	
	/**
	 * Attaches a parent actor to this one.
	 * @param other The other actor to attach to.
	 * @return True on success, false if this actor already has a parent.
	 */
	public boolean attachParent(StratActor other)
	{
		if(parent != null)
			return false;
		
		if(!isSafe(other))
			return false;
		
		parent = other;
		owner = parent.owner;
		parent.attached.add(this);
		parent.freshAttached.add(this);
		
		return true;
	}
	
	/**
	 * Attaches another actor as a child.
	 * @param other Child actor to attach.
	 * @return True on success, false if unable to attach.
	 */
	public boolean attachChild(StratActor other)
	{
		if(other == null)
			return false;
		
		if(parent == other)
			return false;
		
		if(other.parent != null)
			return false;
		
		return other.attachParent(this);

	}
	
	/**
	 * Detaches this actor from its parent.
	 */
	public void detachParent()
	{
		if(parent != null)
			parent.detachChild(this);
		
		parent = null;
	}
	
	/**
	 * Detaches the specified child from this actor.
	 * @param child
	 * @return True if this child was actually attached in the first place.
	 */
	public boolean detachChild(StratActor child)
	{
		if(this.freshAttached.contains(child))
			freshAttached.remove(child);
		
		if(!attached.contains(child))
		{
			attached.remove(child);
			child.detachParent();
		}
		else
			return false;
		
		return true;
	}
	
	/**
	 * 
	 * @return The map sprite, or NULL if unavailable.
	 */
	public Sprite getMapSprite()
	{
		if(mapSprite == null)
		{
			mapSprite = Assets.getSprite(mapSpriteKey);
		}
		
		return mapSprite;
	}
	
	/**
	 * 
	 * @return The icon sprite, or NULL if unavailable.
	 */
	public Sprite getIconSprite()
	{
		if(iconSprite == null)
		{
			iconSprite = Assets.getSprite(iconSpriteKey);
		}
		
		return iconSprite;
	}
	
	public void clearCache()
	{
		this.cachedTraits.clear();
		
		if(parent != null)
			parent.clearCache();
	}
	
	/**
	 * Adds a trait that will override the default rules value for this actor.
	 * @param key Key to add.
	 * @param value Value to add.
	 */
	public void addTrait(String key, String value)
	{
		this.modified.addValue(key, value);
	}

	/**
	 * Adds a trait that will override the default rules value for this actor.
	 * @param key Key to add.
	 * @param value Value to add.
	 */
	public void addTrait(String key, double value)
	{
		this.modified.addValue(key, value);
	}
	
	/**
	 * Adds a trait that will override the default rules value for this actor.
	 * @param key Key to add.
	 * @param value Value to add.
	 */
	public void addTrait(String key, String [] arrayValue)
	{
		this.modified.rmKey(key);
		
		for(String s : arrayValue)
			modified.appendValue(key, s);
	}
	
	/**
	 * Adds a trait that will override the default rules value for this actor.
	 * @param key Key to add.
	 * @param value Value to add.
	 */
	public void addTrait(String key, Double [] arrayValue)
	{
		this.modified.rmKey(key);
		
		for(Double d : arrayValue)
			modified.appendValue(key, d);
	}
	
	
	/**
	 * Gets a double trait from this actor.
	 * @param key
	 * @return
	 */
	public Double getDouble(String key)
	{
		Double d;
		
		d = modified.getDouble(key);
		
		if(d == null)
			d = defaults.getDouble(key);
		
		if(d == null)
			return 0.0;
		
		return d;
	}
	
	/**
	 * Gets a double array trait from this actor.
	 * @param key
	 * @return
	 */
	public Double [] getDoubleArray(String key)
	{
		Double [] da;
		
		da = modified.getDoubleArray(key);
		
		if(da == null)
			da = defaults.getDoubleArray(key);
		
		return da;
	}
	
	
	
	/**
	 * Gets a string trait from this actor.
	 * @param key
	 * @return
	 */
	public String getString(String key)
	{
		String s;
		
		s = modified.getString(key);
		
		if(s == null)
			s = defaults.getString(key);
		
		return s;
	}
	
	/**
	 * Gets a string array trait from this actor.
	 * @param key
	 * @return
	 */
	public String [] getStringArray(String key)
	{
		String [] sa;
		
		sa = modified.getStringArray(key);
		
		if(sa == null && defaults != null)
			sa = defaults.getStringArray(key);
		
		return sa;
	}
	
	/**
	 * Populates all modified traits from a digest source.
	 * @param src  JsonObject containing traits for this actor.
	 */
	public void populateFromJson(JsonObject src)
	{
		int n = 0;
		
		Set<Map.Entry<String, JsonElement>> traits = src.entrySet();
		for(Map.Entry<String, JsonElement> actorTrait : traits)
		{
			String traitKey = actorTrait.getKey();
			

			
			//It's an array
			if(actorTrait.getValue().isJsonArray())
			{
				JsonArray arr = actorTrait.getValue().getAsJsonArray();
				if(arr.size() > 0)
				if(arr.get(0).isJsonPrimitive())
				{
					JsonPrimitive first = arr.get(0).getAsJsonPrimitive();
					
					if(first.isNumber()) //It's a double array
					{
						Double [] da = new Double[arr.size()];
						
						for(int i = 0; i < arr.size(); i++)
							da[i] = arr.get(i).getAsDouble();
						
						this.addTrait(traitKey, da);
						n++;
					}
					else if(first.isString()) //It's a string array
					{
						String [] sa = new String[arr.size()];
						
						for(int i = 0; i < arr.size(); i ++)
							sa[i] = arr.get(i).getAsString();
						
						this.addTrait(traitKey, sa);
						n++;
					}
				}
				
				//else Array is size zero or contains objects, ignore it

			}
			else if(actorTrait.getValue().isJsonPrimitive())//IT MUST BE A SINGLE VALUE THEN
			{
				JsonPrimitive value = actorTrait.getValue().getAsJsonPrimitive();
				
				if(value.isNumber())
					{
						this.addTrait(traitKey, value.getAsDouble());
						n++;
					}
				else if(value.isString())
					{
						this.addTrait(traitKey, value.getAsString());
						n++;
					}
				
			}
			
			//else It's an object, ignore it
		}
		
		//log(n+" trait(s) parsed.");
	}
	
	//TODO: GET RID OF THE OTHER TRAIT SYSTEM TO BRING THE CLIENT IN LINE WITH THE SIMULATOR//////////////
	
	/**
	 * Checks if a trait is posessed by this actor.
	 * @param traitType String ID of the trait.
	 * @param includeChildren If true, will also recursively search all children for the trait.
	 * @return True if found, false if not.
	 */
	public boolean hasTrait(String traitType, boolean includeChildren)
	{
		
		if(!includeChildren)
			return traits.containsKey(traitType);
		
		if(traits.containsKey(traitType))
			return true;
		
		//Check children (recursively, if possible)
		for(StratActor child : attached)
			if(child.hasTrait(traitType, true))
				return true;
		
		//Not found
		return false;	
	}
	
	
	/**
	 * Gets the trait value of this actor, optionally calculated.
	 * @param traitType Trait ID
	 * @param calculated If true, will recursively calculate trait value based on all child members.
	 * @return 0 if trait not found, otherwise trait value
	 */
	public double getTraitValue(String traitType, boolean calculated)
	{
		double value = 0;
		
		Trait thisTrait = traits.get(traitType);
		
		if(thisTrait != null)
			value = thisTrait.value;
		
		//Straight up return the value if we're not looking for calculated or it ignores children
		if(!calculated || (thisTrait != null && thisTrait.relation == Trait.TYPE_IGNORE))
			return value;
		
		if(this.cachedTraits.containsKey(traitType))
			return cachedTraits.get(traitType);
		
		Set<Trait> childTraits = new HashSet<Trait>();
		
		for(StratActor child : attached)
		{
			if(child.hasTrait(traitType, true))
			{
				//Recursively calculate child trait values
				Trait t = new Trait();
				t.value = child.getTraitValue(traitType, true);
				childTraits.add(t);
			}
			
		}
		
		//Default to adding if no other value is available
		if(thisTrait == null)
		{
			thisTrait = new Trait();
			thisTrait.value = value;
			thisTrait.relation = Trait.TYPE_ADD;
		}
		
		return thisTrait.calculate(childTraits);
		
	}
	

}
