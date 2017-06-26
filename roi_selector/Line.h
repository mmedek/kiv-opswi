#pragma once


class Line {
	private:
		double x1;
		double x2;
		double y1;
		double y2;
	public:
		Line(double x1, double x2, double y1, double y2);
		double getX1();
		double getX2();
		double getY1();
		double getY2();
};