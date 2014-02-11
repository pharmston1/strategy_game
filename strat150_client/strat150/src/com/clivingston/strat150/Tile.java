package com.clivingston.strat150;

import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.graphics.g2d.Sprite;


/**
 * This class is a simple sprite/type container for the in-game map tiles.
 * A deserializer subclass is included for
 * 
 * @author Chris Livingston
 *
 */
public class Tile {
	
	//Graphical properties
	private Sprite sprite;
	public String spriteKey;
	
	/**
	 * Gets the sprite from this tile 
	 * @return Sprite class, or null if unavailable.
	 */
	public Sprite getSprite()
	{
		if(spriteKey == null)
			return null;
		
		if(sprite == null)
			sprite = Assets.getSprite(spriteKey);
		
		return sprite;
	}
	
	public Color color;
	
	public String terrainType;
	public int renderLayer;
	
	/**
	 * Parameterized Tile constructor  
	 *
	 * @param  terrainType	String tag for this tile's terrain type.
	 * @param  spriteKey	Key to existing sprite in the static Assets class.
	 * @param  color		Tint color.
	 * 
	 */
	public Tile(String terrainType, String spriteKey, Color color)
	{
		initialize(terrainType, spriteKey, color);
	}

	/**
	 * Template-based tile constructor  
	 *
	 * @param  template		Existing Tile class to clone.
	 */
	public Tile(Tile template)
	{
		terrainType = new String(template.terrainType);
		this.color = new Color(color);
		sprite = template.sprite;
		
		renderLayer = 0;
	}
	
	private void initialize(String terrainType, String spriteKey, Color color)
	{
		this.terrainType = new String(terrainType);
		this.color = new Color(color);
		sprite = Assets.getSprite(spriteKey);
		this.spriteKey = spriteKey;
		
		renderLayer = 0;
	}

}
