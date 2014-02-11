package com.clivingston.strat150;

import java.util.HashSet;
import java.util.Set;


//Simple event container class
public class StratEvent {
	
	int tickNum;
	String alertString;
	Set<StratActor> destroyed;
	Set<StratActor> damaged;
	Set<StratActor> affected;
	
	int x, y;
	
	String instigator;
	
	public StratEvent()
	{
		destroyed = new HashSet<StratActor>();
		damaged = new HashSet<StratActor>();
		affected = new HashSet<StratActor>();
	}
	
	public boolean hasDestroyed()
	{
		return destroyed.size() > 0;
	}
	
	public boolean hasDamaged()
	{
		return damaged.size() > 0;
	}
	
	public boolean hasAffected()
	{
		return affected.size() > 0;
	}

}
