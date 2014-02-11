package com.clivingston.strat150;

import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.Color;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParseException;
import com.google.gson.JsonParser;


/**
 *
 * The GameRules class contains a master list of all units,
 * modules, equipment, terrain types, movement types and just
 * about every other facet of actual gameplay.  
 * 
 * The rules themselves are loaded via a JSON string, presumably
 * downloaded from the server.
 * 
 * @author Chris Livingston
 */
public class GameRules {

	//
	public Map<String, Tile> tileSet;
	
	private Map<String, RulesEntry> singleEntries;
	private Map<String, Map<String, RulesEntry> > groupedEntries;
	
	private void log(String msg)
	{
		Gdx.app.log("Rules", msg);
	}
	
	public GameRules()
	{
		tileSet = new HashMap<String, Tile>();
		singleEntries = new HashMap<String, RulesEntry>();
		groupedEntries = new HashMap<String, Map<String, RulesEntry> >();
	}
	
	/**
	 * Gets a RulesEntry class contained within.
	 * @param groupKey Group containing entry, or null for single entries.
	 * @param entryKey Entry key.
	 * @return RulesEntry class if found, null if not.
	 */
	public RulesEntry getEntry(String groupKey, String entryKey)
	{
		if(groupKey == null)
			return singleEntries.get(entryKey);
		
		if(!groupedEntries.containsKey(groupKey))
			return null;
		
		Map<String, RulesEntry> group = groupedEntries.get(groupKey);
		
		return group.get(entryKey);
	}
	

	/**
	 * 
	 * @return All group keys.
	 */
	public Set<String> getGroupKeys()
	{
		return groupedEntries.keySet();
	}
	
	/**
	 * 
	 * @param groupKey Desired group key or NULL for entry keys.
	 * @return Entry keys or NULL if not found.
	 */
	public Set<String> getEntryKeys(String groupKey)
	{
		//Single entries
		if(groupKey == null)
			return singleEntries.keySet();
		
		//Grouped entries
		if(groupedEntries.containsKey(groupKey))
			return groupedEntries.get(groupKey).keySet();
		else
			return null;
				
	}
	
	///Helper function - checks if a given JSON object is a rules group containing other objects
	private boolean isRulesGroup(JsonObject root)
	{
		Set<Map.Entry<String, JsonElement>> objects = root.entrySet();
		for(Map.Entry<String, JsonElement> object : objects)
		{
			if(object.getValue().isJsonObject())
				return true;
		}
		
		return false;
	}
	
	/**
	 * Populates this object using the contents of a rules.json file.
	 * @param rulesJson Rules as raw JSON string.
	 * @return True on success, false on failure.
	 */
	public boolean parseRules(String rulesJson)
	{
		JsonParser parser = new JsonParser();
		JsonObject root;
		
		
		try{
			root = parser.parse(rulesJson).getAsJsonObject();	
		}catch(Exception e) //Parsing failed
		{
			log(rulesJson);
			log(e.getMessage());
			return false;
		}
		
		//Retrieve the complete entry set
		Set<Map.Entry<String, JsonElement>> objects = root.entrySet();
		
		for(Map.Entry<String, JsonElement> object : objects)
		{
			JsonElement entry = object.getValue();
			
			
			if(entry.isJsonObject())
			{
			//Single entry
			if(!isRulesGroup(entry.getAsJsonObject()))
				{
					RulesEntry re = new RulesEntry();
					re.deserialize(entry.getAsJsonObject(), object.getKey());
					this.singleEntries.put(object.getKey(), re);
					log("Added single entry "+object.getKey());
				}
			else //Grouped entries
				{
					//Cycle through each grouped entry
					String groupKey = object.getKey();
					
					//Allocate the group if needed
					if(!groupedEntries.containsKey(groupKey))
						groupedEntries.put(groupKey, new HashMap<String, RulesEntry>());
					
					for(Map.Entry<String, JsonElement> groupObject : entry.getAsJsonObject().entrySet())
					{
						JsonElement groupEntry = groupObject.getValue();
						String entryKey = groupObject.getKey();
							
						//Sanity check - otherwise non-object entries will crash the parser
						if(entry.isJsonObject())
						{
							RulesEntry re = new RulesEntry();
							re.deserialize(groupEntry.getAsJsonObject(), entryKey);
							groupedEntries.get(groupKey).put(entryKey, re);
							log("Added entry "+entryKey+" in group "+groupKey);
						}
					}
				}
			}
		}
		
		return true;
	}
	
	
}
