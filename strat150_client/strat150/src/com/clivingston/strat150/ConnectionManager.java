package com.clivingston.strat150;

import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import com.badlogic.gdx.Gdx;


/**
 * This class handles asynchronous connections with an outside server.
 * @author Chris Livingston
 *
 */
public class ConnectionManager implements Runnable {
	
	boolean bUseSSL;
	String domain;
	int portNumber;
	
	private String userID;
	private String passcode;
	private String nickname;
	
	
	private String authToken;
	private boolean bAuthenticated;
	private int authIndex; //Authentication connection index
	private int regIndex;  //Registration connection index
	private boolean bAuthFailed;
	private boolean bRegFailed;
	
	private String failMessage;
	
	public boolean isAuthenticated()
	{
		boolean result = !bAuthFailed && !bRegFailed && bAuthenticated && userID != null && authToken != null;
		
		//if(!result && bAuthenticated)
		//	log("Failed auth. "+bAuthFailed);
		
		return result;
	}
	
	
	//Map
	private Map<Integer, ConnectionRequest> requests;
	
	public int activeConnections;
	private int connectionDelta;
	
	private void log(String entry)
	{
		Gdx.app.log("Connection", entry);
	}
	
	public ConnectionManager(String domain, int portNumber, boolean ssl)
	{
		this.domain = domain;
		this.portNumber = portNumber;
		this.bUseSSL = ssl;
		
		nickname = "Player";
		
		requests = new ConcurrentHashMap<Integer, ConnectionRequest>();
	}
	
	
	/**
	 * Sets the player nickname.  If authenticated, will initiate a proper change request. 
	 * @param nick
	 */
	public void setNickname(String nick)
	{		
		nickname = nick;
		
		if(!bAuthenticated) //Don't try to connect if authentication hasn't happened yet
			return;
			
		ConnectionRequest req = createRequest();
		
		if(req != null)
		{
			req.setPathURL("changeNick.php");
			req.addPost("nickname", nickname);
			req.startRequest();
		}
	}
	
	/**
	 * Gets the player nickname.
	 * @return Current player nickname.
	 */
	public String getNickname()
	{
		return nickname;
	}
	
	//Initiate authentication
	public boolean startAuthentication()
	{
		if(userID == null || passcode == null || authIndex != 0)
			return false;
		
		ConnectionRequest req = new ConnectionRequest();
		
		authIndex = req.index;
		
		req.addGet("nickname", nickname);
		req.addGet("userID", userID);
		req.addPost("password", Checksum.MD5(passcode));
		
		req.setPathURL("login.php");
		req.setBaseURL(getBaseURL());
		
		this.requests.put(authIndex, req);
		
		return req.startRequest();
		//return true;
		
	}
	
	/**
	 * 
	 * @return True if registration or authentication has failed.
	 */
	public boolean hasFailed()
	{
		return bRegFailed || bAuthFailed;
	}
	
	public String getFailMsg()
	{
		return failMessage;
	}
	
	public String getUserID()
	{
		return userID;
	}
	
	public String getPasscode()
	{
		return passcode;
	}
	
	/**
	 * Starts the new account/registration process.  Will spawn a connection daemon thread.
	 * @return Generated passcode.
	 */
	public String startRegistration()
	{
		//If no passcode is specified, generate one
		if(passcode == null)
		{
			char code[] = new char[16];
			
			int p = (int) Math.floor(Math.random() * 4);
			
			for(int i = 0; i < 4; i++)
			{
				if(i == p)
					for(int j = i*4; j < (i*4)+4; j++)
					{
						String a = "1234567890abcdefghijklmnopqrstuvwxyz";
						
						for(int k = 0; k < a.length(); k++)
							code[j] = a.charAt((int) (Math.random() * a.length()));
					}
				else
					{
					 	int j = i*4;
					 	
						String z = "!@#$%^&*-=<>:;";
						
						int zi = (int) (Math.random() * z.length());
						
						code[j] = z.charAt(zi);
						code[j+1] = z.charAt(zi);
						code[j+2] = z.charAt(zi);
						code[j+3] = z.charAt(zi);
					}
			}
			passcode = new String(code);
		}
			
		
		String passCheck = Checksum.MD5(passcode);
		
		//log(passcode+" "+passCheck);
		
		ConnectionRequest request = new ConnectionRequest();
		log("Request "+request.index+" created.");
		
		requests.put(request.index, request);
		
		request.setBaseURL(getBaseURL());
		request.setPathURL("registration.php");
		
		request.addGet("nickname", nickname);
		request.addPost("password", passCheck);
		
		request.startRequest();
		
		this.regIndex = request.index;
		
		return passcode;
	}
	
	/**
	 * @param requireAuth If true, requires the client to be authenticated with the server.
	 * @return A ConnectionRequest if successful, NULL if a request cannot be created.
	 */
	public ConnectionRequest createRequest(boolean requireAuth)
	{
		//Do not create a connection if requirements are not met
		if(!requireAuth && this.domain == null)
		{
			log("Failed to create request - domain must be specified");
			return null;
		}else if(requireAuth)
		if(!this.bAuthenticated || this.bAuthFailed || this.domain == null || passcode == null || userID == null)
		{
			log("Failed to create request - has the client been authenticated?");
			return null;
		}
		
		
		ConnectionRequest request = new ConnectionRequest();
		
		requests.put(request.index, request);
		
		request.setBaseURL(getBaseURL());		
		
		if(requireAuth)
		{
			request.addPost("auth", authToken);
			request.addPost("userID", userID);
			//request.addPost("nickname", nickname);
		}
		
		return request;
	}
	
	public ConnectionRequest createRequest()
	{
		return createRequest(true);
	}
	
	/**
	 * Updates all connections, and conducts automatic processing as needed.
	 * Must be called from an outside update loop.
	 */
	public void update()
	{
		
		int actives = 0;
		
		//Iterate through all requests
		Iterator<Map.Entry<Integer, ConnectionRequest>> iter = requests.entrySet().iterator();
		while(iter.hasNext())
		{
			Map.Entry<Integer, ConnectionRequest> pairs = iter.next();
			
			ConnectionRequest req = pairs.getValue();
			 
			if(req.isActive())
			{
				actives++;
			}
			
			//Check for result
			if(req.isReady())
			{

				if(req.index == this.regIndex)
				{
					log("Regsitration is ready.");
					log(req.getPath());
					String received = req.receivedData;
					
					//This token indicates succes
					if(received != null && received.length() > 15 && received.substring(0, 15).contains("userID"))
					{
						this.userID = received.substring(20-13, 20);
						
						log("userID is now "+userID+" passcode is "+passcode);
						this.bRegFailed = false;
						
					}
					else //Ruh roh
					{
						if(received != null && received.length() > 0)
						{
							log("Received "+ "["+ received.length() +"] "+ received);
							
							this.failMessage = "Registration Failed. Server response: " + 
							                   received.substring(0, Math.min(32, received.length()));
												
						}
						else
							this.failMessage = "Registration failed; no response from server.";
						
						log(failMessage);
						this.bRegFailed = true;
					}
					
					
					iter.remove(); //Clean up after ourselves					
				}
				
				//If this is an authentication response
				if(!bAuthenticated && req.index == this.authIndex)
				{
					log("Authentication is ready.");
					log(req.getPath());
					String received = req.receivedData;
					log("Received "+ "["+ received.length() +"] "+ received);					
					
					//Easy checks for failure
					if(received != null && received.length() > 10 && received.substring(0, 10).contains("auth"))
					{
						//TODO:
						//authToken = req.receivedData.substring(0, 32);
						//log("Auth token "+authToken+" received.");
						
						log(received);
						this.bAuthenticated = true;
						this.authToken = received.substring(5, 13+5);
						
						log("Auth token: "+authToken);
						
					}
					else
					{
						
						if(received != null && received.length() > 0)
						{
							log("Received "+ "["+ received.length() +"] "+ received);
							
							this.failMessage = "Login Failed. Server response: " + 
							                   received.substring(0, Math.min(32, received.length()));
												
						}
						else
							this.failMessage = "Login failed; no response from server.";
						
						log(failMessage);

						bAuthenticated = false;
						bAuthFailed = true; //Ruh roh
					}
					
					//Clean up
					iter.remove();
				}
			}
		}
		
		this.connectionDelta = actives - activeConnections;
		
		if(connectionDelta != 0)
			log("Delta: "+connectionDelta);
		
		this.activeConnections = actives;
		
	}
	
	/**
	 * Gets the total number of changed connections.
	 * @return Positive integer indicating new connections, negative indicating finished/dropped connections.
	 */
	public int getConnectionDelta()
	{
		return connectionDelta;
	}
	
	/**
	 * Checks if a given result is prepared.
	 * @param index Given request index.
	 * @return
	 */
	public boolean resultReady(int index)
	{
		if(!requests.containsKey(index))
			return false;
		
		return requests.get(index).bReady;
	}
	
	/**
	 * Gets a result from a particular request.
	 * @param index Request index in question.
	 * @param cull Remove result
	 * @return
	 */
	public String getResult(int index, boolean cull)
	{
		if(!requests.containsKey(index))
			return null;
		
		if(!resultReady(index))
			return null;
		
		ConnectionRequest req = requests.get(index);
		
		if(req.bIsBinary)
			return null;
		
		String result = req.receivedData;
		
		if(cull)
			requests.remove(index);
		
		return result;
	}
	
	/**
	 * Gets a binary result from a particular request.
	 * @param index
	 * @param cull
	 * @return
	 */
	public byte [] getBinaryResult(int index, boolean cull)
	{
		if(!requests.containsKey(index))
			return null;
		
		if(!resultReady(index))
			return null;
		
		ConnectionRequest req = requests.get(index);
		
		if(!req.bIsBinary)
			return null;
		
		if(cull)
			requests.remove(index);
		
		return req.receivedBinaryData;
	}
	
	////////////////////////
    /**
     * Container class for each HTTP request.  Uses an internal thread to manage connection requests.
     * @author Chris Livingston
     */
    public static class ConnectionRequest implements Runnable{
   	
     private Map<String, String> postData;
   	 private Map<String, String> getData;
   	 private boolean bReady;
   	 private boolean bActive;
   	 private boolean bStarted;
   	 private String baseURL;  // Domain 
   	 private String pathURL;  // Path and script file
   	 
   	 public String receivedData;
   	 
   	 public boolean bIsBinary;
   	 public byte [] receivedBinaryData; //Will be null if bIsBinary == true
   	 
   	 public int index;
   	 private static int current = 0;
   	 
   	 public boolean isActive()
   	 { return bActive; }

   	 public boolean isReady()
   	 { return bReady; }
   	 
   	 public void setBaseURL(String base)
   	 {
   		 if(!bActive)
   			 baseURL = base;
   	 }
   	 
   	 public void setPathURL(String path)
   	 {
   		 if(!bActive)
   			pathURL = path;
   	 }
   	 
   	 public String getPath()
   	 {
   		 return baseURL + pathURL;
   	 }
   	 
   	 /**
   	  * Adds a GET argument.
   	  * @param key
   	  * @param value
   	  */
   	 public void addGet(String key, String value)
   	 {
   		if(!bActive)
   			 getData.put(key, value);
   	 }
   	 
   	 /**
   	  * Adds a POST argument
   	  * @param key
   	  * @param value
   	  */
   	 public void addPost(String key, String value)
   	 {
   		if(!bActive)
   			postData.put(key, value);
   	 }
   	 
   	 public ConnectionRequest()
   	 {
   		 postData = new ConcurrentHashMap<String, String>();
   		 getData = new ConcurrentHashMap<String, String>();
   		 
   		 current++;
   		 index = current;
   	 }
   	 
   	 /**
   	  * Initiates HTTP request.
   	  * @return True on successful start, false on failure.
   	  */
   	 public synchronized boolean startRequest()
   	 {
   		 if(bActive || bReady)
   			 return false;
   		 
   		 if(baseURL == null)
   			 return false;
   		
   		 bStarted = true;
   		 
   		 Thread t = new Thread(this);
   		 t.setDaemon(true);
   		 t.start();
   		 
   		 return true;
   	 }
   	 
   	 
   	 

	@Override
	public synchronized void run() {
		if(!bStarted)	//startRequest must be called first!
			return;
		
		if(bActive || bReady) //Cannot run more than once
			return;
		
		bActive = true;
		String args = "";
		
		boolean firstArg = true;
		
		//Fill out the GET arguments
		for(String k : getData.keySet())
		{
			String v = getData.get(k);
			String a = k + "=" + v;
			
			if(firstArg)
			{
				args = args + "?" + a;
			}
			else
			{
				args = args + "&" + a;
			}
			
			firstArg = false;
		}
		
		if(!bIsBinary)
			receivedData = HTTPHandler.postURL(baseURL+pathURL+args, postData);
		else
			receivedBinaryData = HTTPHandler.binaryPostURL(baseURL+pathURL+args, postData);
		
		bActive = false;
		bReady = true;
		bStarted = false;
		
	}
    };////////// End of class
	
	
	/**
	 * Returns authentication status.
	 * @return True if the client has an auth token, false if not.
	 */
	public boolean bIsAuthenticated()
	{
		return bAuthenticated;
	}
	
	
	/**
	 * 
	 * @param userID
	 * @param passcode
	 */
	public void setCredentials(String userID, String passcode)
	{
		this.userID = userID;
		this.passcode = passcode;
	}
	

	
	/**
	 * @return The base URL
	 */
	public String getBaseURL()
	{
		String prefix = "http://";
		if(bUseSSL)
			prefix = "https://";
		
		String port = "";
		if(portNumber != 80)
			port = ":" + portNumber;
		
		return prefix+domain+port+"/";
	}
	

	@Override
	public void run() {
		// TODO Auto-generated method stub
		
	}

}
