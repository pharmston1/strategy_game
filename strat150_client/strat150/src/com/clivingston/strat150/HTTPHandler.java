package com.clivingston.strat150;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLEncoder;
import java.util.Map;

import com.badlogic.gdx.Gdx;


/**
 * This container class handles all HTTP requests.
 * 
 * @author Chris Livingston
 * 
 */
public class HTTPHandler {
	
	protected static void log(String str)
	{Gdx.app.log("HTTP", str);}
	
	
    /**
	 * Sends an HTTP post to the specified URL - receives text data.
	 * Will not work if called from a non-daemon thread!
	 *
	 * @param  urlstr		URL for GET request
	 * @param  postdata		Map of POST arguments and their values.
	 * @return Data received from POST request.
	 */	
    public static String postURL(String urlstr, Map<String, String> postdata)
    {
    	
    	
    	String online = "";
    	
    	//Fail if we're not in a daemon thread
    	if(!Thread.currentThread().isDaemon())
    	{
    		log("Failed to post URL, not in daemon thread.");
    		return "Error";
    	}
    	
    	try {
    	    // Send data
    		URLConnection conn = httpGetConnection(urlstr, postdata);
    		InputStream received = conn.getInputStream();

    	    // Get the response
    	    BufferedReader rd = new BufferedReader(new InputStreamReader(received));
    	    String str;
    	    while ((str = rd.readLine()) != null) {
    	    	online += str + "\n";
    	    }
    	    
    	    //wr.close();
    	    rd.close();
    	} catch (Exception e) {
    		e.printStackTrace();
    	}
    	
    	return online;
    }
    
    /**
	 * Sends an HTTP post to the specified URL - receives binary data.
	 * Will not work if called from a non-daemon thread!
	 * @param  urlstr		URL for GET request
	 * @param  postdata		Map of arguments and their values.
     * @return Byte array of received data, null on failure.
     */
    public static byte[] binaryPostURL(String urlstr, Map<String, String> postdata)
    {
    	
    	
    	byte [] online = null;
    	
    	//Fail if we're not in a daemon thread
    	if(!Thread.currentThread().isDaemon())
    	{
    		log("Failed to post URL, not in daemon thread.");
    		return null;
    	}
    	
    	try {
    	    // Send data
    		URLConnection conn = httpGetConnection(urlstr, postdata);
    	    InputStream raw = conn.getInputStream();
    	    InputStream in = new BufferedInputStream(raw);

    	    //Allocate and receive binary data
    	    online = new byte[conn.getContentLength()];
    	    
    	    int bytesRead = 0;
    	    int offset = 0;
    	    while (offset < conn.getContentLength()) {
    	      bytesRead = in.read(online, offset, online.length - offset);
    	      if (bytesRead == -1)
    	        break;
    	      offset += bytesRead;
    	    }
    	    in.close();
    	    
    	    if (offset != conn.getContentLength()) 
    	    {
    	        throw new IOException("Only read " + offset + " bytes; Expected " + conn.getContentLength() + " bytes");
    	     }
    	    
    	} catch (Exception e) {
    		e.printStackTrace();
    	}
    	
    	return online;
    }

	/**
	 * Internal function to retrieve a raw InputStream from a URL.
	 * @param urlstr
	 * @param postdata
	 * @return
	 * @throws MalformedURLException
	 * @throws IOException
	 * @throws UnsupportedEncodingException
	 */
	private static URLConnection httpGetConnection(String urlstr,
			Map<String, String> postdata) throws MalformedURLException,
			IOException, UnsupportedEncodingException {
		URL url = new URL(urlstr);
		URLConnection conn = url.openConnection();
		conn.setDoOutput(true);
		OutputStreamWriter wr = new OutputStreamWriter(conn.getOutputStream());
		
		// Construct and write data to POST buffer
		//String data = URLEncoder.encode("data", "UTF-8") + "=" + URLEncoder.encode(postdata, "UTF-8");
		boolean first = true;
		if(postdata != null)
		    for(String k : postdata.keySet())
		    {
		    	String data = postdata.get(k);
		    	
		    	if(first)
		    	{
		    		wr.write(URLEncoder.encode(k, "UTF-8") + "=" + URLEncoder.encode(data, "UTF-8"));
		    		first = false;
		    	}
		    	else
		    	{
		    		wr.write("&" + URLEncoder.encode(k, "UTF-8") + "=" + URLEncoder.encode(data, "UTF-8"));
		    	}
		    	
		    	//log(k+" = ", data);
		    }
		
		wr.flush();
		wr.close();
		
		return conn;
	}
}
