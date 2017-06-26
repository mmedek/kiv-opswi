#include "Line.h"

Line::Line(double x1, double x2, double y1, double y2) {
	this->x1 = x1;
	this->x2 = x2;
	this->y1 = y1;
	this->y2 = y2;
}

double Line::getX1() {
	return this->x1;
}

double Line::getX2() {
	return this->x2;
}

double Line::getY1() {
	return this->y1;
}
double Line::getY2() {
	return this->y2;
}