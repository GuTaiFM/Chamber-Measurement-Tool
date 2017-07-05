#pragma once

/*  This is the declaration fo the basic rotator object from which more specialised versions of the rotator will be derived.
	There is no RotatorObj.cpp file since the function declerations are simple and only act as placeholders for more complex functionality which will be used to override some of these basis functions
*/
class RotatorObj {
protected:
	unsigned char m_speed; // Variable which holds the speed at which the rotator rotates. This value is typically an integer from 0 to 255 and will typically be initialised to a value of 1
	unsigned char m_accel; // Variable which holds the acceleration of the rotator. This value is an integer with a value from 0 to 255. Typically the value will be set to 255 for maximum acceleration
	
	float m_stepAngle; // Variable which holds the value which tells the rotator how much it needs to rotate with each step. This value is values between 0 and 360 degrees
	float m_currentPosition; // This variable holds the value which tracks the current position of the rotator.

public:
	/*
		Constructor object for the Rotator
		The inputs for the constructor are as follows:
		- speed: The speed with which the rotator must rotate
		- accel: The acceleration for rotator
		- stepAngle: The angle increment for each step of measurement
	*/
	RotatorObj(unsigned char speed, unsigned char accel, float stepAngle) {
		setSpeed(speed);
		setAccel(accel);

		m_currentPosition = 0.0;
		m_stepAngle = stepAngle;
	}

	/*
		Setter Function
		The setSpeed function is used to be able to modify the value
		of the m_speed variable inside the class from outside the class
		There are simple checks to check whether the values received are sane.
	*/
	virtual void setSpeed(unsigned char speed) {
		if (speed > 255) {
			m_speed = 255;
		}
		else if (speed < 0) {
			m_speed = 0;
		}else{
			m_speed = speed;
		}
	}

	/*
		Getter Function
		Returns the value of the variable m_speed, since the value cannot be accessed
		directly from outside the class by anything that is not a RotatorObj object
	*/
	char getSpeed() {
		return m_speed;
	}

	/*
		Setter Function
		The setAccel function is used to be able to modify the value
		of the m_accel variable inside the class from outside the class
		There are simple checks to check whether the values received are sane.
	*/
	virtual void setAccel(unsigned char accel) {
		if (accel < 1) {
			m_accel = 1;
		}
		else {
			m_accel = accel;
		}
	}

	/*
		Getter Function
		Returns the value of the variable m_accel, since the value cannot be accessed
		directly from outside the class by anythign that is not a RotatorObj object
	*/
	char getAccel() {
		return m_accel;
	}

	/*
		Setter function 
		Used for setting the value of the variable m_stepAngle
	*/
	virtual void setStepAngle(float stepAngle) {
		m_stepAngle = stepAngle;
	}

	/*
		Getter Function
		Returns the value of m_stepAngle
	*/
	float getStepAngle() {
		return m_stepAngle;
	}

	/*
		Setter Function
		Set the value of m_currentPosition from outside the class
	*/
	void setCurrentPosition(float currentPosition) {
		m_currentPosition = currentPosition;
	}

	/*
		Getter Function
		Returns the value of m_currentPosition
	*/
	float getCurrentPosition() {
		return m_currentPosition;
	}
};