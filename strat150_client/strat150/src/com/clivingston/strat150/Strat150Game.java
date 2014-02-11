package com.clivingston.strat150;

import java.util.HashMap;
import java.util.Map;

import com.badlogic.gdx.ApplicationListener;
import com.badlogic.gdx.Game;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.Graphics.DisplayMode;
import com.badlogic.gdx.assets.AssetManager;
import com.badlogic.gdx.assets.loaders.TextureLoader.TextureParameter;
import com.badlogic.gdx.graphics.GL10;
import com.badlogic.gdx.graphics.OrthographicCamera;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.Texture.TextureFilter;
import com.badlogic.gdx.graphics.g2d.Sprite;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.graphics.g2d.TextureRegion;


/**
 * This is the cross-platform root of the application, containing
 * initialization calls and the backbone of the update/render loop.
 * Aside from some asset management and frame calculations, there 
 * really isn't much extra going on here during runtime.  The 
 * currentScreen field features the current menu/game state class.
 * 
 * @author Chris Livingston
 *
 */
public class Strat150Game extends Game {
	
	public OrthographicCamera pixelCamera; //Pixel-perfect camera
	private SpriteBatch batch;
	private float lastFrame;
	
	
	BaseScreen currentScreen;
	public boolean bAssetsActive;
	
	protected void log(String str)
	{Gdx.app.log("Game", str);}

	
	@Override
	public void create() {		
		float w = Gdx.graphics.getWidth();
		float h = Gdx.graphics.getHeight();
		
		pixelCamera = new OrthographicCamera(w, h);
		pixelCamera.position.set(w / 2, h / 2, 0);
		pixelCamera.update();
		
		log("Graphics dimensions "+w+"x"+h);
		batch = new SpriteBatch();
		
		
		//Load assets
		TextureParameter param = new TextureParameter();
		param.minFilter = TextureFilter.Linear;
		param.genMipMaps = true;
		
		//Prepare the assets
		Assets.initialize();
		Assets.loadSprite("libgdx", "data/libgdx.png", true);
		
		///Load these remotely
		//Assets.loadSprite("grass", "data/grass.png", true);
		//Assets.loadSprite("water", "data/liquid.png", true);
		//Assets.loadSprite("stone", "data/stone_1.png", true);
		//Assets.loadSprite("shard", "data/stone_2.png", true);
		Assets.update(true);
		bAssetsActive = true;
		
		
		currentScreen = new GameScreen(this);
		//currentScreen = new MainMenuScreen(this);
		

	}

	@Override
	public void dispose() {
		batch.dispose();
		Assets.dispose();
	}

	@Override
	public void render() {		
		
		//Find a more efficient way to keep the edges even
/*		if(Gdx.graphics.getWidth() % 2 == 1)
		{
			DisplayMode dm;
			//TODO:  Figure out how to keep resizing allowed
			Gdx.graphics.setDisplayMode(Gdx.graphics.getWidth()-1, Gdx.graphics.getHeight(), false);
		}
*/		
		//long t = System.nanoTime();
		lastFrame = Gdx.graphics.getDeltaTime();
		
		bAssetsActive = !Assets.update(false);
		
		currentScreen.update(lastFrame);
		currentScreen.render(lastFrame);
		
		//Set time since last frame
		;//(float) ((System.nanoTime() - t)/(1e9));
		
		
	}
	
	public float getFrameRate()
	{
		return 1/lastFrame;
	}

	@Override
	public void resize(int width, int height) {
		//Do not calculate if no changes occur
		if(width == pixelCamera.viewportWidth && height == pixelCamera.viewportHeight)
			return;
		
		pixelCamera.viewportWidth = width;
		pixelCamera.viewportHeight = height;
		pixelCamera.position.set(width / 2, height / 2, 0);
		
		pixelCamera.update();
		
		//Pass on the call
		currentScreen.resize(width, height);
	}

	@Override
	public void pause() {
	}

	@Override
	public void resume() {
	}
}


