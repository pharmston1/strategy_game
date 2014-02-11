package com.clivingston.strat150;

import java.io.ByteArrayInputStream;
import java.util.Set;
import java.util.zip.ZipInputStream;

import org.omg.CORBA.portable.InputStream;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.graphics.Color;
import com.badlogic.gdx.graphics.GL10;
import com.badlogic.gdx.graphics.OrthographicCamera;
import com.badlogic.gdx.graphics.g2d.Sprite;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.graphics.g2d.BitmapFont.HAlignment;
import com.badlogic.gdx.math.MathUtils;
import com.badlogic.gdx.math.Vector2;
import com.badlogic.gdx.math.Vector3;
import com.badlogic.gdx.scenes.scene2d.Stage;
import com.clivingston.strat150.ConnectionManager.ConnectionRequest;


/**
 * The GameScreen class is the master in-game container class.
 * Game/GUI logic is updated via the update() method and
 * GUI rendering is managed via the render() method.  Game rendering
 * is also called from here via a child class.
 * 
 * @author Chris Livingston
 *
 */
public class GameScreen extends BaseScreen 
{
	Color clientColor;
	
	//Test sprite
	Sprite sprite;
	private SpriteBatch batch;
	
	OrthographicCamera gameCam, guiCam;
	Color colorText;
	String[] logBuffer;
	float[] logFade;
	float gameTime;
	static final float LOG_FADE_TIME = 6;
	
    
	private float pixelRatio;
	
	Vector2 scratchVec;
	Vector2 camDragOrigin;
	
	String stats;
	int statsMode;
	
	ConnectionManager connect;
	private boolean bSentOrders;
	private boolean bReceivedRules;
	private int rulesRequestIndex;
	private boolean bReceivedMap;
	private int mapRequestIndex;
	private int orderIndex;
	
	private String ordersToSend;
	private String assetChecksum;
	private boolean bReceivedAssets;
	private int assetRequestIndex;
	
	private Unit selectedUnit;
	Player clientPlayer;
	GameMap map;
	GameRules rules;
	GameField world;
	
	private int nickIndex; //Nickname index
	private String nickname;
	private String clientFaction;
	
	private String passcode;

	private int digestIndex;
	
	public GameScreen(Strat150Game game) 
	{

		super(game);
		
		// TODO Auto-generated constructor stub
	   	sprite = Assets.getSprite("libgdx");
	   	guiCam = game.pixelCamera;
	   	 
	   	tag = "GameScreen";
	   	batch = new SpriteBatch();
	   	
	   	logBuffer = new String[32];
	   	logFade = new float[32];
	   	
	   	digestIndex = -1;
	   	
	   	
	   	//Base game aspect ratio on screen
	   	 //TODO:  Break this out for resizing code
	   	float w = 20;
	   	float h = w*((float)Gdx.graphics.getHeight()/Gdx.graphics.getWidth());
	   	
	   	pixelRatio = Gdx.graphics.getWidth() / w;
	   	
	   	gameCam = new OrthographicCamera(w, h);
		gameCam.position.set(w / 2, h / 2, 0);
		gameCam.update();
		
		scratchVec = new Vector2();
		camDragOrigin = new Vector2();
		
		log("Game camera dimensions "+w+"x"+h);
		
		//Set the stage
		stage = new Stage(w, h, true, batch);
		stage.setCamera(guiCam);
		
		log("Stage object constructed.");
		
		this.scratchVec3W = new Vector3();
		this.worldTouch = new Vector2();
		
		//TEST CODE BELOW///////////////////////////
		//Hard code the faction for now
		clientColor = new Color(MathUtils.random(0.1f, 1),
				                MathUtils.random(0.1f, 1),
				                MathUtils.random(0.1f, 1), 1);
		clientFaction = "Agile";
		
		
		rules = new GameRules();		
		this.nickIndex = this.promptTextInput("Enter your nickname: ", "Player");
				
		colorText = new Color(Color.WHITE);
		stats = "";
		
	   	//Test address
	   	connect = new ConnectionManager("localhost", 80, false);	  
	   	
	   	//Retrieve the rules
	   	ConnectionRequest request = connect.createRequest(false);
	   	this.rulesRequestIndex = request.index;
	   	request.setPathURL("rules.json");
	   	request.startRequest();
	   	log("Rules requested.");
	   	
	   	
	   	//Retrieve the map
	   	request = connect.createRequest(false);
	   	this.mapRequestIndex = request.index;
	   	request.bIsBinary = true; //Test binary receipt
	   	request.setPathURL("map.json");
	   	request.startRequest();
	   	log("Map requested.");
	   	
	   	//Retrieve assets
	   	request = connect.createRequest(false);
	   	this.assetRequestIndex = request.index;
	   	request.bIsBinary = true;
	   	request.setPathURL("assets.zip");
	   	request.startRequest();
	   	log("Assets requested.");
	   	
	   	
	   	//log(TileBuilder.TEST_JSON);
	   	//this.promptTextInput("Test Prompt", "test");
	   	//this.promptTextInput("Test Prompt #2", "test");
	}

	
	/**
	 * Sends any queued orders to the server
	 */
/*	private void submitOrders() {
		Player p = clientPlayer;
		
		Orders o = new Orders(p);
		
		log(o.serialize());
		
		UnitOrders order = new UnitOrders("1501");
	   	order.appendOrder(2, 2, "2002");
	   	order.appendTarget("1003");
	   	
	   	//log(order.serialize());
	   	o.addUnitOrder(order);
	   	
	   	
	   	BuyOrder buy = o.buy();
		
		buy.setTotalQty(1);
		buy.setUnitType("Chassis");
		String turretLabel = buy.addEquip("Turret", 1);
		buy.addEquip("Ammo", 50, turretLabel);
		buy.addEquip("Plates", 1, turretLabel);
		
		
		o.addDrop("2001", 9000, 10, 30);
		
		ordersToSend = (o.serialize());
		log(ordersToSend);
		
		o.addDrop("9001", 4, 10, 2);
		log(o.serialize());
	}*/
	
	//Initialization update loop
	private void updateInit()
	{
		
		//Check nickname prompt
		if(nickname == null)
		{
			nickname = this.getTextInput(nickIndex, true);
			if(nickname != null)
				log("Player nickname: "+nickname);
		}

		
		//If we have received a nickname, begin registration
		if(this.bReceivedAssets && this.bReceivedRules && this.bReceivedMap && nickname != null && passcode == null)
		{
			
		   	connect.setNickname(nickname);	   	
		   	log("Beginning Registration...");
		   	passcode = connect.startRegistration();
		   	
		   	bSentOrders = false;
		}
		
		//Received the assets?
		if(!bReceivedAssets && connect.resultReady(assetRequestIndex))
		{
			byte [] received = connect.getBinaryResult(assetRequestIndex, true);
			if(received != null)
			{
			
				log("Assets: Received "+received.length/1024+"K");
				ZipInputStream zipin = new ZipInputStream(new ByteArrayInputStream(received));
				
				Assets.processZip(zipin);
				bReceivedAssets = true;
				
			}
			else
			{
				log("Did not retrieve assets!");
				//this.bReceivedAssets = false;
				
			}
				
		}
		
		//Received the rules?
		if(!bReceivedRules && connect.resultReady(rulesRequestIndex))
		{
			log("Rules downloaded.  Parsing...");
			String rulesJson = connect.getResult(rulesRequestIndex, true);
			
			if(rules.parseRules(rulesJson))
			{
				log(rules.getGroupKeys().size() + " groups, " + rules.getEntryKeys(null).size() + " singles loaded.");
				
				//Prepare the map tiles
				Set<String> tileKeys = rules.getEntryKeys("tiletypes");
				
				for(String tileKey : tileKeys)
				{
					Color tileColor = new Color();
					
					RulesEntry tileEntry = rules.getEntry("tiletypes", tileKey);
					
					Double [] c = tileEntry.getDoubleArray("Color");
					
					//Check if color string is corrupted
					if(c.length < 3)
					{
						log("Error processing rules file - tile ["+tileKey+"] has insufficient colors");
						tileColor.r = 1;
						tileColor.g = 1;
						tileColor.b = 1;
						tileColor.a = 1;
					}
					else
					{
						tileColor.r = c[0].floatValue();
						tileColor.g = c[1].floatValue();
						tileColor.b = c[2].floatValue();
						tileColor.a = 1;
					}
					
					rules.tileSet.put(tileKey, new Tile(tileKey, tileEntry.getString("Sprite"), tileColor));
					
					log("Added tile ["+tileKey+"]");
					
					
				}
				
				
				bReceivedRules = true;	
				//for(String groupKey)
			}
			else
			{	
				log("Rules parsing failed!");// Do error stuff
				//TODO:  Error stuff
			}
		}
		
		//If the map is loaded, parse it
		if(bReceivedRules && !bReceivedMap && connect.resultReady(mapRequestIndex))
		{
			byte [] buffer = connect.getBinaryResult(mapRequestIndex, true);
			
			log("Map size: "+buffer.length);
			
			String mapJson = new String(buffer);//connect.getResult(mapRequestIndex, true);
			
			map = new GameMap(mapJson, rules);
			
			log("Map downloaded, parsing...");
			log("Map title:  "+map.title);
			log("Map Dimensions: "+map.getWidth()+"x"+map.getHeight());
			
			//The map has been downloaded!
			bReceivedMap = true;
			
		   	
		}
		
	}
	

	//Added on-screen log buffer
	@Override
	protected void log(String str)
	{
		super.log(str);
		
		//Shift the buffer
		for(int i = logBuffer.length-1;i > 0;i--)
		{
			logBuffer[i] = logBuffer[i-1];
			logFade[i] = logFade[i-1];
		}
		
		logBuffer[0] = str;
		logFade[0] = 1; //Fade factor, not fade time
	}
	
	//Does a quick update
	private void updateDebugStats() 
	{
		int newMode = (int) ((gameTime));
		
		
		if(newMode != statsMode)
		{
			statsMode = newMode;
			
			switch(statsMode)
			{
			case 0:
				if(this.bReceivedMap)
					stats = "Tile count: " + (map.getHeight() * map.getHeight());
				else
					stats = "Waiting for data...";
				break;
				
			case 1:
				stats = "Frame Rate: " + game.getFrameRate();
				break;
				
			default:
				stats = "Stats";
			
			}
			
			if(connect.isAuthenticated() && digestIndex < 0)
			{
				ConnectionRequest digestRequest = connect.createRequest();
				digestRequest.setPathURL("getDigest.php");
				this.digestIndex = digestRequest.index;
				digestRequest.startRequest();
			}
			
			if(world != null)
			{
				Set<String> fAlerts = world.getAlerts();
				
				if(fAlerts != null)
					for(String msg : fAlerts)
						log(msg);
			}
			
			
		}
				
		
	}
	
    @Override
    public void render(float deltaTime) 
    {
            // update and draw stuff
       //  if (Gdx.input.justTouched()) // use your own criterion here
       //      game.setScreen(game.anotherScreen);
         
       Gdx.gl.glClearColor(0, 0, 0, 1);
       Gdx.gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
 		
        
        renderMap(map);
        renderUnits();
 		
 		batch.setProjectionMatrix(guiCam.combined);
 		batch.begin();
 		//sprite.setPosition(0, 0);
 		//sprite.setScale(0.5f);
 		//sprite.draw(batch);
 		
 		//batch.draw(sprite, 0, 0, 0, 0, 64, 64, 1, 1, 0);
 		
 		colorText.r += deltaTime*2;
 		if(colorText.r > 1)
 			colorText.r = 0;
 		
 		renderLog(deltaTime);
    	Assets.defaultFont.setColor(Color.WHITE);
    	Assets.defaultFont.draw(batch, stats, 20, guiCam.viewportHeight - 20);
 		batch.end();
 		
 		
 		
    }
    
    //Draws all world units
    private void renderUnits()
    {
    	if(world == null || !this.bReceivedAssets)
    		return;
    	
    	Sprite s = Assets.getSprite("chassis");
    	Sprite t = Assets.getSprite("turret");
    	batch.begin();
    	batch.setProjectionMatrix(this.gameCam.combined);
    	
    	for(Unit unit : world.visibleUnits)
    	if(unit.isVisible() && world.getVisibility(unit.xActual, unit.yActual))
    	{
    		
    		
    		float selAlpha = 1;
    		
    		if(unit == this.selectedUnit)
    		{
    			selAlpha = (float) (Math.sin(gameTime*5)/4f+0.5f)+0.25f;
    		}
    		
    		Color uColor = unit.owner.color;
    		uColor.a = selAlpha;
    		
    		//if(unit.isVisible())
    		batch.setColor(uColor);
    		batch.draw(s, unit.xpos, unit.ypos, 0, 0, 1, 1, 1, 1, 0);
    		
    		batch.setColor(uColor);
    		batch.draw(t, unit.xpos, unit.ypos, 0, 0, 1, 1, 1, 1, 0);
    		
    		batch.setColor(Color.WHITE);
    		
    	}
    	
    	batch.end();
    }
    
    //Draws the map
    private void renderMap(GameMap renderMap)
    {
    	//If the map cannot be drawn, display a warning bit
    	if(!this.bReceivedAssets || !this.bReceivedMap || !this.bReceivedRules)
    	{
    		batch.setProjectionMatrix(guiCam.combined);
    		batch.begin();
        	Assets.defaultFont.setColor(Color.RED);
        	//Assets.defaultFont.drawMultiLine(spriteBatch, str, x, y)
        	
        	if(!bReceivedAssets)
        		Assets.defaultFont.drawMultiLine(batch, "DOWNLOADING ASSETS", guiCam.viewportWidth/2, guiCam.viewportHeight/2, 0, HAlignment.CENTER);
        	
        	if(!bReceivedMap)
        		Assets.defaultFont.drawMultiLine(batch, "DOWNLOADING MAP", guiCam.viewportWidth/2, guiCam.viewportHeight/2-20, 0, HAlignment.CENTER);

        	if(!bReceivedRules)
        		Assets.defaultFont.drawMultiLine(batch, "DOWNLOADING RULES", guiCam.viewportWidth/2, guiCam.viewportHeight/2+20, 0, HAlignment.CENTER);

        	
        	batch.end();
    		return;
    	}
    	
    	if(renderMap == null)
    	{
    		//Map is loading
    		return;
    	}
    	
    	int width = renderMap.getWidth();
    	int height = renderMap.getHeight();
    	
    	int startx = (int) Math.max(0, gameCam.position.x - gameCam.viewportWidth/2);
    	int starty = (int) Math.max(0, gameCam.position.y - gameCam.viewportHeight/2);
    	
    	int endx = (int) Math.min(width, gameCam.position.x + gameCam.viewportWidth/2+1);
    	int endy = (int) Math.min(height, gameCam.position.y + gameCam.viewportHeight/2+1);
    	
    	batch.setProjectionMatrix(gameCam.combined);
    	batch.begin();
    	
    	int i = 0;
    	
    	//Cycle through all tiles for now
    	for(int x = startx; x < endx; x++)
    		for(int y = starty; y < endy; y++)
    		{
    			Tile t = renderMap.getTile(x, y);
    			
    			//Draw fog of war
    			//TODO:  Finish fog of war
    			
    			if(world == null || world.getVisibility(x, y))
    				batch.setColor(t.color);
    			else
    				batch.setColor(0.2f, 0.2f, 0.2f, 1);
    			
    			
    			Sprite s = t.getSprite();
    			
    			if(s != null)
    				batch.draw(s, x, y, 0, 0, 1, 1, 1.0f, 1.0f, 0);
    			i++;
    		}
    	
    	//log(i+" tiles rendered.");
    	
    	batch.setColor(Color.WHITE);
    	batch.end();
    	
    }
    
    //Renders the debug log to screen
    private void renderLog(float deltaTime)
    {
    	for(int i = 0; i < logBuffer.length; i++)
	    	if(logBuffer[i] != null && logFade[i] > 0)
	    	{
	    		Assets.defaultFont.setColor(1, 1, 1, logFade[i]);
	    		Assets.defaultFont.draw(batch, logBuffer[i], 10, 20+15*i);
	    		
	    		logFade[i] -= deltaTime / LOG_FADE_TIME;

	    	}
    	
    }
    
    
    private Vector3 scratchVec3W; //Scratch vector to avoid repeated object allocations
    private Vector2 worldTouch;   //Scratch vector

	private boolean bCenterOnUnit;
    //Projects touch events into the world and checks them
    private void updateWorldTouch()
    {
    	

    	if(this.bIsTouched[0])
    	{
    		gameCam.unproject(scratchVec3W.set(Gdx.input.getX(0), Gdx.input.getY(0), 0));
    		
			worldTouch.set((float)Math.floor(scratchVec3W.x), (float)Math.floor(scratchVec3W.y));
			
			int xTouch = (int) worldTouch.x;
			int yTouch = (int) worldTouch.y;
			
			if(bJustTouched[0])
			{
				//log("Touched game coordinates: "+xTouch+", "+yTouch);
				if(selectedUnit != null)
				{
					
					selectedUnit.orders = new UnitOrders(selectedUnit.id);
					
					//world.moveUnit(selectedUnit.id, xTouch, yTouch);
					Unit target = world.getUnitAt(xTouch, yTouch);
					
					if(target == null)
					{
						selectedUnit.orders.appendOrder(xTouch, yTouch);
						log("Move "+selectedUnit.getString("DisplayName")+" to "+xTouch+", "+yTouch);
					}
					else
					{
						selectedUnit.orders.appendTarget(target.id);
						
						if(target.owner == null)
							log("Target "+target.getString("DisplayName"));//+" belonging to "+target.owner.nickname);
						else
							log("Target "+target.getString("DisplayName")+" belonging to "+target.owner.nickname);
					}
					
	    			Orders o = new Orders(clientPlayer);
	    			o.addUnitOrder(selectedUnit.orders);
	    			ordersToSend = o.serialize();
	    			this.bSentOrders = false;
	    			log(ordersToSend);
					
					selectedUnit = null;
					//TODO:  Better movement controls
				}
			}
    		
    		
    	}
    	
    	if(this.bJustLifted[0])
    	{
			int xTouch = (int) worldTouch.x;
			int yTouch = (int) worldTouch.y;
			Unit selected = world.getUnitAt(xTouch, yTouch);
			
			if(selected != null)
			if(selected.xActual == selected.xpos && selected.yActual == selected.ypos)
			{
				selectedUnit = selected;
				log("Selected "+selectedUnit.getString("DisplayName")+" ["+selectedUnit.id+"]");    		
			}
    	}
    	
    }
    
    @Override
    public void update(float deltaTime)
    {
   	 
    	super.update(deltaTime);
    	gameTime += deltaTime;
    	
    	updateWorldTouch();
    	updateDebugStats();
    	//stats = "FPS: " + game.getFrameRate();
    	updateInit();
    	if(world != null)
    		world.update(deltaTime);
    	
    	if(selectedUnit == null)
    		updateScrolling();
    	
    	connect.update();
    	
    	if(connect.getConnectionDelta() != 0)
    	{
    		if(connect.activeConnections > 0)
    			log("Request sent...");
    	}
    	
    	if(connect.isAuthenticated())
    	{
    		if(clientPlayer == null) //Create the clientPlayer class upon authentication
    		{
    			log("Authenticated.");
    			clientPlayer = new Player(connect.getUserID(), connect.getNickname(), clientFaction);
    			clientPlayer.color = clientColor;
/*
     			String testDigest = "{ \"units\": { \"unitID\": {  \"unitType\": \"AgileHeavyTank\",  " +
    					            "\"unitOwner\": \"" +connect.getUserID()+ "\",  \"pos\": \"10 42\",  \"dir\": 90,  \"equipment\": [  \"equipmentID\"  ] } }, " +
    					            "\"equipment\": { \"equipID\": {  \"equipType\": \"exampleEquipment\",  \"equipment\": [  \"equipID1\",  \"equipID2\"  ] } }, " +
    					            "\"players\": { \"" +connect.getUserID()+ "\": {  \"balance\": 10000,  \"score\": 200,  \"faction\": \"USA\",  \"nickname\": \"[base64Nick]\",  \"color\": [  1,  1,  1  ] } }, \"messages\": { \"msgID\": {  \"to\": \"userID/group\",  \"from\": \"userID\",  \"body\": \"stuff\" } }, \"diplomacy\": { \"allies\": [  \"userID1 userID2\" ], \"tribute\": [  \"userID1 userID2 10\",  \"userID2 userID1 unitID\" ] }, \"events\": { \"eventID\": {  \"alert\": \"outcome\",  \"destroyed\": [  \"unitID1\",  \"unitID2\"  ],  \"damaged\": [  \"unitID3\"  ],  \"affected\": [  \"unitID4\"  ],  \"origin\": \"x y\",  \"actor\": \"unitID/userID\" } }, \"info\": { \"tick\": 0, \"time\": 1353386474, \"next\": 1353386504, \"motd\": \"Message of the day\" }}";
*/
    		   	world = new GameField(rules, map, clientPlayer);
    			world.clientPlayer = clientPlayer;
    			
    			log("Generating initial orders...");
    			Orders o = new Orders(clientPlayer);
    			//ordersToSend = o.serialize();
    			//this.bSentOrders = false;
    			
    			///////////////////////////////////TEST CODE
    			//world.parseDigest(testDigest);
    		   	//world.moveUnit("unitID", 5, 5);
    		   	
    			BuyOrder buy = o.buy();
    			
    			buy.setTotalQty(1);
    			String turretLabel = buy.addEquip("DefaultLightTurret", 1);
    			buy.addEquip("DefaultCannon", 1, turretLabel);
    			buy.setUnitType("AgileLightTank");
    			
    			buy.setDropCoords((int)(MathUtils.random(5, map.getWidth()-5)), 
    					          (int)(MathUtils.random(5, map.getHeight()-5)));
    			
    			ordersToSend = o.serialize();
    			this.bSentOrders = false;
    			log(ordersToSend);
    		   	
    		   	//world.setVisibility(10, 10, true);
    		   	//world.circleLOS(20, 10, 1);
    		   	//world.circleLOS(20, 20, 2);
    		   	//world.circleLOS(20, 30, 5);
    		   	//world.circleLOS(20, 40, 6);
    		   	//world.circleLOS(10, 5, 9);
    			//log("Point visible: "+world.getVisibility(10, 10));
    			bCenterOnUnit = true;
    			////////////////////////////////////////////

    			
    			
    		}
    		else if(connect.resultReady(digestIndex))
    		{
    			String digest = connect.getResult(digestIndex, true);
    			
    			if(!world.parseDigest(digest))
    				log(digest);
    			else    			
    			if(bCenterOnUnit)
    			{
    				log("Centering on unit...");
    				for(Unit u : clientPlayer.units.values())
    				{
    					//if(u.isVisible())
    					//{
    						this.centerGameCam(u.xActual, u.yActual);
    						break;
    					//}
    				}
    				
    				bCenterOnUnit = false;
    			}
    			
    			digestIndex = -1;
    			
    		}
    		
	    	if(!bSentOrders && ordersToSend != null) //Send orders if available
	    	{
	    		orderIndex = sendOrders(ordersToSend); 
	    		if(orderIndex > 0)
	    		{	
	    			log("Orders sent! ("+orderIndex+")");
	    			ordersToSend = null;
	    			bSentOrders = true;
	    		}
	    	}
    	}		
    	else if(connect.getUserID() != null && connect.startAuthentication()) //Attempt to authenticate
		{
			log("Attempting to log in.");
			log("userID: "+connect.getUserID() + "  Passcode: "+connect.getPasscode());
   		}
    	
		if(bSentOrders)
		{
			String s = connect.getResult(orderIndex, true);
			if(s != null)
				log("From server: "+s);
		}
    	
    }
    
    private void centerGameCam(int x, int y)
    {
    	log("Centering on "+x+", "+y);
    	gameCam.position.set(x, y, 0);
    	gameCam.update();
    }


	private void updateScrolling() {
		//Initialize scrolling
    	if(bJustTouched[0])
    	{
    		camDragOrigin.set(gameCam.position.x, gameCam.position.y);
    		//log("Map Origin: "+gameCam.position.x+", "+gameCam.position.y);
    		
    		//Do a unit touch
    	}
    	
    	//manage drag scrolling
    	if(bIsTouched[0])
    	{
    		scratchVec.set(touchPoints[0]);
    		scratchVec.sub(originPoints[0]);
    		scratchVec.mul(-1/this.pixelRatio);
    		scratchVec.add(camDragOrigin);
    		
    		//log("Drag Vector: "+scratchVec.x+", "+scratchVec.y);
    		
    		this.gameCam.position.set(scratchVec.x, scratchVec.y, 0);
    		gameCam.update();
    	}
	}
    
    /**
     * 
     * @param orderString Serialized orders to submit.
     * @return Request index if >0, <0 on error.
     */
    public int sendOrders(String orderString)
    {
    	log("Sending Orders");
    	String encoded = Base64Coder.encodeString(orderString);
    	String checksum = Checksum.MD5(encoded);
    	
    	ConnectionRequest request = connect.createRequest();
    	
    	if(request == null)
    		return -1;
    	
    	request.setPathURL("ordersToServer.php");
    	request.addPost("orders", encoded);
    	request.addPost("checksum", checksum);
    	
    	if(request.startRequest())
    	   	return request.index;
    	else
    	{
    		log("Order request failed!");
    		return -2;
    	}
    }


	@Override
	public void dispose() {

	}


	@Override
	public void resize(int width, int height) {
		super.resize(width, height);
		
		this.gameCam.viewportWidth = width / pixelRatio;
		this.gameCam.viewportHeight = height / pixelRatio;
		log("New size: "+gameCam.viewportWidth+"x"+gameCam.viewportHeight);
		this.gameCam.update();
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
