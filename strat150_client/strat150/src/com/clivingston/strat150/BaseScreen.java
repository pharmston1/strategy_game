package com.clivingston.strat150;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentSkipListMap;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.Screen;
import com.badlogic.gdx.Input.TextInputListener;
import com.badlogic.gdx.graphics.GL10;
import com.badlogic.gdx.graphics.OrthographicCamera;
import com.badlogic.gdx.graphics.Texture;
import com.badlogic.gdx.graphics.Texture.TextureFilter;
import com.badlogic.gdx.graphics.g2d.Sprite;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.graphics.g2d.TextureRegion;
import com.badlogic.gdx.math.Vector2;
import com.badlogic.gdx.math.Vector3;
import com.badlogic.gdx.scenes.scene2d.Stage;


public class BaseScreen implements Screen {

    Strat150Game game; // Note it's "MyGame" not "Game"

 	protected OrthographicCamera camera;
 	protected String tag;
 	protected Stage stage;
 	
 	//Pixel-perfect touch points
 	protected Vector2[] touchPoints;
 	protected Vector2[] originPoints;
 	protected boolean[] bIsTouched;
 	protected boolean[] bJustTouched;
 	protected boolean[] bJustLifted;
 	protected OrthographicCamera touchCam; //Projection matrix for touch
 	private Vector3 scratchVec3A;
 	public static int TOUCH_COUNT = 4;
 	
 	//Text input handling bits
    private boolean bGettingTextInput;
    private boolean bTextInputReceived;
    private boolean bTextInputCanceled;
    private int currentTextHandle;
    private Map<Integer, String> textInputReceived;
    private Map<Integer, TextInputItem> textInputItems;
    
 	
    // constructor to keep a reference to the main Game class
     public BaseScreen(Strat150Game game)
     {
    	 this.game = game;
    	 tag = "Screen";
    	 
    	 //Text input handlers
    	 this.textInputReceived = new ConcurrentHashMap<Integer, String>();
    	 this.textInputItems = new ConcurrentSkipListMap<Integer, TextInputItem>();
    	 
    	 //Initialize simple touch handling variables
    	 touchCam = game.pixelCamera;
    	 touchPoints = new Vector2[TOUCH_COUNT];
    	 originPoints = new Vector2[TOUCH_COUNT];
    	 bIsTouched = new boolean[TOUCH_COUNT];
    	 bJustTouched = new boolean[TOUCH_COUNT];
    	 bJustLifted = new boolean[TOUCH_COUNT];
    	 scratchVec3A = new Vector3();
         
    	 for(int i = 0; i < TOUCH_COUNT; i++)
    	{
    		 touchPoints[i] = new Vector2();
    		 originPoints[i] = new Vector2();
    	}
    	 
     }
     
 	protected void log(String str)
	{Gdx.app.log(tag, str);}

 	
 	/**
 	 * Renders this screen.  Override in child class to review.
 	 *
 	 * @param  deltaTime	Time elapsed since last frame.
 	 */
     @Override
     public void render(float deltaTime) {
        
     }
          
     
     /**
      * Container class for text input.
      * @author chris
      */
     private static class TextInputItem{
    	 public String promptText;
    	 public String defaultText;
    	 public String receivedText;
    	 public boolean bReady;
    	 public int inputIndex;
    	 public int dialogIndex;
    	 
    	 private static int current = 0;
    	 
    	 public TextInputItem()
    	 {
    		 current++;
    		 inputIndex = current;
    	 }
     };
     
     /**
      * Queues text input from the user.
      * @param textIndex Index received from promptTextInput()
      * @param defText Default text.
      * @return Received input, NULL on failure, blank if cancelled.
      */     
     protected String getTextInput(int textIndex, boolean purge)
     {
    	 TextInputItem item = textInputItems.get(textIndex);
    	 
    	 //Fail if the item isn't present or isn't ready
    	 if(item != null && item.bReady)
    	 {
    		 if(item.receivedText == null)
    			 return "";
    		 
    		 return item.receivedText;
    	 }
    	 else return null;
     }
     
     /**
      * Queues text input from the user.
      * @param prompt Text prompt desired.
      * @param defText Default text.
      * @return Key to value in textInputItems
      */
     protected int promptTextInput(String prompt, String defText)
     {
    	 TextInputItem item = new TextInputItem();
    	 
    	 item.promptText = prompt;
    	 item.defaultText = defText;
    	 item.receivedText = null;
    	 item.bReady = false;
    	 item.dialogIndex = -1;
    	 
    	 textInputItems.put(item.inputIndex, item);
    	 
    	 return item.inputIndex;
     }
     
    /**
     * Gets asynchronous text input.  Results are stored in textInput.
     * @param prompt Prompt text.
     * @param defText Default text.
     * @return Text input handle.
     */
 	private int textDialog(String prompt, String defText)
	{
 		//Fail if input is already in progress
 		if(bGettingTextInput)
 			return -1;
 		
 		currentTextHandle++;
 		final int textHandle = currentTextHandle;
 		
 		bTextInputReceived = false;
 		bTextInputCanceled = false;
 		bGettingTextInput = true;
 		
		Gdx.input.getTextInput(new TextInputListener() {
	        @Override
	        public void input(String text) {
	        	textInputReceived.put(textHandle, text);
	        	log("Received input "+textHandle+": "+text);
	     		bTextInputReceived = true;
	     		bTextInputCanceled = false;
	     		bGettingTextInput = false;
	        }
	        
	        @Override
	        public void canceled() {
	        	//textInputReceived.put(textHandle, null);
	        	log("Text Input "+textHandle+" canceled.");
	     		bTextInputReceived = false;
	     		bTextInputCanceled = true;
	     		bGettingTextInput = false;
	     	}

	      }, prompt, defText);
		
		return textHandle;
	}
     
    /**
     * Updates asynchronous text input.
     */
    private void updateTextInput()
    {
    	//Cycle through each queued item
   	 	for(TextInputItem item : textInputItems.values())
   	 	{
   	 		//Initiate text input if we can
   	 		if(!item.bReady && !bGettingTextInput && item.dialogIndex < 0)
   	 			item.dialogIndex = textDialog(item.promptText, item.defaultText);
   	 		
   	 		//If we've received input that belongs to this item
   	 		if((bTextInputReceived || bTextInputCanceled) && textInputReceived.containsKey(item.dialogIndex))
   	 		{
   	 			item.bReady = true;
   	 			item.receivedText = textInputReceived.get(item.dialogIndex); //Will spit out null on user cancel
   	 			textInputReceived.remove(item.dialogIndex);
   	 		}

   	 	}
   	 
    }

     
     
 	/**
 	 * Updates screen logic.  Override in child class to add additional functionality. 
 	 *
 	 * @param  deltaTime	Time elapsed since last frame.
 	 */
     public void update(float deltaTime){
    	 
    	 //Update touch points if a projection is available
    	 if(touchCam != null)
    	 {
    		boolean globalJustTouched = Gdx.input.justTouched();
    		 
    		for(int i = 0; i < TOUCH_COUNT; i++)
    		{
    			//None were just touched
    			if(!globalJustTouched)
    			{
    				bJustTouched[i] = false;
    			}
    			else //Something was just touched
    			{
    				if(!bIsTouched[i] && Gdx.input.isTouched(i))
    					bJustTouched[i] = true;
    				else
    					bJustTouched[i] = false;
    			}
    			
    			//If a finger/mouse button is lifted
    			if(bIsTouched[i] && !Gdx.input.isTouched(i))
    				bJustLifted[i] = true;
    			else
    				bJustLifted[i] = false;
    				
    			
    			bIsTouched[i] = Gdx.input.isTouched(i);
    			
    			//Unproject touch points
    			if(bIsTouched[i])
    			{
    				
    				touchCam.unproject(scratchVec3A.set(Gdx.input.getX(i), Gdx.input.getY(i), 0));
    				
    				touchPoints[i].set(scratchVec3A.x, scratchVec3A.y);
    				
    				//Now set the origin point, if applicable
    				if(bJustTouched[i])
    				{
    					originPoints[i].set(touchPoints[i]);
    					//log("Touched "+i+" at ("+touchPoints[i].x+", "+touchPoints[i].y+")");
    				}
    			}
    			
    		}
    	 }
    	 
    	 updateTextInput();
    	 
     }


	@Override
	public void dispose() {

	}

	@Override
	public void resize(int width, int height) {
		if(camera == null || touchCam == null) //Don't muck with the cameras if they aren't initialized
			return;
		
		this.camera.viewportWidth = width;
		this.camera.viewportHeight = height;
		this.touchCam.viewportHeight = height;
		this.touchCam.viewportWidth = width;
		camera.update();
		touchCam.update();
	}

	@Override
	public void pause() {
	}



    @Override
     public void resume() {
     }
    
    @Override
    public void show() {
         // called when this screen is set as the screen with game.setScreen();
    }


   @Override
    public void hide() {
         // called when current screen changes from this to a different screen
    }


}
