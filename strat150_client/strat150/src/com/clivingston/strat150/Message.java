package com.clivingston.strat150;

/**
 * Simple message class for in-game communication.
 * @author Chris Livingston
 *
 */
public class Message {

	Player sender;
	Player recipient;
	long timeStamp;
	long tickSent;
	
	String body;
	
	public String genSendString()
	{
		if(body == null || sender == null || recipient == null)
			return null;
		
		StringBuffer r = new StringBuffer();
		
		r.append("\"");
		r.append(recipient.userID);
		r.append("\" : \"");
		
		
		//Escape slashes and quotes
		r.append(body.replace("\\","\\\\").replace("\"", "\\\""));
		r.append("\"");
		
		return r.toString();
	}
	
	
}
