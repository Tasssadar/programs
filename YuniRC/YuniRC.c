#include "yunimin3.h"

#define TURN_VALUE 40

enum moveFlags
{
    MOVE_NONE     = 0x00,
	MOVE_FORWARD  = 0x01,
	MOVE_BACKWARD = 0x02,
	MOVE_LEFT     = 0x04,
	MOVE_RIGHT    = 0x08,
};
uint8_t moveflags;
uint8_t speed;

void SetMovement(char key[])
{
    // Set Movement Flags
	if(key[1] == ' ')
	{
	    if(key[0] == 'W' && !(moveflags & MOVE_BACKWARD))
		{
			if(key[3] == 'd' && !(moveflags & MOVE_FORWARD))
				moveflags |= MOVE_FORWARD;
			else
	    		moveflags &= ~(MOVE_FORWARD);	
		}
		else if(key[0] == 'S' && !(moveflags & MOVE_FORWARD))
		{
			if(key[3] == 'd' && !(moveflags & MOVE_BACKWARD))
				moveflags |= MOVE_BACKWARD;
			else
				moveflags &= ~(MOVE_BACKWARD);	
		}
		else if(key[0] == 'A' && !(moveflags & MOVE_RIGHT))
		{
			if(key[3] == 'd' && !(moveflags & MOVE_LEFT))
				moveflags |= MOVE_LEFT;
			else
				moveflags &= ~(MOVE_LEFT);
		} 
		else if(key[0] == 'D' && !(moveflags & MOVE_LEFT))
		{
			if(key[3] == 'd' && !(moveflags & MOVE_RIGHT))
				moveflags |= MOVE_RIGHT;
			else
				moveflags &= ~(MOVE_RIGHT);
		}
		else if(key[0] == '1')
		    speed = 50;
	    else if(key[0] == '2')
			speed = 100;
	    else if(key[0] == '3')
		    speed = 127;
	}
    else if(key[0] == 'S' && key[4] == 'e' && key[7] == 'd') // Space
	{
		rs232.send("\r\nSensors: \r\n LeftTop: ");
		rs232.sendNumber(getSensorValue(0));
		rs232.send("\r\n LeftMiddle: ");
		rs232.sendNumber(getSensorValue(4));
		rs232.send("\r\n RightMiddle: ");
		rs232.sendNumber(getSensorValue(5));
		rs232.send("\r\n RightTop: ");
		rs232.sendNumber(getSensorValue(1));
		rs232.wait();
		rs232.send("\r\n LeftBottom: ");
		rs232.sendNumber(getSensorValue(2));
		rs232.send("\r\n RightBottom: ");
		rs232.sendNumber(getSensorValue(3));
		rs232.send("\r\n");
	}


	//Set motors
	if(moveflags & MOVE_FORWARD)
	{
		if(moveflags & MOVE_LEFT)
		{
			setLeftMotor(speed-TURN_VALUE);
			setRightMotor(speed);
		}
		else if(moveflags & MOVE_RIGHT)
		{
			setLeftMotor(speed);
			setRightMotor(speed-TURN_VALUE);	
		}
		else
		{
			setLeftMotor(speed);
			setRightMotor(speed);
		}
	}
	else if(moveflags & MOVE_BACKWARD)
	{
		if(moveflags & MOVE_LEFT)
		{
			setLeftMotor(-(speed-TURN_VALUE));
			setRightMotor(-speed);
		}
		else if(moveflags & MOVE_RIGHT)
		{
			setLeftMotor(-speed);
			setRightMotor(-(speed-TURN_VALUE));	
		}
		else
		{
			setLeftMotor(-speed);
			setRightMotor(-speed);
		}
	}
	else if(moveflags & MOVE_LEFT)
	{
		setLeftMotor(-speed);
		setRightMotor(speed);
	}
	else if(moveflags & MOVE_RIGHT)
	{
		setLeftMotor(speed);
		setRightMotor(-speed);
	}
	else
	{
		setLeftMotor(0);
		setRightMotor(0);
	}
}

void run()
{
	char key[10];
	uint8_t key_itr = 0;
	while(key_itr < 10)
		key[++key_itr] = '0';
	key_itr = 0;
	moveflags = 0;
	speed = 100;
    while(true)
	{
	    char ch;
		if(!rs232.peek(ch))
		    continue;

	    key[key_itr] = ch;
		++key_itr;
		if((ch == 'd'  || ch == 'u') && key_itr >= 4)
		{
			while(key_itr < 10)
			    key[++key_itr] = '0';
			key_itr = 0;
		    //rs232.send(key);
		    //rs232.send("\r\n");
			SetMovement(key);
		}
	}
}




