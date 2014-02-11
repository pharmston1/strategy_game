package com.clivingston.strat150;

import java.util.BitSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.Color;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.google.gson.JsonPrimitive;


//Container class for all in-game stuff
public class GameField {
	private Map<String, Player> players;
	private Map<String, StratActor> actors;
	private Map<String, Unit> units;
	private Map<String, Equipment> equipment;
	private Map<Integer, Unit> mapUnits;		//Addressed by coordinate
	
	private Set<String> alerts;
	
	Set<Unit> visibleUnits;
	Set<Equipment> visibleEquipment;

	Map<String, Message> messages;
	Map<String, StratEvent> events;
	Map<Integer, Map<String, StratEvent>> eventHistory; //Cached by tick number
	
	BitSet clientLOS;		//Client line-of-sight for fog of war
	Orders clientOrders;
	
	
	Player clientPlayer;
	Map<String, Unit> clientUnits;
	Map<String, StratActor> clientInventory; //Items that are in the client's inventory
	
	GameRules rules;
	GameMap map;
	
	private int currentTick;
	private long nextTime;
	private long currentTime;
	
	public String motd;
	
	private String lastDigestChecksum;
	
	
	//Super simple log wrapper
	private void log(String msg)
	{
		Gdx.app.log("Field", msg);
		
	}
	
	private void alert(String msg)
	{
		alerts.add(msg);
	}
	
	/**
	 * Gets an actor by unitID/equipID.
	 * @param id
	 * @return
	 */
	public StratActor getActor(String id)
	{
		return actors.get(id);
	}
	
	/**
	 * Get alerts
	 * @return
	 */
	public Set<String> getAlerts()
	{
		Set<String> ralerts = this.alerts;
		
		alerts = new HashSet<String>();
		return ralerts;
	}
	
	public GameField(GameRules rules, GameMap map, Player client)
	{
		currentTick = -1; //Not yet started
		
		alerts = new HashSet<String>();
		
		this.rules = rules;
		this.map = map;
		clientPlayer = client;
		clientOrders = new Orders(client);
		clientLOS = new BitSet(map.getHeight()*map.getWidth());
		log("LOS size: "+clientLOS.size());
		
		
		players = new HashMap<String, Player>();
		actors = new HashMap<String, StratActor>();
		equipment = new HashMap<String, Equipment>();
		units = new HashMap<String, Unit>();
		messages = new HashMap<String, Message>();
		events = new HashMap<String, StratEvent>();
		eventHistory = new HashMap<Integer, Map<String, StratEvent>>();
		
		visibleUnits = new HashSet<Unit>();
		visibleEquipment = new HashSet<Equipment>();
		mapUnits = new HashMap<Integer, Unit>();

		
	}

	/**
	 * Sets the client player.
	 * @param userID ID of player to set.  Must exist in the field.
	 * @return  True on success, false if the userID is not found.
	 */
	public boolean setClient(String userID)
	{
		return setClient(players.get(userID));
	}
	
	/**
	 * Sets the client player, used for generating the LOS grid.
	 * @param player
	 * @return True on success, false if the player is not defined.
	 */
	public boolean setClient(Player player)
	{
		if(player == null)
			return false;
		
		if(players.containsValue(player))
		{
			this.clientPlayer = player;
			return true;
		}
		else return false;
		
	}
	
	/**
	 * Retrieves the unit at the specified coordinates
	 * @param x 
	 * @param y
	 * @return Unit class if found, NULL if not.
	 */
	public Unit getUnitAt(int x, int y)
	{
		int index = y*map.getWidth() + x;
					
		Unit u = mapUnits.get(index);
		
		if(u != null)
			log("Retrieved "+u.id);
		
		return u;
	}
	
	/**
	 * Moves the selected unitID to the given coordinates - DOES NOT SEND ORDERS
	 * @param unitID
	 * @param x
	 * @param y
	 */
	public void moveUnit(String unitID, int x, int y)
	{
		Unit u = units.get(unitID);
		if(u == null)
		{
			log("UnitID "+unitID+" not found.");
			return;
		}
		
		int oldIndex = u.yActual*map.getWidth()+ u.xActual;
		int newIndex = y*map.getWidth() + x;
		
		
		
		
		mapUnits.remove(oldIndex);
		mapUnits.put(newIndex, u);
		u.setNewPos(x, y);
		 
		
		log("Moved "+u.id+" to map index "+newIndex+" from "+oldIndex);
		
	}
	
	/**
	 * Adds an actor to the field.
	 * @param actor
	 */
	private void addActor(StratActor actor)
	{
		actors.put(actor.id, actor);
		
		if(actor instanceof Unit)
		{
			int x = ((Unit) actor).xActual;
			int y = ((Unit) actor).yActual;
			
			if(x >= 0 && y >= 0)
			{
				mapUnits.put(y*map.getWidth()+x, (Unit) actor);	
			}
			
			units.put(actor.id, (Unit) actor);
		}
		
		if(actor instanceof Equipment)
			equipment.put(actor.id, (Equipment) actor);
	}
	
	/**
	 * Removes a StratActor from all indices. 
	 * @param id
	 */
	private void rmActor(String id)
	{
		//try
		//{
			actors.remove(id);
			units.remove(id);
			equipment.remove(id);
			clientInventory.remove(id);
		//}
		//catch(Exception e)
		//{
			
		//}
	}
	
	//Parses known player information
	private void parsePlayers(JsonObject object)
	{
		//Retrieve the complete entry set
		Set<Map.Entry<String, JsonElement>> objects = object.entrySet();
		
		//Cycle through all JSON objects in the digest
		for(Map.Entry<String, JsonElement> playerEntry : objects)
		if(playerEntry.getValue().isJsonObject())
		{
			String userID = playerEntry.getKey();
			JsonObject playerObj = playerEntry.getValue().getAsJsonObject();
			JsonElement element;
			
			String faction = null;
			element = playerObj.get("faction");
			if(element != null)
				faction = element.getAsString();
			
			if(rules.getEntry("factions", faction) == null)
				log("WARNING: Faction "+faction+" not found in rules for userID "+userID);

			double balance = 0;
			element = playerObj.get("balance");
			if(element != null)
				balance = element.getAsDouble();
			
			String nickname = null;
			try{
			element = playerObj.get("nickname");
			if(element != null)
				nickname = Base64Coder.decodeString(element.getAsString());
			}catch(Exception e)
			{
				log("WARNING:  Malformed nickname for userID "+userID);
				nickname = "Player";
			}
			
			
			
			Player player = players.get(userID);
			
			//Create a new player if it doesn't exist
			if(player == null)
			{
				player = new Player(userID, nickname, faction);
				players.put(userID, player);
			
				log("Adding player "+userID+" - "+nickname+" - "+faction);
			}
			else log("Updating player "+userID+" - "+nickname+" - "+faction);
			
			
			
			try{
				player.balance = playerObj.get("balance").getAsDouble();	
			}catch(Exception e)
			{
				//log("No balance found.");
			}
			
			//Lazily evaluate color
			Color color = new Color(Color.WHITE);
			try{
				JsonArray arr = playerObj.get("color").getAsJsonArray();
				color.r = (float) arr.get(0).getAsDouble();
				color.g = (float) arr.get(1).getAsDouble();
				color.b = (float) arr.get(2).getAsDouble();
				
			}catch(Exception e)
			{
				log("WARNING:  Missing or malformed color information for userID "+userID);
			}
			
			player.nickname = nickname;
			player.color = color;
				
			
			player.setFaction(faction);
			player.balance = balance;
			player.setNickname(nickname);
			
		}
		
	}
	
	private void parseUnits(JsonObject object)
	{
		if(object == null)
		{
			log("No unit information for this tick.");
			return;
		}
		
		//mapUnits = new HashMap<Integer, Unit>();
		
		//Retrieve the complete entry set
		Set<Map.Entry<String, JsonElement>> objects = object.entrySet();
		
		//Cycle through all JSON objects in the digest
		for(Map.Entry<String, JsonElement> unitEntry : objects)
		if(unitEntry.getValue().isJsonObject())
		{
			String unitID = unitEntry.getKey();
			JsonObject unitObj = unitEntry.getValue().getAsJsonObject();
			
			Unit unit;
			JsonElement e;
			
			String userID = null;
			e = unitObj.get("unitOwner");
			if(e != null)
				userID = e.getAsString();
			
			String type = null;
			e = unitObj.get("unitType");
			if(e != null)
				type = e.getAsString();
			
			String posStr = "-1 -1";
			e = unitObj.get("pos");
			if(e != null)
				posStr = e.getAsString();
			
			Player owner = players.get(userID);
			RulesEntry entry = rules.getEntry("units", type);
			
			if(owner == null)
				log("WARNING: userID "+userID+" for unit "+unitID+" has not been sent by the server!");
			
			if(entry == null)
				log("WARNING: Unit "+unitID+" type \""+type+"\" not found in rules file!");
			
			//Update for an existing unit
			if(units.containsKey(unitID))
			{
				unit = units.get(unitID);
				//log("Updating known unit "+unitID);
			}
			else //Create a new unit
			{
				unit = new Unit(owner, type, unitID, entry);
				log("New unit "+unitID);
				addActor(unit);	
			}
			
			//Update the unit's traits
			unit.populateFromJson(unitObj);
			
			//Update the unit's owner
			if(owner != null)
			{
				owner.addItem(unit);
				
				if(owner.userID.equals(clientPlayer.userID))
				{
					log("Unit "+unitID+" belongs to client.");
					clientPlayer.addItem(unit);
				}
			}
			else log("Can't find owner for unit.");
				
			
			String [] p = posStr.split(" ");
			
			//Set coordinates
			int x = Integer.parseInt(p[0]);
			int y = Integer.parseInt(p[1]);
			
			if(unit.xActual != x || unit.yActual != y)
			{
				this.moveUnit(unitID, x, y);
				if(unit.isVisible())
					log("Unit "+unitID+" now at "+x+", "+y);
			}

			

			
			//Update the unit's last tick time
			unit.lastTick = this.currentTick;
			
			
		}
	}
	
	private void parseEquipment(JsonObject object)
	{
		if(object == null)
		{
			log("No equipment information for this tick.");
			return;
		}
		
		//Retrieve the complete entry set
		Set<Map.Entry<String, JsonElement>> objects = object.entrySet();
		
		//Cycle through all JSON objects in the digest
		for(Map.Entry<String, JsonElement> equipEntry : objects)
		if(equipEntry.getValue().isJsonObject())
		{
			String equipID = equipEntry.getKey();
			JsonObject equipObj = equipEntry.getValue().getAsJsonObject();
			
			Equipment equip;
			JsonElement e;
			
			String userID = null;
			e = equipObj.get("equipOwner");
			if(e != null)
				userID = e.getAsString();
			
			String type = null;
			e = equipObj.get("equipType");
			if(e != null)
				type = e.getAsString();
			
			Player owner = players.get(userID);
			RulesEntry entry = rules.getEntry("equipment", type);
			
			if(owner == null)
				log("WARNING: userID "+userID+" for equipment "+equipID+" has not been sent by the server!");
			
			if(entry == null)
				log("WARNING: Equipment "+equipID+" type \""+type+"\" not found in rules file!");
			
			//Update for an existing unit
			if(equipment.containsKey(equipID))
			{
				equip = equipment.get(equipID);
			}
			else //Create a new unit
			{
				equip = new Equipment(owner, type, equipID, entry);
				addActor(equip);
			}
			
			//Update the unit's traits
			equip.populateFromJson(equipObj);
			
			//Update the unit's owner
			if(owner != null)
				owner.addItem(equip);
			
			//Update the unit's last tick time
			equip.lastTick = this.currentTick;
		}
	}
	
	//Parses messages
	private void parseMessages(JsonObject object)
	{
		try{
		//Retrieve the complete entry set
		Set<Map.Entry<String, JsonElement>> objects = object.entrySet();
		
		//Cycle through all JSON objects in the digest
		for(Map.Entry<String, JsonElement> msgEntry : objects)
		if(msgEntry.getValue().isJsonObject())
		{
			Message msg = new Message();
			String msgID = msgEntry.getKey();
			JsonObject msgObj = msgEntry.getValue().getAsJsonObject();
			
			//Lazily parse it for now
			try{
				msg.recipient = players.get(msgObj.get("to").getAsString());
				msg.sender = players.get(msgObj.get("from").getAsString());
				msg.body = msgObj.get("body").getAsString();
				log("Message ["+msgID+"], from "+msg.sender.userID+" to "+msg.recipient.userID);
				messages.put(msgID, msg);
				
			}catch(Exception e)
			{
				log("WARNING:  Unable to fully parse message.  Is there an unidentified player?");
				//e.printStackTrace();
			}
			
			
		}
		}catch(Exception e)
		{
			log("No messages...");
			
		}
		
			
	}
	
	//Parses all events
	private void parseEvents(JsonObject object)
	{
		//Create a new event history
		events = new HashMap<String, StratEvent>();
		eventHistory.put(this.currentTick, events);
		
		//Retrieve the complete entry set
		Set<Map.Entry<String, JsonElement>> objects = object.entrySet();
		
		//Cycle through all JSON objects in the digest
		for(Map.Entry<String, JsonElement> eventEntry : objects)
		if(eventEntry.getValue().isJsonObject())
		{
			StratEvent event = new StratEvent();
			JsonObject eventObj = eventEntry.getValue().getAsJsonObject();
			String eventID = eventEntry.getKey();
			
			JsonElement element;
			JsonArray arr;
			
			element = eventObj.get("alert");
			if(element != null)
			{
				event.alertString = element.getAsString();
				alert(event.alertString);
			}
			
			element = eventObj.get("actor");
			if(element != null)
			{
				event.instigator = element.getAsString();
				
				if(actors.containsKey(event.instigator))
					log(eventID+" instigated by unit/equipment "+event.instigator);
				else if(players.containsKey(event.instigator))
					log(eventID+" instigated by player "+event.instigator);
				else
					log(eventID+" instigated by unknown "+event.instigator);
			}
			
			element = eventObj.get("destroyed");
			if(element != null)
			{
				arr = element.getAsJsonArray();
				for(int i = 0; i < arr.size(); i++)
				{
					String actorID = arr.get(i).getAsString();
					StratActor actor = actors.get(actorID);
					event.destroyed.add(actor);
					
					if(actor != null)
					{
						log(actor.id + " destroyed");
						actor.destroy();
					}
					else log("Cannot find destroyed "+actorID);
				}
			}
			
			element = eventObj.get("damaged");
			if(element != null)
			{
				arr = element.getAsJsonArray();
				for(int i = 0; i < arr.size(); i++)
				{
					String actorID = arr.get(i).getAsString();
					StratActor actor = actors.get(actorID);
					event.damaged.add(actor);
					
					if(actor != null)
					{
						log(actor.id + " damaged");
					}
					else log("Cannot find damaged "+actorID);
				}
			}
			
			element = eventObj.get("affected");
			if(element != null)
			{
				arr = element.getAsJsonArray();
				for(int i = 0; i < arr.size(); i++)
				{
					String actorID = arr.get(i).getAsString();
					StratActor actor = actors.get(actorID);
					event.affected.add(actor);
					
					if(actor != null)
					{
						log(actor.id + " affected");
						actor.destroy();
					}
					else log("Cannot find affected "+actorID);
				}
			}
			
		}
	}
	
	//Logs transfers and tracks alliances and what-not
	private void parseDiplomacy(JsonObject object)
	{
		try
		{
			JsonElement element;
			JsonArray arr;
			
			element = object.get("allies");
			if(element != null)
			{
				arr = element.getAsJsonArray();
				
				for(int i = 0; i < arr.size(); i++)
				{
					String [] s = arr.get(i).getAsString().split(" ");
					
					Player a = players.get(s[0]);
					Player b = players.get(s[1]);
					
					if(a == null || b == null)
					{
						log("Cannot find players for alliance from "+s[0]+" to "+s[1]);
					}
					else
					{
						a.setAlly(b);
						log(a.nickname+" has allied with "+b.nickname);
					}	
				}
			}
			
			//Couldn't remember the greatest block in the world, this is just a tribute
			element = object.get("tribute");
			if(element != null)
			{
				arr = element.getAsJsonArray();
				
				for(int i = 0; i < arr.size(); i++)
				{
					String [] s = arr.get(i).getAsString().split(" ");
					
					Player a = players.get(s[0]);
					Player b = players.get(s[1]);
					String itemID = s[2];
					StratActor item = null;
					double amount = 0;
					
					
					try{//Attempt to convert it
						amount = Double.parseDouble(itemID);
					}catch(NumberFormatException e) //It's not an amount
					{
						item = actors.get(itemID);
					}
					
					if(a == null || b == null)
					{
						log("Cannot find players for trasfer from "+s[0]+" to "+s[1]);
					}
					else
					{
						if(amount > 0)
						{
							log("Transferring "+amount+" credits from "+a.nickname+" to "+b.nickname);
							//TODO:  Transaction record
						}
						
						if(item != null)
						{
							log("Transferring "+item.id+" from "+a.nickname+" to "+b.nickname);
							//TODO:  Transaction record
						}
					}	
				}
			}
			
		}catch(Exception e)
		{
			log("WARNING: Malformed diplomacy block!");
		}
	}
	
	
	/**
	 * Parses the info block of the digest
	 * @param object
	 * @return Success status
	 */
	private boolean parseInfo(JsonObject object)
	{
		try{
			int incomingTick = object.get("tick").getAsInt();
		
			//This tick is out of date, ignore it
			/*if(incomingTick < this.currentTick)
			{
				log("Digest rejected - current ["+currentTick+"], received ["+incomingTick+"]");
				return false;
			}*/
			log("Processing tick #"+incomingTick);
			
			long incomingNext = (long) object.get("next").getAsDouble();
			long incomingTime = (long) object.get("time").getAsDouble();
			int delta = (int) (incomingTime - currentTime);
			
			if(delta >= 0)
				log("Time delta: "+delta+"s  Next estimated delta: "+(incomingNext - incomingTime)+"s");
			else
				log("Warning! Timestamp received is earlier than than previous value.");
			
			
			motd = object.get("motd").getAsString();
			log("MOTD: "+motd);
			
			currentTick = incomingTick;
		
		}catch(Exception e){
			e.printStackTrace();
			log("Error in digest.");
			return false;
		}
		
		return true;

	}
	
	
	// Refreshes attachments, etc
	private void refreshActors()
	{	
		this.visibleEquipment.clear();
		this.visibleUnits.clear();
		
		for(StratActor actor : actors.values())
		{
			//log("Checking "+actor.id);
			
			if(actor instanceof Unit)
				if(((Unit)actor).isVisible())
					visibleUnits.add((Unit)actor);
			
			//Refresh attachments
			String [] attIDs = actor.getStringArray("equipment");
			if(attIDs != null)
			{
				for(String attKey : attIDs)
				{
					StratActor attachment = actors.get(attKey);
					if(attachment != null)
					{
						//If attaching failed
						if(!actor.attachChild(attachment))
						{
							attachment.detachParent();
							actor.attachChild(actor);
						}
						
					}
					else log("Cannot equip attachment ["+attKey+"] to "+actor.id+" as no entry exists for it.");
				}
			}
		}
		
		//Refresh visibility set
		for(Equipment eq : equipment.values())
		{
			Unit p = eq.getParentUnit();
			
			if(p != null && p.isVisible())
				visibleEquipment.add(eq);
		}
			

		
		
	}
	
	
	
	/**
	 * Gets tile visibility at a given location
	 * @param x World X-coordinate
	 * @param y World Y-coordinate
	 * @return True if visible to client, false if not.
	 */
	public boolean getVisibility(int x, int y)
	{
		//Projecting 2-dimensional space into 1-dimensional array
		int n = x+y*map.getWidth();
		
		if(n > clientLOS.size() || n < 0)
			return false;
		
		return clientLOS.get(n);
	}
	
	/**
	 * Sets the visibility of a given location - DOES NOT REVEAL INFORMATION NOT SENT BY THE SERVER!
	 * @param x
	 * @param y
	 * @param value True if visible to the client, false if not.
	 */
	public void setVisibility(int x, int y, boolean value)
	{
		//Projecting 2-dimensional space into 1-dimensional array
		int n = x+y*map.getWidth();
		
		
		if(n >= clientLOS.size() || n < 0)
			return;
		
		
		//log(x+", "+y+" : "+n);
		clientLOS.set(n, value);
	}
	
	//Parses client-side LOS across the map
	private void updateClientLOS()
	{
		
		//If no client player is specified, make everything visible
		if(clientPlayer == null)
		{
			//clientLOS.clear();
			clientLOS.set(0, clientLOS.size() - 1);
			return;
		}
		
		clientLOS.clear();
		int n = 0;
		//Go over all player-owned units
		for(Unit unit : units.values())
			if(unit.owner == clientPlayer && unit.isVisible())
			{
				int radius = unit.getDouble("SightRange").intValue() + unit.getDouble("SightBonus").intValue();
				
				//The square occupied by the unit is ALWAYS visible
				setVisibility(unit.xActual, unit.yActual, true);
				circleLOS(unit.xActual, unit.yActual, radius);
				n++;
			}
		
		log("Line-of-sight: "+n+" unit(s) calculated");
		
	}
	
	
	//TODO:  A more generic implementation of this
	public void circleLOS(int x0, int y0, int radius)
	{
		int r2 = radius * radius;
		for(int y=-radius; y<=radius; y++)
		{
			int y2 = y*y;
			
		    for(int x=-radius; x<=radius; x++)
		    {
		    	if(x*x+y2 < r2)
		            setVisibility(x0+x, y0+y, true);
		    }
		}
	}
	
	

	/**
	 * Parses a received digest string from the server
	 * @param digestJson
	 * @return True if successful, false if fatal errors were encountered.
	 */
	public boolean parseDigest(String digestJson)
	{
		log("Raw digest: "+digestJson);
		
		log("=BEGIN DIGEST===============================================");
		JsonParser parser = new JsonParser();
		JsonObject root;
		
		//String [] s = digestJson.split("\n");
		
		//if(s.length > 1)
		//	digestJson = Base64Coder.decodeString(s[1]);
		
		try{
			root = parser.parse(digestJson).getAsJsonObject();	
		}catch(Exception e) //Parsing failed
		{
			log(digestJson);
			log(e.getMessage());
			log("=FAILED=====================================================");
			return false;
		}
		
		JsonObject infoObj = null, 
		           playerObj = null, 
		           unitObj = null, 
		           equipObj = null, 
		           msgObj = null, 
		           eventObj = null, 
		           dipObj = null;
		
		//Retrieve the complete entry set
		Set<Map.Entry<String, JsonElement>> objects = root.entrySet();
		for(Map.Entry<String, JsonElement> object : objects)
		{
			JsonElement entry = object.getValue();
			if(entry.isJsonObject())
			{
				String key = object.getKey();
				
				if(key.equals("info"))
					infoObj = entry.getAsJsonObject();
				
				if(key.equals("units"))
					unitObj = entry.getAsJsonObject();
				
				if(key.equals("players"))
					playerObj = entry.getAsJsonObject(); 
				
				if(key.equals("equipment"))
					equipObj = entry.getAsJsonObject();
				
				if(key.equals("diplomacy"))
					dipObj = entry.getAsJsonObject();
				
				if(key.equals("messages"))
					msgObj = entry.getAsJsonObject(); 
				
				if(key.equals("events"))
					eventObj = entry.getAsJsonObject();
			}
			
		}
		
		try{
		if(parseInfo(infoObj))
			{
				log("=PLAYERS====================================================");
				parsePlayers(playerObj);
				log("=DIPLOMACY==================================================");
				parseDiplomacy(dipObj);
				
				log("=UNITS======================================================");
				parseUnits(unitObj);
				log("=EQUIPMENT==================================================");
				parseEquipment(equipObj);
				
				log("=ATTACHMENTS================================================");
				refreshActors(); //Must be updated before destruction events wipe everything out
				updateClientLOS(); //Note that this occurs before units are potentially destroyed!
				
				log("=EVENTS=====================================================");
				parseEvents(eventObj);
				log("=MESSAGES===================================================");
				parseMessages(msgObj);
				
				log("=SUCCESS====================================================");
				log("Tick "+currentTick+":  "+players.size()+" players, "+units.size()+" units, "+equipment.size()+" equipment");
				log("Player units "+clientPlayer.units.size());
				return true;
			}
		}catch(Exception e)
		{
			e.printStackTrace();
			log("Some other sort of horrible error has occurred while parsing the digest.");
		}
		
		log("=FAILED=====================================================");
		return false;
	}
	
	public void update(float deltaTime)
	{
		
		//Update unit motions/animations
		for(Unit u : units.values())
		{
			u.update(deltaTime);
		}
	}

}
