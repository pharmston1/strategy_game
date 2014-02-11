package com.clivingston.strat150;

import java.io.InputStream;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.assets.AssetManager;
import com.badlogic.gdx.assets.loaders.PixmapLoader;
import com.badlogic.gdx.assets.loaders.TextureLoader;
import com.badlogic.gdx.assets.loaders.TextureLoader.TextureParameter;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.graphics.Pixmap;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.Texture.TextureFilter;
import com.badlogic.gdx.graphics.g2d.BitmapFont;
import com.badlogic.gdx.graphics.g2d.Gdx2DPixmap;
import com.badlogic.gdx.graphics.g2d.Sprite;
import com.badlogic.gdx.graphics.g2d.TextureAtlas.AtlasSprite;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.scenes.scene2d.ui.Button;
import com.badlogic.gdx.scenes.scene2d.ui.Label;
import com.badlogic.gdx.scenes.scene2d.ui.Skin;


/**
 * This static class maintains all in-game assets.  For ease of use,
 * it is globally accessable while still abstracting away the messy
 * bits of cross-platform file I/O to an internal AssetManager class.
 * 
 * @author Chris Livingston 
 */
public class Assets {
	
	private static Map<String, Sprite> sprites;
	private static Map<String, String> queuedSprites;
	
	private static Map<String, AtlasSprite> atlasSprites;
	private static AssetManager manager;
	
	public static BitmapFont defaultFont;
	public static Label.LabelStyle defaultStyle;
	public static Skin defaultSkin;
	
	protected static void log(String str)
	{Gdx.app.log("Assets", str);}

	/**
	 * Initializes the global assets class.  Also loads the built-in font.
	 *
	 */	
	public static void initialize()
	{
		//TODO: Add custom file resolver
		manager = new AssetManager(); 
		
		sprites = new ConcurrentHashMap<String, Sprite>();
		queuedSprites = new ConcurrentHashMap<String, String>();

		//load up the default font before ANYTHING ELSE
		String path = "data/roboto_small.fnt";//"data/roboto_small_0.png";
				
		manager.load(path, BitmapFont.class);
		manager.finishLoading();

        //Texture fTemp = manager.get(path, Texture.class);
        //fTemp.setFilter(TextureFilter.Linear, TextureFilter.Linear);
        //TextureRegion rTemp = new TextureRegion(fTemp, 0, 0, 128, 128);
        Assets.defaultFont = manager.get(path, BitmapFont.class);
        //new BitmapFont(Gdx.files.internal("data/roboto_small.fnt"), rTemp, false);
        
        //Assets.loadSprite(key, path, bInternal)
        
		defaultSkin = new Skin();
		defaultSkin.addResource("smallfont", defaultFont);
		defaultSkin.addStyle("label", new Label.LabelStyle(defaultFont, Color.WHITE));
		defaultSkin.addStyle("button", new Button.ButtonStyle());


	}
	
	/**
	 * Updates asynchronous loading.  
	 *
	 * @param  bForce	blocks all remaining load tasks
	 * @return      true on completion of loading
	 */	
	public static boolean update(boolean bForce)
	{	
		if(bForce)
		{
			manager.finishLoading();
		}
		
		//Use Entry-based iteration to traverse the queue
    	Iterator<Map.Entry<String, String>> entries = queuedSprites.entrySet().iterator();
    	while (entries.hasNext()) 
    	{
    		Entry<String, String> entry = entries.next();
			
			String path = entry.getValue();
			String key = entry.getKey();
			
			//If a texture is ready, add it to the sprites
			if(manager.isLoaded(path))
			{
				Texture tex = null;
				TextureRegion texReg;
				
				tex = manager.get(path, Texture.class);
				
				
				//Initialize sprite settings and set origin
				texReg = new TextureRegion(tex, tex.getWidth(), tex.getHeight());
				Sprite sprite = new Sprite(texReg);
				//sprite.setOrigin(sprite.getWidth()/2, sprite.getHeight()/2);
				//sprite.setPosition(-sprite.getWidth()/2, -sprite.getHeight()/2);
	
				
				//Store sprite in hashmap
				sprites.put(key, sprite);
				log("Loaded texture " + path + " to sprite " + key + " - "+tex.getWidth()+"x"+tex.getHeight());
				
				//Now remove the queue entry
				entries.remove();
			}
		}

    	//Returns true if there are no more updates
		return manager.update();	
	}
	
	/**
	 * Loads an image file to the sprite queue.
	 * Subsequent calls to update() will finish loading the sprite.  
	 *
	 * @param  key			Sprite retrieval key.
	 * @param  path			Texture reference path.
	 * @param  bInternal	The location of the source image file.
	 * @return      Loading success
	 */
	public static boolean loadSprite(String key, String path, boolean bInternal)
	{
		
		//Load the texture with filtering
		TextureParameter param = new TextureParameter();
		param.minFilter = TextureFilter.Linear;
		param.genMipMaps = true;

		manager.load(path, Texture.class, param);
		queuedSprites.put(key, path);
		log("Image "+path+" queued to sprite "+key);
		
		return true;
	}
	
	/**
	 * Loads a sprite from raw image data.  Should be called from a thread.  
	 * @param key		Sprite retrieval key
	 * @param data	Raw PNG/BMP/JPG data.
	 * @return		Loading success
	 */
	public static boolean loadSprite(String key, byte [] data)
	{
		log("Loading sprite "+key+" ("+data.length+" bytes)");
    	try{
    		
    		Texture tex = new Texture(new Pixmap(data, 0, data.length));
    		tex.setFilter(TextureFilter.Linear, TextureFilter.Linear);
    		TextureRegion reg = new TextureRegion(tex, tex.getWidth(), tex.getHeight());
    		
    		sprites.put(key, new Sprite(reg));
    		
    		return true;
    	}catch(Exception e) //An error occurred
    	{
    		e.printStackTrace();
    		//log(new String(data).substring(data.length-20, data.length-1)); //Debug characters
    		return false;
    	}
	}
	
	/**
	 * Loads sprite data from an input stream.
	 * @param key		Sprite retrieval key.
	 * @param stream	Input stream containing PNG data.
	 * @warning 	Expects RGBA8888 data.
	 * @return		Loading success
	 */
	public static boolean loadSprite(String key, InputStream stream)
	{
    	try{
    		
    		//Holy nested constructors Batman!
    		//Indented for clarity
    		Texture tex = new Texture(
    				      new Pixmap(
    				      new Gdx2DPixmap(stream, Gdx2DPixmap.GDX2D_FORMAT_RGBA8888)));
    		
    		tex.setFilter(TextureFilter.Linear, TextureFilter.Linear);
    		TextureRegion reg = new TextureRegion(tex, tex.getWidth(), tex.getHeight());
    		
    		sprites.put(key, new Sprite(reg));
    		
    		log("Sprite "+key+" created from input stream.");
    		
    		return true;
    	}catch(Exception e) //An error occurred
    	{
    		e.printStackTrace();
    		return false;
    	}
	}
	

	public static boolean processZip(ZipInputStream zipStream)
	{
		
		try
		{
			ZipEntry entry = zipStream.getNextEntry();
//			Map<String, ZipEntry> entries = new ConcurrentHashMap<String, ZipEntry>();
			Map<String, byte[]> extracted = new ConcurrentHashMap<String, byte[]>();
			
			//Roll through all ZipEntries
			while(entry != null)
			{
				
				if(!entry.isDirectory())
				{
					byte [] buffer = new byte[(int) entry.getSize()];
					
					int i = 0;
					for(int b = zipStream.read(); b != -1; b = zipStream.read())
						buffer[i++] = (byte) b;
					
					extracted.put(entry.getName(), buffer);	
					
					log("Extracted: "+entry.getName()+" "+buffer.length/1024+"K");
				}
				
				zipStream.closeEntry();
				entry = zipStream.getNextEntry();
			}
			
			//Extract the sprite assets file and read in each asset
			if(extracted.containsKey("sprites.txt"))
			{
				String [] assets = new String(extracted.get("sprites.txt")).split("[\n\r]+");
				
				for(String a : assets)
				{
					String [] asset = a.split("\\s+");
					
					if(asset[1].endsWith(".png"))
						loadSprite(asset[0], extracted.get(asset[1]));
				}
				
			}
			else log("Cannot find sprites.txt in zip file!");
			//ZipEntry dir = entries.get("")
			
			
			
			return false;
			
		}catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}
	
	/**
	 * Disposes all loaded assets - called on exit.
	 *
	 */	
	public static void dispose()
	{
		sprites.clear();
		manager.clear();
	}
	
	/**
	 * Returns a sprite
	 *
	 * @param  key		Sprite retrieval key
	 * @return      Sprite, or null on failure
	 */
	public static Sprite getSprite(String key)
	{
		
		Sprite s = sprites.get(key);
		
		if(s == null)
		{
			if(queuedSprites.containsKey(key))
				log("WARNING! Sprite " + key + " has not finished loading!");
//			else
//				log("WARNING! Sprite " + key + " does not exist!");
		}
		
		return s;
	}

}
