package com.clivingston.strat150;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;

/**
 * Contains one rules entry
 * @author Chris Livingston
 *
 */
public class RulesEntry {
	
	private Map<String, String> strings;
	private Map<String, String[]> stringArrays;
	private Map<String, Double> doubles;
	private Map<String, Double[]> doubleArrays;

	public String title;
	
	public RulesEntry()
	{
		strings = new HashMap<String, String>();
		stringArrays = new HashMap<String, String[]>();
		doubles = new HashMap<String, Double>();
		doubleArrays = new HashMap<String, Double[]>();
	}
	
	
	/**
	 * Populates this RulesEntry from a GSON JsonObject
	 * @param entry
	 */
	public void deserialize(JsonObject entry, String title)
	{
		this.title = title;
		
		Set<Map.Entry<String, JsonElement>> objects = entry.entrySet();
		for(Map.Entry<String, JsonElement> object : objects)
		{
			JsonElement element = object.getValue();
			
			//If it's not an array
			if(element.isJsonPrimitive())
			{
				JsonPrimitive p = element.getAsJsonPrimitive();
				
				if(p.isNumber())
					addValue(object.getKey(), p.getAsDouble());
				
				if(p.isString())
					addValue(object.getKey(), p.getAsString());
				
				
			}
			else //Hold on to your butts, it's an array
			{
				JsonArray ar = element.getAsJsonArray();
				
				//Double array
				if(ar.get(0).getAsJsonPrimitive().isNumber())
				{
					Double [] da = new Double[ar.size()];
					
					for(int i = 0; i < ar.size(); i++)
						da[i] = ar.get(i).getAsDouble();
					
					this.doubleArrays.put(object.getKey(), da);
					
				}
				
				//String array
				if(ar.get(0).getAsJsonPrimitive().isString())
				{
					String [] sa = new String[ar.size()];
					
					for(int i = 0; i < ar.size(); i++)
						sa[i] = ar.get(i).getAsString();
					
					this.stringArrays.put(object.getKey(), sa);
				}				
			}
				
			
		}
	}
	
	
	///Getters
	
	public String getString(String key)
	{
		return strings.get(key);
	}
	
	public String[] getStringArray(String key)
	{
		return stringArrays.get(key);
	}
	
	public Double getDouble(String key)
	{
		return doubles.get(key);
	}
	
	public Double[] getDoubleArray(String key)
	{
		return doubleArrays.get(key);
	}
	
	///Keyset getters
	
	public Set<String> getStringKeys()
	{
		return strings.keySet();
	}
	
	public Set<String> getStringArrayKeys()
	{
		return stringArrays.keySet();
	}
	
	public Set<String> getDoubleKeys()
	{
		return doubles.keySet();
	}
	
	public Set<String> getDoubleArrayKeys()
	{
		return doubleArrays.keySet();
	}
	
	/**
	 * Checks if this key is contained in the entry; DOES NOT specify which map the key is contained in.
	 * @param key Key to search for.
	 * @return True if the key is contained anywhere in this entry.
	 */
	public boolean containsKey(String key)
	{
		return strings.containsKey(key) || stringArrays.containsKey(key) 
		    || doubles.containsKey(key) || doubleArrays.containsKey(key);
	}
	
	/**
	 * Removes all instances of specified key.
	 * @param key Key to remove.
	 */
	public void rmKey(String key)
	{
		strings.remove(key);
		stringArrays.remove(key);
		doubles.remove(key);
		doubleArrays.remove(key);
	}
	

	/**
	 * Add a single String value
	 * @param key Key of value
	 * @param value Value to add
	 */
	public void addValue(String key, String value)
	{
		strings.put(key, value);
	}
	
	
	/**
	 * Add a single double value
	 * @param key Key of value
	 * @param value Double to add
	 */
	public void addValue(String key, double value)
	{
		doubles.put(key, value);
	}
	
	/**
	 * Appends a value to an array, if array does not exist it will create one.
	 * @param key Key of array
	 * @param value Value to be appended
	 */
	public void appendValue(String key, String value)
	{
		String [] v = stringArrays.get(key);
		
		//TODO: Find a nicer way to do this
		if(v == null)
		{
			v = new String[1];
			
			v[0] = value;
			stringArrays.put(key, v);
		}
		else
		{
			String [] vn = new String[v.length + 1];
			
			int i = 0;
			for(String s : v)
				vn[i++] = s;
			
			vn[i] = value;
			
			stringArrays.put(key, vn);
		}
	}
	
	/**
	 * Appends a value to an array, if array does not exist it will create one.
	 * @param key Key of array
	 * @param value Value to be appended
	 */
	public void appendValue(String key, double value)
	{
		Double [] v = doubleArrays.get(key);
		
		//TODO: Find a nicer way to do this
		if(v == null)
		{
			v = new Double[1];
			
			v[0] = value;
			doubleArrays.put(key, v);
		}
		else
		{
			Double [] vn = new Double[v.length + 1];
			
			int i = 0;
			for(Double d : v)
				vn[i++] = d;
			
			vn[i] = value;
			
			doubleArrays.put(key, vn);
		}
	}

}
