package com.clivingston.strat150;

import java.util.Set;

/**
 * 
 * @author chris
 *
 */
public class Trait {

	final public static int TYPE_IGNORE = 0;
	final public static int TYPE_OVERRIDE_MIN = 1;
	final public static int TYPE_OVERRIDE_MAX = 2;
	final public static int TYPE_ADD = 3;
	final public static int TYPE_MULTIPLY = 4;
	final public static int TYPE_AVERAGE = 5;
	
	
	public double value;
	public String label;
	
	public int relation;
	
	
	private double op(int type, double x)
	{
		switch(type)
		{
		case TYPE_IGNORE:
			return value;
			
		case TYPE_OVERRIDE_MIN:
			return Math.min(x, value);
			
		case TYPE_OVERRIDE_MAX:
			return Math.max(x, value);
			
		case TYPE_ADD:
			return x + value;
			
		case TYPE_MULTIPLY:
			return x + value;
			
		case TYPE_AVERAGE:
			return (x + value)/2;
					
		}
		
		return value;
	}
	
	private double op(int type, double [] x)
	{
		double t;
		
		switch(type)
		{
		case TYPE_IGNORE:
			return value;

		case TYPE_OVERRIDE_MIN:
			t = value;
			for(double d : x)
				t = Math.min(t, d);
			return t;

		case TYPE_OVERRIDE_MAX:
			t = value;
			for(double d : x)
				t = Math.max(t, d);
			return t;

		case TYPE_ADD:
			t = value;
			for(double d : x)
				t += d;			
			return t;
			
		case TYPE_MULTIPLY:
			t = value;
			for(double d : x)
				t *= d;			
			return t;
			
		case TYPE_AVERAGE:
			t = value;
			for(double d : x)
				t += d;
			
			return t/(x.length + 1);
		
		}
		
		return value;
		
	}
	
	public double calculate(Trait child)
	{
		return op(relation, child.value);
	}
	
	public double calculate(Set<Trait> children)
	{
		if(children.size() == 0)
			return value;
		
		if(children.size() == 1) //Single case
			return op(relation, children.iterator().next().value);
		
		//Allocate and fill in a temp array
		double [] a = new double[children.size()];
		int i = 0;
		for(Trait child : children)
			a[i++] = child.value;
		
		return op(relation, a);
	}
	
}
