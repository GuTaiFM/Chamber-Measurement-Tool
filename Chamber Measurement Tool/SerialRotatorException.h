#pragma once

/*
	this is a custom exception class which is used to return customised error messages
	This will be expanded at a later stage to allow for more specific error messages to
	make diagnosis easier
*/
class SerialRotatorException {
public:
	SerialRotatorException(const char *pStr = "There was a problem with the rotator") : pMessage(pStr) {}
	const char *what() const { return pMessage; };

private:
	const char *pMessage;
};