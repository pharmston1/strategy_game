package com.clivingston.strat150;

import java.util.ArrayList;

import com.badlogic.gdx.Gdx;

/**
 * This class contains the in-game map.  The TileBuilder class
 * is used to actually construct the map, and tile access is
 * restricted to get/put methods.
 * 
 * @author Chris Livingston
 *
 */
public class GameMap 
{

	//Referred when the tile
	private Tile defaultTile;
	
	private int width, height;
	public String title;
	public String description;
	
	
	public int getWidth()
	{
		return width;
	}
	
	public int getHeight()
	{
		return height;
	}
	
	//Sparse tile array
	private Tile[][] tiles;
	
 	protected void log(String str)
	{Gdx.app.log("Map", str);}
 	
 	public GameMap(String mapData, GameRules rules)
 	{
 		initialize(mapData, rules);
 	}
	
	//Sets up the map
	private void initialize(String mapData, GameRules rules)
	{
		
		//Construct the map!
		TileBuilder tb = new TileBuilder();
		TileBuilder.MapFile mapFile = tb.constructMap(mapData, rules.tileSet, tiles);
		
		tiles = mapFile.tiles;
		defaultTile = rules.tileSet.get(mapFile.defaultTile);
		description = mapFile.description;
		width = mapFile.bounds.get(0);
		height = mapFile.bounds.get(1);
		title = mapFile.title;
		description = mapFile.description; 
		
		log("Map initialized.");
		
	}
	
	/**
	 * Retrieves a tile at a given point. 
	 *
	 * @param  x	X-coordinate
	 * @param  y	Y-coordinate
	 * @return      Requested tile, or null if out of bounds.
	 */
	public Tile getTile(int x, int y)
	{
		if(x > width || y > height || x < 0 || y < 0)
		{
			log("Attempted to access out of bounds tile at (" + x + ", " + y + ")");
			return null;
		}
		
		Tile t = tiles[x][y];
		
		//Sparse retrieval for uninitialized space
		if(t == null)
			return defaultTile;
		
		return t;	
	}
	
	/**
	 * Places a tile at a given point.  Used in the editor only. 
	 *
	 * @param  x	X-coordinate
	 * @param  y	Y-coordinate
	 * @param  tile		Tile template class.
	 */	
	public void putTile(int x, int y, Tile tile)
	{
		if(x > width || y > height || x < 0 || y < 0)
		{
			log("Attempted to access out of bounds tile at (" + x + ", " + y + ")");
		}
		
		tiles[x][y] = tile;

	}
	
	
	
}
