/**
 * 
 */
package com.clivingston.strat150;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.Color;
import com.google.gson.Gson;
import com.google.gson.annotations.SerializedName;


/**
 * This rather convoluted class uses Google's GSON library
 * to read the raw map data into a format understandable by the
 * GameMap class.  Two internal helper structures are used
 * (MapFile and MapRect) as reflection targets for GSON.
 * 
 * @author Chris Livingston
 * 
 */
public class TileBuilder {
	
	protected void log(String str)
	{Gdx.app.log("TileBuilder", str);}
	
	//GSON parsing class for map rectangles
	public class MapRect
	{
		@SerializedName("Type")
		public String type;
		
		@SerializedName("Bounds")
		public List<Integer> bounds;
		
		public MapRect()
		{
			
		}
	}
	
	//GSON parsing class for map file	
	public class MapFile
	{
		@SerializedName("Title")
		public String title;
		
		@SerializedName("Description")
		public String description;
		
		@SerializedName("DefaultRules")
		public String rulesName;
		
		@SerializedName("DefaultTile")
		public String defaultTile;
		
		@SerializedName("Dimensions")
		public List<Integer> bounds;
		
		@SerializedName("Rectangles")
		public List<MapRect> rectangles;
				
		public MapFile()
		{
			
		}
		
		
		public Tile[][] tiles;
	}
	
	
	
	public TileBuilder()
	{
		//Test out Gson
		
		log("Created.");
	
	}
	
	
	
	/**
	 * Constructs a map given raw JSON data. 
	 *
	 * @param  mapData	Map file in string form.
	 * @param  tileSet	Data structure holding tile class types.  Will be created if null. 
	 * @param  tiles	2-Dimensional Tile array
	 * @return      Parsed JSON container classes.
	 */
	public MapFile constructMap(String mapData, Map<String, Tile> tileSet, Tile[][] tiles)
	{
		Gson gson = new Gson();
		log("Parsing map data...");
		
		//If no tile set is specified (testing only)
		if(tileSet == null)
		{
			log("No tile set specified, one will be created using default parameters.");
			tileSet = new HashMap<String, Tile>();
		}
		
		
		try
		{
			MapFile map = gson.fromJson(mapData, MapFile.class);
			
			//Prepare tile array
			int mapWidth = map.bounds.get(0);
			int mapHeight = map.bounds.get(1);
			tiles = new Tile[mapWidth+1][mapHeight+1];
			
			log("Map size: "+mapWidth+"x"+mapHeight);
			
			log("Default tile: "+map.defaultTile);
			
			//Create the default tile if otherwise unavailable
			if(!tileSet.containsKey(map.defaultTile))
			{
				tileSet.put(map.defaultTile, new Tile(map.defaultTile, "libgdx", Color.WHITE));
			}
			
			int fillCount = 0;
			
			//Begin filling
			for(MapRect rect : map.rectangles)
			{
				int x = rect.bounds.get(0);
				int y = rect.bounds.get(1);
				int w = rect.bounds.get(2);
				int h = rect.bounds.get(3);

				log("Adding "+w+"x"+h+" "+rect.type+" at ("+x+","+y+")");
				
				//Retrieve the tile involved
				Tile t = tileSet.get(rect.type);
				
				//Create a placeholder if one isn't defined
				if(t == null)
				{
					log("WARNING: No tile defined in rules for type "+rect.type+", generating a placeholder.");
					t = new Tile(rect.type, "libgdx", Color.RED);
					tileSet.put(rect.type, t);
				}
				
				//Fill rectangle in array
				for(int i = 0; i < w; i++)
					for(int j = 0; j < h; j++)
					{
						//Quick sanity check, skip out-of-bounds tiles
						if(i+x > mapWidth || j+y > mapHeight)
						{
							log("("+i+","+j+") is out of bounds.");
							continue;
						}
						
						tiles[i+x][j+y] = t;
						fillCount++;
					}
			}
			
			//Fill rectangle in array			
			int tileCount = 0;

			for(int i = 0; i < mapWidth; i++)
				for(int j = 0; j < mapHeight; j++)
					if(tiles[i][j] != null)
						tileCount++;
			
			log(tileCount+" tiles filled, "+(fillCount-tileCount)+" overlaps.");
				
			
			map.tiles = tiles;
			
			return map;
		}
		catch(Exception e) //Something went wrong
		{
			log("Error parsing map!");
			log(e.getMessage());
			e.printStackTrace();
			return null;
		}
	}
	
	
	//Hard-coded testing map
	public static final String TEST_JSON = 
		   "{"+"\n"
		 + "    \"Title\": \"Example Map\","+"\n"
		 + "    \"Description\": \"The place where examples do battle\","+"\n"
		 + "    \"Dimensions\": [100, 100],"+"\n"
		 + "    \"DefaultRules\": \"rules.json\","+"\n"
		 + "    \"DefaultTile\": \"Grass\"," + "\n"
		 + " "+"\n"
		 + "  \"Rectangles\":"+"\n"
		 + "  ["+"\n"
		 + "    {\"Bounds\": [30, 0, 50, 50], \"Type\": \"Sand\"},"+"\n"
		 + "    {\"Bounds\": [0, 27, 100, 73], \"Type\": \"Water\"},"+"\n"

		 + "    {\"Bounds\": [24, 24, 7, 7], \"Type\": \"Wall\"},"+"\n"
		 + "    {\"Bounds\": [25, 25, 5, 5], \"Type\": \"Sand\"},"+"\n" 

		 + "    {\"Bounds\": [1, 1, 50, 1], \"Type\": \"Pavement\"},"+"\n"
		 + "    {\"Bounds\": [5, 1, 1, 20], \"Type\": \"Pavement\"},"+"\n"
		 
		 + "    {\"Bounds\": [5, 20, 30, 1], \"Type\": \"Pavement\"},"+"\n"
		 + "    {\"Bounds\": [27, 20, 1, 6], \"Type\": \"Pavement\"},"+"\n"
		 
		 
		 + "    {\"Bounds\": [16, 4, 18, 12], \"Type\": \"Water\"},"+"\n"
		 + "    {\"Bounds\": [15, 5, 20, 10], \"Type\": \"Water\"}"+"\n"
		 
		 + "  ]"+"\n"
		 + "}"+"\n";
	
	
	
}

